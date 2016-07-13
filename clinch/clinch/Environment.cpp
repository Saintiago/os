#include "stdafx.h"
#include "Environment.h"


CEnvironment::CEnvironment(int procCount, int resCount)
{
	Process proc;
	Resource res;

	processList.assign(procCount, proc);
	resourceList.assign(resCount, res);

	for (int i = 0; i < procCount; ++i)
	{
		processList.at(i).id = i;
	}
	for (int i = 0; i < resCount; ++i)
	{
		resourceList.at(i).id = i;
	}
}

void CEnvironment::Require(int procId, int resId)
{
	if ((procId >= processList.size()) || (resId >= resourceList.size()))
	{
		return;
	}

	processList[procId].resIds.insert(resId);
	resourceList[procId].procIds.push(procId);
}

void CEnvironment::Start()
{
	Require(2, 2);
	Require(2, 1);
	Require(2, 0);
	Require(0, 2);
	Require(0, 1);
	Require(0, 0);
	Require(1, 2);
	Require(1, 1);
	Require(1, 0);
	
	bool processEnded = false;
	do
	{
		processEnded = false;
		for (auto proc : processList)
		{
			processEnded = Resolve(proc.id) || processEnded;
		}
	} 
	while (processEnded);
}

bool CEnvironment::Resolve(int procId)
{
	bool resolveNeeded = true;

	if (!processList[procId].resIds.empty())
	{
		for (int resId : processList[procId].resIds)
		{
			if (resourceList[resId].procIds.front() != procId)
			{
				resolveNeeded = false;
				break;
			}
		}

		if (resolveNeeded)
		{
			for (int resId : processList[procId].resIds)
			{
				resourceList[resId].procIds.pop();
			}
			processList[procId].resIds.clear();
		}
	}
	else
	{
		resolveNeeded = false;
	}

	return resolveNeeded;
}

void CEnvironment::ShowProcessList()
{
	for (auto proc : processList)
	{
		if (!proc.resIds.empty())
		{
			std::cout << proc.id << std::endl;
		}
	}
}


CEnvironment::~CEnvironment()
{
}
