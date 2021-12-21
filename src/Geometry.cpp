#include "Geometry.h"

Geometry::Geometry()
{
	consoleWidth = 120;
	consoleHeight = 60;

	outConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	inConsoleHandle = GetStdHandle(STD_INPUT_HANDLE);

	console = nullptr;
	rectWindow = { 0 };

	memset(newKeyStates, 0, 256 * sizeof(short));
	memset(oldKeyStates, 0, 256 * sizeof(short));
	memset(keys, 0, 256 * sizeof(KeyState));
	memset(mouse, 0, 5 * sizeof(KeyState));
	memset(oldMouseStates, 0, 5 * sizeof(bool));
	memset(newMouseStates, 0, 5 * sizeof(bool));

	mouseX = 0;
	mouseY = 0;
	consoleInFocus = true;
	appName = L"3D model";
}

Geometry::~Geometry()
{
	SetConsoleActiveScreenBuffer(orgConsoleHandle);
	delete[] console;
}

int16_t Geometry::constructConsole(int16_t width, int16_t height, int16_t fontW, int16_t fontH, wstring consoleName)
{
	appName = consoleName;

	if (outConsoleHandle == INVALID_HANDLE_VALUE)
	{
		return error(L"Invalid handle value error");
	}
	consoleWidth = width;
	consoleHeight = height;

	rectWindow = { 0, 0, 1, 1 };
	SetConsoleWindowInfo(outConsoleHandle, TRUE, &rectWindow);

	// Установка размера screen buffer
	COORD coord = { (int16_t)consoleWidth, (int16_t)consoleHeight };
	if (!SetConsoleScreenBufferSize(outConsoleHandle, coord))
	{
		error(L"SetConsoleScreenBufferSize error");
	}
	if (!SetConsoleActiveScreenBuffer(outConsoleHandle))
	{
		return error(L"SetConsoleActiveScreenBuffer error");
	}

	// Установка размера шрифта в screen buffer
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = fontW;
	cfi.dwFontSize.Y = fontH;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	
	wcscpy_s(cfi.FaceName, L"Consolas");
	if (!SetCurrentConsoleFontEx(outConsoleHandle, false, &cfi))
	{
		return error(L"SetCurrentConsoleFontEx error");
	}

	// Проверка на максимально разрешенный размер окна
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(outConsoleHandle, &csbi))
	{
		return error(L"GetConsoleScreenBufferInfo error");
	}
	if (consoleHeight > csbi.dwMaximumWindowSize.Y)
	{
		return error(L"Screen height / font height error");
	}
	if (consoleWidth > csbi.dwMaximumWindowSize.X)
	{
		return error(L"Screen width / font width error");
	}

	// Установка размера окна консоли
	rectWindow = { 0, 0, (int16_t)consoleWidth - 1, (int16_t)consoleHeight - 1 };
	if (!SetConsoleWindowInfo(outConsoleHandle, TRUE, &rectWindow))
	{
		return error(L"SetConsoleWindowInfo error");
	}

	// Разрешение перехвата событий мыши		
	if (!SetConsoleMode(inConsoleHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
	{
		return error(L"SetConsoleMode error");
	}
	console = new CHAR_INFO[consoleWidth * consoleHeight];
	memset(console, 0, sizeof(CHAR_INFO) * consoleWidth * consoleHeight);
	return 0;
}

int16_t Geometry::error(const wchar_t* msg)
{
	wchar_t buf[256];

	setConsoleDefault();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	SetConsoleActiveScreenBuffer(orgConsoleHandle);
	wprintf(L"ERROR: %s\n\t%s\n", msg, buf);
	return 1;
}

void Geometry::setConsoleDefault()
{
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 10;
	cfi.dwFontSize.Y = 15;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;

	wcscpy_s(cfi.FaceName, L"Lucida Console");
	SetCurrentConsoleFontEx(outConsoleHandle, false, &cfi);

	// Установка размера screen buffer
	COORD coord = { 150, 50 };
	SetConsoleScreenBufferSize(outConsoleHandle, coord);
	SetConsoleActiveScreenBuffer(outConsoleHandle);

	// Установка размера окна консоли
	rectWindow = { 0, 0, 145, 45 };
	SetConsoleWindowInfo(outConsoleHandle, TRUE, &rectWindow);
}

void Geometry::run()
{
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	userCreateHandle();

	bool isExit = false;
	bool ifKeyPressed = true;

	while (!isExit)
	{
		// Таймирование
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Перехват событий клавиатурой
		for (int16_t i = 0; i < 256; i++)
		{
			newKeyStates[i] = GetAsyncKeyState(i);
			keys[i].bPressed = false;
			keys[i].bReleased = false;
			if (newKeyStates[i] != oldKeyStates[i])
			{
				if (newKeyStates[i] & 0x8000)
				{
					keys[i].bPressed = !keys[i].bHeld;
					keys[i].bHeld = true;
				}
				else
				{
					keys[i].bReleased = true;
					keys[i].bHeld = false;
				}
				ifKeyPressed = true;
			}
			oldKeyStates[i] = newKeyStates[i];
		}

		// Перехват событий мыши
		INPUT_RECORD inBuf[32];
		DWORD events = 0;
		GetNumberOfConsoleInputEvents(inConsoleHandle, &events);
		if (events > 0)
		{
			ReadConsoleInput(inConsoleHandle, inBuf, events, &events);
		}

		// Обработка событий
		for (DWORD i = 0; i < events; i++)
		{
			switch (inBuf[i].EventType)
			{
				case FOCUS_EVENT:
				{
					consoleInFocus = inBuf[i].Event.FocusEvent.bSetFocus;
				}
				break;
				case MOUSE_EVENT:
				{
					switch (inBuf[i].Event.MouseEvent.dwEventFlags)
					{
						case MOUSE_MOVED:
						{
							mouseX = inBuf[i].Event.MouseEvent.dwMousePosition.X;
							mouseY = inBuf[i].Event.MouseEvent.dwMousePosition.Y;
						}
						break;
						case 0:
						{
							for (int16_t m = 0; m < 5; m++)
							{
								newMouseStates[m] = (inBuf[i].Event.MouseEvent.dwButtonState & (1 << m)) > 0;
							}

						}
						break;
						default:
							break;
					}
				}
				break;
				default:
					break;
				}
			}

		for (int16_t m = 0; m < 5; m++)
		{
			mouse[m].bPressed = false;
			mouse[m].bReleased = false;
			if (newMouseStates[m] != oldMouseStates[m])
			{
				if (newMouseStates[m])
				{
					mouse[m].bPressed = true;
					mouse[m].bHeld = true;
				}
				else
				{
					mouse[m].bReleased = true;
					mouse[m].bHeld = false;
				}
			}
			oldMouseStates[m] = newMouseStates[m];
		}

		if (ifKeyPressed)
		{
			userUpdateHandle(fElapsedTime);
		}

		wchar_t s[256];
		swprintf_s(s, 256, L"%s - FPS: %3.2f", appName.c_str(), 1.0f / fElapsedTime);
		SetConsoleTitle(s);
		WriteConsoleOutput(outConsoleHandle, console, { consoleWidth, consoleHeight }, { 0,0 }, &rectWindow);
	}
}

int16_t Geometry::getConsoleWidth()
{
	return consoleWidth;
}

int16_t Geometry::getConsoleHeight()
{
	return consoleHeight;
}

Geometry::KeyState& Geometry::getKey(int16_t keyId)
{
	return keys[keyId];
}

void Geometry::simpleDraw(int16_t x, int16_t y, int16_t sym, int16_t col)
{
	if (x >= 0 && x < consoleWidth && y >= 0 && y < consoleHeight)
	{
		console[y * consoleWidth + x].Char.UnicodeChar = sym;
		console[y * consoleWidth + x].Attributes = col;
	}
}

void Geometry::drawBresenhamLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t sym, int16_t col)
{
	int16_t x, y;
	int16_t deltaX, deltaY;
	int16_t signX, signY;
	int16_t balance;

	signX = (x2 > x1) ? 1 : -1;
	signY = (y2 > y1) ? 1 : -1;
	deltaX = (signX > 0) ? (x2 - x1) : (x1 - x2);
	deltaY = (signY > 0) ? (y2 - y1) : (y1 - y2);
	x = x1; y = y1;

	if (deltaX >= deltaY)
	{
		deltaY <<= 1;
		balance = deltaY - deltaX;
		deltaX <<= 1;

		while (x != x2)
		{
			simpleDraw(x, y, sym, col);
			if (balance >= 0)
			{
				y += signY;
				balance -= deltaX;
			}
			balance += deltaY;
			x += signX;
		}
		simpleDraw(x, y, sym, col);
	}
	else
	{
		deltaX <<= 1;
		balance = deltaX - deltaY;
		deltaY <<= 1;

		while (y != y2)
		{
			simpleDraw(x, y, sym, col);
			if (balance >= 0)
			{
				x += signX;
				balance -= deltaY;
			}
			balance += deltaX;
			y += signY;
		}
		simpleDraw(x, y, sym, col);
	}
}

void Geometry::drawPolygon(vector<Point2D>& points, int16_t sym, int16_t col)
{
	size_t i;

	for (i = 0; i < points.size() - 1; i++)
	{
		drawBresenhamLine(roundf(points[i].x), roundf(points[i].y), roundf(points[i + 1].x), roundf(points[i + 1].y), sym, col);
	}
	drawBresenhamLine(roundf(points[i].x), roundf(points[i].y), roundf(points[0].x), roundf(points[0].y), sym, col);
}

void Geometry::fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t sym, int16_t col)
{
	clip(x1, y1);
	clip(x2, y2);
	for (int16_t x = x1; x <= x2; x++)
	{
		for (int16_t y = y1; y <= y2; y++)
		{
			simpleDraw(x, y, sym, col);
		}
	}
}

void Geometry::clip(int16_t& x, int16_t& y)
{
	if (x < 0)
	{
		x = 0;
	}
	else if (x >= consoleWidth)
	{
		x = consoleWidth;
	}
	if (y < 0)
	{
		y = 0;
	}
	else if (y >= consoleHeight)
	{
		y = consoleHeight;
	}
}

void Geometry::shadePolygonScanLine(const vector<Point2D>& points, int16_t sym, int16_t col, int16_t yMin, int16_t yMax,
	int16_t xMin, int16_t xMax)
{
	vector<ScanLineStruct> edges(points.size());
	int16_t minY, maxY;
	vector<int16_t> scanex;

	minY = maxY = round(points[0].y);

	for (size_t i = 0; i < points.size(); i++)
	{
		edges[i].x1 = round(points[i].x);
		edges[i].y1 = round(points[i].y);
		edges[i].x2 = ((i + 1) == points.size()) ? round(points[0].x) : round(points[i + 1].x);
		edges[i].y2 = ((i + 1) == points.size()) ? round(points[0].y) : round(points[i + 1].y);
		edges[i].delX = edges[i].x2 - edges[i].x1;
		edges[i].delY = edges[i].y2 - edges[i].y1;
		edges[i].delXY = (edges[i].delY == 0) ? 0 : edges[i].delX / edges[i].delY;

		if (edges[i].y2 > maxY)
		{
			maxY = edges[i].y2;
		}
		else if (edges[i].y2 < minY)
		{
			minY = edges[i].y2;
		}
	}

	// Warnock алгоритм
	yMin = (yMin == -1) ? 0 : yMin;
	yMax = (yMax == -1) ? consoleHeight : yMax;
	xMin = (xMin == -1) ? 0 : xMin;
	xMax = (xMax == -1) ? consoleWidth : xMax;

	minY = (minY < yMin) ? yMin : minY;
	maxY = (maxY > yMax) ? yMax : maxY;

	for (int16_t y = minY; y < maxY; y++)
	{
		for (size_t i = 0; i < points.size(); i++)
		{
			if ((edges[i].y1 >= y && edges[i].y2 < y) || (edges[i].y1 < y && edges[i].y2 >= y))
			{
				scanex.push_back(edges[i].x1 + edges[i].delXY * (y - edges[i].y1));
			}
			else if (edges[i].y1 == y && edges[i].y2 == y)
			{
				scanex.push_back(edges[i].x1);
				scanex.push_back(edges[i].x2);
			}
		}
		if (scanex.size())
		{
			sort(scanex.begin(), scanex.end());

			for (size_t i = 0; i < scanex.size() - 1; i += 2)
			{
				int16_t x1, x2;
				x1 = (scanex[i] < xMin) ? xMin : scanex[i];
				x2 = (scanex[i + 1] > xMax) ? xMax : scanex[i + 1];
				drawBresenhamLine(x1, y, x2, y, sym, col);
			}
			scanex.clear();
		}
	}
}

void Geometry::shadePolygonFloodFillRecursion(const vector<Point2D>& points, int16_t sym, int16_t col, int16_t colEdges)
{
	Point2D center;

	for (auto& point : points)
	{
		center += point;
	}
	center /= points.size();
	center.x = roundf(center.x);
	center.y = roundf(center.y);
	
	drawBresenhamLine(0, 0, consoleWidth - 1, 0, sym, colEdges);
	drawBresenhamLine(0, 0, 0, consoleHeight - 1, sym, colEdges);
	drawBresenhamLine(consoleWidth - 1, 0, consoleWidth - 1, consoleHeight - 1, sym, colEdges);
	drawBresenhamLine(0, consoleHeight - 1, consoleWidth - 1, consoleHeight - 1, sym, colEdges);

	if (center.x <= 0.0f || center.x >= consoleWidth || center.y <= 0.0f || center.y >= consoleHeight)
	{
		Point2D new_center;
		int16_t counter = 0;

		for (auto& point : points)
		{
			if (point.x >= 0.0f && point.x < consoleWidth && point.y >= 0.0f && point.y < consoleHeight)
			{
				new_center += point;
				counter++;
			}
		}
		new_center += center;
		counter++;
		new_center /= counter;
		center = new_center;
	}

	if (center.x >= 0.0f && center.x < consoleWidth && center.y >= 0.0f && center.y < consoleHeight)
	{
		CHAR_INFO* consolePtr = &console[(int16_t)center.y * consoleWidth + (int16_t)center.x];

		if (consolePtr->Attributes != colEdges && consolePtr->Attributes != col)
		{
			makeFloodFill(consolePtr, center.x, center.y, sym, col, colEdges);
		}
	}
}

void Geometry::makeFloodFill(CHAR_INFO* consolePtr, int16_t x, int16_t y, int16_t sym, int16_t col, int16_t colEdges)
{
	auto on_screen = [this](int16_t x, int16_t y)
	{
		if (x >= 0.0f && x < consoleWidth && y >= 0.0f && y < consoleHeight)
		{
			return true;
		}
		return false;
	};
	simpleDraw(x, y, sym, col);
	consolePtr = &console[(int16_t)y * consoleWidth + (int16_t)x];

	if (on_screen(x, y - 1) && (consolePtr - consoleWidth)->Attributes != colEdges && (consolePtr - consoleWidth)->Attributes != col)
	{
		makeFloodFill(consolePtr, x, y - 1, sym, col, colEdges);
	}
	if (on_screen(x, y + 1) && (consolePtr + consoleWidth)->Attributes != colEdges && (consolePtr + consoleWidth)->Attributes != col)
	{
		makeFloodFill(consolePtr, x, y + 1, sym, col, colEdges);
	}
	if (on_screen(x - 1, y) && (consolePtr - 1)->Attributes != colEdges && (consolePtr - 1)->Attributes != col)
	{
		makeFloodFill(consolePtr, x - 1, y, sym, col, colEdges);
	}
	if (on_screen(x + 1, y) && (consolePtr + 1)->Attributes != colEdges && (consolePtr + 1)->Attributes != col)
	{
		makeFloodFill(consolePtr, x + 1, y, sym, col, colEdges);
	}
}

bool Geometry::onSegment(const Point3D& p, const Point3D& q, const Point3D& r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
	{
		return true;
	}
	return false;
}

bool Geometry::checkPointAndSegment(const Point3D& start, const Point3D& p, const Point3D& end)
{
	float delX = end.x - start.x;
	float delY = end.y - start.y;
	float delXY = (abs(delY) <= 0.00001f) ? 0.0f : delX / delY;
	float delYX = (abs(delX) <= 0.00001f) ? 0.0f : delY / delX;

	if (onSegment(start, p, end))
	{
		if (abs(delX) <= 0.00001f)
		{
			return true;
		}

		if (abs(delY) <= 0.00001f)
		{
			return true;
		}

		if (start.x + delXY * (p.y - start.y) - 1.0f <= p.x && start.x + delXY * (p.y - start.y) + 1.0f >= p.x)
		{
			if (start.y + delYX * (p.x - start.x) - 1.0f <= p.y && start.y + delYX * (p.x - start.x) + 1.0f >= p.y)
			{
				return true;
			}
		}
	}

	return false;
}

void Geometry::paintAlgorithm(vector<triangle>& vecTrianglesToRaster, Point3D& viewPoint, Point3D& barycenter, int16_t sym, int16_t col, int16_t colEdge)
{
	Point3D vec1, vec2;
	vector<triangle> vecVisibleSurfaces;
	bool itsEdge = false;

	for (auto& tri : vecTrianglesToRaster)
	{
		vec1 = tri.points[0] - tri.points[1];
		vec2 = tri.points[2] - tri.points[1];

		float d;
		Point3D v;
		v = vectorCrossProduct(vec1, vec2);
		d = -vectorDotProduct(v, tri.points[0]);

		if ((vectorDotProduct(v, barycenter) + d) < 0.0f)
		{
			v *= -1.0f;;
			d *= -1.0f;
		}

		if ((vectorDotProduct(v, viewPoint) + d) < 0.0f)
		{
			if (checkPointAndSegment(tri.points[0], tri.points[2], tri.points[1]))
			{
				itsEdge = true;
			}
			else if (checkPointAndSegment(tri.points[0], tri.points[1], tri.points[2]))
			{
				itsEdge = true;
			}
			else if (checkPointAndSegment(tri.points[1], tri.points[0], tri.points[2]))
			{
				itsEdge = true;
			}
			if (!itsEdge)
			{
				vector<Point2D> points(3);
				for (int16_t i = 0; i < 3; i++)
				{
					points[i].x = tri.points[i].x;
					points[i].y = tri.points[i].y;
				}
				drawPolygon(points, sym, FG_YELLOW);
				shadePolygonFloodFillRecursion(points, sym, col, FG_YELLOW);
				vecVisibleSurfaces.push_back(tri);
			}
			itsEdge = false;
		}
	}
}

void Geometry::drawShadow(vector<triangle>& vecTrianglesToRaster, Point3D& light)
{
	vector<triangle> vecShadow = vecTrianglesToRaster;

	for (auto& tri : vecShadow)
	{
		for (int16_t i = 0; i < 3; i++)
		{
			tri.points[i].z *= tri.points[i].w;
			tri.points[i].x -= light.x * (tri.points[i].y / light.y);
			tri.points[i].z = -tri.points[i].z - light.z * (tri.points[i].y / light.y);
			tri.points[i].y = 0.95f * static_cast<float>(consoleHeight) + tri.points[i].z * 10.0f;
		}
	}

	vector<Point2D> lines(3);

	for (auto& tri : vecShadow)
	{
		for (int16_t i = 0; i < 3; i++)
		{
			lines[i].x = tri.points[i].x;
			lines[i].y = tri.points[i].y;
		}
		shadePolygonScanLine(lines, PIXEL_SOLID, BG_GREY);
	}
}

float Geometry::vectorDotProduct(Point3D& v1, Point3D& v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

float Geometry::vectorLength(Point3D& v)
{
	return sqrt(vectorDotProduct(v, v));
}

Geometry::Point3D Geometry::vectorNormalise(Point3D& v)
{
	float l = vectorLength(v);
	return v / l;
}

Geometry::Point3D Geometry::vectorCrossProduct(Point3D& v1, Point3D& v2)
{
	Point3D v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

Geometry::Point2D Geometry::multiplyMatrix(matrix3x3& m, Point2D& v)
{
	Point2D v1;
	v1.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.w * m.m[2][0];
	v1.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.w * m.m[2][1];
	v1.w = v.x * m.m[0][2] + v.y * m.m[1][2] + v.w * m.m[2][2];
	return v1;
}

Geometry::Point3D Geometry::multiplyMatrix(matrix4x4& m, Point3D& v)
{
	Point3D v1;
	v1.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0];
	v1.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1];
	v1.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2];
	v1.w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3];
	return v1;
}

Geometry::matrix4x4 Geometry::makeIdentity()
{
	matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeRotationX(float fAngleRad)
{
	matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeRotationY(float fAngleRad)
{
	matrix4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeRotationZ(float fAngleRad)
{
	matrix4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeScale(float x, float y, float z)
{
	matrix4x4 matrix;
	matrix.m[0][0] = x;
	matrix.m[1][1] = y;
	matrix.m[2][2] = z;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeTranslation(float x, float y, float z)
{
	matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * PI);
	matrix4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::makeProjectionIzometric()
{
	matrix4x4 matrix;
	matrix.m[0][0] = sqrt(1.0f / 2.0f);
	matrix.m[0][1] = -sqrt(1.0f / 6.0f);
	matrix.m[0][2] = sqrt(1.0f / 3.0f);
	matrix.m[1][1] = sqrt(2.0f / 3.0f);
	matrix.m[1][2] = sqrt(1.0f / 3.0f);
	matrix.m[2][0] = -sqrt(1.0f / 2.0f);
	matrix.m[2][1] = -sqrt(1.0f / 6.0f);
	matrix.m[2][2] = sqrt(1.0f / 3.0f);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Geometry::matrix4x4 Geometry::multiplyMatrix(matrix4x4& m1, matrix4x4& m2)
{
	matrix4x4 matrix;
	for (int16_t c = 0; c < 4; c++)
	{
		for (int16_t r = 0; r < 4; r++)
		{
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		}
	}
	return matrix;
}