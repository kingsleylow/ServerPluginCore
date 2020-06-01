#include "stdafx.h"
#include "CProcessor.h"
#include "TaskManagement.h"
#include "MyLOG.h"
#include "Common.h"
 
//#define DIRECT

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
//	m_threadServer = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
 	m_funcThread = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapperRequest, (void*)this, 0, &id);
	InitializeCriticalSectionAndSpinCount(&m_cs, 10000);;
	this->request_current_id = 0;
	 
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
	// cannot update portfolio
	TaskManagement* man = TaskManagement::getInstance();
	if (man->checkMaster(trade->login) == false) {
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
 

 	this->HandlerAddOrder(tmp,user,symbol,mode);
}



void CProcessor::SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{

	//TaskManagement* man = TaskManagement::getInstance();
	//if (man->checkMaster(trade->login) == false) {
	//	return;
	//}
 

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
 
		
			this->HandlerAddOrder(tmp, user, &symcfg, mode);
			break;
		case UPDATE_CLOSE:

 
			this->HandlerCloseOrder(tmp, user, mode);
			break;
		case UPDATE_DELETE:
			break;
		default:
			break;
		}
	 



}

 

void CProcessor::SrvDealerConfirm(const int id, const UserInfo *us, double *prices)
{
	//m_ContextLock.Lock();
	//std::map<int, RequestMetaData>::iterator it = requestsMadeByCode.find(id);
	//if (it == requestsMadeByCode.end()) {
	//	m_ContextLock.UnLock();
	//	return;
	//}
	//m_ContextLock.UnLock();
	//LOG(CmdTrade,"LifeByte::SrvDealerConfirm", "LifeByte::SrvDealerConfirm: request %d", id);

 
 

	OrderTask* task = new OrderTask();
	task->id = id;
	task->prices_0 = prices[0];
	task->prices_1 = prices[1];
 
 
	this->excutor.commit(confirm_order_worker_thread, task);
	 
	
}

 

 


 

 
//+------------------------------------------------------------------+
//| order_worker_thread
//|  place order to db or close order on db
//+------------------------------------------------------------------+

UINT __cdecl  CProcessor::confirm_order_worker_thread(void* param) {
	OrderTask* task = (OrderTask*)param;
	int id = task->id;
 
	EnterCriticalSection(&m_cs);

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
			ExtProcessor.LOG(false, "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s, type %d", id, login, cmd, symbol.c_str(), type);
			ExtProcessor.LOG(CmdTrade, "LifeByte::SrvDealerConfirm: request %d,login %d, cmd %d, symbol %s, type %d", id,login, cmd, symbol.c_str(),type);

			if ( type == TT_ORDER_MK_OPEN) {
				price = ( cmd == OP_BUY) ? price1 : price0;
		 
				ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::open order  login %d ,cmd %d,symbol %s, volume %d ",   login, cmd,symbol.c_str(),volume);
				ExtProcessor.LOG(false,   "LifeByte::confrom LifeByte::open order  login %d ,cmd %d,symbol %s, volume %d ", login, cmd, symbol.c_str(), volume);
				int res = ExtProcessor.OrdersOpen(login, cmd, symbol.c_str(), price,volume, comment);
				if (res!= 0) {
					ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::open order  login %d ,order %d success", login, res);
					ExtProcessor.LOG(false, "LifeByte::confrom LifeByte::open order  login %d ,order %d success",  login, res);
				}
				else {
					ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::open order  login %d failed", login );
					ExtProcessor.LOG(false, "LifeByte::confrom LifeByte::open order  login %d failed ", login );
				}
			}
			else if ( type == TT_ORDER_MK_CLOSE) {
				price = ( cmd == OP_BUY) ? price0 : price1;
			 
				int res = ExtProcessor.OrdersClose( order,  volume, price, comment);
				if (res==TRUE) {
					ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::close order id %d, login %d ,order %d, volume %d  success", order,  login,  order, volume);
					ExtProcessor.LOG(false, "LifeByte::confrom LifeByte::close order id %d, login %d,order %d, volume %d success ",  order,  login,  order,  volume);
				}
				else {
					ExtProcessor.LOG(CmdTrade, "LifeByte::confrom", "LifeByte::close order id %d, login %d ,order %d, volume %d failed ",  order,  login,  order,  volume);
					ExtProcessor.LOG(false, "LifeByte::confrom LifeByte::close order id %d, login %d,order %d, volume %d failed ",  order,  login,  order,  volume);
				}
			}


			UserInfo us = { 0 };
			if (ExtProcessor.UserInfoGet( login, &us) == TRUE) {
			 
				int  res = ExtServer->RequestsDelete(id);
				 
		    	ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::close request id %d, res %d ", id, res);
				ExtProcessor.LOG(false, "LifeByte::closeLifeByte::close request id %d, res %d ", id, res);

			}

			ExtProcessor.requestsMadeByCode.erase(it);
		//	ExtProcessor.processing_login.erase(data.login);
		//	ExtProcessor.checkRequestTaskByLogin(data.login);
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



		if (ExtServer->SymbolsGet(symbol.c_str(), &symcfg) == FALSE) {
			LOG(false, " askLPtoCloseTrade: SymbolsGet failed");
			return;
		}
		if (ExtServer->GroupsGet(info.group, &grpcfg) == FALSE) {
			LOG(false, " askLPtoCloseTrade: GroupsGet failed ");
			return;
		}



		int security = symcfg.type;
		int isTrade = grpcfg.secgroups[security].trade;
		int isShow = grpcfg.secgroups[security].show;
		if (isTrade==0 || isShow ==0) {
			LOG(false, "askLPtoCloseTrade: symbol disable");
			return;
		}

		int lot_max = grpcfg.secgroups[security].lot_max;
		int lot_min = grpcfg.secgroups[security].lot_min;
		int lot_step = grpcfg.secgroups[security].lot_min;
		if (volumeInCentiLots % lot_step != 0) {
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
		const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);


		if (res == RET_TRADE_ACCEPTED) {
			RequestMetaData data;
			data.info = trans;
			data.login = login;
			data.order = order;
			requestsMadeByCode[reqId] = data;
			//	this->processing_login.insert(login);
		}
		LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoCloseTrade login %d  request id %d, res %d ,order %d",login, reqId, res, order);
		LOG(false, "LifeByte::ask LifeByte::askLPtoCloseTrade login %d  request id %d, res %d ,order %d", login, reqId, res, order);



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
		if (volumeInCentiLots % lot_step !=0) {
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
		const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);
	 
		if (res == RET_TRADE_ACCEPTED) {
			RequestMetaData data;
			data.info = trans;
			data.login = login;
			requestsMadeByCode[reqId] = data;
		//	this->processing_login.insert(login);

		}
		LOG(CmdTrade, "LifeByte::ask", "LifeByte::askLPtoOpenTrade login %d request id %d  res %d",login, reqId, res);
		LOG(false, "LifeByte::ask LifeByte::askLPtoOpenTrade login %d request id %d  res %d", login, reqId, res);
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
		LOG(CmdTrade, "LifeByte::OrdersOpen", "add quick close order %d, login %d", masterOld, login);
		LOG(false, "LifeByte::OrdersOpen add quick close order %d, login %d", order, login);
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
	if (ExtServer->OrdersClose(&trans, &info) == FALSE) {

		LOG(false, " CloseAnOrder:close order failed %d",order);
		return(FALSE); // Error
	}
		
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
			LOG(false, "LifeByte::OrdersClose order %d, login %d  close failed ", order, login);
			LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d close failed", order, login);
			return(FALSE); // Error
		}

		LOG(false, "LifeByte::OrdersClose order %d, login %d", order, login);
		LOG(CmdTrade, "LifeByte::OrdersClose", "order %d, login %d", order, login);


		int masterNewOrder = GetToOrderComment(master_old_trade.comment);



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

	ExtProcessor.LOG(CmdTrade, "LifeByte::synchronize task thread exit ", " LifeByte::synchronize task thread exit");
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
static volatile int task_check_cycle = 0;
void CProcessor::ThreadProcess(void)
{


	 
	while (true) 
	{
		
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
			return;
		//	continue;
		}
 
		if (iocp->level != ADM_LEVEL) {
			this->pool->ReleaseConnection(iocp);
			 
		 	continue;
		}
		TaskManagement* man = TaskManagement::getInstance();
		if (  man->initialTask == INITIAL_NO) {
			man->initial_count = 0;
			iocp->SendInitTask(this->plugin_id);
		}
		
	   
	

		if (man->initialTask == INITIAL_FINISH) {
			man->initial_count = 0;
			task_check_cycle++;
			if (task_check_cycle > 2) {
				task_check_cycle = 0;
				//10 sec
				iocp->SendInitTask(this->plugin_id);
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


 

void CProcessor::HandlerAddOrder(MyTrade*trade, const UserInfo *user, const ConSymbol *symbol, const int mode) {

	m_ContextLock.Lock();
	TaskManagement* man = TaskManagement::getInstance();
	list<TradeTask*> m_task;
	man->getTaskList(m_task);
	man->isWorking = true;
	for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {
		 
		TradeTask* task = *tmp ;
	
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}

		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}

		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);

	


		if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {

			vol = task->follower_max_vol;
		}
	


		int cmd = man->getStrategy(task->master_strategy, trade->cmd);
		string comment = ORDER_COMMENT_PRE + to_string(trade->order);
		int follower_id = atoi(task->follower_id.c_str());
		if (task->follower_server_id == this->plugin_id) {

		 


			LOG(false, "LifeByte::New receive open order %d, state %d,login %d, name %s, symbol %s,comment %s,mode %d", trade->order, trade->state, user->login, user->name, trade->symbol, trade->comment, mode);
			LOG(CmdTrade, "LifeByte::New receive open order", "%d,%d,%d,%s,%s,%s,%d", trade->order, trade->state, user->login, user->name, trade->symbol, trade->comment, mode);

			//this->AddRequestTask(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl, 0, TS_OPEN_NORMAL);
		 	askLPtoOpenTrade(follower_id, trade->symbol, cmd, vol, comment, trade->tp, trade->sl);
		}
		else {
			//handle another server
		 
			MyIOCP* iocp = this->pool->GetConnection();
			if (iocp == NULL) {
				continue;
			}
			iocp->openOrderRequest(task->follower_server_id, task->follower_id, trade->symbol, cmd, vol, comment);

			this->pool->ReleaseConnection(iocp);
		}
		
	}

    
	if (trade!=NULL) {
		delete trade;
		trade = NULL;
	}
	man->clearTask(m_task);
	man->isWorking = false;
	m_ContextLock.UnLock();
}
 

void CProcessor::HandlerCloseOrder(MyTrade *trade, UserInfo *user, const int mode) {
 	m_ContextLock.Lock();
 

	TaskManagement* man = TaskManagement::getInstance();
//	man->isWorking = true;
	list<TradeTask*> m_task;
	man->getTaskList(m_task);
	for (auto tmp = m_task.cbegin(); tmp != m_task.cend(); ++tmp) {
	 
		TradeTask* task = *tmp;
		if (task->follower_disable == true || task->master_disable == true) {
			continue;
		}

		if (task->master_server_id != this->plugin_id || atoi(task->master_id.c_str()) != trade->login) {
			continue;
		}
		int vol = round(trade->volume * task->master_ratio * task->follower_ratio);

		if (task->follower_max_vol != 0 && vol > task->follower_max_vol) {

			vol = task->follower_max_vol;
		}


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
					if (vol > record.volume) {
						vol = record.volume;
					}

					if (trade->state == TS_CLOSED_NORMAL) {
						vol = record.volume;
					}


					LOG(false, "LifeByte::New receive close order %d, state %d,login %d, symbol %s,comment %s,mode %d, vol %d", trade->order, trade->state, follower_id,  trade->symbol, trade->comment, mode ,vol);
					LOG(CmdTrade, "LifeByte::New receive close order", "order %d, state %d,  login %d, symbol %s, comment %s, mode %d ,vol % d", trade->order, trade->state, follower_id, trade->symbol, trade->comment, mode ,vol);

				


					ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, trade->symbol, comment, vol);
					//	ExtProcessor.AddRequestTask(follower_id, trade->symbol, cmd, vol, comment, 0, 0, record.order, TS_CLOSED_NORMAL);
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
		 
			iocp->closeOrderRequest(task->follower_server_id, task->follower_id, trade->order,vol,trade->symbol,cmd,mode);
			this->pool->ReleaseConnection(iocp);
		}
	}
	if (trade != NULL) {
		delete trade;
		trade = NULL;
	}
//	man->isWorking = false;
	man->clearTask(m_task);
 	m_ContextLock.UnLock();
}

void CProcessor::HandlerActiveOrder(MyTrade *trade, UserInfo *user, const int mode) {
	m_ContextLock.Lock();
 
	//if (trade->login == 4) {
	//	string comment = ORDER_COMMENT_PRE + to_string(trade->order);
	//	askLPtoOpenTrade(5, trade->symbol, trade->cmd, trade->volume, comment, trade->tp, trade->sl);
	//}
	  //--- get symbol config

 
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


UINT __cdecl CProcessor::close_order_worker_thread(void* param) {

	EnterCriticalSection(&m_cs);
	MyTrade* trade = (MyTrade*)param;
	TaskManagement* man = TaskManagement::getInstance();
	list<TradeTask*> m_task;
	man->getTaskList(m_task);
	man->isWorking = true;
	ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte::man->m_task size %d  ",m_task.size());
	for (auto tmp =  m_task.cbegin(); tmp !=  m_task.cend(); ++tmp) {

		TradeTask* task = *tmp;
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

					ExtProcessor.AddRequestTask(follower_id, trade->symbol, cmd, vol, comment, 0, 0, record.order, TS_CLOSED_NORMAL);
					/*	if (ExtProcessor.processing_login.find(follower_id) == ExtProcessor.processing_login.end()) {
							ExtProcessor.askLPtoCloseTrade(follower_id, record.order, cmd, trade->symbol, comment, vol);

						}
						else {
							ExtProcessor.AddRequestTask(follower_id, trade->symbol, cmd, vol, comment, 0, 0, record.order, TS_CLOSED_NORMAL);

						}*/

				}
			}

			// handle master open and close order at the same time issue
			if (find_order == false) {
				ExtProcessor.AddToQuickCloseQueue(follower_id, trade);
			}


			ExtProcessor.LOG(CmdTrade, "LifeByte::close", "LifeByte:: order %d, find %d,  ", order, find_order);
			HEAP_FREE(records);
		}
		/*	else {
				MyIOCP* iocp = ExtProcessor.pool->GetConnection();
				if (iocp == NULL) {
					continue;
				}
				iocp->closeOrderRequest(task->follower_server_id, task->follower_id, trade->order, vol);
			}*/
	}




	if (trade != NULL) {
		delete trade;
		trade = NULL;
	}
	man->isWorking = false;;
	man->clearTask(m_task);
	LeaveCriticalSection(&m_cs);
	return 0;
}

void CProcessor::AddToQuickCloseQueue(int follower_id, MyTrade  trade) {

	TaskManagement* man = TaskManagement::getInstance();
	MyTrade* tmp = new MyTrade();
	tmp->order = trade.order;
	tmp->cmd = trade.cmd;
	tmp->login = trade.login;
	tmp->tp = trade.tp;
	tmp->sl = trade.sl;
	tmp->volume = trade.volume;
	COPY_STR(tmp->symbol, trade.symbol);

	man->addToCloseOrder(follower_id, tmp);
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

void CProcessor::AddRequestTask(int login, string symbol, int cmd, int volume, string comment, double tp, double sl, int order, int type) {
	m_ContextLock.Lock();
	RequestTask* task = new RequestTask();
	task->cmd = cmd;
	task->comment = comment;
	task->login = login;
	task->order = order;
	task->sl = sl;
	task->symbol = symbol;
	task->tp = tp;
	task->volumeInCentiLots = volume;

	if (this->request_current_id > 60000 || this->request_current_id < 0) {
		this->request_current_id = 0;
	}
 


	task->task_id = to_string(this->request_current_id++);
	task->type  = type;
	this->request_task.push_back( task);



	m_ContextLock.UnLock();
}

 

 
 

void CProcessor::processingDeadRequest() {
	if (ExtServer==NULL) {
		return;
  }


 

 	m_ContextLock.Lock();
	int key = 0;
	RequestInfo reqs[512];
	int max = 512;
	int total = ExtServer->RequestsGet(&key, reqs, max);
	list<RequestTask*>::iterator it;

	RequestTask* task = NULL;
	for (  it = this->request_task.begin(); it != this->request_task.end();it++) {
		task = *it;
		int login = task->login;

		bool find = false;
	 
	
		 
		/*for (int i = 0; i < total && i < max; i++) {
			RequestInfo req = reqs[i];
		 

			if (req.login==login && req.status != DC_EMPTY) {
				find = true;
				 
			}

		}*/


		if (find == false ) {
		 
			if (task->type == TS_OPEN_NORMAL) {
				this->askLPtoOpenTrade(task->login, task->symbol, task->cmd, task->volumeInCentiLots, task->comment, task->tp, task->sl);

				break;
			}
			else if (task->type == TS_CLOSED_NORMAL) {
				this->askLPtoCloseTrade(task->login, task->order, task->cmd, task->symbol, task->comment, task->volumeInCentiLots);


				break;
			}

		}
	}
	if (it!=this->request_task.end()) {
		if (task != NULL) {
			delete task;
			task = NULL;
		}
		this->request_task.erase(it);
	}
	 
 	m_ContextLock.UnLock();

	 
}
