#include "stdafx.h"
#include "CProcessor.h"
#include "TaskManagement.h"
#include "MyLOG.h"
#include "Common.h"
 
//#define DIRECT

extern CServerInterface *ExtServer;
CProcessor               ExtProcessor;
TaskManagement			 TaskManager;

CRITICAL_SECTION m_cs;
class OrderTask {
public:
	 int id; 
 
	 double prices_0;
	 double prices_1;
	 
};


CProcessor::CProcessor()
{
	bool isExist = Utils::isExistFolder(LOG_DIR);
	if (isExist == false) {
		Utils::createFolder(LOG_DIR);
	}
	  request_sleep_time = REQUEST_TASK_WAIT;
	  request_buffer = REQUEST_TASK_PRE;
	  request_busy_size = REQUEST_TASK_BUSY;
	 
	plugin_id = D_PLUGIN_ID;

	InitializeCriticalSectionAndSpinCount(&m_cs, 10000);;
	this->request_current_id = 0;
	
}

void CProcessor::startCheckingThread() {
	UINT id = 0;
//	
 //	m_funcThread = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
	this->excutor.commit(ThreadWrapper, this);
}

static volatile int request_check_cycle = 0;
void CProcessor::startRequestThread() {
	UINT id = 0;
	

	//while (true)
//{
	int sleep_time = this->request_sleep_time;

	if (sleep_time < 2 || sleep_time >20) {
		sleep_time = REQUEST_TASK_WAIT;
	}

	//	Sleep(sleep_time);

	request_check_cycle++;
	if (request_check_cycle > sleep_time) {
		request_check_cycle = 0;
	//	(HANDLE)_beginthreadex(NULL, 256000, HandeleThreadWrapper, (void*)this, 0, &id);

		this->excutor.commit(HandeleThreadWrapper,this);
	}
}


CProcessor::~CProcessor()
{
	if (pool !=NULL) {
		delete pool;
		pool = NULL;
	}
	DeleteCriticalSection(&m_cs);
}
//initialize setting in plugin
void CProcessor::Initialize(void) {

 
	ExtConfig.GetString(CORE_IP, m_ip, sizeof(m_ip) - 1, DEFAULT_IP);
	//	ExtConfig.GetString(CORE_BACKUP_IP, m_ip_backup, sizeof(m_ip_backup) - 1, DEFAULT_IP);

	ExtConfig.GetString(CORE_PORT, m_port, sizeof(m_port) - 1, DEFAULT_PORT);
//	ExtConfig.GetString(CORE_BACKUP_PORT, m_port_backup, sizeof(m_port_backup) - 1, DEFAULT_PORT);

	ExtConfig.GetString(CORE_KEY, m_key, sizeof(m_key) - 1, DEFAULT_KEY);

	ExtConfig.GetString(PLUGIN_ID, m_plugin_id, sizeof(m_plugin_id) - 1, PLUGIN_ID);
	
	plugin_id = atoi(m_plugin_id);

	ExtConfig.GetString(REQUEST_SLEEP, m_request_sleep, sizeof(m_request_sleep) - 1, REQUEST_SLEEP_DEFAULT);

	request_sleep_time = atoi(m_request_sleep);

	ExtConfig.GetString(REQUEST_BUFFER, m_reqeust_buffer, sizeof(m_reqeust_buffer) - 1, REQUEST_BUFFER_DEFAULT);

	request_buffer = atoi(m_reqeust_buffer);

	ExtConfig.GetString(REQUEST_BUSY_MAX, m_request_busy, sizeof(m_request_busy) - 1, REQUEST_BUSY_MAX_DEFAULT);

	request_busy_size = atoi(m_request_busy);


	ExtConfig.GetString(SYMBOL_SEPARATOR, m_seperator, sizeof(m_seperator) - 1, SYMBOL_SEPARATOR_DEFAULT);

	this->symbol_seperator = m_seperator;

	ExtConfig.GetString(SYMBOL_SEPARATOR_POSITION, m_seperator_position, sizeof(m_seperator_position) - 1, SYMBOL_SEPARATORPOSITION_DEFAULT);

	ExtConfig.GetString(CROSS_TRADE_REQUEST_TIME, m_request_trade_time, sizeof(m_request_trade_time) - 1, CROSS_TRADE_REQUEST_TIME_DAFAULT);

	this->request_trade_time = atoi(m_request_trade_time);

	this->symbol_seperator_position = atoi(m_seperator_position);

	if (this->symbol_seperator_position !=0 || this->symbol_seperator_position != 1) {
		this->symbol_seperator_position = 0;
	}

	ExtConfig.GetString(TASK_REQUEST_TIME, m_request_task_time, sizeof(m_request_task_time) - 1, TASK_REQUEST_TIME_MIN_DEAFULT);


	this->request_task_time = atoi(m_request_task_time);





 	TaskManagement::getInstance()->init_symbol_group();

	this->setupSocket();

 	this->excutor.addThread(THREADPOOL_MAX_NUM);
}

 
void CProcessor::setupSocket() {
	 

	if (pool==NULL) {
		pool = new SocketConnectionPool();
	}
	pool->DestoryConnPool();
	if (pool->checkParams(m_ip, atoi(m_port), m_key) == false) {
		return;
	}

	
	pool->setParams(m_ip, atoi(m_port), m_key);
	pool->initPool(MAN_POOL_SIZE_INIT);
 
	
	 
}
void CProcessor::LOG(const int code, LPCSTR ip, LPCSTR msg, ...) const
{
	 
	char buffer[2046] = { 0 };
	//---- checks
	if (msg == NULL || ExtServer == NULL) return;
	//---- formating string
	va_list ptr;
	va_start(ptr, msg);
	vsnprintf_s(buffer, sizeof(buffer) - 1, msg, ptr);
	va_end(ptr);
	//---- output
	ExtServer->LogsOut(code, ip, buffer);

	 

}

void CProcessor::LOG(bool debug, string msg, ...) const {
	 
	char buffer[2046] = { 0 };
	va_list ptr;
	va_start(ptr, msg);
	vsnprintf_s(buffer, sizeof(buffer) - 1, msg.c_str(), ptr);
	va_end(ptr);
	if (debug == true) {
	 
 
		MyLOG::getInstance()->LOG_I(string(buffer));
 
	}
	else {
		MyLOG::getInstance()->LOG_I(string(buffer));
	}
	 

}

void CProcessor::SrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id)
{
	LOG(CmdTrade, "LifeByte::SrvTradeTransaction", "LifeByte::SrvTradeTransaction");
}

void CProcessor::SrvDealerGet(const ConManager *manager, const RequestInfo *request)
{
	LOG(CmdTrade, "LifeByte::SrvDealerGet", "LifeByte::SrvDealerGet");
}
 
//+------------------------------------------------------------------+
//| Prepare UserInfo for login                                       |
//+------------------------------------------------------------------+
int CProcessor::UserInfoGet(const int login, UserInfo *info)
{
	UserRecord user = { 0 };
	//---- checks
	if (login < 1 || info == NULL || ExtServer == NULL) return(FALSE);
	//---- clear info
	ZeroMemory(info, sizeof(UserInfo));
	//---- get user record
	if (ExtServer->ClientsUserInfo(login, &user) == FALSE) return(FALSE);
	//---- fill login
	info->login = user.login;
	//---- fill permissions
	info->enable = user.enable;
	info->enable_read_only = user.enable_read_only;
	//---- fill trade data
	info->leverage = user.leverage;
	info->agent_account = user.agent_account;
	info->credit = user.credit;
	info->balance = user.balance;
	info->prevbalance = user.prevbalance;
	//---- fill group
	COPY_STR(info->group, user.group);
	//---- ok
	return(TRUE);
}

int CProcessor::UpdateComment(const int order, const string comment) {
	TradeRecord    oldtrade = { 0 };
	ConSymbol      symcfg = { 0 };
	UserInfo       info = { 0 };
	//--- Checks
	if (order <= 0 || ExtServer == NULL) return(FALSE);
	//--- Getting order
	if (ExtServer->OrdersGet(order, &oldtrade) == FALSE)
		return(FALSE); // Error
	//--- Receiving client data
	if (UserInfoGet(oldtrade.login, &info) == FALSE)
		return(FALSE); // Error


	string newComment = oldtrade.comment + string(" ") + comment;
	COPY_STR(oldtrade.comment, newComment.c_str(), sizeof(oldtrade.comment) - 1);
	//--- Position closure
	
	if (ExtServer->OrdersUpdate(&oldtrade, &info, UPDATE_NORMAL) == FALSE)
		return(FALSE); // Error
  //--- Position closed
	return(TRUE);
}

void CProcessor::SrvTradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol)
{


	LOG(CmdTrade, "LifeByte::SrvTradesAdd", "%d,%d,%d,%s,%s,%s", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment);
	
}




void CProcessor::SrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode)
{
	if (mode != OPEN_NEW) {
		return;
	}
	if (trade->cmd !=OP_SELL && trade->cmd != OP_BUY) {
		return;
	}
 

 
	 
	MyTrade* tmp = new MyTrade();
	tmp->login = trade->login;
	tmp->cmd = trade->cmd;
	tmp->sl = trade->sl;
	tmp->tp = trade->tp;
	COPY_STR(tmp->symbol, trade->symbol);
	tmp->volume = trade->volume;
	tmp->order = trade->order;
	COPY_STR(tmp->comment, trade->comment)
	tmp->state = trade->state;
	tmp->mode = mode;
	tmp->task_id = "";
	tmp->server_id = this->plugin_id;
 //	this->HandlerAddOrder(tmp,user,symbol,mode);
	
		this->excutor.commit(open_order_worker_thread, tmp);
 

}



void CProcessor::SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{


 

		MyTrade* tmp = new MyTrade();
		tmp->login = trade->login;
		tmp->cmd = trade->cmd;
		tmp->sl = trade->sl;
		tmp->tp = trade->tp;
		COPY_STR(tmp->symbol, trade->symbol);
		tmp->volume = trade->volume;
		tmp->order = trade->order;
		tmp->price = trade->close_price;
		COPY_STR(tmp->comment, trade->comment)
		tmp->state = trade->state;
		tmp->task_id = "";
		tmp->server_id = this->plugin_id;
		ConSymbol      symcfg = { 0 };
		if (mode == UPDATE_ACTIVATE && ExtServer->SymbolsGet(tmp->symbol, &symcfg) == FALSE)
		{
			LOG(false, " HandlerActiveOrder: SymbolsGet failed");
			return; // error
		}
		switch (mode)
		{
		case UPDATE_NORMAL:

			break;
		case UPDATE_ACTIVATE:
 
			this->excutor.commit(open_order_worker_thread, tmp);
			//this->HandlerAddOrder(tmp, user, &symcfg, mode);
			break;
		case UPDATE_CLOSE:

			this->excutor.commit(close_order_worker_thread, tmp);
		//	this->HandlerCloseOrder(tmp, user, mode);
			break;
		case UPDATE_DELETE:
			break;
		default:
			break;
		}
	 



}


void CProcessor::SrvDealerReset(const int id, const UserInfo *us, const char flag)
{



 
	OrderTask* task = new OrderTask();
	task->id = id;

	this->excutor.commit(reject_order_worker_thread, task);


}

void CProcessor::SrvDealerConfirm(const int id, const UserInfo *us, double *prices)
{
 
 

	OrderTask* task = new OrderTask();
	task->id = id;
	task->prices_0 = prices[0];
	task->prices_1 = prices[1];
 
 
	this->excutor.commit(confirm_order_worker_thread, task);
	 
	
}

 

 UINT __cdecl  CProcessor::open_order_worker_thread(void* param) {
	 EnterCriticalSection(&m_cs);
	 MyTrade* trade = (MyTrade*)param;
	 TaskManagement* man = TaskManagement::getInstance();
	 list<TradeTask*> m_task;
	 man->getTaskList(m_task);
	 man->isWorking = true;
	 set<int> send_cross ;

#ifdef _DEBUG
 ExtProcessor.LOG(CmdTrade, "LifeByte::New receiver", " task size,%d", m_task.size());

#endif
	 for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {

		 TradeTask* task = *tmp;
		// if (trade->task_id.empty() == true) {
			 if (task->master_server_id != trade->server_id || atoi(task->master_id.c_str()) != trade->login) {
				 continue;
			 }
		/* }
		 else if (trade->task_id.compare(task->task_id) != 0) {
			 continue;
		 }*/

		 if (task->follower_disable == true || task->master_disable == true) {
			 continue;
		 }

		

		 int vol = round(trade->volume * task->master_ratio * task->follower_ratio);




		 if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {

			 vol = task->follower_max_vol;
		 }



		 int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		 string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		 int follower_id = atoi(task->follower_id.c_str());
		 int master_id = atoi(task->master_id.c_str());
		 int master_server_id = task->master_server_id;
		 int follower_server_id = task->follower_server_id;
		 if (task->follower_server_id == ExtProcessor.plugin_id) {

////////////////////////////////////////
			 UserInfo info = { 0 };
			 if (ExtProcessor.UserInfoGet(follower_id, &info) == FALSE) {

				 continue;
			 }
			 string trade_symbol = trade->symbol;
			 if (ExtProcessor.checkSymbolIsEnable(trade_symbol, info.group) == false) {
				 string new_symbol;
				 bool is_get = ExtProcessor.getNewSymbol(trade_symbol, info.group, new_symbol);
				 if (is_get == true) {
					 trade_symbol = new_symbol;
				 }
				 else {
					 continue;
				 }
			 }
			 bool find_symbol = false;
			 for (auto it = task->symbol_filter.begin(); it != task->symbol_filter.end(); it++) {
			
				 if (trade_symbol.compare(*it) == 0) {
					 find_symbol = true;
					 break;
				 }

			 }
			 if (find_symbol == true) {
				 continue;
			 }


			 int total = 0;
			 TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);



			 bool find_order = false;
			 for (int i = 0; i < total; i++) {
				 TradeRecord record = records[i];
				 string fo_comment(record.comment);
				 int master_order = GetOOrderNumberFromComment(fo_comment.c_str());
				 if( master_order ==(trade->order) ) {
					 find_order = true;
					 break;
				 }
			 }
			 HEAP_FREE(records);

			
			 if (find_order ==false) {
				 ExtProcessor.LOG(false, "LifeByte::New receive open order %d, state %d,login %d,   symbol %s,comment %s,mode %d", trade->order, trade->state, trade->login, trade->symbol, trade->comment, trade->mode);
				 ExtProcessor.LOG(CmdTrade, "LifeByte::New receive open order", "%d,%d,%d,%s, %s,%d", trade->order, trade->state, trade->login, trade->symbol, trade->comment, trade->mode);

				 ExtProcessor.AddOpenRequestToQueue(follower_id, trade_symbol, cmd, vol, comment, master_id, master_server_id, follower_server_id);

			 }
			 else {
				 ExtProcessor.LOG(false, "LifeByte::New receive exist order %d, state %d,login %d,   symbol %s,comment %s,mode %d", trade->order, trade->state, trade->login, trade->symbol, trade->comment, trade->mode);
				 ExtProcessor.LOG(CmdTrade, "LifeByte::New receive exist order", "%d,%d,%d,%s, %s,%d", trade->order, trade->state, trade->login, trade->symbol, trade->comment, trade->mode);

			 }
	
			 ////////////////////////////////////////
		 }
		 else {
		  // skip non-original request;
			 if (trade->task_id.empty() == false) {
				 continue;
			 }
			 if (send_cross.find(task->follower_server_id) == send_cross.end()) {

				 //handle another server
				 MyIOCP* iocp = ExtProcessor.pool->GetConnection();
				 if (iocp == NULL) {
					 continue;
				 }
				 iocp->openOrderRequest(task->follower_server_id, to_string(trade->login), trade->symbol, trade->cmd, trade->volume, trade->order, trade->mode, trade->state, trade->comment, task->task_id, trade->server_id);

				 ExtProcessor.pool->ReleaseConnection(iocp);
				 send_cross.insert(task->follower_server_id);
			 }
		 }

	 }


	 if (trade != NULL) {
		 delete trade;
		 trade = NULL;
	 }
	 man->clearTask(m_task);
	 man->isWorking = false;
	 LeaveCriticalSection(&m_cs);
	 return 0;
 }
 
 UINT __cdecl  CProcessor::close_order_worker_thread(void* param) {
	 EnterCriticalSection(&m_cs);
	 TaskManagement* man = TaskManagement::getInstance();
	 //	man->isWorking = true;
	 MyTrade* trade = (MyTrade*)param;
	 list<TradeTask*> m_task;
	 man->getTaskList(m_task);
	 set<int> send_cross;
	 for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {

		 TradeTask* task = *tmp;

		// if (trade->task_id.empty() == true) {
			 if (task->master_server_id != trade->server_id || atoi(task->master_id.c_str()) != trade->login) {
				 continue;
			 }
		/* }
		 else if (trade->task_id.compare(task->task_id) != 0) {
			 continue;
		 }*/



		 if (task->follower_disable == true || task->master_disable == true) {
			 continue;
		 }
		 
		 
		 int vol = round(trade->volume * task->master_ratio * task->follower_ratio);

		 if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {

			 vol = task->follower_max_vol;
		 }


		 int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		 if (task->follower_server_id == ExtProcessor.plugin_id) {
			 int total = 0;
			 int order = trade->order;
			 int follower_id = atoi(task->follower_id.c_str());
			 int master_id = atoi(task->master_id.c_str());
			 int master_server_id = task->master_server_id;
			 int follower_server_id = task->follower_server_id;
			 UserInfo info = { 0 };
			 if (ExtProcessor.UserInfoGet(follower_id, &info) == FALSE) {
				 continue;
			 }

			 TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);



			 bool find_order = false;
			 for (int i = 0; i < total; i++) {
				 TradeRecord record = records[i];
				 
				 string fo_comment(record.comment);
				 int master_order = GetOOrderNumberFromComment(fo_comment.c_str());
				 if (master_order == (trade->order)) {
					 
					 find_order = true;
					 if (vol > record.volume) {
						 vol = record.volume;
					 }

					 if (trade->state == TS_CLOSED_NORMAL) {
						 vol = record.volume;
					 }


					 ExtProcessor.LOG(false, "LifeByte::New receive close order %d, state %d,login %d, symbol %s,comment %s,mode %d, vol %d", trade->order, trade->state, follower_id, trade->symbol, trade->comment, trade->mode, vol);
					 ExtProcessor.LOG(CmdTrade, "LifeByte::New receive close order", "order %d, state %d,  login %d, symbol %s, comment %s, mode %d ,vol % d", trade->order, trade->state, follower_id, trade->symbol, trade->comment, trade->mode, vol);

					 string str(record.symbol);
					 bool find_symbol = false;
					 for (auto it = task->symbol_filter.begin(); it != task->symbol_filter.end(); it++) {

						 if (str.compare(*it) == 0) {
							 find_symbol = true;
							 break;
						 }

					 }
					 if (find_symbol == true) {
						 continue;
					 }

					 //	ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, record.symbol, comment, vol);
					 ExtProcessor.AddCloseRequestToQueue(follower_id, record.order, cmd, record.symbol, trade->comment, vol, master_id, master_server_id, follower_server_id);
				 }
			 }
			 //only for cross server
			 if (trade->task_id.empty() == false && find_order==false) {
				 TaskManagement::getInstance()->AddDelayClose(order);
			 }
	 

			 HEAP_FREE(records);

		 }
		 else {
			 // skip non-original request;
			 if (trade->task_id.empty() == false) {
				 continue;
			 }
			 if (send_cross.find(task->follower_server_id) == send_cross.end()) {
				 MyIOCP* iocp = ExtProcessor.pool->GetConnection();
				 if (iocp == NULL) {
					 continue;
				 }

				 iocp->closeOrderRequest(task->follower_server_id, to_string(trade->login) , trade->order, trade->volume, trade->symbol, trade->cmd, trade->mode, trade->state ,trade->comment,task->task_id, trade->server_id);
				 ExtProcessor.pool->ReleaseConnection(iocp);
				 send_cross.insert(task->follower_server_id);
			 }
		 }
	 }
	 if (trade != NULL) {
		 delete trade;
		 trade = NULL;
	 }
	 //	man->isWorking = false;
	 man->clearTask(m_task);
	 LeaveCriticalSection(&m_cs);
	 return 0;
 }
 //+------------------------------------------------------------------+
//| order_worker_thread
//|  place order to db or close order on db
//+------------------------------------------------------------------+

 UINT __cdecl  CProcessor::reject_order_worker_thread(void* param) {
	 EnterCriticalSection(&m_cs);
	 OrderTask* task = (OrderTask*)param;
	 int id = task->id;
	 std::map<int, RequestMetaData>::iterator it = ExtProcessor.requestsMadeByCode.find(id);
	 if (it != ExtProcessor.requestsMadeByCode.end()) {
		 RequestMetaData data = it->second;
		 int login = data.login;
		 
		 int order = data.order;
		 ExtProcessor.LOG(false, "LifeByte::MtSrvDealerReset: request %d,login %d, order %d  ", id, login,order);
		 ExtProcessor.LOG(CmdTrade, "LifeByte::MtSrvDealerReset", "LifeByte::SrvDealerConfirm: request %d,login %d, order %d", id, login, order);

		 ExtProcessor.requestsMadeByCode.erase(it);
	 }
	 if (task != NULL) {
		 delete task;
		 task = NULL;
	 }
	 LeaveCriticalSection(&m_cs);
	 return 0;
 }
//+------------------------------------------------------------------+
//| order_worker_thread
//|  place order to db or close order on db
//+------------------------------------------------------------------+

UINT __cdecl  CProcessor::confirm_order_worker_thread(void* param) {

 
 	EnterCriticalSection(&m_cs);
	OrderTask* task = (OrderTask*)param;
	int id = task->id;
		std::map<int, RequestMetaData>::iterator it = ExtProcessor.requestsMadeByCode.find(id);
	 	 
	 	if (it != ExtProcessor.requestsMadeByCode.end()) {
			RequestMetaData data = it->second;
			TradeTransInfo info = data.info;

			double price = 0.0;
			double price0 = task->prices_0;
			double price1 = task->prices_1;

			int login = data.login;
			int cmd = info.cmd;
			string symbol = info.symbol;
			int type = info.type;
			int volume = info.volume;
			string comment = info.comment;
			int order = data.order;
			int master_id = data.master_id;
			int master_server_id = data.master_server_id;
			int login_id = data.login_server_id;
	 		ExtProcessor.LOG(false, "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s, type %d", id, login, cmd, symbol.c_str(), type);
	 		ExtProcessor.LOG(CmdTrade,"LifeByte::SrvDealerConfirm", "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s, type %d", id,login, cmd, symbol.c_str(),type);

			if ( type == TT_ORDER_MK_OPEN) {
				price = ( cmd == OP_BUY) ? price1 : price0;
			
	  			ExtProcessor.LOG(CmdTrade, "LifeByte::confirm", "LifeByte::open order  login %d ,cmd %d,symbol %s, volume %d ",   login, cmd,symbol.c_str(),volume);
	 		ExtProcessor.LOG(false,   "LifeByte::confirm LifeByte::open order  login %d ,cmd %d,symbol %s, volume %d ", login, cmd, symbol.c_str(), volume);
				int res = ExtProcessor.OrdersOpen(login, cmd, symbol.c_str(), price,volume, comment, master_id,master_server_id,login_id);
			
				if (res!= 0) {
	 				ExtProcessor.LOG(CmdTrade, "LifeByte::confirm", "LifeByte::open order  login %d ,order %d success", login, res);
	 				ExtProcessor.LOG(false, "LifeByte::confirm LifeByte::open order  login %d ,order %d success",  login, res);
				}
				else {
	 				ExtProcessor.LOG(CmdTrade, "LifeByte::confirm", "LifeByte::open order  login %d failed", login );
	 				ExtProcessor.LOG(false, "LifeByte::confirm LifeByte::open order  login %d failed ", login );
				}
			}
			else if ( type == TT_ORDER_MK_CLOSE) {
				price = ( cmd == OP_BUY) ? price0 : price1;
			
				int res = ExtProcessor.OrdersClose( order,  volume, price, comment, master_id, master_server_id, login_id);
			
				if (res==TRUE) {
	 				ExtProcessor.LOG(CmdTrade, "LifeByte::confirm", "LifeByte::close order id %d, login %d ,order %d, volume %d  success", order,  login,  order, volume);
	 				ExtProcessor.LOG(false, "LifeByte::confirm LifeByte::close order id %d, login %d,order %d, volume %d success ",  order,  login,  order,  volume);
				}
				else {
	 				ExtProcessor.LOG(CmdTrade, "LifeByte::confirm", "LifeByte::close order id %d, login %d ,order %d, volume %d failed ",  order,  login,  order,  volume);
	 				ExtProcessor.LOG(false, "LifeByte::confirm LifeByte::close order id %d, login %d,order %d, volume %d failed ",  order,  login,  order,  volume);
				}
			}


	 
				int  res = ExtServer->RequestsDelete(id);
				 
 
		 
			ExtProcessor.requestsMadeByCode.erase(it);
	 
		}
		else {
			ExtProcessor.LOG(false, "LifeByte::SrvDealerConfirm No request");
		} 

	



		if (task!=NULL) {
			delete task;
			task = NULL;
		}
	  LeaveCriticalSection(&m_cs);
	 
	return 0;
}



void CProcessor::askLPtoCloseTrade(int login, int order, int cmd, string symbol, string comment, int volumeInCentiLots, int master_id, int master_server_id, int login_server_id) {
	//Check if cmd is OP_BUY, OP_SELL, volume is positive
	//Check symbol is not long_only and cmd is sell
	//Check symbol is tradeable (TRADE_FULL)
	//TradesCheckSecurity
	//TradesCheckTickSize
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	ConSymbol      symcfg = { 0 };
	ConGroup grpcfg = { 0 };

	//TradesCheckVolume
	//TradesCheckStops
	//TradesMarginCheck

	{

		if (UserInfoGet(login, &info) == FALSE) {
			LOG(false, "askLPtoCloseTrade: login %d get user failed", login);
			return; // error
		}


		if (info.enable == 0) {
			LOG(false, "askLPtoCloseTrade: login %d  disable", login);
			return;
		}

		if (info.enable_read_only != 0) {
			LOG(false, "askLPtoCloseTrade: login %d RO", login);
			return;
		}


		if (this->checkSymbolIsEnable(symbol, info.group) == false) {
			string new_symbol;
			bool is_get = this->getNewSymbol(symbol, info.group, new_symbol);
			if (is_get == true) {
				symbol = new_symbol;
			}
			else {
				return;
			}
		}



		if (ExtServer->GroupsGet(info.group, &grpcfg) == FALSE) {
			LOG(false, " askLPtoCloseTrade: GroupsGet failed ");
			return;
		}


	
	

		if (ExtServer->SymbolsGet(symbol.c_str(), &symcfg) == FALSE) {
			LOG(false, " askLPtoCloseTrade: SymbolsGet failed");
			return;
		}

		int security = symcfg.type;
		int isTrade = grpcfg.secgroups[security].trade;
		int isShow = grpcfg.secgroups[security].show;
		if (isTrade==0 || isShow ==0) {
			LOG(false, "askLPtoCloseTrade: symbol disable");

			string new_symbol = "";
			bool res = this->getNewSymbol(symbol, info.group,new_symbol);
				 
			if (res == true) {
				if (ExtServer->SymbolsGet(new_symbol.c_str(), &symcfg) == FALSE) {
					LOG(false, " askLPtoCloseTrade: SymbolsGet failed");
					return;
				}
			}
			else {
				return;
			}


			return;
		}

		int lot_max = grpcfg.secgroups[security].lot_max;
		int lot_min = grpcfg.secgroups[security].lot_min;
		int lot_step = grpcfg.secgroups[security].lot_min;
		if (lot_step!=0 && volumeInCentiLots % lot_step != 0) {
			volumeInCentiLots = volumeInCentiLots / lot_step * lot_step;
		}

		if (volumeInCentiLots > lot_max) {
			volumeInCentiLots = lot_max;
		}
		if (volumeInCentiLots < lot_min) {
			volumeInCentiLots = lot_min;
		}

		trans.type = TT_ORDER_MK_CLOSE;
		trans.flags = TT_FLAG_API;
		trans.cmd = cmd;
		trans.volume = volumeInCentiLots;
		//trans.tp = tp;
		//trans.sl = sl;

		strncpy(trans.symbol, symbol.c_str(), sizeof(trans.symbol) - 1);
		strncpy(trans.comment, comment.c_str(), sizeof(trans.comment) - 1);
		trans.price = 0.0;

		trans.order = order;


		RequestInfo reqInfo = { 0 };
		reqInfo.status = DC_EMPTY;

		reqInfo.login = login;
		strncpy(reqInfo.group, info.group, sizeof(reqInfo.group));
		reqInfo.balance = info.balance;
		reqInfo.credit = info.credit;
		reqInfo.trade = trans;

			double prices[2];
			ExtServer->HistoryPricesGroup(&reqInfo, prices);

			trans.price = (cmd == OP_SELL) ? prices[1] : prices[0];
			reqInfo.trade.price = trans.price;
		int reqId;
		m_ContextLock.Lock();


		int mode = ExtProcessor.checkSymbolExecuteMode(symbol, info.group);
		if (mode == EXECUTION_MANUAL) {
			const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);


			if (res == RET_TRADE_ACCEPTED) {
				RequestMetaData data;
				data.info = trans;
				data.login = login;
				data.order = order;
				data.master_id = master_id;
				data.login_server_id = login_server_id;
				data.master_server_id = master_server_id;
				requestsMadeByCode[reqId] = data;
				//	this->processing_login.insert(login);
			}
			LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoCloseTrade login %d  request id %d, res %d ,order %d", login, reqId, res, order);
			LOG(false, "LifeByte::ask LifeByte::askLPtoCloseTrade login %d  request id %d, res %d ,order %d", login, reqId, res, order);
		}
		else if (mode == EXECUTION_AUTO) {
			int res = ExtProcessor.OrdersClose(trans.order, trans.volume, trans.price, comment,master_id,master_server_id,login_server_id);

			if (res == TRUE) {
				ExtProcessor.LOG(CmdTrade, "LifeByte::ask", "LifeByte::close order id %d, login %d ,order %d, volume %d  success", order, login, order, trans.volume);
				ExtProcessor.LOG(false, "LifeByte::ask LifeByte::close order id %d, login %d,order %d, volume %d success ", order, login, order, trans.volume);
			}
			else {
				ExtProcessor.LOG(CmdTrade, "LifeByte::ask", "LifeByte::close order id %d, login %d ,order %d, volume %d failed ", order, login, order, trans.volume);
				ExtProcessor.LOG(false, "LifeByte::ask LifeByte::close order id %d, login %d,order %d, volume %d failed ", order, login, order, trans.volume);
			}
		}

		



		m_ContextLock.UnLock();
	}
}

void CProcessor::askLPtoOpenTrade(int login,   std::string symbol, int cmd, int volumeInCentiLots, const std::string& comment,
	double tp, double sl, int master_id, int master_server_id, int login_server_id) {
	//Check if cmd is OP_BUY, OP_SELL, volume is positive
	//Check symbol is not long_only and cmd is sell
	//Check symbol is tradeable (TRADE_FULL)
	//TradesCheckSecurity
	//TradesCheckTickSize
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	ConSymbol      symcfg = { 0 };
	ConGroup grpcfg = {0};


	//TradesCheckVolume
	//TradesCheckStops
	//TradesMarginCheck

	{

		if (UserInfoGet(login, &info) == FALSE) {
			LOG(false, "askLPtoOpenTrade: login %d get user failed", login);
			return; // error
		}

		if (info.enable == 0) {
			LOG(false, "askLPtoOpenTrade: login %d disable", login);
			return;
		}

		if (info.enable_read_only != 0){
			LOG(false, "askLPtoOpenTrade: login %d RO", login);
			return;
        }


		if (this->checkSymbolIsEnable(symbol, info.group) == false) {
			string new_symbol;
			bool is_get = this->getNewSymbol(symbol, info.group, new_symbol);
			if (is_get == true) {
				symbol = new_symbol;
			}
			else {
				return;
			}
		}



	 
		if (ExtServer->SymbolsGet(symbol.c_str(), &symcfg) == FALSE) {
			LOG(false, " askLPtoOpenTrade: SymbolsGet failed");
			return;
		}
		if (ExtServer->GroupsGet(info.group, &grpcfg) == FALSE) {
			LOG(false, " askLPtoOpenTrade: GroupsGet failed");
			return;
		}
	
		int security = symcfg.type;
		int isTrade = grpcfg.secgroups[security].trade;
		int isShow = grpcfg.secgroups[security].show;
		if (isTrade == 0 || isShow == 0) {
			LOG(false, "askLPtoOpenTrade: symbol  disable");
			return;
		}



		int lot_max = grpcfg.secgroups[security].lot_max;
		int lot_min = grpcfg.secgroups[security].lot_min;
		int lot_step = grpcfg.secgroups[security].lot_min;

		if (lot_step!=0 && volumeInCentiLots % lot_step !=0) {
			volumeInCentiLots = volumeInCentiLots / lot_step * lot_step;
		}

		if (volumeInCentiLots >lot_max) {
			volumeInCentiLots = lot_max;
		}
		if (volumeInCentiLots < lot_min) {
			volumeInCentiLots = lot_min;
		}


		trans.type = TT_ORDER_MK_OPEN;
		trans.flags = TT_FLAG_API;
		trans.cmd = cmd;
		trans.volume = volumeInCentiLots;
		trans.tp = tp;
		trans.sl = sl;
		strncpy(trans.symbol, symbol.c_str(), sizeof(trans.symbol) - 1);
		strncpy(trans.comment, comment.c_str(), sizeof(trans.comment) - 1);
		trans.price = 0.0;
		trans.orderby = login;


		RequestInfo reqInfo = { 0 };
		reqInfo.status = DC_EMPTY;
	 
		reqInfo.login = login;
		strncpy(reqInfo.group, info.group, sizeof(reqInfo.group));
		reqInfo.balance = info.balance;
		reqInfo.credit = info.credit;
		reqInfo.trade = trans;

		double prices[2];
		ExtServer->HistoryPricesGroup(&reqInfo, prices);

		trans.price = (cmd == OP_BUY) ? prices[1] : prices[0];
		reqInfo.trade.price = trans.price;


		double margin, freemargin, equity;

		if (ExtServer->TradesMarginInfo(&info, &margin, &freemargin, &equity) == TRUE) {
			double profit, newmargin;
			double r = ExtServer->TradesMarginCheck(&info, &trans, &profit, &freemargin, &newmargin);
			LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoOpenTrade login %d  margin request margin %1f ,free margin  %1f, new margin  %1f", login, r, freemargin, newmargin);
			LOG(false, "LifeByte::ask LifeByte::askLPtoOpenTrade login %d   margin request id %1f  ,free margin  %1f, new margin  %1f ", login,r , freemargin, newmargin);

			if (  freemargin<0) {
				LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoOpenTrade login %d  ", login);
				LOG(false, "askLPtoOpenTrade: not enough margin failed");
				return;
			}
		}
		else {
			LOG(CmdTrade, "LifeByte::askLPtoOpenTrade", "get margin failed");
			LOG(false, "askLPtoOpenTrade: get margin failed");
			return;
		}




		int reqId;
		m_ContextLock.Lock();


		int mode = ExtProcessor.checkSymbolExecuteMode(symbol, info.group);

		// 	askLPtoOpenTrade(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl);
		if (mode == EXECUTION_MANUAL) {
			const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);

			if (res == RET_TRADE_ACCEPTED) {
				RequestMetaData data;
				data.info = trans;
				data.login = login;
				data.master_id = master_id;
				data.login_server_id = login_server_id;
				data.master_server_id = master_server_id;
				requestsMadeByCode[reqId] = data;
				//	this->processing_login.insert(login);

			}
			LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoOpenTrade login %d request id %d  res %d", login, reqId, res);
			LOG(false, "LifeByte::ask LifeByte::askLPtoOpenTrade login %d request id %d  res %d", login, reqId, res);
		}
		else if (mode == EXECUTION_AUTO) {

			int res = ExtProcessor.OrdersOpen(trans.orderby, trans.cmd, trans.symbol, trans.price, trans.volume, comment,master_id,master_server_id,login_server_id);

			if (res != 0) {
				ExtProcessor.LOG(CmdTrade, "LifeByte::ask", "LifeByte::open order  login %d ,order %d success", login, res);
				ExtProcessor.LOG(false, "LifeByte::ask LifeByte::open order  login %d ,order %d success", login, res);
			}
			else {
				ExtProcessor.LOG(CmdTrade, "LifeByte::ask", "LifeByte::open order  login %d failed", login);
				ExtProcessor.LOG(false, "LifeByte::ask LifeByte::open order  login %d failed ", login);
			}
		
		}






	
	
		m_ContextLock.UnLock();
	}
}
 
//+------------------------------------------------------------------+
//| Opening a BUY or SELL position using OrdersOpen             |
//+------------------------------------------------------------------+
int CProcessor::OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
	const double open_price, const int volume, string comment,int master_id,int master_server_id,int login_id)
{
	ConGroup       grpcfg = { 0 };
	time_t         currtime;
	ConSymbol      symcfg = { 0 };
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            order = 0;
	//--- Checks
	if (login <= 0 || cmd<OP_BUY || cmd>OP_SELL || symbol == NULL || open_price <= 0 || volume <= 0 || ExtServer == NULL) {
		LOG(false, " OpenAnOrder: checking failed");
		return(0);
	}
		

	//--- get symbol config
	if (ExtServer->SymbolsGet(symbol, &symcfg) == FALSE)
	{
		LOG(false, " OpenAnOrder: SymbolsGet failed");
		return(FALSE); // error
	}
	//---- get the current server time  
	currtime = ExtServer->TradeTime();
	if (ExtServer->TradesCheckSessions(&symcfg, currtime) == FALSE) {
		LOG(false, " OpenAnOrder: invalid time");
		return false;
	}

	//--- Receiving client data
	if (UserInfoGet(login, &info) == FALSE) {
		LOG(false, " OpebnAnOrder: UserInfoGet failed");
		return(FALSE); // error
	}

	//--- get group config
	if (ExtServer->GroupsGet(info.group, &grpcfg) == FALSE) {
		LOG(false, " OpenAnOrder: GroupsGet failed");
		return(FALSE); // Error
	}

	//--- Preparing transaction
	trans.cmd = cmd;
	trans.volume = volume;
	trans.price = open_price;

	COPY_STR(trans.symbol, symbol);
	//--- Adding SL, TP, comment
	COPY_STR(trans.comment, comment.c_str());
	//--- Check of long-only permission
	if (cmd == OP_BUY && symcfg.long_only != 0) {
		LOG(false, " OpenAnOrder: long-only");
		return(FALSE); // Error
	}
	//--- Check of close-only permission
	if (symcfg.trade != TRADE_FULL) {
		LOG(false, " OpenAnOrder: Not TRADE_FULL");
		return(FALSE); // Error
	}
	//--- Check of tick size
 
	if (ExtServer->TradesCheckTickSize(open_price, &symcfg) == FALSE)
	{
		LOG(false, " OpenbAnOrder: invalid price");
		return(FALSE); // invalid price
	}

#ifndef _DEBUG



	//--- Symbol check
		//--- check secutiry
	if (ExtServer->TradesCheckSecurity(&symcfg, &grpcfg) != RET_OK)
	{
		LOG(false, " CloseAnOrder: trade disabled or market closed");
		return(FALSE); // trade disabled, market closed, or no prices for long time
	}
#endif // DEBUG
	//--- Volume check
		//--- check volume
	if (ExtServer->TradesCheckVolume(&trans, &symcfg, &grpcfg, TRUE) != RET_OK)
	{
		LOG(false, "OpenAnOrder: invalid volume");
		return(FALSE); // invalid volume
	}
	//--- Check of stop levels// for pendig order
 

	//--- Opening an order and checking margin
 

	if ((order = ExtServer->OrdersOpen(&trans, &info)) == 0) {
		LOG(CmdTrade, "LifeByte::OrdersOpen", " login %d open order failed", login);
		LOG(false, "LifeByte::OrdersOpen login %d open order failed", login);
		return(0); // Error
	}
	LOG(CmdTrade, "LifeByte::OrdersOpen"," open order %d, login %d", order, login);
	LOG(false, "LifeByte::OrdersOpen open order %d, login %d", order, login);
	 



	int masterOld = GetOOrderNumberFromComment(comment.c_str());
    if(masterOld!=0){
	//	LOG(CmdTrade, "LifeByte::OrdersOpen", "add quick close order %d, login %d", masterOld, login);
	//	LOG(false, "LifeByte::OrdersOpen add quick close order %d, login %d", order, login);
 


		if(TaskManagement::getInstance()->CheckDelayClose(masterOld)==true) {
			this->AddCloseRequestToQueue(login, order, cmd, symbol, comment, volume, master_id, master_server_id, login_id);
			return(order);
		}




				//--- get order
		TradeRecord master_trade = { 0 };
		if (ExtServer->OrdersGet(masterOld, &master_trade) == FALSE) {
			 
			return(order); // Error
		}


		if (master_trade.login == master_id &&master_trade.close_time!=0) {
			this->AddCloseRequestToQueue(login,order,cmd,symbol,comment,volume,master_id,master_server_id,login_id);
		}


	}

				   //--- Position is open: return order
	return(order);
}



 
//+------------------------------------------------------------------+
//| Closing a BUY or SELL position using OrdersClose                 |
//+------------------------------------------------------------------+
int CProcessor::OrdersClose(const int order,  const int volume, const double close_price, const string comment,   int master_id, int master_server_id, int login_id) {
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            login;


	time_t         currtime;
	TradeRecord    old_trade = { 0 };
 
	ConGroup       grpcfg = { 0 };
	ConSymbol      symcfg = { 0 };
 

	 
	//--- Checks
	if (order <= 0 || volume <= 0 || close_price <= 0 || ExtServer == NULL) {
		LOG(false, "CloseAnOrder: checking failed failed");
		return(FALSE);
	}
	 
	//--- get order
	if (ExtServer->OrdersGet(order, &old_trade) == FALSE) {
		LOG(false, "CloseAnOrder: OrdersGet failed");
		return(FALSE); // Error
	}
	 
	//--- Getting login from the order
	//if ((login = ExtServer->TradesFindLogin(order)) == 0) {
	//
	//	return(FALSE); // Error
	//}
	login = old_trade.login;
  //--- Getting client data
	if (UserInfoGet(login, &info) == FALSE) {
		LOG(false, " CloseAnOrder: UserInfoGet failed");
		return(FALSE); // Error
	}
	 

		//--- get group config
	if (ExtServer->GroupsGet(info.group, &grpcfg) == FALSE) {
		LOG(false, " CloseAnOrder: GroupsGet failed");
		return(FALSE); // Error
	}
	 
   //--- get symbol config
	if (ExtServer->SymbolsGet(old_trade.symbol, &symcfg) == FALSE)
	{	 
		LOG(false, " CloseAnOrder: SymbolsGet failed ");
		return(FALSE); // error
	}

  //--- Preparing transaction
	COPY_STR(trans.comment, comment.c_str());
	trans.order = order;
	trans.volume = volume;
	trans.price = close_price;
	 
 
	//--- Instrument check   no

		//---- get the current server time  
	currtime = ExtServer->TradeTime();
	if (ExtServer->TradesCheckSessions(&symcfg, currtime) == FALSE) {
		LOG(false, " CloseAnOrder: invalid time");
		return false;
	}
 //--- check tick size
	if (ExtServer->TradesCheckTickSize(close_price, &symcfg) == FALSE)
	{
		LOG(false, " CloseAnOrder: invalid price");
		return(FALSE); // invalid price
	}
#ifndef _DEBUG



	//--- check secutiry
	if (ExtServer->TradesCheckSecurity(&symcfg, &grpcfg) != RET_OK)
	{
		LOG(false, " CloseAnOrder: trade disabled or market closed");
		return(FALSE); // trade disabled, market closed, or no prices for long time
	}
#endif // !_DEBUG
	//--- check volume
  if(ExtServer->TradesCheckVolume(&trans,&symcfg,&grpcfg,TRUE)!=RET_OK)
    {
	  LOG(false, "CloseAnOrder: invalid volume");
     //return(FALSE); // invalid volume
    }
	//--- Freeze level check  
	if (ExtServer->TradesCheckFreezed(&symcfg, &grpcfg, &old_trade) != RET_OK)
	{
		LOG(false, " CloseAnOrder: position freezed");
		return(FALSE); // position freezed
	}
	//--- check stops 
	if (ExtServer->TradesCheckStops(&trans, &symcfg, &grpcfg, &old_trade) != RET_OK)
	{
		LOG(false, " CloseAnOrder: stop level");
		return(FALSE); // position freezed
	}
	 

	//--- Closing position
	if (ExtServer->OrdersClose(&trans, &info) == FALSE) {

		LOG(false, " CloseAnOrder:close order failed %d",order);
		return(FALSE); // Error
	}
 
  //--- Position closed


	//-- update new order comment
	if (volume< old_trade.volume) {
		//--- get order
		//TradeRecord follower_old_trade = {0};
		//if (ExtServer->OrdersGet(order, &follower_old_trade) == FALSE) {
		//	LOG(false, "CloseAnOrder: OrdersGet failed");
		//	return(FALSE); // Error
		//}
		//int followerNewOrder = GetToOrderComment(follower_old_trade.comment);
		// 
		//int masterOld = GetOOrderNumberFromComment(comment.c_str());

		//TradeRecord master_old_trade = { 0 };
		////--- get order
		//if (ExtServer->OrdersGet(masterOld, &master_old_trade) == FALSE) {
		//	LOG(false, "LifeByte::OrdersClose order %d, login %d  close failed ", order, login);
		//	LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d close failed", order, login);
		//	return(FALSE); // Error
		//}

		//LOG(false, "LifeByte::OrdersClose order %d, login %d", order, login);
		//LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d", order, login);


		//int masterNewOrder = GetToOrderComment(master_old_trade.comment);



		//string newComment = ORDER_COMMENT_PRE + to_string(masterNewOrder);
		//if (this->UpdateComment(followerNewOrder, newComment) == FALSE) {
		//	LOG(false, "CloseAnOrder: update failed");
		//	LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d update comment failed", order, login);
		//	return(FALSE); // Error
		//}
				//--- get order
		TradeRecord follower_old_trade = {0};
		if (ExtServer->OrdersGet(order, &follower_old_trade) == FALSE) {
			LOG(false, "CloseAnOrder: OrdersGet failed");
			return(FALSE); // Error
		}
		 int masterNewOrder = GetToOrderComment(trans.comment);
		int followerNewOrder = GetToOrderComment(follower_old_trade.comment);

 
		 string newComment = ORDER_COMMENT_PRE + to_string(masterNewOrder);
		if (this->UpdateComment(followerNewOrder, newComment) == FALSE) {
			LOG(false, "CloseAnOrder: update failed");
			LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d update comment failed", order, login);
			return(FALSE); // Error
		}
	}



 

	return(TRUE);
}





//+------------------------------------------------------------------+
//| Thread wrapper                                                   |
//+------------------------------------------------------------------+
UINT __stdcall CProcessor::ThreadWrapperRequest(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->ThreadProcesRequest();
	//---
	return(0);
}


//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CProcessor::ThreadProcesRequest(void) {
	while (true) {
		 
		UINT id = 0;

		if (m_threadServer == NULL) {
			m_threadServer = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
		}
	

		DWORD dwRet = WaitForSingleObject(m_threadServer, 1000 * 60);
		LOG(CmdTrade, "LifeByte::synchronize guard checking", "LifeByte::synchronize guard checking");
		if (dwRet == WAIT_OBJECT_0) {
			CloseHandle(m_threadServer);
			m_threadServer = NULL;
		}
	}
}
//+------------------------------------------------------------------+
//| Thread wrapper                                                   |
//+------------------------------------------------------------------+
UINT __stdcall CProcessor::ThreadWrapper(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->ThreadProcess();
	//---

	//ExtProcessor.LOG(CmdTrade, "LifeByte::synchronize task thread exit ", " LifeByte::synchronize task thread exit");
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
static volatile int task_check_cycle = 0;
void CProcessor::ThreadProcess(void)
{


	 
	//while (true) 
	{
		
	//	Sleep(5000);
		if (this->pool == NULL) {
			return;
		 //	continue;
		}
	
		bool res = this->pool->checkConnection();
 
		if (res==false) {
			return;
		 //	continue;
		}
//
		MyIOCP* iocp = this->pool->GetConnection();
		if (iocp == NULL ) {
			this->pool->ReleaseConnection(iocp);
			//continue;
			return;
		}
 
		if (iocp->level != ADM_LEVEL) {
			this->pool->ReleaseConnection(iocp);
			 
			return;
		}
		TaskManagement* man = TaskManagement::getInstance();
		if (  man->initialTask == INITIAL_NO) {
			man->initial_count = 0;
			iocp->SendInitTask(this->plugin_id);
		}
		
	   
	

		if (man->initialTask == INITIAL_FINISH) {
			man->initial_count = 0;
			task_check_cycle++;
			//if (task_check_cycle > 2)
			{
				task_check_cycle = 0;
				//10 sec
				iocp->SendInitTask(this->plugin_id);


				man->PoPDelayClose();

				LOG(CmdTrade,"LifeByte::synchronize task ", "LifeByte::synchronize task size %d", man->getTaskSize());
			}
	 

		}


		man->initial_count++;

		if (man->initial_count>5) {
			man->initial_count = 0;
			man->initialTask = INITIAL_NO;
			LOG(CmdTrade, "LifeByte::synchronize task ", "LifeByte::synchronize task timeout");
		}


		this->pool->ReleaseConnection(iocp);
	}
 
}


 

//void CProcessor::HandlerAddOrder(MyTrade*trade, const UserInfo *user, const ConSymbol *symbol, const int mode) {
//
//	m_ContextLock.Lock();
//	TaskManagement* man = TaskManagement::getInstance();
//	list<TradeTask*> m_task;
//	man->getTaskList(m_task);
//	man->isWorking = true;
//	for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {
//		 
//		TradeTask* task = *tmp ;
//	
//		if (task->follower_disable == true || task->master_disable == true) {
//			continue;
//		}
//
//		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
//			continue;
//		}
//
//		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
//
//	
//
//
//		if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {
//
//			vol = task->follower_max_vol;
//		}
//	
//
//
//		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
//		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
//		int follower_id = atoi(task->follower_id.c_str());
//		if (task->follower_server_id == this->plugin_id) {
//
//		 
//			UserInfo info = { 0 };
//			if (UserInfoGet(follower_id, &info) == FALSE) {
//
//				continue;
//			}
//			string trade_symbol = trade->symbol;
//				if (this->checkSymbolIsEnable(trade_symbol, info.group) == false) {
//					string new_symbol;
//					bool is_get = this->getNewSymbol(trade_symbol, info.group, new_symbol);
//					if (is_get == true) {
//						trade_symbol = new_symbol;
//					}
//					else {
//						continue;
//					}
//				}
//
//				for (auto it = task->symbol_filter.begin(); it != task->symbol_filter.end(); it++) {
//				 
//					if (trade_symbol.compare(*it) == 0) {
//						continue;
//					}
//				}
//
//
//
//
//		 	LOG(false, "LifeByte::New receive open order %d, state %d,login %d, name %s, symbol %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, trade->symbol, trade->comment, mode);
//		 	LOG(CmdTrade, "LifeByte::New receive open order", "%d,%d,%d,%s,%s,%s,%d", trade->order, trade->state, user->login, user->name, trade->symbol, trade->comment, mode);
//
//			 
//		// 	askLPtoOpenTrade(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl);
//			AddOpenRequestToQueue(follower_id, trade_symbol, cmd, vol, comment,atoi(task->master_id.c_str()),task->master_server_id, task->follower_server_id);
//		}
//		else {
//			//handle another server
//		 
//			MyIOCP* iocp = this->pool->GetConnection();
//			if (iocp == NULL) {
//				continue;
//			}
//			iocp->openOrderRequest(task->follower_server_id, task->follower_id, trade->symbol, trade->cmd, trade->volume, trade->order, trade->mode, trade->state, trade->comment, task->task_id);
//
//			this->pool->ReleaseConnection(iocp);
//		}
//		
//	}
//
//    
//	if (trade!=NULL) {
//		delete trade;
//		trade = NULL;
//	}
//	man->clearTask(m_task);
//	man->isWorking = false;
//	m_ContextLock.UnLock();
//}
// 
//
//void CProcessor::HandlerCloseOrder(MyTrade *trade, UserInfo *user, const int mode) {
// 	m_ContextLock.Lock();
// 
//
//	TaskManagement* man = TaskManagement::getInstance();
////	man->isWorking = true;
//	list<TradeTask*> m_task;
//	man->getTaskList(m_task);
//	for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {
//	 
//		TradeTask* task = *tmp;
//		if (task->follower_disable == true || task->master_disable == true) {
//			continue;
//		}
//
//		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
//			continue;
//		}
//		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
//
//		if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {
//
//			vol = task->follower_max_vol;
//		}
//
//
//		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
//		if (task->follower_server_id == this->plugin_id) {
//			int total = 0;
//			int order = trade->order;
//			int follower_id = atoi(task->follower_id.c_str());
//			UserInfo info = { 0 };
//			if (UserInfoGet(follower_id, &info) == FALSE) {
//				continue;
//			}
// 
//			TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);
//			 
//
//
//			bool find_order = false;
//			for (int i = 0; i < total; i++) {
//				TradeRecord record = records[i];
//				string comment(record.comment);
//				if ((comment).find(to_string(order)) != std::string::npos) {
//					find_order = true;
//					if (vol > record.volume) {
//						vol = record.volume;
//					}
//
//					if (trade->state == TS_CLOSED_NORMAL) {
//						vol = record.volume;
//					}
//
//
//		 		LOG(false, "LifeByte::New receive close order %d, state %d,login %d, symbol %s,comment %s,mode %d, vol %d", trade->order, trade->state, follower_id,  trade->symbol, trade->comment, mode ,vol);
//                LOG(CmdTrade, "LifeByte::New receive close order", "order %d, state %d,  login %d, symbol %s, comment %s, mode %d ,vol % d", trade->order, trade->state, follower_id, trade->symbol, trade->comment, mode ,vol);
//
//					string str(record.symbol);
//					for (auto it = task->symbol_filter.begin(); it != task->symbol_filter.end(); it++) {
//
//						if (str.compare(*it) == 0) {
//							continue;
//						}
//					}
//
//				//	ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, record.symbol, comment, vol);
//					AddCloseRequestToQueue(follower_id, record.order, cmd, record.symbol, comment, vol, atoi(task->master_id.c_str()), task->master_server_id, task->follower_server_id);
//				}
//			}
//			 
//			
//			// handle master open and close order at the same time issue
//		/*	if (find_order == false) {
//		 		ExtProcessor.AddToQuickCloseQueue(follower_id, trade);
//			}*/
//
//
//			HEAP_FREE(records);
//			
//		}
//		else {
//			MyIOCP* iocp = this->pool->GetConnection();
//			if (iocp == NULL) {
//				continue;
//			}
//		 
//			iocp->closeOrderRequest(task->follower_server_id, task->follower_id, trade->order, trade->volume, trade->symbol, trade->cmd, trade->mode, trade->state, trade->comment, task->task_id);
//			this->pool->ReleaseConnection(iocp);
//		}
//	}
//	if (trade != NULL) {
//		delete trade;
//		trade = NULL;
//	}
////	man->isWorking = false;
//	man->clearTask(m_task);
// 	m_ContextLock.UnLock();
//}

 


 
 
 

 
bool CProcessor::AddOpenRequestToQueue(int login, string symbol, int cmd, int vol, string comment,int master_id,int master_server_id,int login_server_id) {
	m_ContextLock.Lock();
	RequestTask* task = new RequestTask();

	task->login = login;
	task->symbol = symbol;
	task->cmd = cmd;
	task->volumeInCentiLots = vol;
	task->comment = comment;
	task->type = TS_OPEN_NORMAL;


	task->master_server_id = master_server_id;
	task->master_id = master_id;
	task->login_server_id = login_server_id;

	this->request_task.push_back(task);
	m_ContextLock.UnLock();
	return true;
}
bool CProcessor::AddCloseRequestToQueue(int login, int order, int cmd, string symbol, string comment, int vol,int master_id,int master_server_id,int login_server_id) {
	m_ContextLock.Lock();
	RequestTask* task = new RequestTask();
	task->order = order;
	task->login = login;
	task->symbol = symbol;
	task->cmd = cmd;
	task->volumeInCentiLots = vol;
	task->comment = comment;
	task->type = TS_CLOSED_NORMAL;
	task->master_server_id = master_server_id;
	task->master_id = master_id;
	task->login_server_id = login_server_id;
	this->request_task.push_back(task);
	m_ContextLock.UnLock();
	return true;
}

//+------------------------------------------------------------------+
//| Thread wrapper                                                   |
//+------------------------------------------------------------------+
UINT __stdcall CProcessor::ThreadWrapperHanderRequest(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->ThreadProcesHandlerRequest();
	//---
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CProcessor::ThreadProcesHandlerRequest(void) {
	while (true) {

		UINT id = 0;

		if (m_threadHandlerRequest == NULL) {
			m_threadHandlerRequest = (HANDLE)_beginthreadex(NULL, 256000, HandeleThreadWrapper, (void*)this, 0, &id);
		}


		DWORD dwRet = WaitForSingleObject(m_threadHandlerRequest, 1000 * 60);
		LOG(CmdTrade, "LifeByte::request thread guard checking", "LifeByte::request thread guard checking");
		if (dwRet == WAIT_OBJECT_0) {
			CloseHandle(m_threadHandlerRequest);
			m_threadHandlerRequest = NULL;
		}
	}
}

//+------------------------------------------------------------------+
//| Thread wrapper                                                   |
//+------------------------------------------------------------------+
UINT __stdcall CProcessor::HandeleThreadWrapper(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->RequestProcess();
	//---

 //	ExtProcessor.LOG(CmdTrade, "LifeByte::request thread exit ", " LifeByte::request thread exit");
	return(0);
}


void CProcessor::RequestProcess(void)
{



	//while (true)
	//{
	//	int sleep_time = this->request_sleep_time;

	//	if (sleep_time<2 || sleep_time >20) {
	//		sleep_time = REQUEST_TASK_WAIT;
	//	}

	////	Sleep(sleep_time);
 //
	//request_check_cycle++;
	//if (request_check_cycle> sleep_time) {
	//	request_check_cycle = 0;
	//
	//}
		
	ExtProcessor.CheckRequest();

}

void  CProcessor::CheckRequest() {


 
	if (this->CheckBusyQueue() ==true) {
		return;
	}


	
	int i = this->request_buffer;

	if (i<50 || i>256) {
		i =   REQUEST_TASK_PRE;
	}
	while (i>0) {
		if (this->request_task.empty() == true) {
		 

			break;
		}
		i--;
		RequestTask* task = this->getRequestTask();
		 
		if (task == NULL) {
			continue;
		}

		int type = task->type;
		if (task->type == TS_OPEN_NORMAL) {
		 
			this->askLPtoOpenTrade(task->login, task->symbol, task->cmd, task->volumeInCentiLots, task->comment, task->tp, task->sl, task->master_id, task->master_server_id, task->login_server_id);
		 

		}
		else if (task->type == TS_CLOSED_NORMAL) {
			 
			this->askLPtoCloseTrade(task->login, task->order, task->cmd, task->symbol, task->comment, task->volumeInCentiLots, task->master_id, task->master_server_id, task->login_server_id);
			 
			 

		}

		if (task != NULL) {
			delete task;
			task = NULL;
		}

		
	}

	
}

RequestTask* CProcessor::getRequestTask() {
	m_ContextLock.Lock();
	
 
	RequestTask* task = NULL;


	while (this->request_task.size()>0) {
		 task = this->request_task.front();
		this->request_task.pop_front();
		if (task == NULL) {
			continue;
		}
		bool isInQueue = this->CheckInQueue(task->login);
		if (isInQueue == false) {
			break;
		  
		}
		this->request_task.push_back(task);
	}



	
	m_ContextLock.UnLock();
	
	

	return task;;
}



bool CProcessor::CheckInQueue(int login) {
	int key = 0;
	RequestInfo reqs[256];
	int max = 256;
	int total = ExtServer->RequestsGet(&key, reqs, max);
	int cnt = 0;
 
	for (int i = 0; i < total;i++) {

		RequestInfo req = reqs[i];
		if (req.login==login && req.status == DC_REQUEST) {
			cnt++;
		 
		}
		 
		if (cnt>=3) {
			return true;
		}
	 
	}

	return false;

}

bool CProcessor::CheckBusyQueue() {
	int key = 0;
	RequestInfo reqs[128];
	int max = 128;
	int total = ExtServer->RequestsGet(&key, reqs, max);
 
	int requesting_cnt = 0;

	int busy_time = this->request_busy_size;
	if (busy_time<10 || busy_time>200) {
		busy_time = REQUEST_TASK_BUSY;
	}


	for (int i = 0; i < total; i++) {

		RequestInfo req = reqs[i];
		 
		if (req.status == DC_REQUEST) {

			requesting_cnt++;
		}
		 
		if (requesting_cnt >= busy_time) {
			ExtProcessor.LOG(CmdTrade, "LifeByte::request size ", " LifeByte::request size %d", total);
			return true;
		}
	}

	return false;

}

bool CProcessor::getNewSymbol(string symbol, string group, string& new_symbol) {
	string seperator = this->symbol_seperator;
	int fix = this->symbol_seperator_position;
	string src_symbol;
	if (symbol.find(seperator) != std::string::npos) {
		try
		{
			size_t pos = symbol.find(seperator);
			if (fix == atoi(SYMBOL_SEPARATOR_POSITION_PRE)) {
				src_symbol = symbol.substr(0, pos);
			}
			else {
				src_symbol = symbol.substr(pos, symbol.size() - 1);
			}


		}
		catch (const std::exception&)
		{

			return "";
		}
	}
	else {
		src_symbol = symbol;
	}

	TaskManagement * task_man = TaskManagement::getInstance();

	set<string> symbols = task_man->group_symbol[group];
	for (auto it2 = symbols.begin(); it2 != symbols.end(); it2++) {
		string tSym = *it2;
		string dst_symbol;



		if (tSym.find(seperator) != std::string::npos) {
			try
			{
				size_t pos = tSym.find(seperator);

				if (fix == atoi(SYMBOL_SEPARATOR_POSITION_PRE)) {
					dst_symbol = tSym.substr(0, pos);
				}
				else {
					dst_symbol = tSym.substr(pos, tSym.size() - 1);
				}
			}
			catch (const std::exception&)
			{

				return  false;
			}
		}
		else {
			dst_symbol = tSym;
		}

		if (dst_symbol.compare(src_symbol) == 0) {
			new_symbol = tSym;
			return true;
		}
	}

	return  false;
}


bool  CProcessor::checkSymbolIsEnable(string symbol, string group) {
	ConSymbol      symcfg = { 0 };
	ConGroup grpcfg = { 0 };
	if (ExtServer->SymbolsGet(symbol.c_str(), &symcfg) == FALSE) {
		LOG(false, " checkSymbolIsEnable: SymbolsGet failed");
		return false;
	}
	if (ExtServer->GroupsGet( group.c_str(), &grpcfg) == FALSE) {
		LOG(false, " checkSymbolIsEnable: GroupsGet failed ");
		return false;
	}
	int security = symcfg.type;
	int isTrade = grpcfg.secgroups[security].trade;
	int isShow = grpcfg.secgroups[security].show;
	if (isTrade == 0 || isShow == 0) {
		 


		return false;;
	}
	return true;
}
int  CProcessor::checkSymbolExecuteMode(string symbol, string group) {
	ConSymbol      symcfg = { 0 };
	ConGroup grpcfg = { 0 };
	if (ExtServer->SymbolsGet(symbol.c_str(), &symcfg) == FALSE) {
		LOG(false, " checkSymbolIsEnable: SymbolsGet failed");
		return false;
	}
	if (ExtServer->GroupsGet(group.c_str(), &grpcfg) == FALSE) {
		LOG(false, " checkSymbolIsEnable: GroupsGet failed ");
		return false;
	}
	int security = symcfg.type;
	int execute_mode = grpcfg.secgroups[security].execution;
	return execute_mode;
}

void  CProcessor::ExternalOpenOrder(int server_id, int login, string symbol, int vol, int cmd, string comment, int mode,string task_id,int order, int state) {
	/*if (server_id != this->plugin_id) {
		return;
	}*/
	
	MyTrade* tmp = new MyTrade();
	tmp->login = login;
	tmp->cmd = cmd;
	COPY_STR(tmp->symbol, symbol.c_str());
	tmp->volume = vol;
    COPY_STR(tmp->comment, comment.c_str())
	tmp->mode = mode;
	tmp->task_id = task_id;
	tmp->order = order;
	tmp->server_id = server_id;
	tmp->state = state;
	this->excutor.commit(open_order_worker_thread, tmp);
}


void  CProcessor::ExternalCloseOrder(int server_id, int login, string symbol, int vol, int cmd, string comment, int mode, int order, string task_id,int state) {
	
	
	//if (server_id!=this->plugin_id) {
	//	return;
	//}
	
	MyTrade* tmp = new MyTrade();
	tmp->login = login;
	tmp->cmd = cmd;
	COPY_STR(tmp->symbol, symbol.c_str());
	tmp->volume = vol;
	COPY_STR(tmp->comment, comment.c_str())
	tmp->mode = mode;
	tmp->order = order;
	tmp->task_id = task_id;
	tmp->server_id = server_id;
	tmp->state = state;
	this->excutor.commit(close_order_worker_thread, tmp);
}