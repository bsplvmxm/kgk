#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <chrono>
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>

constexpr float PI = 3.14159f;

using namespace std;

enum COLOUR
{
	FG_BLACK = 0x0000,
	FG_GREY = 0x0007,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_RED = 0x000C,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_GREY = 0x0070,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_RED = 0x00C0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

enum PIXEL_TYPE
{
	PIXEL_SOLID = 0x2588,
	PIXEL_THREEQUARTERS = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591,
};

class Geometry
{
protected:
	wstring appName;
	int16_t consoleWidth, consoleHeight;
	SMALL_RECT rectWindow;
	HANDLE outConsoleHandle;
	HANDLE inConsoleHandle;
	HANDLE orgConsoleHandle;
	CHAR_INFO* console;								

	struct KeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	};

	int16_t oldKeyStates[256];
	int16_t newKeyStates[256];
	KeyState keys[256];
	KeyState mouse[5];
	bool oldMouseStates[5];
	bool newMouseStates[5];
	bool consoleInFocus;
	int16_t mouseX;
	int16_t mouseY;

	int16_t error(const wchar_t* msg);
	virtual void userCreateHandle() = 0;
	virtual void userUpdateHandle(float fElapsedTime) = 0;

private:
	void setConsoleDefault();

public:
	Geometry();
	~Geometry();

	int16_t constructConsole(int16_t width, int16_t height, int16_t fontW, int16_t fontH, wstring consoleName = L"3D model");
	int16_t getConsoleWidth();
	int16_t getConsoleHeight();
	KeyState& getKey(int16_t keyId);
	int getMouseX() 
	{ 
		return mouseX;
	}
	int getMouseY() 
	{ 
		return mouseY;
	}
	KeyState getMouse(int nMouseButtonId) 
	{ 
		return mouse[nMouseButtonId];
	}
	bool isFocused() 
	{ 
		return consoleInFocus; 
	}
	void run();

// Геометрические фигуры и методы
protected:
	struct Point3D;

	struct matrix3x3
	{
		float m[3][3] = { 0 };
	};
	struct Point2D
	{
		float x;
		float y;
		float w;

	public:
		Point2D(): x(0.0f), y(0.0f), w(0.0f) {}
		Point2D(float x, float y, float w = 1.0f): x(x), y(y), w(w) {}

		Point2D& operator=(const Point2D& obj)
		{
			x = obj.x;
			y = obj.y;
			w = obj.w;
			return *this;
		}
		Point2D& operator=(const Point3D& obj)
		{
			x = obj.x;
			y = obj.y;
			w = obj.w;
			return *this;
		}
		Point2D& operator+=(const Point2D& obj)
		{
			x += obj.x;
			y += obj.y;
			return *this;
		}
		Point2D& operator-=(const Point2D& obj)
		{
			x -= obj.x;
			y -= obj.y;
			return *this;
		}
		Point2D& operator*=(float value)
		{
			x *= value;
			y *= value;
			return *this;
		}
		Point2D& operator/=(float value)
		{
			x /= value;
			y /= value;
			return *this;
		}

		Point2D operator+(const Point2D& obj)
		{
			return Point2D(x + obj.x, y + obj.y);
		}
		Point2D operator-(const Point2D& obj)
		{
			return Point2D(x - obj.x, y - obj.y);
		}
		Point2D operator*(float value)
		{
			return Point2D(x * value, y * value);
		}
		Point2D operator/(float value)
		{
			return Point2D(x / value, y / value);
		}

		void multiplyMatrix(matrix3x3& m)
		{
			x = x * m.m[0][0] + x * m.m[1][0] + x * m.m[2][0];
			y = y * m.m[0][1] + y * m.m[1][1] + y * m.m[2][1];
		}
	};
	// Для сканирующей прямой
	struct ScanLineStruct
	{
		int16_t x1, x2, y1, y2;
		float delX, delY, delXY, delYX;

		ScanLineStruct()
		{
			x1 = x2 = y1 = y2 = 0;
			delX = delY = delXY = delYX = 0.0f;
		}

		ScanLineStruct& operator=(const ScanLineStruct& obj)
		{
			x1 = obj.x1;
			x2 = obj.x2;
			y1 = obj.y1;
			y2 = obj.y2;
			delX = obj.delX;
			delY = obj.delY;
			delXY = obj.delXY;
			delYX = obj.delYX;
			return *this;
		}
	};

	struct matrix4x4
	{
		float m[4][4] = { 0 };

		matrix4x4 operator*(matrix4x4& m1)
		{
			matrix4x4 matrix;
			for (int16_t c = 0; c < 4; c++)
			{
				for (int16_t r = 0; r < 4; r++)
				{
					matrix.m[r][c] = m[r][0] * m1.m[0][c] + m[r][1] * m1.m[1][c] + m[r][2] * m1.m[2][c] + m[r][3] * m1.m[3][c];
				}
			}
			return matrix;
		}
	};
	struct Point3D
	{
		float x, y, z, w;

	public:
		Point3D() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
		Point3D(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}

		Point3D& operator=(const Point3D& obj)
		{
			x = obj.x;
			y = obj.y;
			z = obj.z;
			w = obj.w;
			return *this;
		}
		Point3D& operator=(const Point2D& obj)
		{
			x = obj.x;
			y = obj.y;
			z = 0.0f;
			w = obj.w;
			return *this;
		}
		Point3D& operator=(float value)
		{
			x = value;
			y = value;
			z = value;
			return *this;
		}
		Point3D& operator+=(const Point3D& obj)
		{
			x += obj.x;
			y += obj.y;
			z += obj.z;
			return *this;
		}
		Point3D& operator-=(const Point3D& obj)
		{
			x -= obj.x;
			y -= obj.y;
			z -= obj.z;
			return *this;
		}
		Point3D& operator*=(float value)
		{
			x *= value;
			y *= value;
			z *= value;
			return *this;
		}
		Point3D& operator/=(float value)
		{
			x /= value;
			y /= value;
			z /= value;
			return *this;
		}

		bool operator==(Point3D& obj)
		{
			if (fabsf(x - obj.x) < 0.001f)
			{
				if (fabsf(y - obj.y) < 0.001f)
				{
					if (fabsf(z - obj.z) < 0.001f)
					{
						return true;
					}
				}
			}
			return false;
		}

		Point3D operator+(const Point3D& obj)
		{
			return Point3D(x + obj.x, y + obj.y, z + obj.z, w);
		}
		Point3D operator-(const Point3D& obj)
		{
			return Point3D(x - obj.x, y - obj.y, z - obj.z, w);
		}
		Point3D operator*(float value)
		{
			return Point3D(x * value, y * value, z * value, w);
		}
		Point3D operator/(float value)
		{
			return Point3D(x / value, y / value, z / value, w);
		}
	};
	struct triangle
	{
		Point3D points[3];

		int16_t sym = PIXEL_SOLID;
		int16_t col = FG_WHITE;

		triangle() {};
		triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
		{
			points[0].x = x1; 
			points[0].y = y1;
			points[0].z = z1;
			points[1].x = x2; 
			points[1].y = y2;
			points[1].z = z2;
			points[2].x = x3;
			points[2].y = y3;
			points[2].z = z3;
		}
		triangle(const Point2D& p1, const Point2D& p2, const Point2D& p3)
		{
			points[0] = p1;
			points[1] = p2;
			points[2] = p3;
		}
	};

	struct Mesh
	{
		vector<triangle> tris;
	};

public: 
	// Метод отрисовки
	void simpleDraw(int16_t x, int16_t y, int16_t sym = ' ', int16_t col = BG_WHITE);
	void drawBresenhamLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t sym = ' ', int16_t col = BG_WHITE);
	void drawPolygon(vector<Point2D>& points, int16_t sym = ' ', int16_t col = BG_WHITE);
	void fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t sym = PIXEL_SOLID, int16_t col = FG_BLACK);
	void clip(int16_t& x, int16_t& y);
	void shadePolygonScanLine(const vector<Point2D>& points, int16_t sym = ' ', int16_t col = BG_WHITE,
		int16_t yMin = -1, int16_t yMax = -1, int16_t xMin = -1, int16_t xMax = -1);
	void shadePolygonFloodFillRecursion(const vector<Point2D>& points, int16_t sym = ' ',
		int16_t col = BG_WHITE, int16_t colEdges = BG_RED);
	void paintAlgorithm(vector<triangle>& vecTrianglesToRaster, Point3D& viewPoint, Point3D& barycenter,
		int16_t sym = PIXEL_SOLID, int16_t col = FG_YELLOW, int16_t colEdge = BG_RED);
	void drawShadow(vector<triangle>& vecTrianglesToRaster, Point3D& light);

private:
	void makeFloodFill(CHAR_INFO* consolePtr, int16_t x, int16_t y, int16_t sym, int16_t col, int16_t colEdges);
	bool onSegment(const Point3D& p, const Point3D& q, const Point3D& r);
	bool checkPointAndSegment(const Point3D& start, const Point3D& p, const Point3D& end);

public:
	// Методы матриц для работы с 3D
	float vectorDotProduct(Point3D& v1, Point3D& v2);
	float vectorLength(Point3D& v);
	Point3D vectorNormalise(Point3D& v);
	Point3D vectorCrossProduct(Point3D& v1, Point3D& v2);
	Point2D multiplyMatrix(matrix3x3& m, Point2D& v);
	Point3D multiplyMatrix(matrix4x4& m, Point3D& v);
	matrix4x4 makeIdentity();
	matrix4x4 makeRotationX(float fAngleRad);
	matrix4x4 makeRotationY(float fAngleRad);
	matrix4x4 makeRotationZ(float fAngleRad);
	matrix4x4 makeScale(float x, float y, float z);
	matrix4x4 makeTranslation(float x, float y, float z);
	matrix4x4 makeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
	matrix4x4 makeProjectionIzometric();
	matrix4x4 multiplyMatrix(matrix4x4& m1, matrix4x4& m2);
};

#endif 
