#pragma once
#include "Stdafx.h"
#include "TradeTask.h"
 
#define INITIAL_NO 0
#define INITIAL_BEGIN 1
#define INITIAL_REC_BUFF 2
#define INITIAL_FINISH_BUFF 3
#define INITIAL_FINISH 4

#define STRATEGY_ERROR -1
#define STRATEGY_POSITIVE_BY_ORDER 0 /// trace order
#define STRATEGY_NEGATIVE_BY_ORDER 1
#define STRATEGY_TRADE_BY_ORDER 2 /// trace summary


class TaskManagement
{
public:
	static TaskManagement* taskManager;
	static TaskManagement* getInstance();
	TaskManagement();
	~TaskManagement();
	int initialTask;
	std::list<TradeTask*> m_buff;
	std::list<TradeTask*> m_task;
	bool inital(string data);
	bool updataTask(nlohmann::json task);
	TradeTask* TaskManagement::getTask(nlohmann::json task);
	void TaskManagement::checkData();
	void TaskManagement::updataTask(TradeTask* buf, TradeTask* run);
	IOCPMutex m_ContextLock;

	void TaskManagement::AddOrder(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode , const int server_id);
	int TaskManagement::getStrategy(int userConfig, int originalCmd);
	void TaskManagement::testData();
	void TaskManagement::startInit();
	void TaskManagement::finishInit();
	string TaskManagement::printTask();
};

