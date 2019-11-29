#include "stdafx.h"
#include "TaskManagement.h"

TaskManagement* TaskManagement::taskManager;
TaskManagement* TaskManagement::getIntance() {
	if (taskManager == NULL) {
		taskManager = new TaskManagement();
	}
	return taskManager;
}





TaskManagement::TaskManagement()
{
	this->initialTask = false;
	 
}


TaskManagement::~TaskManagement()
{
	this->initialTask = false;
}

bool TaskManagement::inital(MyIOCP* iocp)
{
	bool res = true;
	iocp->SendRequest(CMD_QUERY_GOD_PORTFOLIO_MULTIPLE, NULL);

	//this->initalTask = res;
	return res;
}

bool TaskManagement::updataTask()
{
	bool res = true;

	return res;
}

