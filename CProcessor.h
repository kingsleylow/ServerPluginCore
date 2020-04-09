#pragma once
 
#include <map>

 
#include "Threadpool.h"
#include "SocketConnectionPool.h"
#include <thread>
#include "executePool.h"
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
	void SrvTradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol);
	void SrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode);
	void SrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode);
	void SrvTradeRequestApply(RequestInfo *request, const int isdemo);
	void SrvDealerGet(const ConManager *manager, const RequestInfo *request);
	void SrvDealerConfirm(const int id, const UserInfo *us, double *prices);
	void SrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id);
	 
	int CProcessor::UserInfoGet(const int login, UserInfo *info);
	void CProcessor::askLPtoCloseTrade(int login, int order, int cmd,string symbol, string comment,int volumeInCentiLots);
	void CProcessor::askLPtoOpenTrade(int login, const std::string& symbol, int cmd, int volumeInCentiLots, const std::string& comment, double tp, double sl);
	void CProcessor::setupSocket();
	void CProcessor::LOG(bool debug = false,string msg = "", ... ) const;
	void CProcessor::LOG(const int code, LPCSTR ip, LPCSTR msg, ...) const;
	void CProcessor::Initialize(void);
	SocketConnectionPool* pool;
	std::threadpool excutor{0};
	IOCPMutex m_ContextLock;
	int				  plugin_id;
	int OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
		const double open_price, const int volum, const string comment);
private:
	//---- out to server log
	int CProcessor::UpdateComment(const int order, const string comment);
	int CProcessor::OrdersClose(const int order,   const int volume, const double close_price,   const string comment);


	void CProcessor::HandlerAddOrder(MyTrade *trade, const UserInfo *user, const ConSymbol *symbol, const int mode);
	void CProcessor::HandlerCloseOrder(MyTrade *trade, UserInfo *user, const int mode);
	void CProcessor::HandlerActiveOrder(MyTrade *trade, UserInfo *user, const int mode);
	//bool CProcessor::ActionCheck(const int order,  const int login, const double price);
	void CProcessor::HandleQuickCloseIssue(int login, int order);
	void CProcessor::AddToQuickCloseQueue(int follower_id, MyTrade* trade);
	static UINT __cdecl order_worker_thread(void* param);
	static UINT __cdecl add_order_worker_thread(void* param);
	static UINT __cdecl close_order_worker_thread(void* param);
	static UINT __cdecl CProcessor::open_order_worker_thread(void* param);
	static UINT __cdecl CProcessor::direct_close_order_worker_thread(void* param);
	char              m_ip[128];       //ip
	char              m_port[32];   // port
	char              m_ip_backup[128];       //ip
	char              m_port_backup[32];   // port
	char			  m_key[64];   // key
	char			  m_key_backup[64];   // key
	char	          m_plugin_id[32]; // plugin id

	HANDLE            m_threadServer;    // thread handle
	HANDLE            m_funcThread;    // thread handle
	int request_current_id;
	list<RequestTask> request_task;
	list<RequestTask> buffer;
	set<int> processing_login;
	std::map<int, RequestMetaData> requestsMadeByCode;

protected:
	virtual void      ThreadProcess(void);
	static UINT __stdcall ThreadWrapper(LPVOID pParam);
	virtual void      ThreadProcesRequest(void);
	static UINT __stdcall ThreadWrapperRequest(LPVOID pParam);
	void CProcessor::AddRequestTask(int login, string symbol, int cmd, int volume, string comment, double tp, double sl, int order, int type);
	void CProcessor::checkRequestTask();
	int CProcessor::getTaskId();
	bool CProcessor::checkProcessingRequestByLogin(int login);
	bool CProcessor::DealAddRequest(int login, string symbol, int cmd, int volume, string comment, double tp, double sl, int order, int type);
	void CProcessor::checkRequestTaskByLogin(int login);
};
extern CProcessor ExtProcessor;