#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

class WebSocketClient {
public:
    using Client = websocketpp::client<websocketpp::config::asio_client>;
    using ConnectionPtr = websocketpp::connection_hdl;
    using MessagePtr = Client::message_ptr;

    enum class PinStatus {
        NotVerified,
        Verified,
        Error
    };

private:
    Client client;
    ConnectionPtr connection;
    std::thread client_thread;
    bool connected = false;
    std::mutex mutex;
    std::condition_variable cv;
    std::string server_address;
    int server_port;
    std::string pin;
    PinStatus pinStatus = PinStatus::NotVerified;
    std::condition_variable pinVerificationCv;

public:
    WebSocketClient() {
        // 清除所有通道
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.clear_error_channels(websocketpp::log::elevel::all);

        // 初始化ASIO
        client.init_asio();

        // 设置消息处理函数
        client.set_message_handler([this](ConnectionPtr hdl, MessagePtr msg) {
            OnMessage(hdl, msg);
            });

        // 设置打开连接处理函数
        client.set_open_handler([this](ConnectionPtr hdl) {
            OnOpen(hdl);
            });

        // 设置关闭连接处理函数
        client.set_close_handler([this](ConnectionPtr hdl) {
            OnClose(hdl);
            });

        // 设置失败连接处理函数
        client.set_fail_handler([this](ConnectionPtr hdl) {
            OnFail(hdl);
            });
    }

    ~WebSocketClient() {
        Disconnect();
    }

    bool Connect(const std::string& address, const std::string& userPin) {
        try {
            // 确保之前的连接已经完全清理
            Disconnect();

            if (client_thread.joinable()) {
                client_thread.join();
            }

            // 重置客户端状态
            ResetClient();

            {
                std::lock_guard<std::mutex> lock(mutex);
                connected = false;
                pinStatus = PinStatus::NotVerified;
                server_address = address;
                pin = userPin;
            }

            std::string uri = "ws://" + address;
            std::cout << "Attempting to connect to: " << uri << std::endl;

            websocketpp::lib::error_code ec;
            Client::connection_ptr con = client.get_connection(uri, ec);

            if (ec) {
                std::cout << "Get connection error: " << ec.message() << std::endl;
                return false;
            }

            connection = con->get_handle();
            client.connect(con);

            // 启动客户端线程
            client_thread = std::thread([this]() {
                std::cout << "Client thread started" << std::endl;
                try {
                    client.run();
                }
                catch (const std::exception& e) {
                    std::cout << "Exception in client thread: " << e.what() << std::endl;
                }
                std::cout << "Client thread ended" << std::endl;
                });

            // 等待连接成功
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (!cv.wait_for(lock, std::chrono::seconds(5), [this]() { return connected; })) {
                    std::cout << "Connection timeout" << std::endl;
                    Disconnect();
                    return false;
                }
                pinStatus = PinStatus::Verified;
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cout << "Exception in Connect: " << e.what() << std::endl;
            Disconnect();
            return false;
        }
    }

    void Disconnect() {
        try {
            websocketpp::lib::error_code ec;

            // 先停止 ASIO 服务
            client.stop_perpetual();

            // 安全地关闭连接
            if (connection.lock()) {
                client.close(connection, websocketpp::close::status::normal, "Closing connection", ec);
            }

            // 停止客户端
            client.stop();

            // 等待线程结束
            if (client_thread.joinable()) {
                std::cout << "Joining client thread..." << std::endl;
                client_thread.join();
                std::cout << "Client thread joined" << std::endl;
            }

            {
                std::lock_guard<std::mutex> lock(mutex);
                connected = false;
                pinStatus = PinStatus::NotVerified;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception in Disconnect: " << e.what() << std::endl;
        }
    }

    bool SendMessage(const std::string& message) {
        try {
            if (!connected) {
                std::cout << "[ERROR] SendMessage failed: not connected" << std::endl;
                return false;
            }

            websocketpp::lib::error_code ec;
            client.send(connection, message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cout << "[ERROR] SendMessage failed: " << ec.message() << std::endl;
                return false;
            }
            //std::cout << "[SENT] " << message << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cout << "[ERROR] Exception in SendMessage: " << e.what() << std::endl;
            return false;
        }
    }
    bool SendBinaryMessage(const std::string& message) {
        try {
            if (!connected) {
                std::cout << "[ERROR] SendBinaryMessage failed: not connected" << std::endl;
                return false;
            }

            if (message.empty()) {
                std::cout << "[ERROR] SendBinaryMessage failed: empty message" << std::endl;
                return false;
            }

            websocketpp::lib::error_code ec;
            client.send(connection, message.data(), message.size(), websocketpp::frame::opcode::binary, ec);

            if (ec) {
                std::cout << "[ERROR] SendBinaryMessage failed: " << ec.message() << std::endl;
                return false;
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cout << "[ERROR] Exception in SendBinaryMessage: " << e.what() << std::endl;
            return false;
        }
    }

    bool IsConnected() const {
        return connected;
    }

    PinStatus GetPinStatus() const {
        return pinStatus;
    }

private:
    void ResetClient() {
        try {
            // 停止当前客户端
            if (connection.lock()) {
                websocketpp::lib::error_code ec;
                client.close(connection, websocketpp::close::status::normal, "Resetting client", ec);
            }
            client.stop_perpetual();
            client.stop();

            // 重新初始化现有客户端
            client.reset();  // 重置 ASIO
            client.clear_access_channels(websocketpp::log::alevel::all);
            client.clear_error_channels(websocketpp::log::elevel::all);
            client.init_asio();

            // 重新设置所有处理函数
            client.set_message_handler([this](ConnectionPtr hdl, MessagePtr msg) {
                OnMessage(hdl, msg);
                });
            client.set_open_handler([this](ConnectionPtr hdl) {
                OnOpen(hdl);
                });
            client.set_close_handler([this](ConnectionPtr hdl) {
                OnClose(hdl);
                });
            client.set_fail_handler([this](ConnectionPtr hdl) {
                OnFail(hdl);
                });
        }
        catch (const std::exception& e) {
            std::cout << "Exception in ResetClient: " << e.what() << std::endl;
        }
    }

    void OnOpen(ConnectionPtr hdl) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            connected = true;
            std::cout << "WebSocket connection established" << std::endl;
        }
        cv.notify_one();
    }

    void OnClose(ConnectionPtr hdl) {
        bool shouldReconnect = false;
        {
            std::lock_guard<std::mutex> lock(mutex);
            shouldReconnect = connected; // 只有之前是连接状态才需要重连
            connected = false;
            pinStatus = PinStatus::NotVerified;
            std::cout << "Connection closed" << std::endl;
        }

        // 修改重连逻辑
        if (!client.stopped() && shouldReconnect) {
            // 使用独立线程进行重连，避免死锁
            std::thread([this]() {
                std::cout << "Starting reconnection..." << std::endl;

                // 等待一段时间再重连
                std::this_thread::sleep_for(std::chrono::seconds(2));

                int retryCount = 0;
                const int maxRetries = 3;

                while (retryCount < maxRetries) {
                    try {
                        std::cout << "Attempting to reconnect... (attempt " << retryCount + 1 << ")" << std::endl;

                        // 确保在重连之前完全断开之前的连接
                        Disconnect();

                        // 重置所有状态
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            connected = false;
                            pinStatus = PinStatus::NotVerified;
                        }

                        // 尝试重新连接
                        if (Connect(server_address, pin)) {
                            std::cout << "Reconnection successful" << std::endl;
                            break;
                        }

                        retryCount++;
                        if (retryCount < maxRetries) {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                        }
                    }
                    catch (const std::exception& e) {
                        std::cout << "Exception during reconnection attempt: " << e.what() << std::endl;
                        retryCount++;
                        if (retryCount < maxRetries) {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                        }
                    }
                }

                if (retryCount >= maxRetries) {
                    std::cout << "Reconnection failed after " << maxRetries << " attempts" << std::endl;
                }
                }).detach();
        }
    }

    void OnFail(ConnectionPtr hdl) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            connected = false;
            pinStatus = PinStatus::NotVerified;
            std::cout << "Connection failed" << std::endl;
        }
        cv.notify_one();
    }

    void OnMessage(ConnectionPtr hdl, MessagePtr msg) {
        try {
            std::string payload = msg->get_payload();
            //std::cout << "[RECEIVED] " << payload << std::endl;

            if (payload == "connection:success") {
                return;
            }

            if (payload == "PIN:OK") {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    pinStatus = PinStatus::Verified;
                    //std::cout << "[INFO] PIN verified successfully" << std::endl;
                    Utils::Log(1, U8("PIN验证成功"));
                }
                pinVerificationCv.notify_one();
            }
            else if (payload == "PIN:ERROR") {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    pinStatus = PinStatus::Error;
                    //std::cout << "[ERROR] PIN verification failed" << std::endl;
                    Utils::Log(2, U8("PIN验证失败"));
                }
                pinVerificationCv.notify_one();
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception in OnMessage: " << e.what() << std::endl;
        }
    }
};