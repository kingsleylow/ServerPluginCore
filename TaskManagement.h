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

private :
	std::set<int> delay_close_list;
	int delay_cnt;
	std::list<TradeTask*> m_buff;
	std::list<TradeTask*> m_task;
public:
	map<string, set<string>> group_symbol;
	static TaskManagement* taskManager;
	static TaskManagement* getInstance();
	TaskManagement();
	~TaskManagement();
	int initialTask;
	int initial_count;
	int reqeust_trade_cnt;
	std::map<string,MyTrade*> m_close_trade;	
	set<string> master_set;
	set<string> follower_set;
	bool inital(string data);
	bool updataTask(nlohmann::json task);
	TradeTask* TaskManagement::getTask(nlohmann::json task);
	void TaskManagement::checkData();
	void TaskManagement::updataTask(TradeTask* buf, TradeTask* run);
	IOCPMutex m_ContextLock;
	bool isWorking;
	void TaskManagement::AddOrder(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode , const int server_id);
	int TaskManagement::getStrategy(int userConfig, int originalCmd);
	void TaskManagement::testData();
	void TaskManagement::startInit();
	void TaskManagement::finishInit();
	string TaskManagement::printTask();
	void TaskManagement::clearTask();


	void TaskManagement::addToCloseOrder(int login, MyTrade* trade);
	MyTrade* TaskManagement::findCloseOrder(int login, int order);

	string TaskManagement::genMissOrderKey(int login, int master_order);
	string TaskManagement::getFullAccout(int login, int server);;
	bool TaskManagement::checkMaster(int login, int server_id);
	void TaskManagement::AddMaster(int login, int server_id);
	void TaskManagement::DeleteMaster(int login, int server_id);
	bool TaskManagement::checkFollower(int login, int server_id);
	void TaskManagement::AddFollower(int login, int server_id);
	void TaskManagement::DeleteFollower(int login, int server_id);
	string TaskManagement::getTaskKey(int master_server_id,string master_id,int follower_server_id, string follower_id);
	string TaskManagement::getTaskKey(TradeTask* task);

	void TaskManagement::getTaskList(list<TradeTask*>& query);
	void TaskManagement::clearTask(list<TradeTask*>& m_task);
	int  TaskManagement::getTaskSize();

	void TaskManagement::init_symbol_group();
	void TaskManagement::AddDelayClose(int order);
	bool TaskManagement::CheckDelayClose(int order);
	void TaskManagement::PoPDelayClose();
};

