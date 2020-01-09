#pragma once
#include "TradeTask.h"
 
#define INITIAL_NO 0
#define INITIAL_BEGIN 1
#define INITIAL_REC_BUFF 2
#define INITIAL_FINISH_BUFF 3
#define INITIAL_FINISH 4
class TaskManagement
{
public:
	static TaskManagement* taskManager;
	static TaskManagement* getInstance();
	TaskManagement();
	~TaskManagement();
	int initialTask;
	std::vector<TradeTask*> m_task;
	bool inital();
	bool updataTask();



	std::vector<TradeTask*> m_buff;
};

