#pragma once
 
#include <map>

 
#include "Threadpool.h"
#include "SocketConnectionPool.h"
#include <thread>
#include "executePool.h"
#define TIME_RATE ((double)1.6777216)
#define OURTIME(stdtime) ((DWORD)((double)(stdtime)/TIME_RATE))
#define CORE_IP "IP"
#define CORE_BACKUP_IP "BACKUP_IP"
#define DEFAULT_IP "0"
#define CORE_PORT "PORT"
#define CORE_BACKUP_PORT "BACKUP_PORT"
#define DEFAULT_PORT "0"
#define REQUEST_SLEEP "REQUEST_SLEEP"
#define REQUEST_BUFFER "REQUEST_BUFFER"
#define REQUEST_BUSY_MAX "REQUEST_BUSY_MAX"
#define REQUEST_SLEEP_DEFAULT "100"
#define REQUEST_BUFFER_DEFAULT "100"
#define REQUEST_BUSY_MAX_DEFAULT "200"

#define CORE_KEY "KEY"
#define CORE_KEY_BACKUP "BACKUP_KEY"
#define DEFAULT_KEY ""
#define PLUGIN_ID "PLUGIN_ID"
#define DEFAULT_PLUGIN_ID "0"
#define D_PLUGIN_ID 0


#define REQUEST_TASK_BUSY   200
#define REQUEST_TASK_PRE  100
#define REQUEST_TASK_WAIT   100


#define SYMBOL_SEPARATOR "SYMBOL SEPARATOR"
#define SYMBOL_SEPARATOR_POSITION "SYMBOL SEPARATOR POSITION"
#define SYMBOL_SEPARATOR_DEFAULT "."

#define SYMBOL_SEPARATOR_POSITION_PRE "0"
#define SYMBOL_SEPARATOR_POSITION_SUF "1"
#define SYMBOL_SEPARATORPOSITION_DEFAULT SYMBOL_SEPARATOR_POSITION_PRE
struct RequestMetaData {
	TradeTransInfo info;
	int login;
	int order; // user for close order
};
struct RequestTask {
	int login;
	string symbol;
	int cmd;
	int volumeInCentiLots;
	string comment;
	double tp;
	double sl;

	int order;// close
	int type;
	string task_id;
};
class CProcessor
{
public:
	CProcessor();
	~CProcessor();
	//trade hooks
	bool CProcessor::findCloseOrderAndAskClose(int master_order, int trade_state, int follower_id, int vol ,int cmd  );
	void SrvTradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol);
	void SrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode);
	void SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode);
 
	void SrvDealerGet(const ConManager *manager, const RequestInfo *request);
	void SrvDealerConfirm(const int id, const UserInfo *us, double *prices);
	void SrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id);
	 
	int CProcessor::UserInfoGet(const int login, UserInfo *info);
	void CProcessor::askLPtoCloseTrade(int login, int order, int cmd,string symbol, string comment,int volumeInCentiLots);
	void CProcessor::askLPtoOpenTrade(int login,   std::string symbol, int cmd, int volumeInCentiLots, const std::string& comment, double tp, double sl);
	void CProcessor::setupSocket();
	void CProcessor::LOG(bool debug = false,string msg = "", ... ) const;
	void CProcessor::LOG(const int code, LPCSTR ip, LPCSTR msg, ...) const;
	void CProcessor::Initialize(void);
	SocketConnectionPool* pool;
	std::threadpool excutor{0};
	IOCPMutex m_ContextLock;
	int				  plugin_id;
	int request_sleep_time;
	int request_buffer;
	int request_busy_size;
	string symbol_seperator;
	int symbol_seperator_position;
	int OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
		const double open_price, const int volum, const string comment);
	static UINT __cdecl  CProcessor::open_order_worker_thread(void* param);
	static UINT __cdecl  CProcessor::close_order_worker_thread(void* param);
 
	void  CProcessor::ExternalOpenOrder(int server_id,int login,string symbol,int vol,int cmd,string comment,int mode,string task_id);
	void  CProcessor::ExternalCloseOrder(int server_id, int login, string symbol, int vol, int cmd, string comment, int mode, int order, string task_id);
private:
	friend class MyIOCP;
	//---- out to server log
	int CProcessor::UpdateComment(const int order, const string comment);
	int CProcessor::OrdersClose(const int order,   const int volume, const double close_price,   const string comment);
 
	void CProcessor::HandlerAddOrder(MyTrade *trade, const UserInfo *user, const ConSymbol *symbol, const int mode);
	void CProcessor::HandlerCloseOrder(MyTrade *trade, UserInfo *user, const int mode);
 
	//bool CProcessor::ActionCheck(const int order,  const int login, const double price);
 
 
	static UINT __cdecl confirm_order_worker_thread(void* param);

	char              m_ip[128];       //ip
	char              m_port[32];   // port
	char              m_ip_backup[128];       //ip
	char              m_port_backup[32];   // port
	char			  m_key[64];   // key
	char			  m_key_backup[64];   // key
	char	          m_plugin_id[32]; // plugin id

	char	          m_request_sleep[32]; // sleep time
	char	          m_reqeust_buffer[32]; // send task once time
	char	          m_request_busy[32]; // plugin busy size

	char              m_seperator[32];
	char              m_seperator_position[32];

	HANDLE            m_threadServer;    // thread handle
	HANDLE            m_threadHandlerRequest;    // thread handle
	HANDLE            m_funcThread;    // thread handle
	int request_current_id;
	list<RequestTask*> request_task;
	 
	set<int> processing_login;
	std::map<int, RequestMetaData> requestsMadeByCode;

protected:
	virtual void      RequestProcess(void);
	virtual void      ThreadProcess(void);
	static UINT __stdcall ThreadWrapper(LPVOID pParam);
	virtual void      ThreadProcesRequest(void);
	virtual void      ThreadProcesHandlerRequest(void);
	static UINT __stdcall ThreadWrapperRequest(LPVOID pParam);
	static UINT __stdcall CProcessor::HandeleThreadWrapper(LPVOID pParam);
 
	static UINT __stdcall ThreadWrapperHanderRequest(LPVOID pParam);
 
	bool AddOpenRequestToQueue(int login, string symbol,int cmd,int vol, string comment);
	bool AddCloseRequestToQueue(int login, int order, int cmd, string symbol, string comment, int vol);
	void CheckRequest();
	bool CheckInQueue(int login);
	RequestTask*  getRequestTask();
	bool CProcessor::CheckBusyQueue();
	bool CProcessor::getNewSymbol(string old_symbol, string group, string& new_symbol);
	bool  CProcessor::checkSymbolIsEnable(string symbol, string group);
};
extern CProcessor ExtProcessor;