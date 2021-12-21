#ifndef _NEW_GRAPHICS_H_
#define _NEW_GRAPHICS_H_

#include "Geometry.h"

class ThreeDModel : public Geometry
{
private:
	float scale;
	float coordX, coordY, coordZ;
	float thetaX, thetaY, thetaZ;
	float sx, sy, sm, sa;
	Point3D light;
	Point3D barycenter;
	vector<Mesh> shapes;
	matrix4x4 matrixProjection;

	virtual void userCreateHandle() override;
	virtual void userUpdateHandle(float fElapsedTime) override;
};

#endif