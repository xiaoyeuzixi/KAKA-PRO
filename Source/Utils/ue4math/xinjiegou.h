#pragma once
#include <iostream>
#include <array>
//#include "struct.h"

#define SMALL_NUMBER 1e-8
#define M_INCH2METRE( x )	( ( x ) * 0.0254f )

static std::mutex g_tri_mutex{};
// 首先需要定义 Vector3 和 Quaternion 结构
struct Vector3
{
	float x, y, z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
	float Distance(Vector3 v) {
		return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}
	// 向量加法
	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	// 向量缩放
	Vector3 operator*(float scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
     Vector3& operator*=(const Vector3& v) {
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}
	// 向量减法
	Vector3 operator-() const {
		return Vector3(-x, -y, -z);
	}

	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	// 计算向量的长度
	float Length() const {
		return sqrt(x * x + y * y + z * z);
	}
	// 标准化向量
	Vector3 Normalize() const {
		float len = Length();
		return { x / len, y / len, z / len };
	}
	[[nodiscard]] bool IsZero() const {
		return (std::fpclassify(this->x) == FP_ZERO &&
			std::fpclassify(this->y) == FP_ZERO &&
			std::fpclassify(this->z) == FP_ZERO);
	}
	float dot(const Vector3& b) const
	{
		return (x * b.x) + (y * b.y) + (z * b.z);
	}
	 float Dot(const Vector3& vOther) const
	{
		const Vector3& a = *this;

		return(a.x * vOther.x + a.y * vOther.y + a.z * vOther.z);
	}




	// 其他必要的运算符和方法...
};
inline Vector3 CrossProduct(const Vector3& a, const Vector3& b)
{
	return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
struct triangle
{
	Vector3 v1, v2, v3;

	triangle(Vector3 v1, Vector3 v2, Vector3 v3) : v1(v1), v2(v2), v3(v3) {}
	bool intersect(Vector3 ray_origin, Vector3 ray_end) const {
		const float EPSILON = 0.0000001f;
		Vector3 edge1, edge2, h, s, q;
		float a, f, u, v, t;
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		h = CrossProduct(ray_end - ray_origin, edge2);
		a = edge1.Dot(h);

		if (a > -EPSILON && a < EPSILON)
			return false;    // 光线与三角形平行，不相交

		f = 1.0 / a;
		s = ray_origin - v1;
		u = f * s.Dot(h);

		if (u < 0.0 || u > 1.0)
			return false;

		q = CrossProduct(s, edge1);
		v = f * (ray_end - ray_origin).Dot(q);

		if (v < 0.0 || u + v > 1.0)
			return false;

		// 计算 t 来找到交点
		t = f * edge2.Dot(q);

		if (t > EPSILON && t < 1.0) // 确保 t 在 0 和 1 之间，表示交点在线段上
			return true;

		return false; // 这意味着光线与三角形不相交或者在三角形的边界上
	}
};
inline std::vector<triangle> g_triangles;



class Quaternion {
public:
	float x, y, z, w;

	Quaternion() : x(0), y(0), z(0), w(1) {}
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	// 四元数乘法
	Quaternion operator*(const Quaternion& other) const {
		return Quaternion(
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w,
			w * other.w - x * other.x - y * other.y - z * other.z
		);
	}

	Vector3 rotate(const Vector3& v) const {
		const float vx = 2.0f * v.x;
		const float vy = 2.0f * v.y;
		const float vz = 2.0f * v.z;
		const float w2 = w * w - 0.5f;
		const float dot2 = (x * vx + y * vy + z * vz);
		return Vector3(
			(vx * w2 + (y * vz - z * vy) * w + x * dot2),
			(vy * w2 + (z * vx - x * vz) * w + y * dot2),
			(vz * w2 + (x * vy - y * vx) * w + z * dot2)
		);
	}

	Vector3 rotateInv(const Vector3& v) const {
		const float vx = 2.0f * v.x;
		const float vy = 2.0f * v.y;
		const float vz = 2.0f * v.z;
		const float w2 = w * w - 0.5f;
		const float dot2 = (x * vx + y * vy + z * vz);
		return Vector3(
			(vx * w2 - (y * vz - z * vy) * w + x * dot2),
			(vy * w2 - (z * vx - x * vz) * w + y * dot2),
			(vz * w2 - (x * vy - y * vx) * w + z * dot2)
		);
	}

	Quaternion getConjugate() const {
		return Quaternion(-x, -y, -z, w);
	}
};

struct Transform3D
{
	Quaternion Rotation;
	Vector3   Translation;

	Transform3D() : Translation(0, 0, 0), Rotation(0, 0, 0, 1) {}
	Transform3D(const Vector3& translation, const Quaternion& rotation)
		: Translation(translation), Rotation(rotation) {}

	Transform3D t(const Transform3D& src) const {
		// 旋转并加上平移
		Vector3 newTranslation = Rotation.rotate(src.Translation) + Translation;
		// 组合旋转
		Quaternion newRotation = Rotation * src.Rotation;

		// 返回新的变换
		return Transform3D(newTranslation, newRotation);
	}

	// 重载乘法运算符
	Transform3D operator*(const Transform3D& x) const
	{
		return t(x);
	}

	Transform3D getInverse() const {
		return Transform3D(Rotation.rotateInv(-Translation), Rotation.getConjugate());
	}
};

class PxMat33
{
public:
	explicit PxMat33(const Quaternion& q)
	{
		const float x = q.x;
		const float y = q.y;
		const float z = q.z;
		const float w = q.w;

		const float x2 = x + x;
		const float y2 = y + y;
		const float z2 = z + z;

		const float xx = x2 * x;
		const float yy = y2 * y;
		const float zz = z2 * z;

		const float xy = x2 * y;
		const float xz = x2 * z;
		const float xw = x2 * w;

		const float yz = y2 * z;
		const float yw = y2 * w;
		const float zw = z2 * w;

		column0 = Vector3(1.0f - yy - zz, xy + zw, xz - yw);
		column1 = Vector3(xy - zw, 1.0f - xx - zz, yz + xw);
		column2 = Vector3(xz + yw, yz - xw, 1.0f - xx - yy);
	}



	// New constructor for creating a matrix from three vectors
	PxMat33(const Vector3& col0, const Vector3& col1, const Vector3& col2)
		: column0(col0), column1(col1), column2(col2) {}

	inline static const PxMat33 createDiagonal(const Vector3& d)
	{
		return PxMat33(Vector3(d.x, 0.0f, 0.0f), Vector3(0.0f, d.y, 0.0f), Vector3(0.0f, 0.0f, d.z));
	}

	inline const PxMat33 operator*(float scalar) const
	{
		return PxMat33(column0 * scalar, column1 * scalar, column2 * scalar);
	}

	Vector3 column0, column1, column2; // the three base vectors
};


class PxVec4
{
public:


	PxVec4(Vector3 v, float vv) : x(v.x), y(v.y), z(v.z), w(vv)
	{

	}
	PxVec4(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw)
	{
	}

	PxVec4 operator*(float f) const
	{
		return { x * f, y * f, z * f, w * f };
	}

	PxVec4 operator+(const PxVec4& v) const
	{
		return PxVec4(x + v.x, y + v.y, z + v.z, w + v.w);
	}


	Vector3 getXYZ() const
	{
		return Vector3(x, y, z);
	}

	float x, y, z, w;
};

class PxMat44
{
public:
	PxMat44(const PxMat33& axes, const Vector3& position)
		: column0(axes.column0, 0.0f), column1(axes.column1, 0.0f), column2(axes.column2, 0.0f), column3(position, 1.0f)
	{
	}

	PxVec4 Transform(const PxVec4& other) const
	{
		return column0 * other.x + column1 * other.y + column2 * other.z + column3 * other.w;
	}

	Vector3 Transform(const Vector3& other) const
	{
		return Transform(PxVec4(other, 1.0f)).getXYZ();
	}

	PxVec4 column0, column1, column2, column3;
};
// 计算法线

struct f_minimal_view_info final
{
	Vector3                               location;
	Vector3                               rotation;
	char pad1[4];
	float                                  fov;
};
inline f_minimal_view_info view;

struct FMatrix1
{
	float m[4][4];
};





inline FMatrix1 Matrix(Vector3 rot, Vector3 origin)
{
	float radPitch = (rot.x * M_PI / 180.f);
	float radYaw = (rot.y * M_PI / 180.f);
	float radRoll = (rot.z * M_PI / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	FMatrix1 matrix{};
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

struct Rigid_Dynamic
{
	char m_pad0[0x8]{};
	int16_t actor_type;//0x8
	char m_pad1[0x1E]{};
	uint64_t shapes;//0x28
	uint16_t num_shape;//0x30
	char m_pad2[0x36]{};
	uint8_t rigid_dynamic_control_state;//0x68
	char m_pad3[0x7]{};
	uint64_t global_transform_cache;//0x70
	char m_pad4[0x18]{};
	Transform3D global_transform;//0x90
	char m_pad5[0x4]{};
	Transform3D tb;//0xb0;
	char m_pad6[0x74]{};
	Transform3D ta;//0x140;
};

struct Convex_Mesh
{
	char m_pad0[0x44]{};
	uint16_t tmp1;//0X44
	uint8_t num_vertices;//0X46
	uint8_t num_polygons;//0X47
	uint64_t polygons;//0X48
};

struct PxConcreteType
{
	enum Enum
	{
		eUNDEFINED,

		eHEIGHTFIELD,
		eCONVEX_MESH,
		eTRIANGLE_MESH_BVH33,
		eTRIANGLE_MESH_BVH34,
		eCLOTH_FABRIC,

		eRIGID_DYNAMIC,
		eRIGID_STATIC,
		eSHAPE,
		eMATERIAL,
		eCONSTRAINT,
		eCLOTH,
		ePARTICLE_SYSTEM,
		ePARTICLE_FLUID,
		eAGGREGATE,
		eARTICULATION,
		eARTICULATION_LINK,
		eARTICULATION_JOINT,
		ePRUNING_STRUCTURE,

		ePHYSX_CORE_COUNT,
		eFIRST_PHYSX_EXTENSION = 256,
		eFIRST_VEHICLE_EXTENSION = 512,
		eFIRST_USER_EXTENSION = 1024
	};
};

struct FGuid
{
	int32_t                                       A;                                                            // 0x0000(0x0004)
	int32_t                                       B;                                                            // 0x0004(0x0004)
	int32_t                                       C;                                                            // 0x0008(0x0004)
	int32_t                                       D;                                                            // 0x000C(0x0004)
};
inline float Getfov(float x1, float x2, float y1, float y2)
{
	double dis;
	float deltax = (x1 - x2);
	float deltay = (y1 - y2);
	return dis = sqrt(pow(deltax, 2.0f) + pow(deltay, 2.0f));
}
inline  float Getdistance(Vector3 p1, Vector3 p2) {
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float dz = p1.z - p2.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// credits tni & learn_more (www.unknowncheats.me/forum/3868338-post34.html)
#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define get_byte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

template <typename Ty>
std::vector<Ty> bytes_to_vec(const std::string& bytes)
{
	const auto num_bytes = bytes.size() / 3;
	const auto num_elements = num_bytes / sizeof(Ty);

	std::vector<Ty> vec;
	vec.resize(num_elements + 1);

	const char* v1 = bytes.c_str();
	uint8_t* v2 = reinterpret_cast<uint8_t*>(vec.data());
	while (*v1 != '\0')
	{
		if (*v1 == ' ')
		{
			++v1;
		}
		else
		{
			*v2++ = get_byte(v1);
			v1 += 2;
		}
	}

	return vec;
}

#ifndef Amax
#define Amax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef Amin
#define Amin(a,b)            (((a) < (b)) ? (a) : (b))
#endif


struct BoundingBox {
	Vector3 vmin, vmax;

	bool intersect(const Vector3& ray_origin, const Vector3& ray_end) const { //Slabs method
		Vector3 dir = ray_end - ray_origin;
		dir = dir.Normalize(); // 确保方向向量是单位向量

		float t1 = (vmin.x - ray_origin.x) / dir.x;
		float t2 = (vmax.x - ray_origin.x) / dir.x;
		float t3 = (vmin.y - ray_origin.y) / dir.y;
		float t4 = (vmax.y - ray_origin.y) / dir.y;
		float t5 = (vmin.z - ray_origin.z) / dir.z;
		float t6 = (vmax.z - ray_origin.z) / dir.z;

		float tmin = Amax(Amax(Amin(t1, t2), Amin(t3, t4)), Amin(t5, t6));
		float tmax = Amin(Amin(Amax(t1, t2), Amax(t3, t4)), Amax(t5, t6));

		// 如果 tmax < 0，光线与盒子相交在光线的反方向上，所以不相交
		if (tmax < 0) {
			return false;
		}

		// 如果 tmin > tmax，光线不会穿过盒子，所以不相交
		if (tmin > tmax) {
			return false;
		}

		return true;
	}
};



struct KDNode {
	BoundingBox bbox;
	std::vector<triangle> triangle;
	KDNode* left, * right = nullptr;
	int axis;

	void deleteKDTree(KDNode* node) {
		if (node == nullptr) return;

		// 递归地删除子节点
		deleteKDTree(node->left);
		deleteKDTree(node->right);

		// 删除当前节点
		delete node;
	}
};

inline bool rayIntersectsKDTree(KDNode* node, const Vector3& ray_origin, const Vector3& ray_end) {
	if (node == nullptr) return false;

	if (!node->bbox.intersect(ray_origin, ray_end)) {
		return false;
	}

	if (node->triangle.size() > 0) {
		bool hit = false;
		for (const auto& tri : node->triangle) {
			if (tri.intersect(ray_origin, ray_end)) {
				hit = true;
			}
		}
		return hit;
	}

	bool hit_left = rayIntersectsKDTree(node->left, ray_origin, ray_end);
	bool hit_right = rayIntersectsKDTree(node->right, ray_origin, ray_end);

	return hit_left || hit_right;
}

inline BoundingBox calculateBoundingBox(const std::vector<triangle>& triangles) {
	BoundingBox box;
	// 初始化为第一个三角形的第一个点
	box.vmin = box.vmax = triangles[0].v1;
	for (const auto& tri : triangles) {
		for (const auto& p : { tri.v1, tri.v2, tri.v3 }) {
			box.vmin.x = Amin(box.vmin.x, p.x);
			box.vmin.y = Amin(box.vmin.y, p.y);
			box.vmin.z = Amin(box.vmin.z, p.z);
			box.vmax.x = Amax(box.vmax.x, p.x);
			box.vmax.y = Amax(box.vmax.y, p.y);
			box.vmax.z = Amax(box.vmax.z, p.z);
		}
	}
	return box;
}

inline KDNode* buildKDTree(std::vector<triangle>& triangles, int depth = 0) {
	if (triangles.empty()) return nullptr;

	KDNode* node = new KDNode();
	node->bbox = calculateBoundingBox(triangles);
	node->axis = depth % 3; // 分割轴是根据深度选择的

	if (triangles.size() <= 3) {
		node->triangle = triangles;
		return node;
	}

	auto comparator = [axis = node->axis](const triangle& a, const triangle& b) {
		// 比较函数使用 node->axis 来获取当前的分割轴
		float a_center, b_center;
		switch (axis) {
		case 0:
			a_center = (a.v1.x + a.v2.x + a.v3.x) / 3;
			b_center = (b.v1.x + b.v2.x + b.v3.x) / 3;
			break;
		case 1:
			a_center = (a.v1.y + a.v2.y + a.v3.y) / 3;
			b_center = (b.v1.y + b.v2.y + b.v3.y) / 3;
			break;
		case 2:
			a_center = (a.v1.z + a.v2.z + a.v3.z) / 3;
			b_center = (b.v1.z + b.v2.z + b.v3.z) / 3;
			break;
		}
		return a_center < b_center;
		};

	std::nth_element(triangles.begin(), triangles.begin() + triangles.size() / 2, triangles.end(), comparator);

	std::vector<triangle> left_triangles(triangles.begin(), triangles.begin() + triangles.size() / 2);
	std::vector<triangle> right_triangles(triangles.begin() + triangles.size() / 2, triangles.end());

	node->left = buildKDTree(left_triangles, depth + 1);
	node->right = buildKDTree(right_triangles, depth + 1);

	return node;
}
//KDNode* kd_tree;
//void unload() {
//	kd_tree->deleteKDTree(kd_tree);
//	printf("[+]Map reset\n");
//}
class map_loader {
public:
	std::vector<triangle> triangles;
	KDNode* kd_tree;
	bool isloadmap;
	void unload() {
		kd_tree->deleteKDTree(kd_tree);
		printf("[+]Map reset\n");
	}



	//void load_map(std::string map_name) {
	//	auto begin = std::chrono::steady_clock::now();

	//	std::ifstream in("tri/" + map_name + ".tri", std::ios::in);
	//	std::istreambuf_iterator<char> beg(in), end;
	//	std::string strdata(beg, end);
	//	triangles = bytes_to_vec<triangle>(strdata);

	//	std::string().swap(strdata);
	//	in.close();
	//	kd_tree = buildKDTree(triangles);
	//	std::vector<triangle>().swap(triangles);

	//	auto i_end = std::chrono::steady_clock::now();
	//	std::cout << "[MAP] Loaded {" << map_name << "} " << std::chrono::duration<double, std::milli>(i_end - begin).count() << "ms" << std::endl;
	//}

	bool is_visible(Vector3 ray_origin, Vector3 ray_end) {
		return !rayIntersectsKDTree(kd_tree, ray_origin, ray_end);
	}
}; extern map_loader mapload1;














#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <tuple>

struct Point {
	float x, y, z;  // 三维坐标
};

struct triangledd {
	Vector3 p1, p2, p3;  // 三个顶点
};

// 定义一个用于比较点的函数
 struct PointHash {
	std::size_t operator()(const Vector3& pt) const {
		return std::hash<float>()(pt.x) ^ std::hash<float>()(pt.y) ^ std::hash<float>()(pt.z);
	}
};
 //float  DotProduct(Vector3 VecA, Vector3 VecB)
 //{
	// return VecA.x * VecB.x + VecA.y * VecB.y + VecA.z * VecB.z;
 //}
// 定义一个用于比较点的比较函数
struct PointEqual {
	bool operator()(const Vector3& a, const Vector3& b) const {
		return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
	}
};

// 保存三角形到文件的函数
inline void saveTrianglesToFile(const std::vector<triangle>& triangles, const std::string& filename) {
	std::ofstream outFile(filename);


	if (!outFile) {
		std::cerr << "无法打开文件: " << filename << std::endl;
		return;
	}

	std::set<Vector3, PointEqual> uniquePoints; 

	for (const auto& triangle : triangles) {

		if (uniquePoints.insert(triangle.v1).second &&
			uniquePoints.insert(triangle.v2).second &&
			uniquePoints.insert(triangle.v3).second) {
		
			outFile << "Triangle: ("
				<< triangle.v1.x << ", " << triangle.v1.y << ", " << triangle.v1.z << "), ("
				<< triangle.v2.x << ", " << triangle.v2.y << ", " << triangle.v2.z << "), ("
				<< triangle.v3.x << ", " << triangle.v3.y << ", " << triangle.v3.z << ")\n";
		}
	}

	outFile.close();
	std::cout << "三角形坐标已成功保存到 " << filename << std::endl;
}



//static Vector3 project_world_to_screen(f_minimal_view_info view_info, Vector3 world_location, CameraData LLL)
//{
//	Vector3 screen_location = Vector3(0, 0, 0);
//	FMatrix1 tempMatrix = Matrix(Vector3(LLL.Location.X, LLL.Location.Y, LLL.Location.Z), Vector3(0, 0, 0));
//
//
//
//	Vector3 v_axis_x = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]),
//		v_axis_y = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]),
//		v_axis_z = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);
//
//	Vector3 v_delta = world_location - Vector3(GameData.Camera.Location.X, GameData.Camera.Location.Y, GameData.Camera.Location.Z);
//	Vector3 v_transformed = Vector3(DotProduct(v_delta, v_axis_y), DotProduct(v_delta, v_axis_z), DotProduct(v_delta, v_axis_x));
//
//	if (v_transformed.z < 1.f) v_transformed.z = 1.f;
//	const ImVec2 vecDisplaySize = ImVec2(1920, 1080);
//
//	float screen_center_x = vecDisplaySize.x / 2.0f;
//	float screen_center_y = vecDisplaySize.y / 2.0f;
//
//	screen_location.x = screen_center_x + v_transformed.x * screen_center_x / GameData.Camera.FOV / v_transformed.z;
//	screen_location.y = screen_center_y - v_transformed.y * screen_center_x / GameData.Camera.FOV / v_transformed.z;
//
//	return screen_location;
//}
//static void draw_box(const f_minimal_view_info& mini, std::array<Vector3, 8>& box, CameraData LLL)
//{
//	if (GameData.Config.ESP.boxxx)
//	{
//		int edges[12][2] = {
//		{0, 1}, {1, 2}, {2, 3}, {3, 0},
//		{4, 5}, {5, 6}, {6, 7}, {7, 4},
//		{0, 4}, {1, 5}, {2, 6}, {3, 7}
//		};
//
//
//		for (const auto& edge : edges) {
//
//			auto v1 = project_world_to_screen(mini, box[edge[0]], LLL);
//			auto v2 = project_world_to_screen(mini, box[edge[1]], LLL);
//
//			ImGui::GetForegroundDrawList()->AddLine({ v1.x,v1.y }, { v2.x,v2.y }, ImColor(255, 255, 255));
//
//		}
//	}
//}


//
//inline std::vector<triangle> readTrianglesFromFile(const std::string& filename) {
//	std::vector<triangle> triangles;
//	std::ifstream inFile(filename);
//
//	if (!inFile) {
//		std::cerr << "无法打开文件: " << filename << std::endl;
//		return triangles;
//	}
//
//	triangledd triangles;
//	while (inFile >> triangles.p1.x) {
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p1.y;
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p1.z;
//		inFile >> triangles10.p2.x;
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p2.y;
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p2.z;
//		inFile >> triangles10.p3.x;
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p3.y;
//		inFile.ignore(); // 忽略逗号
//		inFile >> triangles10.p3.z;
//		triangles1.push_back(triangles1);
//	}
//
//	inFile.close();
//	return triangles;
//}

struct filter_data_t
{

	uint32_t word0;
	uint32_t word1;
	uint32_t word2;
	uint32_t word3;

	bool operator == (const filter_data_t& other)const
	{
		return word0 == other.word0 && word1 == other.word1
			&& word2 == other.word2 && word3 == other.word3;
	}

};

struct tr
{
	std::vector<triangle>triangle;
	filter_data_t filter;
};


struct PxHeightFieldSample
{

	int16_t			height;

	char            pad[2];
};
struct HeightScale
{
	float					heightScale;
	float					rowScale;
	float					columnScale;
};

struct center_extents_t
{

	Vector3 m_center{};
	Vector3 m_extents{};

};

struct gu_mesh_factory_t
{

};
struct px_height_field_sample_t
{

	int16_t m_height{};
	char m_material_index_0{};
	char m_material_index_1{};


};
struct height_field_data_t
{

	center_extents_t m_aabb{};
	uint32_t m_rows{};
	uint32_t m_columns{};
	float m_row_limit{};
	float m_column_limit{};
	float m_nb_columns{};
	px_height_field_sample_t* m_samples{};
	float m_thickness{};
	float m_convex_edge_threshold{};
	uint16_t m_flags{};
	uint8_t m_format{};

};

struct height_field_t
{
	char m_pad[0x8]{};
	uint16_t m_type{};
	uint16_t m_base_flags{};
	uintptr_t m_ref_countable_vfptr{};
	int64_t m_ref_count{};
	height_field_data_t m_data{};
	uint32_t m_sample_stride{};
	uint32_t m_nb_samples{};
	float m_min_height{};
	float m_max_height{};
	int32_t m_modify_count{};
	gu_mesh_factory_t* m_mesh_factory{};

};
inline std::vector<Vector3>point;

inline std::vector<std::array<Vector3, 8> > v_box;

inline std::array<Vector3, 8> get_box(const Vector3& halfExtents, const PxMat44& m) 
{

	std::array<Vector3, 8> vertices = {
			m.Transform({-halfExtents.x, -halfExtents.y, -halfExtents.z}), // 0
			m.Transform({halfExtents.x, -halfExtents.y, -halfExtents.z}),  // 1
			m.Transform({halfExtents.x, halfExtents.y, -halfExtents.z}),   // 2
			m.Transform({-halfExtents.x, halfExtents.y, -halfExtents.z}),  // 3
			m.Transform({-halfExtents.x, -halfExtents.y, halfExtents.z}),  // 4
			m.Transform({halfExtents.x, -halfExtents.y, halfExtents.z}),   // 5
			m.Transform({halfExtents.x, halfExtents.y, halfExtents.z}),    // 6
			m.Transform({-halfExtents.x, halfExtents.y, halfExtents.z})     // 7
	};

	return vertices;
}




struct hull_polygon
{
	float			m_plane[4];
	uint16_t		index_base;
	uint16_t		num_vertices;
};


struct PxPlane
{
	Vector3 v;
	float d;
};
struct HullPolygonData
{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		// PT: this structure won't be allocated with PX_NEW because polygons aren't allocated alone (with a dedicated alloc).
		// Instead they are part of a unique allocation/buffer containing all data for the ConvexHullData class (polygons, followed by
		// hull vertices, edge data, etc). As a result, ctors for embedded classes like PxPlane won't be called.

	PxPlane	mPlane;			//!< Plane equation for this polygon	//Could drop 4th elem as it can be computed from any vertex as: d = - p.dot(n);
	uint16_t	mVRef8;			//!< Offset of vertex references in hull vertex data (CS: can we assume indices are tightly packed and offsets are ascending?? DrawObjects makes and uses this assumption)
	uint8_t	mNbVerts;		//!< Number of vertices/edges in the polygon
	uint8_t	mMinIndex;		//!< Index of the polygon vertex that has minimal projection along this plane's normal.

};

struct Triangle_Mesh
{
	char m_pad0[0x20]{};
	uint32_t num_vertices;//0x20
	uint32_t num_triangles;//0x24
	uint64_t vertices;//0x28
	uint64_t triangles;//0x30
	char m_pad1[0x24]{};
	uint8_t mesh_flags;//0x5c
};