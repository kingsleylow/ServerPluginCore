#include "Stdafx.h"
#include "TaskManagement.h"
#include "CProcessor.h"
extern CServerInterface *ExtServer;
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
	this->isWorking = false;
	this->m_task.clear();
	this->m_buff.clear();
	this->m_close_trade.clear();
	this->initial_count = 0;
}


//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

TaskManagement::~TaskManagement()
{
	this->initialTask = false;
	this->isWorking = false;



	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {

		TradeTask* task = *tmp;
		if (task != NULL) {
			delete task;
			task = NULL;
		}
	}

	for (auto tmp = this->m_buff.cbegin(); tmp != this->m_buff.cend(); ++tmp) {

		TradeTask* task = *tmp;
		if (task != NULL) {
			delete task;
			task = NULL;
		}
	}



	for (auto tmp = this->m_close_trade.cbegin(); tmp != this->m_close_trade.cend(); ++tmp) {

		MyTrade* task = tmp->second;
		if (task != NULL) {
			delete task;
			task = NULL;
		}
	}

	this->m_task.clear();
	this->m_buff.clear();
	this->m_close_trade.clear();

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
	if (this->isWorking == true) {
		return;
	 }
	 
	m_ContextLock.Lock();

	this->initialTask = INITIAL_FINISH;



	for (auto it = this->m_buff.begin(); it != this->m_buff.end();it++) {
		TradeTask *task = *it;
		this->master_set.insert(atoi(task->master_id.c_str()));
	}
	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {

		TradeTask* task = *tmp;
		if (task != NULL) {
			delete task;
			task = NULL;
		}
	}
	this->m_task = this->m_buff;
	this->m_buff.clear();
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
	bool is_d = nlohmann::json::accept(data);
	if (is_d == true) {
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
			//	string key = this->getTaskKey(task);
				this->m_buff.push_back(task);
			}
			 
		}



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

	nlohmann::json symbol_filter = data["symbol_filter"];
	int size =data["symbol_filter_size"];

	for (int i = 0; i < size;i++) {
		string symbol = symbol_filter[i].get<string>();
		task->symbol_filter.insert(symbol);
	}


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
	 
		/*TradeTask* task = (*tmp);
		map<string, TradeTask*>::iterator it;
		 */



	}

	m_ContextLock.UnLock();
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::testData() {
	 
	for (int i = 0; i < 50;i++) {
		TradeTask* task = new TradeTask();
		time_t myTime;
		time(&myTime);
		 
		task->task_id = to_string(myTime);;
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
		//string key = this->getTaskKey(task);
			this->m_buff.push_back(task);
	}
	this->finishInit();
	
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void TaskManagement::AddOrder(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode, int server_id) {
	m_ContextLock.Lock();
	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {
	 
		TradeTask* task = *tmp;

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
	 
		TradeTask* task = *tmp;
		ExtProcessor.LOG(CmdTrade, "LifeByte::Testing", task->dumpTask().c_str());
		str = str+ task->dumpTask()  ;
	}
	m_ContextLock.UnLock();
	return str;


 }


int  TaskManagement::getTaskSize() {
	m_ContextLock.Lock();
	int size = this->m_task.size();
	m_ContextLock.UnLock();

	return size;
}

void TaskManagement::clearTask(list<TradeTask*>& m_task) {
	for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {

		TradeTask* task = *tmp;
		if (task != NULL) {
			delete task;
			task = NULL;
		}
	}
}
void TaskManagement::clearTask() {
	m_ContextLock.Lock();

	for (auto tmp = this->m_task.cbegin(); tmp != this->m_task.cend(); ++tmp) {

		TradeTask* task = *tmp;
		if (task!=NULL) {
			delete task;
			task = NULL;
		}
	}
	m_ContextLock.UnLock();
}


string TaskManagement::genMissOrderKey(int login, int master_order) {
	return to_string(login) + "_" + to_string(master_order);
}



void TaskManagement::addToCloseOrder(int login,MyTrade* trade) {

	if (trade==NULL) {
		return;
	}

	m_ContextLock.Lock();


	string key = this->genMissOrderKey(login, trade->order);
	this->m_close_trade[key] = trade;
	ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::addToCloseOrder login   %d  order %d", login, trade->order);
	m_ContextLock.UnLock();
}
MyTrade* TaskManagement::findCloseOrder(int login,int order) {
	m_ContextLock.Lock();
	 
	string key = this->genMissOrderKey(login, order);
	MyTrade* trade = this->m_close_trade[key];


	if  (trade ==NULL) {
		ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::findCloseOrder  cannot find  login   %d  order %d", login, order);
	}
	else {
		ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::findCloseOrder login   %d  order %d", login, trade->order);
	}


	m_ContextLock.UnLock();
	return trade;
}

bool TaskManagement::checkMaster(int login) {
	bool res = false;
	m_ContextLock.Lock();
	if (this->master_set.find(login) != this->master_set.end()) {
		res = true;
	}
	m_ContextLock.UnLock();
	return res;
}
void TaskManagement::AddMaster(int login) {
	m_ContextLock.Lock();
	this->master_set.insert(login);
	m_ContextLock.UnLock();
}
void TaskManagement::DeleteMaster(int login) {
	m_ContextLock.Lock();
	this->master_set.erase(login);
	m_ContextLock.UnLock();
}



string TaskManagement::getTaskKey(int master_server_id, string master_id, int follower_server_id, string follower_id) {

	return to_string(master_server_id) + "_" + master_id + to_string(follower_server_id) + "_" + follower_id;

}

string TaskManagement::getTaskKey(TradeTask* task) {
	if (task==NULL) {
		return "NULL";
	}
	return this->getTaskKey(task->master_server_id, task->master_id, task->follower_server_id, task->follower_id);
}

void TaskManagement::getTaskList(list<TradeTask*>& query) {
	m_ContextLock.Lock();
	 
	for (auto it = this->m_task.begin(); it != this->m_task.end();it++) {
		TradeTask* tmp = *it;
		TradeTask* task = new TradeTask();
	 

		task->task_id = tmp->task_id;
		task->follower_ratio = tmp->follower_ratio;
		task->follower_max_vol = tmp->follower_max_vol;
		task->follower_id = tmp->follower_id;
		task->master_id = tmp->master_id;
		task->portal_id = tmp->portal_id;
		task->follower_disable = tmp->follower_disable;
		task->follower_max_drawback = tmp->follower_max_drawback;
		task->auto_reconciliation = tmp->auto_reconciliation;
		task->master_ratio = tmp->master_ratio;
		task->master_strategy = tmp->master_strategy;
		task->master_disable = tmp->master_disable;
		task->master_server_id = tmp->master_server_id;
		task->follower_server_id = tmp->follower_server_id;
		task->symbol_filter = tmp->symbol_filter;
		query.push_back(task);
	}


	m_ContextLock.UnLock();
}



void TaskManagement::init_symbol_group() {
	this->group_symbol.clear();
	int group_pos = 0;
	
	ConSymbol symb;
	ConGroup groupb;


	while (ExtServer->GroupsNext(group_pos,&groupb)!=FALSE) {
		string group_name = groupb.group;
		int symbol_pos = 0;
			while (ExtServer->SymbolsNext(symbol_pos, &symb) != FALSE) {

				int security = symb.type;
				string symbol_name = symb.symbol;
				int isTrade = groupb.secgroups[security].trade;
				int isShow = groupb.secgroups[security].show;
				if (isTrade !=0 && isShow!=0) {
					this->group_symbol[group_name].insert(symbol_name);
				}
				symbol_pos++;
			}
			group_pos++;
	}
	
}


 
