// clinch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Environment.h"

using namespace std;

int main()
{
	CEnvironment env(3, 3);
	
	env.Start();

	cout << "Remaining processes: " << endl;
	env.ShowProcessList();

	return 0;
}