#include "ThreeDModel.h"

void ThreeDModel::userCreateHandle()
{
	shapes.resize(2);

	// Призма
	shapes[0].tris =
	{
			{ 0.0f, 0.0f, 0.0f,    0.0f, 2.0f, 0.0f,    1.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 2.0f, 0.0f,    1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 2.0f, 0.0f,    1.0f, 2.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 2.0f, 1.0f,    1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f,    0.0f, 2.0f, 0.0f,    1.0f, 2.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 1.0f,    1.0f, 2.0f, 1.0f },
			{ 0.0f, 2.0f, 0.0f,    1.0f, 2.0f, 1.0f,    1.0f, 2.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }
	};

	// Пирамида
	shapes[1].tris =
	{
			{ 0.0f, 0.0f, 0.0f,    2.0f, 0.0f, 0.0f,    1.0f, 0.0f, 2.0f },                                                    
			{ 0.0f, 0.0f, 0.0f,    1.0f, 2.0f, 1.0f,    2.0f, 0.0f, 0.0f },                                                      
			{ 2.0f, 0.0f, 0.0f,    1.0f, 2.0f, 1.0f,    1.0f, 0.0f, 2.0f },                                                   
			{ 1.0f, 0.0f, 2.0f,    1.0f, 2.0f, 1.0f,    0.0f, 0.0f, 0.0f }
	};

	matrixProjection = makeProjection(90.0f, static_cast<float>(getConsoleHeight()) / static_cast<float>(getConsoleWidth()), 1.0f, 10.0f);
	sx = sy = 0.4f;
	sa = -4.0f;
	sm = 0.1f;

	light.x = 1.0f;
	light.y = -100.0f;
	light.z = 1.0f;
	
	scale = 1.0f;							
	coordX = 0.5f; coordY = 0.5f; coordZ = 4.0f;
	thetaX = thetaY = thetaZ = 0.0f;
}

void ThreeDModel::userUpdateHandle(float fElapsedTime)
{
	fill(0, 0, getConsoleWidth(), getConsoleHeight());
	fill(0, consoleHeight / 2, consoleWidth, consoleHeight, PIXEL_SOLID, BG_BLUE);

	// Движение вокруг оси
	if (getKey(L'W').bHeld)
	{
		thetaX += 8.0f * fElapsedTime;
	}
	if (getKey(L'S').bHeld)
	{
		thetaX -= 8.0f * fElapsedTime;
	}
	if (getKey(L'A').bHeld)
	{
		thetaY += 8.0f * fElapsedTime;
	}
	if (getKey(L'D').bHeld)
	{
		thetaY -= 8.0f * fElapsedTime;
	}
	if (getKey(L'Q').bHeld)
	{
		thetaZ += 8.0f * fElapsedTime;
	}
	if (getKey(L'E').bHeld)
	{
		thetaZ -= 8.0f * fElapsedTime;
	}

	// Масштабирование фигур
	if (getKey(L'Z').bHeld)
	{
		scale = (scale <= 1.2f) ? scale + 0.01f : scale;
	}
	if (getKey(L'X').bHeld)
	{
		scale = (scale >= 0.5f) ? scale - 0.01f : scale;
	}

	if (isFocused())
	{
		if (getKey(VK_LBUTTON).bHeld)
		{
			// вправо
			if (getMouseX() >= 0.9f * consoleWidth)
			{
				coordX += 0.05f * sm;
			}
			// влево
			if (getMouseX() <= 0.2f * consoleWidth)
			{
				coordX -= 0.05f * sm;
			}
			// вверх
			if (getMouseY() <= 0.2f * consoleHeight)
			{
				coordY -= 0.05f * sm;
			}
			// вниз
			if (getMouseY() >= 0.9f * consoleHeight)
			{
				coordY += 0.05f * sm;
			}
			// вперед
			if ((getMouseX() >= 0.40f * consoleWidth) && (getMouseX() <= 0.60f * consoleWidth)
				&& (getMouseY() >= 0.35f * consoleHeight) && (getMouseY() <= 0.49f * consoleHeight))
			{
				coordZ = (coordZ < 7.0f) ? coordZ + 0.1f : coordZ;
			}
			// назад
			if ((getMouseX() >= 0.40f * consoleWidth) && (getMouseX() <= 0.60f * consoleWidth)
				&& (getMouseY() >= 0.51f * consoleHeight) && (getMouseY() <= 0.65f * consoleHeight))
			{
				coordZ = (coordZ > 4.5f) ? coordZ - 0.1f : coordZ;
			}
		}
	}

	matrix4x4 matRotX, matRotY, matRotZ;
	matRotX = makeRotationX(thetaX * 0.5f);
	matRotY = makeRotationY(thetaY * 0.5f);
	matRotZ = makeRotationZ(thetaZ * 0.5f);

	matrix4x4 ScalingMatrix;
	ScalingMatrix = makeScale(scale, scale, scale);

	matrix4x4 TranslationMatrix;
	TranslationMatrix = makeTranslation(0.0f, 0.0f, coordZ);

	matrix4x4 WorldMatrix;
	WorldMatrix = makeIdentity();
	WorldMatrix = matRotY * matRotX * matRotZ * ScalingMatrix * TranslationMatrix;

	vector<triangle> vecTrianglesToRaster;

	float  t = 0.0f;
	int16_t triColor = FG_RED;
	int16_t countTris = 0;
	for (auto& sh: shapes) 
	{
		for (auto tri : sh.tris)
		{
			triangle triProjected, triTransformed;

			for (int16_t i = 0; i < 3; i++)
			{
				triTransformed.points[i] = multiplyMatrix(WorldMatrix, tri.points[i]);

				// 3D в 2D
				triProjected.points[i] = multiplyMatrix(matrixProjection, triTransformed.points[i]);
				triProjected.points[i] = triProjected.points[i] / triProjected.points[i].w;
				triProjected.points[i].x *= -1.0f;
				triProjected.points[i].y *= -1.0f;
			}

			// Масштабирование под размер консоли
			for (int16_t i = 0; i < 3; i++)
			{
				triProjected.points[i].x += coordX + t;
				triProjected.points[i].y += coordY;
				triProjected.points[i].x *= (0.1f + sx) * static_cast<float>(getConsoleWidth());
				triProjected.points[i].y *= (0.1f + sy) * static_cast<float>(getConsoleHeight());
				barycenter += triProjected.points[i];
			}
			countTris++;

			// Смена цвета
			triColor = triColor == BG_BLUE ? FG_RED : BG_BLUE;
			triProjected.col = triColor;
			vecTrianglesToRaster.push_back(triProjected);
		}

		barycenter /= countTris * 3;

		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.points[0].z + t1.points[1].z + t1.points[2].z) / 3.0f;
				float z2 = (t2.points[0].z + t2.points[1].z + t2.points[2].z) / 3.0f;
				return z1 > z2;
			}
		);

		for (auto& tri : vecTrianglesToRaster)
		{
			for (int16_t i = 0; i < 3; i++)
			{
				tri.points[i].x = roundf(tri.points[i].x);
				tri.points[i].y = roundf(tri.points[i].y);
			}
		}

		drawShadow(vecTrianglesToRaster, light);

		Point3D viewPoint = { static_cast<float>(consoleWidth) / 2.0f, static_cast<float>(consoleHeight) / 2.0f, -100.0f };
		paintAlgorithm(vecTrianglesToRaster, viewPoint, barycenter, PIXEL_SOLID, FG_RED);
		
		t += 5.0f + sa;
		countTris = 0;
		barycenter = 0.0f;
		vecTrianglesToRaster.clear();
	}
}