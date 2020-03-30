#include "stdafx.h"
#include "CProcessor.h"
#include "TaskManagement.h"
#include "MyLOG.h"
#include "Common.h"
 
extern CServerInterface *ExtServer;
CProcessor               ExtProcessor;
TaskManagement			 TaskManager;
#define TIME_RATE ((double)1.6777216)
#define OURTIME(stdtime) ((DWORD)((double)(stdtime)/TIME_RATE))
#define CORE_IP "IP"
#define CORE_BACKUP_IP "BACKUP_IP"
#define DEFAULT_IP "0"
#define CORE_PORT "PORT"
#define CORE_BACKUP_PORT "BACKUP_PORT"
#define DEFAULT_PORT "0"

#define CORE_KEY "KEY"
#define CORE_KEY_BACKUP "BACKUP_KEY"
#define DEFAULT_KEY ""
#define PLUGIN_ID "PLUGIN_ID"
#define DEFAULT_PLUGIN_ID "0"
#define D_PLUGIN_ID 0
struct RequestMetaData {
	TradeTransInfo info;
	int login;
	int order; // user for close order
};
std::map<int, RequestMetaData> requestsMadeByCode;
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
 
	 
	plugin_id = D_PLUGIN_ID;
	UINT id = 0;
	  m_threadServer = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
 
	  InitializeCriticalSectionAndSpinCount(&m_cs, 10000);;
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
	char buffer[1024] = { 0 };
	//---- checks
	if (msg == NULL || ExtServer == NULL) return;
	//---- formating string
	va_list ptr;
	va_start(ptr, msg);
	_vsnprintf_s(buffer, sizeof(buffer) - 1, msg, ptr);
	va_end(ptr);
	//---- output
	ExtServer->LogsOut(code, ip, buffer);



}

void CProcessor::LOG(bool debug, string msg, ...) const {

	char buffer[1024] = { 0 };
	va_list ptr;
	va_start(ptr, msg);
	_vsnprintf_s(buffer, sizeof(buffer) - 1, msg.c_str(), ptr);
	va_end(ptr);
	if (debug == true) {
	 
#ifdef _DEBUG
		MyLOG::getInstance()->LOG_I(string(buffer));
#endif
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


void CProcessor::SrvTradeRequestApply(RequestInfo *request, const int isdemo)
{
	LOG(CmdTrade, "LifeByte::SrvTradeRequestApply", "%d,%d,%d", request->id, request->login, isdemo);
}

void CProcessor::SrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode)
{
	if (mode != OPEN_NEW) {
		return;
	}
	if (trade->cmd !=OP_SELL && trade->cmd != OP_BUY) {
		return;
	}


	LOG(false, "LifeByte::SrvTradesAddExt order %d, state %d,login %d, name %s, symbol %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment, mode);
	LOG(CmdTrade, "LifeByte::SrvTradesAddExt", "%d,%d,%d,%s,%s,%s,%d", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment, mode);


	MyTrade* tmp = new MyTrade(); 
	tmp->login = trade->login;
	tmp->cmd = trade->cmd;
	tmp->sl = trade->sl;
	tmp->tp = trade->tp;
	COPY_STR(tmp->symbol,trade->symbol);
	tmp->volume = trade->volume;
	tmp->order = trade->order;
	this->excutor.commit(add_order_worker_thread, tmp);


//	this->HandlerAddOrder(trade,user,symbol,mode);
}



void CProcessor::SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{
	LOG(CmdTrade, "LifeByte::SrvTradesUpdate", "%d,%d,%d,%s,%s,%d", trade->order, trade->state, user->login, user->name, trade->comment, mode);
	LOG(false, "LifeByte::SrvTradesUpdate order %d, state %d,login %d,name %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, trade->comment, mode);


 
		MyTrade* tmp = new MyTrade();
		tmp->login = trade->login;
		tmp->cmd = trade->cmd;
		tmp->sl = trade->sl;
		tmp->tp = trade->tp;
		COPY_STR(tmp->symbol, trade->symbol);
		tmp->volume = trade->volume;
		tmp->order = trade->order;
		switch (mode)
		{
		case UPDATE_NORMAL:

			break;
		case UPDATE_ACTIVATE:
	 		this->excutor.commit(add_order_worker_thread, tmp);
		//	HandlerActiveOrder(tmp, user, mode);
			break;
		case UPDATE_CLOSE:
	 		this->excutor.commit(close_order_worker_thread, tmp);
		//	 	HandlerCloseOrder(tmp, user, mode);
			break;
		case UPDATE_DELETE:
			break;
		default:
			break;
		}
	 



}

 

void CProcessor::SrvDealerConfirm(const int id, const UserInfo *us, double *prices)
{
	m_ContextLock.Lock();
	std::map<int, RequestMetaData>::iterator it = requestsMadeByCode.find(id);
	if (it == requestsMadeByCode.end()) {
		m_ContextLock.UnLock();
		return;
	}
	m_ContextLock.UnLock();
	LOG(CmdTrade,"LifeByte::SrvDealerConfirm", "LifeByte::SrvDealerConfirm: request %d", id);

 

	OrderTask* task = new OrderTask();
	task->id = id;
	task->prices_0 = prices[0];
	task->prices_1 = prices[1];
 
 
	this->excutor.commit(order_worker_thread, task);

	
}

UINT __cdecl CProcessor::add_order_worker_thread(void* param) {


	EnterCriticalSection(&m_cs);


	MyTrade* trade = (MyTrade*)param;

	TaskManagement* man = TaskManagement::getInstance();
	man->isWorking = true;
	for (auto tmp = man->m_task.cbegin(); tmp != man->m_task.cend(); ++tmp) {

		TradeTask* task = (*tmp);
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}
		
		if (task->master_server_id != ExtProcessor.plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}

		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		int follower_id = atoi(task->follower_id.c_str());
		if (task->follower_server_id == ExtProcessor.plugin_id) {


			ExtProcessor.askLPtoOpenTrade(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl);
		}
		else {
			//handle another server

			MyIOCP* iocp = ExtProcessor.pool->GetConnection();
			if (iocp == NULL) {
				continue;
			}
			iocp->openOrderRequest(task->follower_server_id, task->follower_id, trade->symbol, cmd, vol, comment);
		}

	}

	if (trade!=NULL) {
		delete trade;
		trade = NULL;
	}
	man->isWorking = false;
	LeaveCriticalSection(&m_cs);
	return 0;
}

UINT __cdecl CProcessor::close_order_worker_thread(void* param) {

	EnterCriticalSection(&m_cs);
	MyTrade* trade = (MyTrade*)param;
	TaskManagement* man = TaskManagement::getInstance();
	man->isWorking = true;
	ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::man->m_task size %d  ", man->m_task.size());
	for (auto tmp = man->m_task.cbegin(); tmp != man->m_task.cend(); ++tmp) {
		
		TradeTask* task = (*tmp);
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}

		if (task->master_server_id != ExtProcessor.plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}
		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		if (task->follower_server_id == ExtProcessor.plugin_id) {
			int total = 0;
			int order = trade->order;
			int follower_id = atoi(task->follower_id.c_str());
			UserInfo info = { 0 };
			if (ExtProcessor.UserInfoGet(follower_id, &info) == FALSE)
				continue;
			TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);
			bool find_order = false;
			for (int i = 0; i < total; i++) {
				TradeRecord record = records[i];
				string comment(record.comment);
				if ((comment).find(to_string(order)) != std::string::npos) {
				    find_order = true;
					ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, trade->symbol, comment, vol);
				}
			}

			// handle master open and close order at the same time issue
			if (find_order==false) {
				ExtProcessor.AddToQuickCloseQueue(follower_id,trade);
			}


			ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte:: order %d, find %d,  ",order,find_order);
			HEAP_FREE(records);
		}
		else {
			MyIOCP* iocp = ExtProcessor.pool->GetConnection();
			if (iocp == NULL) {
				continue;
			}

			iocp->closeOrderRequest(task->follower_server_id, task->follower_id, trade->order, vol);
		}
	}




	if (trade != NULL) {
		delete trade;
		trade = NULL;
	}
	man->isWorking = false;;
	LeaveCriticalSection(&m_cs);
	return 0;
}
//+------------------------------------------------------------------+
//| order_worker_thread
//|  place order to db or close order on db
//+------------------------------------------------------------------+

UINT __cdecl  CProcessor::order_worker_thread(void* param) {
	OrderTask* task = (OrderTask*)param;
	int id = task->id;
 
	EnterCriticalSection(&m_cs);

		std::map<int, RequestMetaData>::iterator it = requestsMadeByCode.find(id);
	 	if (it != requestsMadeByCode.end()) {
			RequestMetaData data = it->second;
			TradeTransInfo info = data.info;

			double price = 0.0;
			double price0 = task->prices_0;
			double price1 = task->prices_1;


	 
			ExtProcessor.LOG(false, "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s,info.price %f, price0  %f , price1  %f , type %d", id, data.login, info.cmd, info.symbol, info.price, price0,price1, info.type);

			if (info.type == TT_ORDER_MK_OPEN) {
				price = (info.cmd == OP_BUY) ? price1 : price0;
		 
				ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::open order  login %d  ",   data.login);
				ExtProcessor.OrdersOpen(data.login, info.cmd, info.symbol, price, info.volume, info.comment);
			}
			else if (info.type == TT_ORDER_MK_CLOSE) {
				price = (info.cmd == OP_BUY) ? price0 : price1;
				ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::close order id %d, login %d ", data.order, data.login);
				ExtProcessor.OrdersClose(data.order, info.volume, price,info.comment);
			}


			UserInfo us = { 0 };
			if (ExtProcessor.UserInfoGet(data.login, &us) == TRUE) {
		  	int res =	ExtServer->RequestsDelete(id);
			 
			ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::close request id %d, res %d ", id, res);


			}

			requestsMadeByCode.erase(it);
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




void CProcessor::askLPtoCloseTrade(int login, int order, int cmd, string symbol, string comment, int volumeInCentiLots) {
	//Check if cmd is OP_BUY, OP_SELL, volume is positive
	//Check symbol is not long_only and cmd is sell
	//Check symbol is tradeable (TRADE_FULL)
	//TradesCheckSecurity
	//TradesCheckTickSize
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
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
	//TradesCheckVolume
	//TradesCheckStops
	//TradesMarginCheck

	{

		if (UserInfoGet(login, &info) == FALSE)
			return; // error
		RequestInfo reqInfo = { 0 };
		reqInfo.status = DC_EMPTY;

		reqInfo.login = login;
		strncpy(reqInfo.group, info.group, sizeof(reqInfo.group));
		reqInfo.balance = info.balance;
		reqInfo.credit = info.credit;
		reqInfo.trade = trans;
	 
	//	double prices[2];
	//	ExtServer->HistoryPricesGroup(&reqInfo, prices);

	//	trans.price = (cmd == OP_BUY) ? prices[1] : prices[0];

		int reqId;
		m_ContextLock.Lock();
		const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);

	
		if (res == RET_TRADE_ACCEPTED) {
			RequestMetaData data;
			data.info = trans;
			data.login = login;
			data.order = order;
			requestsMadeByCode[reqId] = data;
		 
	}
		LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoCloseTrade request id %d, res %d ,order", reqId, res,order);

		

		
		m_ContextLock.UnLock();
	}
}

void CProcessor::askLPtoOpenTrade(int login, const std::string& symbol, int cmd, int volumeInCentiLots, const std::string& comment, double tp, double sl) {
	//Check if cmd is OP_BUY, OP_SELL, volume is positive
	//Check symbol is not long_only and cmd is sell
	//Check symbol is tradeable (TRADE_FULL)
	//TradesCheckSecurity
	//TradesCheckTickSize
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
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

	//TradesCheckVolume
	//TradesCheckStops
	//TradesMarginCheck

	  {

		if (UserInfoGet(login, &info) == FALSE)
			return ; // error



		RequestInfo reqInfo = { 0 };
		reqInfo.status = DC_EMPTY;
	 
		reqInfo.login = login;
		strncpy(reqInfo.group, info.group, sizeof(reqInfo.group));
		reqInfo.balance = info.balance;
		reqInfo.credit = info.credit;
		reqInfo.trade = trans;

	//	double prices[2];
	//	ExtServer->HistoryPricesGroup(&reqInfo, prices);

	//	trans.price = (cmd == OP_BUY) ? prices[1] : prices[0];

		int reqId;
		m_ContextLock.Lock();
		const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);
	 
		if (res == RET_TRADE_ACCEPTED) {
			RequestMetaData data;
			data.info = trans;
			data.login = login;
			requestsMadeByCode[reqId] = data;
			 

		}
		LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoOpenTrade request id %d  res %d", reqId, res);

		m_ContextLock.UnLock();
	}
}

//+------------------------------------------------------------------+
//| Opening a BUY or SELL position using OrdersOpen             |
//+------------------------------------------------------------------+
int CProcessor::OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
	const double open_price, const int volume, string comment)
{
	ConGroup       grpcfg = { 0 };
	time_t         currtime;
	ConSymbol      symcfg = { 0 };
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            order = 0;
	//--- Checks
	if (login <= 0 || cmd<OP_BUY || cmd>OP_SELL || symbol == NULL || open_price <= 0 || volume <= 0 || ExtServer == NULL)
		return(0);

	//--- get symbol config
	if (ExtServer->SymbolsGet(symbol, &symcfg) == FALSE)
	{
		LOG(false, " OpenAnOrder: SymbolsGet failed [%s]", string(symbol));
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
		LOG(false, " OpenAnOrder: GroupsGet failed [%s]", string(info.group));
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
	//--- Symbol check
		//--- check secutiry
	if (ExtServer->TradesCheckSecurity(&symcfg, &grpcfg) != RET_OK)
	{
		LOG(false, " CloseAnOrder: trade disabled or market closed");
		return(FALSE); // trade disabled, market closed, or no prices for long time
	}
	//--- Volume check
		//--- check volume
	if (ExtServer->TradesCheckVolume(&trans, &symcfg, &grpcfg, TRUE) != RET_OK)
	{
		LOG(false, "OpenAnOrder: invalid volume");
		//return(FALSE); // invalid volume
	}
	//--- Check of stop levels// for pendig order
 

	//--- Opening an order and checking margin
 

	if ((order = ExtServer->OrdersOpen(&trans, &info)) == 0) {
		return(0); // Error
	}
	int masterOld = GetOOrderNumberFromComment(comment.c_str());
    if(masterOld!=0){
		this->HandleQuickCloseIssue(login, masterOld);

	}

				   //--- Position is open: return order
	return(order);
}





//+------------------------------------------------------------------+
//| Closing a BUY or SELL position using OrdersClose                 |
//+------------------------------------------------------------------+
int CProcessor::OrdersClose(const int order,  const int volume, const double close_price, const string comment) {
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            login;


	time_t         currtime;
	TradeRecord    old_trade = { 0 };
 
	ConGroup       grpcfg = { 0 };
	ConSymbol      symcfg = { 0 };
 


	//--- Checks
	if (order <= 0 || volume <= 0 || close_price <= 0 || ExtServer == NULL) return(FALSE);

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
		LOG(false, " CloseAnOrder: GroupsGet failed [%s]",string(info.group));
		return(FALSE); // Error
	}
	 
   //--- get symbol config
	if (ExtServer->SymbolsGet(old_trade.symbol, &symcfg) == FALSE)
	{	 
		LOG(false, " CloseAnOrder: SymbolsGet failed [%s]", string( old_trade.symbol));
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
	//--- check secutiry
	if (ExtServer->TradesCheckSecurity(&symcfg, &grpcfg) != RET_OK)
	{
		LOG(false, " CloseAnOrder: trade disabled or market closed");
		return(FALSE); // trade disabled, market closed, or no prices for long time
	}
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
	if (ExtServer->OrdersClose(&trans, &info) == FALSE)
		return(FALSE); // Error
  //--- Position closed


	//-- update new order comment
	if (volume< old_trade.volume) {
		//--- get order
		TradeRecord follower_old_trade = {0};
		if (ExtServer->OrdersGet(order, &follower_old_trade) == FALSE) {
			LOG(false, "CloseAnOrder: OrdersGet failed");
			return(FALSE); // Error
		}
		int followerNewOrder = GetToOrderComment(follower_old_trade.comment);
		 
		int masterOld = GetOOrderNumberFromComment(comment.c_str());

		TradeRecord master_old_trade = { 0 };
		//--- get order
		if (ExtServer->OrdersGet(masterOld, &master_old_trade) == FALSE) {
			LOG(false, "CloseAnOrder: OrdersGet failed");
			return(FALSE); // Error
		}
		int masterNewOrder = GetToOrderComment(master_old_trade.comment);



		string newComment = ORDER_COMMENT_PRE + to_string(masterNewOrder);
		if (this->UpdateComment(followerNewOrder, newComment) == FALSE) {
			LOG(false, "CloseAnOrder: update failed");
			return(FALSE); // Error
		}

	}





	return(TRUE);
}

//+------------------------------------------------------------------+
//| Thread wrapper                                                   |
//+------------------------------------------------------------------+
UINT __stdcall CProcessor::ThreadWrapper(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->ThreadProcess();
	//---
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
static volatile int task_check_cycle = 0;
void CProcessor::ThreadProcess(void)
{


	 
	while (true) {
		ExtProcessor.LOG(true, "coming");
		Sleep(5000);
		if (this->pool == NULL) {
	 	       
			continue;
		}
	
		bool res = this->pool->checkConnection();
 
		if (res==false) {
			continue;
		}
//
		MyIOCP* iocp = this->pool->GetConnection();
		if (iocp == NULL ) {
			this->pool->ReleaseConnection(iocp);
			continue;
		}
 
		if (iocp->level != ADM_LEVEL) {
			this->pool->ReleaseConnection(iocp);
			continue;
		}
		TaskManagement* man = TaskManagement::getInstance();
		if (  man->initialTask == INITIAL_NO) {
			iocp->SendInitTask(this->plugin_id);
		}
		
		this->pool->ReleaseConnection(iocp);
	

		if (man->initialTask == INITIAL_FINISH) {

			task_check_cycle++;
			if (task_check_cycle > 2) {
				task_check_cycle = 0;
				//10 sec
				iocp->SendInitTask(this->plugin_id);
			}

#ifdef _DEBUG
		//	LOG("Testing","Testing", man->printTask());
#endif

		//	LOG(false, man->printTask());
		}
	}
 
}



 

 

void CProcessor::HandlerAddOrder(MyTrade*trade, const UserInfo *user, const ConSymbol *symbol, const int mode) {

	m_ContextLock.Lock();
	TaskManagement* man = TaskManagement::getInstance();
	for (auto tmp = man->m_task.cbegin(); tmp != man->m_task.cend(); ++tmp) {
		 
		TradeTask* task = (*tmp) ;
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}

		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}

		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		int follower_id = atoi(task->follower_id.c_str());
		if (task->follower_server_id == this->plugin_id) {
		 
			
			askLPtoOpenTrade(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl);
		}
		else {
			//handle another server
		 
			MyIOCP* iocp = this->pool->GetConnection();
			if (iocp == NULL) {
				continue;
			}
			iocp->openOrderRequest(task->follower_server_id, task->follower_id, trade->symbol, cmd, vol, comment);
		}
		
	}

    
	if (trade!=NULL) {
		delete trade;
		trade = NULL;
	}
	m_ContextLock.UnLock();
}

void CProcessor::HandlerCloseOrder(MyTrade *trade, UserInfo *user, const int mode) {
	m_ContextLock.Lock();
 

	TaskManagement* man = TaskManagement::getInstance();
	for (auto tmp = man->m_task.cbegin(); tmp != man->m_task.cend(); ++tmp) {
	 
		TradeTask* task = (*tmp) ;
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}

		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}
		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		if (task->follower_server_id == this->plugin_id) {
			int total = 0;
			int order = trade->order;
			int follower_id = atoi(task->follower_id.c_str());
			UserInfo info = { 0 };
			if (UserInfoGet(follower_id, &info) == FALSE)
				continue;
			TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);
			 


			bool find_order = false;
			for (int i = 0; i < total; i++) {
				TradeRecord record = records[i];
				string comment(record.comment);
				if ((comment).find(to_string(order)) != std::string::npos) {
					find_order = true;
					ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, trade->symbol, comment, vol);
				}
			}

			// handle master open and close order at the same time issue
			if (find_order == false) {
				ExtProcessor.AddToQuickCloseQueue(follower_id, trade);
			}



			HEAP_FREE(records);
		}
		else {
			MyIOCP* iocp = this->pool->GetConnection();
			if (iocp == NULL) {
				continue;
			}
		 
			iocp->closeOrderRequest(task->follower_server_id, task->follower_id, trade->order,vol);
		}
	}
	if (trade != NULL) {
		delete trade;
		trade = NULL;
	}
	m_ContextLock.UnLock();
}

void CProcessor::HandlerActiveOrder(MyTrade *trade, UserInfo *user, const int mode) {
	m_ContextLock.Lock();
 
	//if (trade->login == 4) {
	//	string comment = ORDER_COMMENT_PRE + to_string(trade->order);
	//	askLPtoOpenTrade(5, trade->symbol, trade->cmd, trade->volume, comment, trade->tp, trade->sl);
	//}
	  //--- get symbol config
	ConSymbol      symcfg = { 0 };
	if (ExtServer->SymbolsGet(trade->symbol, &symcfg) == FALSE)
	{
		LOG(false, " HandlerActiveOrder: SymbolsGet failed [%s]", string(trade->symbol));
		return ; // error
	}
	this->HandlerAddOrder(trade, user, &symcfg, mode);
 
	m_ContextLock.UnLock();
}
 
//+------------------------------------------------------------------+
//|HandleQuickCloseIssue                 |
//+------------------------------------------------------------------+


void CProcessor::HandleQuickCloseIssue(int login,int order) {
	TaskManagement* man = TaskManagement::getInstance();

	MyTrade* tmp = man->findCloseOrder(login, order);
	if (tmp!=NULL) {
		this->excutor.commit(close_order_worker_thread, tmp);
	}
	else {

	}

}

void CProcessor::AddToQuickCloseQueue(int follower_id,MyTrade* trade) {
	
	TaskManagement* man = TaskManagement::getInstance();
	MyTrade* tmp = new MyTrade();
	tmp->order = trade->order;
	tmp->cmd = trade->cmd;
	tmp->login = trade->login;
	tmp->tp = trade->tp;
	tmp->sl = trade ->sl;
	tmp->volume = trade->volume;
	COPY_STR(tmp->symbol, trade->symbol);

	man->addToCloseOrder(follower_id,tmp);
}

