#include "Stdafx.h"
#include "TaskManagement.h"

TaskManagement* TaskManagement::taskManager;
TaskManagement* TaskManagement::getInstance() {
	if (taskManager == NULL) {
		taskManager = new TaskManagement();
	}
	return taskManager;
}




//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

TaskManagement::TaskManagement()
{
	this->initialTask = false;
	 
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

TaskManagement::~TaskManagement()
{
	this->initialTask = false;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

int TaskManagement::getStrategy(int userConfig, int originalCmd) {

	int tCmd = STRATEGY_ERROR;

	if (userConfig == STRATEGY_POSITIVE_BY_ORDER) {
		tCmd = originalCmd;

	}
	else if (userConfig == STRATEGY_NEGATIVE_BY_ORDER) {

		if (originalCmd == OP_BUY || originalCmd == OP_SELL) {

			tCmd = originalCmd == OP_BUY ? OP_SELL : OP_BUY;
		}
		else if (originalCmd == OP_BUY_LIMIT || originalCmd == OP_SELL_STOP) {

			//	tCmd = originalCmd == OP_BUY_LIMIT ? OP_SELL_STOP : OP_BUY_LIMIT;
			tCmd = originalCmd;
		}

		else if (originalCmd == OP_BUY_STOP || originalCmd == OP_SELL_LIMIT) {

			//	tCmd = originalCmd == OP_BUY_STOP ? OP_SELL_LIMIT : OP_BUY_STOP;
			tCmd = originalCmd;
		}
	}


	return tCmd;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::startInit() {
	m_ContextLock.Lock();
	this->initialTask = INITIAL_REC_BUFF;
	this->m_buff.clear();
	m_ContextLock.UnLock();
}	

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::finishInit() {
	m_ContextLock.Lock();
	this->initialTask = INITIAL_FINISH;
	this->m_task.clear();
	this->m_task = this->m_buff ;
	m_ContextLock.UnLock();
}
//+------------------------------------------------------------------+
//|                    
//| inital copy trade task
//+------------------------------------------------------------------+

bool TaskManagement::inital(string data)
{
	m_ContextLock.Lock();
	bool res = true;
	try {
		nlohmann::json j = nlohmann::json::parse(data);
		if (!j.contains("data")) {
			return false;

		}
		if (!j["data"].contains("list")) {
			return false;
		}
		if (!j["data"].contains("size")) {
			return false;
		}
		nlohmann::json tasks = j["data"]["list"];
		int size = j["data"]["size"];
		 
		

		for (size_t i = 0; i < tasks.size(); i++) {
			nlohmann::json tmp = tasks[i];
			TradeTask* task = this->getTask(tmp);
			if (task != NULL) {
				this->m_buff.push_back(   task);
			}
			 
		}



	}
	catch (exception& e) {

		res = false;
	}
	m_ContextLock.UnLock();
	return res;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

bool TaskManagement::updataTask(nlohmann::json task)
{
	bool res = true;
	return res;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::updataTask(TradeTask* buf, TradeTask* run) {

}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

TradeTask* TaskManagement::getTask(nlohmann::json data) {
	
	if (data.contains("task_id") == false||
		data.contains("follower_ratio") == false ||
		data.contains("follower_max_vol") == false ||
		data.contains("follower_id") == false ||
		data.contains("master_id") == false ||
		data.contains("portal_id") == false ||
		data.contains("follower_disable") == false ||
		data.contains("max_drawBack") == false ||
		data.contains("reconciliation_type") == false ||
		data.contains("master_ratio") == false ||
		data.contains("master_strategy") == false ||
		data.contains("master_disable") == false ||
		data.contains("master_server_id") == false ||
		data.contains("follower_server_id") == false
		) {
		return NULL;
	}
	TradeTask* task = new TradeTask();

	task->task_id = data["task_id"].get<string>();
	task->follower_ratio = data["follower_ratio"].get<double>();
	task->follower_max_vol = data["follower_max_vol"].get<int>();
	task->follower_id = data["follower_id"].get<string>();
	task->master_id = data["master_id"].get<string>();
	task->portal_id = data["portal_id"].get<string>();
	task->follower_disable = data["follower_disable"].get<bool>();
	task->follower_max_drawback = data["max_drawBack"].get<double>();
	task->auto_reconciliation = data["reconciliation_type"].get<int>();
	task->master_ratio = data["master_ratio"].get<double>();
	task->master_strategy = data["master_strategy"].get<int>();
	task->master_disable = data["master_disable"].get<bool>();
	task->master_server_id = data["master_server_id"].get<int>();
	task->follower_server_id = data["follower_server_id"].get<int>();
	return task;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::checkData() {
	/*if (this->m_buff.size() != this->m_task.size()) {

	}*/
	m_ContextLock.Lock();
	for (auto tmp = this->m_buff.cbegin(); tmp != this->m_buff.cend(); ++tmp) {
	 
		TradeTask* task = (*tmp);
		map<string, TradeTask*>::iterator it;
		 



	}

	m_ContextLock.UnLock();
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::testData() {
	TradeTask* task = new TradeTask();

	task->task_id = 1;
	task->follower_ratio = 1.0;
	task->follower_max_vol = 500;
	task->follower_id = "5";
	task->master_id = "4";
	task->portal_id = "1";
	task->follower_disable = false;
	task->follower_max_drawback = 10000.0;
	task->auto_reconciliation = 0;
	task->master_ratio = 1.0;
	task->master_strategy = 0;
	task->master_disable = false;
	task->master_server_id = 1;
	task->follower_server_id = 1;
	this->m_buff.push_back(task);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::AddOrder(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode, int server_id) {
	m_ContextLock.Lock();
	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {
	 
		TradeTask* task = (*tmp);

		if (task->follower_disable == true || task->master_disable == true) {
			return;
		}

		if (task->master_server_id != server_id || atoi(task->master_id.c_str()) != trade->login) {
			return;
		}

		string com(trade->comment);
		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
	 
		int cmd = this->getStrategy(task->master_strategy, trade->cmd);
	}

	m_ContextLock.UnLock();
}

string TaskManagement::printTask() {
	m_ContextLock.Lock();
	string str = "task list: ";
	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {
	 
		TradeTask* task = (*tmp);
		str += task->dumpTask();
	}
	m_ContextLock.UnLock();
	return str;


 }