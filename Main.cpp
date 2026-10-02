#define _PHYSX_DEBUG
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <Overlay/Overlay.h>
#include <Common/Data.h>
#include <Utils/Utils.h>
#include <Hack/Hack.h>
#include <Common/Offset.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <thread>
#include <vector>
#include <DMALibrary/Memory/Memory.h>
#include <cstdint>
#include <cmath>
#include <Utils/w3c/w3c.h>
#include <Utils/MachineCodeGenerator.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "wininet.lib")

FGameData GameData;
using namespace std;

namespace
{
	constexpr const wchar_t* kVersionFileName = L"version.txt";
	constexpr const wchar_t* kTempUpdateFileName = L"update_package.bin";
	constexpr const wchar_t* kUpdateBatchFileName = L"update_program.bat";
	constexpr const wchar_t* kUpdateExtractDirectoryName = L"update_extract";

	enum class UpdatePayloadType
	{
		Unknown,
		PortableExecutable,
		ZipArchive
	};

	std::string Utf8FromWide(const wchar_t* text)
	{
		if (text == nullptr) {
			return {};
		}

		const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1) {
			return {};
		}

		std::string utf8(static_cast<size_t>(size - 1), '\0');
		WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8.data(), size - 1, nullptr, nullptr);
		return utf8;
	}

	std::string Utf8FromWide(const std::wstring& text)
	{
		return Utf8FromWide(text.c_str());
	}

	std::wstring GetExecutablePath()
	{
		wchar_t currentPath[MAX_PATH] = { 0 };
		GetModuleFileNameW(nullptr, currentPath, MAX_PATH);
		return currentPath;
	}

	std::wstring GetExecutableDirectory()
	{
		const std::wstring exePath = GetExecutablePath();
		const size_t pos = exePath.find_last_of(L"\\/");
		return pos == std::wstring::npos ? L"." : exePath.substr(0, pos);
	}

	std::wstring CombinePath(const std::wstring& directory, const wchar_t* fileName)
	{
		if (directory.empty()) {
			return fileName;
		}

		if (directory.back() == '\\' || directory.back() == '/') {
			return directory + fileName;
		}

		return directory + L"\\" + fileName;
	}

	std::wstring GetFileNameFromPath(const std::wstring& path)
	{
		const size_t pos = path.find_last_of(L"\\/");
		return pos == std::wstring::npos ? path : path.substr(pos + 1);
	}

	const std::wstring& GetVersionFilePath()
	{
		static const std::wstring path = CombinePath(GetExecutableDirectory(), kVersionFileName);
		return path;
	}

	const std::wstring& GetTempUpdateFilePath()
	{
		static const std::wstring path = CombinePath(GetExecutableDirectory(), kTempUpdateFileName);
		return path;
	}

	const std::wstring& GetUpdateBatchFilePath()
	{
		static const std::wstring path = CombinePath(GetExecutableDirectory(), kUpdateBatchFileName);
		return path;
	}

	const std::wstring& GetUpdateExtractDirectoryPath()
	{
		static const std::wstring path = CombinePath(GetExecutableDirectory(), kUpdateExtractDirectoryName);
		return path;
	}

	std::string ReadLocalVersion()
	{
		std::string localVersion = "1.0";
		std::ifstream versionFile{ std::filesystem::path(GetVersionFilePath()) };
		if (versionFile.is_open()) {
			std::getline(versionFile, localVersion);
			std::cout << Utf8FromWide(L"\u8BFB\u53D6\u5230\u672C\u5730\u7248\u672C\u53F7: ") << localVersion << std::endl;
		}
		else {
			std::cout << Utf8FromWide(L"\u672A\u627E\u5230 version.txt\uFF0C\u4F7F\u7528\u9ED8\u8BA4\u7248\u672C\u53F7: ") << localVersion << std::endl;
		}

		return localVersion;
	}

	bool SaveVersionToFile(const std::string& version)
	{
		std::ofstream versionFile{ std::filesystem::path(GetVersionFilePath()), std::ios::trunc };
		if (!versionFile.is_open()) {
			return false;
		}

		versionFile << version;
		return true;
	}

	bool DeleteFileIfExists(const std::wstring& filePath)
	{
		if (GetFileAttributesW(filePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
			return true;
		}

		return ::DeleteFileW(filePath.c_str()) != 0;
	}

	UpdatePayloadType DetectUpdatePayloadType(const std::wstring& filePath, WORD* machineType = nullptr)
	{
		std::ifstream input{ std::filesystem::path(filePath), std::ios::binary };
		if (!input.is_open()) {
			return UpdatePayloadType::Unknown;
		}

		unsigned char header[4] = { 0 };
		input.read(reinterpret_cast<char*>(header), sizeof(header));
		if (input.gcount() >= 4 && header[0] == 'P' && header[1] == 'K') {
			return UpdatePayloadType::ZipArchive;
		}

		input.clear();
		input.seekg(0, std::ios::beg);

		IMAGE_DOS_HEADER dosHeader = {};
		input.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
		if (!input.good() || dosHeader.e_magic != IMAGE_DOS_SIGNATURE || dosHeader.e_lfanew <= 0) {
			return UpdatePayloadType::Unknown;
		}

		input.seekg(dosHeader.e_lfanew, std::ios::beg);
		DWORD peSignature = 0;
		input.read(reinterpret_cast<char*>(&peSignature), sizeof(peSignature));
		if (!input.good() || peSignature != IMAGE_NT_SIGNATURE) {
			return UpdatePayloadType::Unknown;
		}

		IMAGE_FILE_HEADER fileHeader = {};
		input.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
		if (!input.good()) {
			return UpdatePayloadType::Unknown;
		}

		if (machineType != nullptr) {
			*machineType = fileHeader.Machine;
		}

		return UpdatePayloadType::PortableExecutable;
	}

	bool ValidateDownloadedPayload(const std::wstring& payloadPath, UpdatePayloadType& payloadType)
	{
		WORD payloadMachine = 0;
		payloadType = DetectUpdatePayloadType(payloadPath, &payloadMachine);

		if (payloadType == UpdatePayloadType::PortableExecutable) {
			WORD currentMachine = 0;
			if (DetectUpdatePayloadType(GetExecutablePath(), &currentMachine) == UpdatePayloadType::PortableExecutable &&
				currentMachine != 0 && payloadMachine != 0 && currentMachine != payloadMachine) {
				std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u7684\u66F4\u65B0\u6587\u4EF6\u67B6\u6784\u4E0E\u5F53\u524D\u7A0B\u5E8F\u4E0D\u4E00\u81F4\uFF0C\u5DF2\u963B\u6B62\u66FF\u6362\u3002") << std::endl;
				return false;
			}

			std::cout << Utf8FromWide(L"\u68C0\u6D4B\u5230 EXE \u66F4\u65B0\u5305\u3002") << std::endl;
			return true;
		}

		if (payloadType == UpdatePayloadType::ZipArchive) {
			std::cout << Utf8FromWide(L"\u68C0\u6D4B\u5230 ZIP \u66F4\u65B0\u5305\uFF0C\u5C06\u66F4\u65B0\u6574\u4E2A\u53D1\u5E03\u76EE\u5F55\u3002") << std::endl;
			return true;
		}

		std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u7684\u66F4\u65B0\u6587\u4EF6\u65E2\u4E0D\u662F\u53EF\u6267\u884C EXE\uFF0C\u4E5F\u4E0D\u662F ZIP \u66F4\u65B0\u5305\u3002") << std::endl;
		return false;
	}

	bool DownloadFileWithProgress(const std::string& url, const std::wstring& outputFile)
	{
		DeleteFileIfExists(outputFile);

		HINTERNET session = InternetOpenA("dada-Updater/1.0", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
		if (!session) {
			std::cout << Utf8FromWide(L"\u6253\u5F00\u7F51\u7EDC\u4F1A\u8BDD\u5931\u8D25\uFF0C\u9519\u8BEF\u4EE3\u7801: ") << GetLastError() << std::endl;
			return false;
		}

		HINTERNET request = InternetOpenUrlA(session, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
		if (!request) {
			std::cout << Utf8FromWide(L"\u6253\u5F00\u66F4\u65B0\u5730\u5740\u5931\u8D25\uFF0C\u9519\u8BEF\u4EE3\u7801: ") << GetLastError() << std::endl;
			InternetCloseHandle(session);
			return false;
		}

		DWORD fileSize = 0;
		DWORD querySize = sizeof(fileSize);
		const bool hasFileSize = HttpQueryInfoA(request, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &fileSize, &querySize, nullptr) != FALSE;
		const double totalMB = fileSize / (1024.0 * 1024.0);

		std::ofstream output{ std::filesystem::path(outputFile), std::ios::binary | std::ios::trunc };
		if (!output.is_open()) {
			std::cout << Utf8FromWide(L"\u521B\u5EFA\u66F4\u65B0\u4E34\u65F6\u6587\u4EF6\u5931\u8D25: ") << Utf8FromWide(outputFile) << std::endl;
			InternetCloseHandle(request);
			InternetCloseHandle(session);
			return false;
		}

		std::vector<char> buffer(8192);
		DWORD bytesRead = 0;
		DWORD downloadedBytes = 0;
		DWORD lastTick = GetTickCount();
		DWORD lastBytes = 0;
		BOOL readResult = TRUE;

		std::cout << Utf8FromWide(L"\u6B63\u5728\u4E0B\u8F7D\u66F4\u65B0...") << std::endl;
		if (hasFileSize) {
			std::cout << Utf8FromWide(L"\u6587\u4EF6\u5927\u5C0F: ") << std::fixed << std::setprecision(2) << totalMB << " MB" << std::endl;
		}

		while ((readResult = InternetReadFile(request, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead)) && bytesRead > 0) {
			output.write(buffer.data(), bytesRead);
			if (!output.good()) {
				std::cout << Utf8FromWide(L"\u5199\u5165\u66F4\u65B0\u6587\u4EF6\u5931\u8D25\u3002") << std::endl;
				output.close();
				InternetCloseHandle(request);
				InternetCloseHandle(session);
				return false;
			}

			downloadedBytes += bytesRead;
			const DWORD currentTick = GetTickCount();
			const DWORD elapsed = currentTick - lastTick;

			if (elapsed >= 500 || (hasFileSize && downloadedBytes == fileSize)) {
				const double downloadedMB = downloadedBytes / (1024.0 * 1024.0);
				const double speedKB = elapsed > 0 ? ((downloadedBytes - lastBytes) * 1000.0 / elapsed) / 1024.0 : 0.0;

				std::cout << "\r";
				if (hasFileSize && fileSize > 0) {
					const double progress = static_cast<double>(downloadedBytes) * 100.0 / fileSize;
					std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u8FDB\u5EA6: ") << std::fixed << std::setprecision(1) << progress << "% ("
						<< std::setprecision(2) << downloadedMB << " MB / " << totalMB << " MB)";
				}
				else {
					std::cout << Utf8FromWide(L"\u5DF2\u4E0B\u8F7D: ") << std::fixed << std::setprecision(2) << downloadedMB << " MB";
				}

				if (speedKB > 1024.0) {
					std::cout << Utf8FromWide(L"  \u901F\u5EA6: ") << std::fixed << std::setprecision(2) << (speedKB / 1024.0) << " MB/s";
				}
				else if (speedKB > 0.0) {
					std::cout << Utf8FromWide(L"  \u901F\u5EA6: ") << std::fixed << std::setprecision(2) << speedKB << " KB/s";
				}

				std::cout << std::flush;
				lastTick = currentTick;
				lastBytes = downloadedBytes;
			}
		}

		output.close();
		InternetCloseHandle(request);
		InternetCloseHandle(session);
		std::cout << std::endl;

		if (!readResult) {
			std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u66F4\u65B0\u6587\u4EF6\u5931\u8D25\uFF0C\u9519\u8BEF\u4EE3\u7801: ") << GetLastError() << std::endl;
			return false;
		}

		std::ifstream checkFile{ std::filesystem::path(outputFile), std::ios::binary | std::ios::ate };
		if (!checkFile.is_open()) {
			std::cout << Utf8FromWide(L"\u65E0\u6CD5\u6821\u9A8C\u66F4\u65B0\u6587\u4EF6\u3002") << std::endl;
			return false;
		}

		const std::streamsize downloadedSize = checkFile.tellg();
		if (downloadedSize <= 0) {
			std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u7684\u66F4\u65B0\u6587\u4EF6\u4E3A\u7A7A\u3002") << std::endl;
			return false;
		}

		std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u5B8C\u6210\uFF0C\u6587\u4EF6\u5927\u5C0F: ") << std::fixed << std::setprecision(2)
			<< (downloadedSize / (1024.0 * 1024.0)) << " MB" << std::endl;
		return true;
	}

	bool ReplaceAndLaunchNewVersion(const std::wstring& payloadPath, UpdatePayloadType payloadType, const std::string& newVersion)
	{
		const std::wstring exeName = GetFileNameFromPath(GetExecutablePath());
		const std::wstring payloadFileName = GetFileNameFromPath(payloadPath);

		std::ofstream batchFile{ std::filesystem::path(GetUpdateBatchFilePath()), std::ios::trunc };
		if (!batchFile.is_open()) {
			std::cout << Utf8FromWide(L"\u521B\u5EFA\u66F4\u65B0\u811A\u672C\u5931\u8D25\u3002") << std::endl;
			return false;
		}

		batchFile << "@echo off" << std::endl;
		batchFile << "setlocal EnableExtensions" << std::endl;
		batchFile << "chcp 65001 > NUL" << std::endl;
		batchFile << "set \"APPDIR=%~dp0\"" << std::endl;
		batchFile << "if \"%APPDIR:~-1%\"==\"\\\" set \"APPDIR=%APPDIR:~0,-1%\"" << std::endl;
		batchFile << "set \"CURRENT_EXE=%APPDIR%\\" << Utf8FromWide(exeName) << "\"" << std::endl;
		batchFile << "set \"PAYLOAD=%APPDIR%\\" << Utf8FromWide(payloadFileName) << "\"" << std::endl;
		batchFile << "set \"STAGE=%APPDIR%\\" << Utf8FromWide(kUpdateExtractDirectoryName) << "\"" << std::endl;
		batchFile << "set \"NEW_VERSION=" << newVersion << "\"" << std::endl;
		batchFile << "echo Running updater, please wait..." << std::endl;
		batchFile << "timeout /t 5 /nobreak > NUL" << std::endl;

		if (payloadType == UpdatePayloadType::PortableExecutable) {
			batchFile << "set COUNTER=0" << std::endl;
			batchFile << ":retry_exe" << std::endl;
			batchFile << "set /a COUNTER+=1" << std::endl;
			batchFile << "if %COUNTER% GTR 15 goto failed" << std::endl;
			batchFile << "del \"%CURRENT_EXE%\" > NUL 2>&1" << std::endl;
			batchFile << "move /Y \"%PAYLOAD%\" \"%CURRENT_EXE%\" > NUL 2>&1" << std::endl;
			batchFile << "if not exist \"%CURRENT_EXE%\" (" << std::endl;
			batchFile << "  timeout /t 1 /nobreak > NUL" << std::endl;
			batchFile << "  goto retry_exe" << std::endl;
			batchFile << ")" << std::endl;
		}
		else {
			batchFile << "if exist \"%STAGE%\" rmdir /s /q \"%STAGE%\"" << std::endl;
			batchFile << "powershell -NoProfile -ExecutionPolicy Bypass -Command \"$payload = [Environment]::GetEnvironmentVariable('PAYLOAD'); $stage = [Environment]::GetEnvironmentVariable('STAGE'); $app = [Environment]::GetEnvironmentVariable('APPDIR'); Expand-Archive -LiteralPath $payload -DestinationPath $stage -Force; $items = Get-ChildItem -LiteralPath $stage -Force; if ($items.Count -eq 1 -and $items[0].PSIsContainer) { $source = $items[0].FullName } else { $source = $stage }; $targetExe = Join-Path $source '" << Utf8FromWide(exeName) << "'; if (-not (Test-Path -LiteralPath $targetExe)) { exit 20 }; Get-ChildItem -LiteralPath $source -Force | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $app -Recurse -Force }\"" << std::endl;
			batchFile << "if errorlevel 1 goto failed" << std::endl;
		}

		batchFile << "> \"%APPDIR%\\version.txt\" echo %NEW_VERSION%" << std::endl;
		batchFile << "start \"\" \"%CURRENT_EXE%\"" << std::endl;
		batchFile << "goto cleanup" << std::endl;
		batchFile << ":failed" << std::endl;
		batchFile << "echo Automatic update failed, starting current version..." << std::endl;
		batchFile << "if exist \"%CURRENT_EXE%\" start \"\" \"%CURRENT_EXE%\"" << std::endl;
		batchFile << ":cleanup" << std::endl;
		batchFile << "powershell -NoProfile -ExecutionPolicy Bypass -Command \"$payload = [Environment]::GetEnvironmentVariable('PAYLOAD'); $stage = [Environment]::GetEnvironmentVariable('STAGE'); if (Test-Path -LiteralPath $payload) { Remove-Item -LiteralPath $payload -Force -ErrorAction SilentlyContinue }; if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue }\" > NUL 2>&1" << std::endl;
		batchFile << "del \"%~f0\"" << std::endl;
		batchFile.close();

		SHELLEXECUTEINFOW executeInfo = { sizeof(executeInfo) };
		executeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		executeInfo.lpVerb = L"open";
		executeInfo.lpFile = GetUpdateBatchFilePath().c_str();
		executeInfo.nShow = SW_SHOW;

		if (!ShellExecuteExW(&executeInfo)) {
			std::cout << Utf8FromWide(L"\u542F\u52A8\u66F4\u65B0\u811A\u672C\u5931\u8D25\uFF0C\u9519\u8BEF\u4EE3\u7801: ") << GetLastError() << std::endl;
			return false;
		}

		if (executeInfo.hProcess) {
			CloseHandle(executeInfo.hProcess);
		}

		std::cout << Utf8FromWide(L"\u66F4\u65B0\u811A\u672C\u5DF2\u542F\u52A8\uFF0C\u5F53\u524D\u7A0B\u5E8F\u5373\u5C06\u9000\u51FA\u3002") << std::endl;
		return true;
	}

	bool HandleUpdateCheck()
	{
		const std::string localVersion = ReadLocalVersion();
		std::cout << Utf8FromWide(L"\u672C\u5730\u7248\u672C: ") << localVersion << std::endl;
		std::cout << Utf8FromWide(L"\u670D\u52A1\u5668\u7248\u672C: ") << GameData.Version << std::endl;

		if (localVersion != GameData.Version || GameData.pbForceUpdating) {
			std::cout << Utf8FromWide(L"\u68C0\u6D4B\u5230\u9700\u8981\u66F4\u65B0\uFF0C\u5F00\u59CB\u6267\u884C\u66F4\u65B0\u6821\u9A8C...") << std::endl;

			if (GameData.Url.empty()) {
				if (GameData.pbForceUpdating) {
					std::cout << Utf8FromWide(L"\u5F53\u524D\u4E3A\u5F3A\u5236\u66F4\u65B0\uFF0C\u4F46\u672A\u83B7\u53D6\u5230\u4E0B\u8F7D\u5730\u5740\uFF0C\u7A0B\u5E8F\u5C06\u9000\u51FA\u3002") << std::endl;
					Sleep(2000);
					return false;
				}

				std::cout << Utf8FromWide(L"\u672A\u83B7\u53D6\u5230\u66F4\u65B0\u5730\u5740\uFF0C\u7EE7\u7EED\u4F7F\u7528\u5F53\u524D\u7248\u672C\u3002") << std::endl;
				return true;
			}

			if (DownloadFileWithProgress(GameData.Url, GetTempUpdateFilePath())) {
				UpdatePayloadType payloadType = UpdatePayloadType::Unknown;
				if (!ValidateDownloadedPayload(GetTempUpdateFilePath(), payloadType)) {
					DeleteFileIfExists(GetTempUpdateFilePath());
					if (GameData.pbForceUpdating) {
						std::cout << Utf8FromWide(L"\u5F53\u524D\u4E3A\u5F3A\u5236\u66F4\u65B0\uFF0C\u4F46\u4E0B\u8F7D\u5230\u7684\u66F4\u65B0\u5305\u65E0\u6548\uFF0C\u7A0B\u5E8F\u5C06\u9000\u51FA\u3002") << std::endl;
						Sleep(2000);
						return false;
					}

					std::cout << Utf8FromWide(L"\u66F4\u65B0\u5305\u65E0\u6548\uFF0C\u5DF2\u8DF3\u8FC7\u81EA\u52A8\u66F4\u65B0\uFF0C\u7EE7\u7EED\u4F7F\u7528\u5F53\u524D\u7248\u672C\u3002") << std::endl;
					return true;
				}

				std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u6210\u529F\uFF0C\u51C6\u5907\u66FF\u6362\u7A0B\u5E8F...") << std::endl;
				if (ReplaceAndLaunchNewVersion(GetTempUpdateFilePath(), payloadType, GameData.Version)) {
					Sleep(3000);
					return false;
				}

				std::cout << Utf8FromWide(L"\u81EA\u52A8\u66F4\u65B0\u5931\u8D25\uFF0C\u8BF7\u624B\u52A8\u66FF\u6362\u7A0B\u5E8F\u6587\u4EF6\u3002") << std::endl;
				DeleteFileIfExists(GetTempUpdateFilePath());
				std::cout << Utf8FromWide(L"\u6309\u4EFB\u610F\u952E\u7EE7\u7EED\u4F7F\u7528\u5F53\u524D\u7248\u672C...") << std::endl;
				system("pause");
				return true;
			}

			std::cout << Utf8FromWide(L"\u4E0B\u8F7D\u66F4\u65B0\u6587\u4EF6\u5931\u8D25\u3002") << std::endl;
			if (GameData.pbForceUpdating) {
				std::cout << Utf8FromWide(L"\u5F53\u524D\u4E3A\u5F3A\u5236\u66F4\u65B0\uFF0C\u7A0B\u5E8F\u5C06\u9000\u51FA\u3002") << std::endl;
				Sleep(2000);
				return false;
			}

			std::cout << Utf8FromWide(L"\u6309\u4EFB\u610F\u952E\u7EE7\u7EED\u4F7F\u7528\u5F53\u524D\u7248\u672C...") << std::endl;
			system("pause");
			return true;
		}

		std::cout << Utf8FromWide(L"\u7248\u672C\u53F7\u4E00\u81F4\uFF0C\u65E0\u9700\u66F4\u65B0\u3002") << std::endl;
		if (SaveVersionToFile(GameData.Version)) {
			std::cout << Utf8FromWide(L"\u5DF2\u540C\u6B65\u7248\u672C\u53F7: ") << GameData.Version << std::endl;
		}

		return true;
	}
}

void SetConsoleStyle()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleTitle(L"\u6B22\u8FCE\u4F7F\u7528\u54D2\u54D2\u54D2\uFF0C\u516C\u76CA\u8F6F\u4EF6\uFF0C\u4EC5\u4F9B\u5A31\u4E50\u3002\u4E00\u5207\u540E\u679C\u81EA\u884C\u627F\u62C5\uFF01\uFF01");

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!(dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
	{
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
}

int DENGLU;
//
//static int Refresh()
//{
//	while (true)
//	{
//		mem.RefreshAll();
//
//		Sleep(1000 * 60 * 15);
//	}
//}

BOOL WINAPI ConsoleHandler(DWORD event) {
	if (event == CTRL_CLOSE_EVENT) {
		HWND Progman = FindWindowA("Progman", NULL);
		HWND TrayWnd = FindWindowA("Shell_TrayWnd", NULL);
		ShowWindow(Progman, SW_SHOW);
		ShowWindow(TrayWnd, SW_SHOW);
		return TRUE;
	}
	return FALSE;
}

int main(int argc, char* argv[]) {
	Utils::SetDiagnosticLogging(argc > 1);



	if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
	{
		return -1;
	}

	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL)
	{
		ShowWindow(hwnd, SW_SHOW);
	}

	std::thread OverlayloginThread(&Overlay::login);

	while (!GameData.Config.Window.IsLogin) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	SetConsoleStyle();

	Utils::Log(0, " ");
	Utils::Log(0, U8("  ====================================================================================================="));
	Utils::Log(0, U8("  =                                                                                                   ="));
	Utils::Log(0, U8("  =                           ██    ██░      ██████░       ██   ██░                                  ="));
	Utils::Log(0, U8("  =                           ██    ██░      ██░░░░       ██    ██░                                  ="));
	Utils::Log(0, U8("  =                           ██    ██░      ██████░        ██ ██░░                                  ="));
	Utils::Log(0, U8("  =                           ██    ██░      ██░░░░         ████░                                    ="));
	Utils::Log(0, U8("  =                            ██  ██░       ██████░         ████░                                   ="));
	Utils::Log(0, U8("  =                             ████░        ██░░░░        ██  ██░                                   ="));
	Utils::Log(0, U8("  =                              ██░         ██████░      ██    ██░                                  ="));
	Utils::Log(0, U8("  =                                                                                                   ="));
	Utils::Log(0, U8("  =                                 >>>   V E X  <<<   PUBG                                           ="));
	Utils::Log(0, U8("  ====================================================================================================="));
	Utils::Log(1, Utf8FromWide(L"[-] [OUTPUT] 欢迎使用VEX公益DMA----功能正在启动中........").c_str());


	GameData.Config.Window.IsLogin = true;
	ShowWindow(hwnd, SW_SHOW);
	Config::Load();

	std::thread HackThread(Hack::Init);
	std::thread([] {
		while (true)
		{
			const uint64_t controller = GameData.PlayerController;
			const auto players = Data::GetCachePlayers();
			if (controller >= 0x10000 && controller <= 0x00007FFFFFFFFFFFULL && !players.empty())
			{
				for (uint64_t offset = 0x400; offset <= 0x520; offset += 8)
				{
					const uint64_t candidate = mem.Read<uint64_t>(controller + offset);
					if (players.find(candidate) != players.end())
					{
						if (GameData.Offset["AcknowledgedPawn"] != offset)
							Utils::Log(1, "[OFFSET] AcknowledgedPawn auto-selected: 0x%llX", offset);
						GameData.Offset["AcknowledgedPawn"] = offset;
						GameData.AcknowledgedPawn = candidate;
						break;
					}
				}
			}

			for (uint64_t offset = 0x4C0; offset <= 0x510; offset += 8)
			{
				const uint64_t candidate = mem.Read<uint64_t>(controller + offset);
				if (candidate < 0x10000 || candidate > 0x00007FFFFFFFFFFFULL)
					continue;
				const float fov = mem.Read<float>(candidate + GameData.Offset["CameraCacheFOV"]);
				if (std::isfinite(fov) && fov >= 20.0f && fov <= 180.0f)
				{
					if (GameData.Offset["PlayerCameraManager"] != offset)
						Utils::Log(1, "[OFFSET] PlayerCameraManager auto-selected: 0x%llX (FOV %.1f)", offset, fov);
					GameData.Offset["PlayerCameraManager"] = offset;
					GameData.PlayerCameraManager = candidate;
					break;
				}
			}

			const uint64_t pawn = GameData.AcknowledgedPawn;
			if (pawn >= 0x10000 && pawn <= 0x00007FFFFFFFFFFFULL)
				GameData.CameraViewTarget = pawn;
			Sleep(50);
		}
	}).detach();
	if (Utils::IsDiagnosticLoggingEnabled())
	{
		std::thread([] {
			while (true)
			{
				const auto entities = Data::GetCacheEntitys();
				const auto players = Data::GetCachePlayers();
				static uint64_t dumpedPawn = 0;
				const uint64_t currentPawn = GameData.AcknowledgedPawn;
				if (currentPawn >= 0x10000 && currentPawn <= 0x00007FFFFFFFFFFFULL &&
					currentPawn != dumpedPawn)
				{
					dumpedPawn = currentPawn;
					std::vector<unsigned char> pawnDump(0x4000);
					if (mem.Read(currentPawn, pawnDump.data(), static_cast<DWORD>(pawnDump.size())))
						std::ofstream("pawn_runtime_dump.bin", std::ios::binary | std::ios::trunc)
							.write(reinterpret_cast<const char*>(pawnDump.data()), pawnDump.size());

					const uint64_t weaponProcessor = mem.Read<uint64_t>(currentPawn + GameData.Offset["WeaponProcessor"]);
					if (weaponProcessor >= 0x10000 && weaponProcessor <= 0x00007FFFFFFFFFFFULL)
					{
						std::vector<unsigned char> weaponDump(0x1000);
						if (mem.Read(weaponProcessor, weaponDump.data(), static_cast<DWORD>(weaponDump.size())))
							std::ofstream("weapon_processor_runtime_dump.bin", std::ios::binary | std::ios::trunc)
								.write(reinterpret_cast<const char*>(weaponDump.data()), weaponDump.size());
					}
				}
				const uint64_t giA70 = mem.Read<uint64_t>(GameData.UWorld + 0xA70);
				const uint64_t gi888 = mem.Read<uint64_t>(GameData.UWorld + 0x888);
				const uint64_t level228 = mem.Read<uint64_t>(GameData.UWorld + 0x228);
				const uint64_t level850 = mem.Read<uint64_t>(GameData.UWorld + 0x850);
				Utils::Log(1,
					"[DRAW] scene=%d base=0x%llX world=0x%llX gameInstance=0x%llX gameState=0x%llX level=0x%llX actorArray=0x%llX gnames=0x%llX entities=%zu players=%zu objId=0x%llX localPlayer=0x%llX controller=0x%llX pawn=0x%llX camera=0x%llX selfEntity=0x%llX mesh=0x%llX weaponProcessor=0x%llX weapon=0x%llX health=%.1f weaponIndex=%u giA70=0x%llX gi888=0x%llX level228=0x%llX level850=0x%llX",
					(int)GameData.Scene, GameData.GameBase, GameData.UWorld,
					GameData.GameInstance, GameData.GameState, GameData.CurrentLevel,
					GameData.ActorArray, GameData.GNames, entities.size(), players.size(),
					GameData.Offset["ObjID"], GameData.LocalPlayer, GameData.PlayerController,
					GameData.AcknowledgedPawn, GameData.PlayerCameraManager,
					GameData.LocalPlayerInfo.Entity, GameData.LocalPlayerInfo.MeshComponent,
					GameData.LocalPlayerInfo.WeaponProcessor, GameData.LocalPlayerInfo.CurrentWeapon,
					GameData.LocalPlayerInfo.Health, (unsigned int)GameData.LocalPlayerInfo.CurrentWeaponIndex,
					giA70, gi888, level228, level850);
				Sleep(3000);
			}
		}).detach();
	}

	Overlay::Init(0, DrawMain);
	while (true)
	{
		if (GameData.Config.Window.Main) {
			Sleep(500);
			continue;
		}
		break;
	}

	return 0;
}
