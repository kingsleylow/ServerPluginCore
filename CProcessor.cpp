#include "stdafx.h"
#include "CProcessor.h"
#include "TaskManagement.h"
#include "MyLOG.h"

 
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

CProcessor::CProcessor()
{
	bool isExist = Utils::isExistFolder(LOG_DIR);
	if (isExist == false) {
		Utils::createFolder(LOG_DIR);
	}
 
	 
	plugin_id = D_PLUGIN_ID;
	UINT id = 0;
	 m_threadServer = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
	//m_funcThread = (HANDLE)_beginthreadex(NULL, 256000, FuncWrapper, (void*)this, 0, &id);
	
}
CProcessor::~CProcessor()
{
	if (pool !=NULL) {
		delete pool;
		pool = NULL;
	}
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
}

 
void CProcessor::setupSocket() {
	 

	if (pool==NULL) {
		pool = new SocketConnectionPool();
	}
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
	if (trade->cmd !=OP_SELL || trade->cmd != OP_BUY) {
		return;
	}


	LOG(false, "LifeByte::SrvTradesAddExt order %d, state %d,login %d, name %s, symbol %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment, mode);
	LOG(CmdTrade, "LifeByte::SrvTradesAddExt", "%d,%d,%d,%s,%s,%s,%d", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment, mode);

	this->HandlerAddOrder(trade,user,symbol,mode);
}



void CProcessor::SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{
	LOG(CmdTrade, "LifeByte::SrvTradesUpdate", "%d,%d,%d,%s,%s,%d", trade->order, trade->state, user->login, user->name, trade->comment, mode);
	LOG(false, "LifeByte::SrvTradesUpdate order %d, state %d,login %d,name %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, trade->comment, mode);



	switch (mode)
	{
	case UPDATE_NORMAL:

		break;
	case UPDATE_ACTIVATE:
		HandlerActiveOrder(trade, user, mode);
		break;
	case UPDATE_CLOSE:
		HandlerCloseOrder(trade, user, mode);
		break;
	case UPDATE_DELETE:
		break;
	default:
		break;
	}

}

 

void CProcessor::SrvDealerConfirm(const int id, const UserInfo *us, double *prices)
{
	LOG(CmdTrade, "LifeByte::SrvDealerConfirm", "LifeByte::SrvDealerConfirm");



	std::thread t([this, id, us, prices]() {

		m_ContextLock.Lock();

		std::map<int, RequestMetaData>::iterator it = requestsMadeByCode.find(id);
		 	if (it != requestsMadeByCode.end()) {
				RequestMetaData data = it->second;
				TradeTransInfo info = data.info;

				double price = 0.0;
				double price0 = prices[0];
				double price1 = prices[1];
				
				LOG(false, "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s,info.price %f, price0  %f , price1  %f , type %d", id, data.login, info.cmd, info.symbol, info.price, price0,price1, info.type);

				if (info.type == TT_ORDER_MK_OPEN) {
					price = (info.cmd == OP_BUY) ? price1 : price0;
					OrdersOpen(data.login, info.cmd, info.symbol, price, info.volume, info.comment);
				}
				else if (info.type == TT_ORDER_MK_CLOSE) {
					price = (info.cmd == OP_BUY) ? price0 : price1;
					OrdersClose(data.order, info.volume, price,info.comment);
				}
	
				requestsMadeByCode.erase(it);
			}
			else {
				LOG(false, "LifeByte::SrvDealerConfirm No request");
			} 

		m_ContextLock.UnLock();
	});

	t.detach();
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
		LOG(false, "LifeByte::askLPtoCloseTrade request id %d, res %d", reqId,res);
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
	 
		LOG(false,  "LifeByte::askLPtoOpenTrade request id %d  res %d", reqId,res);
		if (res == RET_TRADE_ACCEPTED) {
			RequestMetaData data;
			data.info = trans;
			data.login = login;
			requestsMadeByCode[reqId] = data;
		}
		m_ContextLock.UnLock();
	}
}

//+------------------------------------------------------------------+
//| Opening a BUY or SELL position using OrdersOpen             |
//+------------------------------------------------------------------+
int CProcessor::OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
	const double open_price, const int volume, string comment)
{

	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            order = 0;
	//--- Checks
	if (login <= 0 || cmd<OP_BUY || cmd>OP_SELL || symbol == NULL || open_price <= 0 || volume <= 0 || ExtServer == NULL)
		return(0);
	//--- Receiving client data
	if (UserInfoGet(login, &info) == FALSE)
		return(FALSE); // error
					   //--- Preparing transaction
	trans.cmd = cmd;
	trans.volume = volume;
	trans.price = open_price;

	COPY_STR(trans.symbol, symbol);
	//--- Adding SL, TP, comment
	COPY_STR(trans.comment, comment.c_str());
	//--- Check of long-only permission
	//--- Check of close-only permission
	//--- Check of tick size
	//--- Symbol check
	//--- Volume check
	//--- Check of stop levels
	//--- Opening an order and checking margin
	if ((order = ExtServer->OrdersOpen(&trans, &info)) == 0)
		return(0); // Error
				   //--- Position is open: return order
	return(order);
}
//+------------------------------------------------------------------+
//| Closing a BUY or SELL position using OrdersClose                 |
//+------------------------------------------------------------------+
int CProcessor::OrdersClose(const int order, const int volume, const double close_price, const string comment) {
	UserInfo       info = { 0 };
	TradeTransInfo trans = { 0 };
	int            login;
	//--- Checks
	if (order <= 0 || volume <= 0 || close_price <= 0 || ExtServer == NULL) return(FALSE);
	//--- Getting login from the order
	if ((login = ExtServer->TradesFindLogin(order)) == 0)
		return(FALSE); // Error
  //--- Getting client data
	if (UserInfoGet(login, &info) == FALSE)
		return(FALSE); // Error
  //--- Preparing transaction
	COPY_STR(trans.comment, comment.c_str());
	trans.order = order;
	trans.volume = volume;
	trans.price = close_price;
	 
	//--- Check of tick size
	//--- Instrument check
	//--- Volume check
	//--- Freeze level check
	//--- Closing position
	if (ExtServer->OrdersClose(&trans, &info) == FALSE)
		return(FALSE); // Error
  //--- Position closed
	return(TRUE);
}

bool CProcessor::ActionCheck() {
	time_t         currtime;
	//---- get the current server time  
	currtime=ExtServer->TradeTime();
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
void CProcessor::ThreadProcess(void)
{

	//
	 
	//while (true) {
	//	if (this->pool == NULL) {
	//	
	//		Sleep(5000);
	//	 	LOG("LifeByte::Pool IS NULL");
	//		continue;
	//	}
	//	Sleep(5000);
	//	bool res = this->pool->checkConnection();
 //	//LOG(true, "LifeByte::Pool Size " +   to_string(this->pool->GetPoolSize()));
	////LOG(true,"LifeByte::Pool current Size " + to_string(this->pool->current_size));

	//	if (res==false) {
	//		continue;
	//	}

	//	MyIOCP* iocp = this->pool->GetConnection();
	//	if (iocp == NULL ) {
	//		continue;
	//	}
	//	if(res == false){
	//		this->pool->ReleaseConnection(iocp);
	//	 continue;
	//	}
	//	if (iocp->level != ADM_LEVEL) {
	//		this->pool->ReleaseConnection(iocp);
	//		continue;
	//	}
	//	TaskManagement* man = TaskManagement::getInstance();
	//	if (  man->initialTask == INITIAL_NO || man->initialTask == INITIAL_FINISH) {
	//		iocp->SendInitTask();
	//	}
	//	
	//	this->pool->ReleaseConnection(iocp);
	//
	// 	 
	//}
}



UINT __stdcall CProcessor::FuncWrapper(LPVOID pParam)
{
	//---
	if (pParam != NULL) ((CProcessor*)pParam)->FuncProcess();
	//---
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CProcessor::FuncProcess(void)
{
	while (true) {
		if (this->pool == NULL) {

			Sleep(5000);
			LOG(CmdTrade, "LifeByte::Pool IS NULL", "%d", 0);
			continue;
		}
		m_ContextLock.Lock();
		MyIOCP* iocp = this->pool->GetConnection();
		if (iocp!=NULL) {
			 	
			TaskManagement* man = TaskManagement::getInstance();
		/*	if (man->initialTask == INITIAL_NO) {
				man->initialTask = INITIAL_BEGIN;
				 iocp->SendInitTask();
			
			}*/
			//iocp->SendInitTask();
	    }


		this->pool->ReleaseConnection(iocp);
		Sleep(1000);
		m_ContextLock.UnLock();
	}
}

void CProcessor::HandlerAddOrder(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode) {

	m_ContextLock.Lock();
	TaskManagement* man = TaskManagement::getInstance();

#ifdef _DEBUG
	if (trade->login == 4) {
	 
		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		askLPtoOpenTrade(5, trade->symbol, trade->cmd, trade->volume, comment, trade->tp, trade->sl);
	}
#else


	for (auto tmp = man->m_buff.cbegin(); tmp != man->m_buff.cend(); ++tmp) {
		string task_id = (*tmp).first;
		TradeTask* task = (*tmp).second;
		if (task->follower_disable == true || task->master_disable == true) {
			break;
		}

		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			break;
		}
		string com(trade->comment);
		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);
		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		askLPtoOpenTrade(atoi(task->follower_id.c_str()), trade->symbol, cmd, vol, trade->comment, trade->tp, trade->sl);
	}

#endif
	m_ContextLock.UnLock();
}

void CProcessor::HandlerCloseOrder(TradeRecord *trade, UserInfo *user, const int mode) {
	m_ContextLock.Lock();
#ifdef _DEBUG
	if (trade->login == 4) {
		UserInfo info = { 0 };
		if (UserInfoGet(5, &info) == FALSE)
			return; // error
		int total = 0;
		int order = trade->order;
		TradeRecord* records = ExtServer->OrdersGetOpen(&info,&total);
		for (int i = 0; i < total; i++) {
			TradeRecord record = records[i];
			string comment(record.comment);
			if ((comment).find(to_string(order)) != std::string::npos) {
				askLPtoCloseTrade(5, record.order, trade->cmd, trade->symbol,  comment, trade->volume);
			}

		}
	}
#else

#endif
	m_ContextLock.UnLock();
}

void CProcessor::HandlerActiveOrder(TradeRecord *trade, UserInfo *user, const int mode) {
	m_ContextLock.Lock();
#ifdef _DEBUG
	if (trade->login == 4) {
		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		askLPtoOpenTrade(5, trade->symbol, trade->cmd, trade->volume, comment, trade->tp, trade->sl);
	}
#else

#endif
	m_ContextLock.UnLock();
}
 

//int CProcessor::StopoutsApply(const UserInfo *user, const ConGroup *group, const char *comment) {
//	TradeRecord   *trades, *trade;  
//	int            total, i;  
//	ConSymbol      symbol = { 0 }; 
//	TradeTransInfo trans = { 0 }; 
//	time_t         currtime;   char     
//		stopout[64] = "[stopout]", *cp;
//	//---- checks  
//	if(user==NULL || group==NULL || comment==NULL) return(RET_OK);
//	//---- do everything in lock 
//	m_sync.Lock();
//	//---- check whether we can process the client 
//	if(CheckGroup(m_groups,group->group)==FALSE)     {   
//		m_sync.Unlock();      return(RET_OK);   
//	}
//	//---- get the list of open positions of the client  
//	if((trades=ExtServer->OrdersGetOpen(user,&total))==NULL)
//		return(RET_OK);
//	//---- get the current server time 
//	currtime=ExtServer->TradeTime();
//	//---- prepare a message about stopout, take it from the comment to the most unprofitable order 
//	if((cp=strrchr(comment,'['))!=NULL) COPY_STR(stopout,cp);
//	//---- write that we are closing positions of the client
//	Out(CmdOK,"Stopouts","'%d': close all orders due stop out %s",user->login,stopout);
//	//---- go through trades and close  
//	for(i=0,trade=trades;i     {   
//		//---- this is an open position in the first place  
//		if(trade->cmd>OP_SELL)                   
//		continue;     
//		//---- check whether we can process this symbol   
//			if(CheckGroup(m_symbols,trade->symbol)==FALSE)  
//				continue;    
//		//---- check the volume of the position   
//				if(m_max_volume!=0 && trade->volume>m_max_volume)
//					continue;    
//		//---- get the symbol for a trade  
//					if(ExtServer->SymbolsGet(trade->symbol,&symbol)==FALSE)        {    
//						//---- inform about the unexpected error  
//						Out(CmdAtt,"Stopouts","receiving information for %s symbol failed",trade->symbol);  
//						continue;      
//					}     
//	//---- check whether we are within the trade session for the symbol    
//	if(ExtServer->TradesCheckSessions(&symbol,currtime)==FALSE) continue;  
//	//---- prepare the transaction to close the position   
//	trans.order =trade->order;  
//	trans.price =trade->close_price; 
//	trans.volume=trade->volume;   
//	//---- prepare a comment   
//	if(trade->comment[0]!=0)   
//	{        
//		COPY_STR(trans.comment,trade->comment); 
//		if(strstr(trans.comment,stopout)==NULL)       
//			_snprintf(trans.comment,sizeof(trans.comment)-1,"%s %s",trans.comment,stopout);    
//	}      else COPY_STR(trans.comment,stopout);     
//	//---- close the client's position    
//	if(ExtServer->OrdersClose(&trans,const_cast(user))==FALSE)        { 
//		Out(CmdAtt,"Stopouts","stop out #%d failed",trade->order);  
//		continue;       
//	}   
//		}
//		//---  
//	m_sync.Unlock();
//		//---- free memory 
//	HEAP_FREE(trades);
//	//---- since we have already done everything by ourselves, the server does not need to process anything  
//	return(RET_OK_NONE);  
//} 
//}