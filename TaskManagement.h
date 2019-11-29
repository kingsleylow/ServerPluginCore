#pragma once
#include "TradeTask.h"
#include "MyIOCP.h"
 
class TaskManagement
{
public:
	static TaskManagement* taskManager;
	static TaskManagement* getIntance();
	TaskManagement();
	~TaskManagement();
	bool initialTask;
	std::vector<TradeTask*> m_task;
	bool inital(MyIOCP* iocp);
	bool updataTask();
};

