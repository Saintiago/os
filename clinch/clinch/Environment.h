#pragma once

struct Process
{
	int id;
	std::set<int> resIds;
};

struct Resource
{
	int id;
	std::queue<int> procIds;
};

class CEnvironment
{
public:
	CEnvironment(int processes, int resources);
	~CEnvironment();

	void Require(int procId, int resId);
	void Start();
	bool Resolve(int procId);
	void ShowProcessList();

	std::vector<Process> processList;
	std::vector<Resource> resourceList;
};

