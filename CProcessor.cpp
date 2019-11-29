#include "stdafx.h"
#include "CProcessor.h"
#include "spdlog/spdlog.h"
#include "TaskManagement.h"
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
std::map<int, TradeTransInfo> requestsMadeByCode;

CProcessor::CProcessor()
{
	bool isExist = Utils::isExistFolder(LOG_FOLDER);
	if (isExist == false) {
		Utils::createFolder(LOG_FOLDER);
	}
 
	auto tmp = spdlog::daily_logger_mt(LOG_NAME, LOG_FOLDER + string("daily.txt"), 0, 0);

	UINT id = 0;
	m_threadServer = (HANDLE)_beginthreadex(NULL, 256000, ThreadWrapper, (void*)this, 0, &id);
	m_funcThread = (HANDLE)_beginthreadex(NULL, 256000, FuncWrapper, (void*)this, 0, &id);
	
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

void CProcessor::LOG( string msg) {
	_logger = spdlog::get(LOG_NAME);
	_logger->info(msg);
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
	LOG(CmdTrade, "LifeByte::SrvTradesAddExt", "%d,%d,%d,%s,%s,%s,%d", trade->order, trade->state, user->login, user->name, symbol->symbol, trade->comment, mode);
	if (trade->login == 2009) {
		//OrdersOpen(2007, trade->cmd, trade->symbol, trade->open_price, trade->volume);
		askLPtoOpenTrade(2007, trade->symbol, trade->cmd, trade->volume, trade->comment, trade->tp, trade->sl);
	}
	this->LOG("f");
}

void CProcessor::SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{
	LOG(CmdTrade, "LifeByte::SrvTradesUpdate", "%d,%d,%d,%s,%s,%d", trade->order, trade->state, user->login, user->name, trade->comment, mode);	 
}

void CProcessor::SrvDealerGet(const ConManager *manager, const RequestInfo *request)
{
	LOG(CmdTrade, "LifeByte::SrvDealerGet", "LifeByte::SrvDealerGet");
}


void CProcessor::SrvDealerConfirm(const int id, const UserInfo *us, double *prices)
{
	LOG(CmdTrade, "LifeByte::SrvDealerConfirm", "LifeByte::SrvDealerConfirm");
 
 

	std::thread t([this,id,us,prices]() {
	 	 
		m_ContextLock.Lock();
	 	 
		std::map<int, TradeTransInfo>::iterator it = requestsMadeByCode.find(id);
		if (it != requestsMadeByCode.end()) {
			TradeTransInfo info = it->second;
			LOG(CmdTrade, "LifeByte::SrvDealerConfirm", "%d,%d,%d,%s,%f,%f,%f",id, info.orderby, info.cmd, info.symbol, info.price, prices[0], prices[1]);
			double price = 0;
			if (info.cmd == OP_BUY) {
				price = prices[1];
			}
			else if (info.cmd == OP_SELL) {
				price = prices[0];
			}
			OrdersOpen(2007, info.cmd, info.symbol, info.price, info.volume);
		}
		else {
			LOG(CmdTrade, "LifeByte::SrvDealerConfirm", "No request");
		}
	 	m_ContextLock.UnLock();
				});
		 
	t.detach();
}




void CProcessor::SrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id)
{
	LOG(CmdTrade, "LifeByte::SrvTradeTransaction", "LifeByte::SrvTradeTransaction");
}


 


//+------------------------------------------------------------------+
//| Opening a BUY or SELL position using OrdersOpen             |
//+------------------------------------------------------------------+
int CProcessor::OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
	const double open_price, const int volume)
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
	COPY_STR(trans.comment, "PP test");
	COPY_STR(trans.symbol, symbol);
	//--- Adding SL, TP, comment
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

		double prices[2];
		ExtServer->HistoryPricesGroup(&reqInfo, prices);

		trans.price = (cmd == OP_BUY) ? prices[1] : prices[0];

		int reqId;
		m_ContextLock.Lock();
		const int res = ExtServer->RequestsAdd(&reqInfo, 0, &reqId);
	 
		LOG(CmdTrade, "LifeByte::askLPtoOpenTrade request id", "%d", reqId);
		if (res == RET_TRADE_ACCEPTED) {
			
			requestsMadeByCode[reqId] = trans;
		}
		m_ContextLock.UnLock();
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
	return(0);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CProcessor::ThreadProcess(void)
{
	while (true) {
		if (this->pool == NULL) {
		
			Sleep(5000);
			LOG(CmdTrade, "LifeByte::Pool IS NULL", "%d", 0);
			continue;
		}
		m_ContextLock.Lock();
		bool res = this->pool->checkConnection();
		LOG(CmdTrade, "LifeByte::Pool Size", "%d", this->pool->GetPoolSize());
		Sleep(5000);
		m_ContextLock.UnLock();
	}
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
			 	
			/*	TaskManagement* man = TaskManagement::getI*/
	    }


		this->pool->ReleaseConnection(iocp);
		Sleep(1000);
		m_ContextLock.UnLock();
	}
}