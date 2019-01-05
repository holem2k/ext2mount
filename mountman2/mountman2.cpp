// mountman2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "driver.h"


int main(int argc, char* argv[])
{
	CDriver driver("Mount2", "e:\\ext2fsd\\mount\\build\\i386\\mount.sys");
	int a = driver.Load();
	a = driver.Open();
	DWORD f;
	driver.DeviceControl(0, NULL, 0, NULL, 0, &f, NULL);
	a = driver.Close();
	//driver.Unload();
	return 0;
}
 