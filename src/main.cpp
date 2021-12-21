#include "ThreeDModel.h"

int main()
{
	ThreeDModel model;
	
	if (!model.constructConsole(400, 250, 2, 2, L"3D model"))
	{
		model.run();
	}
	return 0;
}