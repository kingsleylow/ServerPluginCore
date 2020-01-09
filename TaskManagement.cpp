#include "stdafx.h"
#include "TaskManagement.h"

TaskManagement* TaskManagement::taskManager;
TaskManagement* TaskManagement::getInstance() {
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

bool TaskManagement::inital()
{
	bool res = true;
	//this->initialTask = INITIAL_BEGIN;
	//iocp->SendRequest(CMD_QUERY_ALL_TASK, NULL);

	////this->initalTask = res;
	return res;
}

bool TaskManagement::updataTask()
{
	bool res = true;

	return res;
}

