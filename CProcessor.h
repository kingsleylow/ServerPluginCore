#pragma once
 
#include <map>

#include "spdlog/sinks/daily_file_sink.h"
#include "Threadpool.h"
#include "SocketConnectionPool.h"
#include <thread>
#include "executePool.h"
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
	 
	int UserInfoGet(const int login, UserInfo *info);
	void askLPtoOpenTrade(int login, const std::string& symbol, int cmd, int volumeInCentiLots, const std::string& comment, double tp, double sl);
	void setupSocket();
	void LOG(string msg); 
	void LOG(const int code, LPCSTR ip, LPCSTR msg, ...) const;
	void CProcessor::Initialize(void);
	SocketConnectionPool* pool;
private:
	//---- out to server log

	int OrdersOpen(const int login, const int cmd, LPCTSTR symbol,
		const double open_price, const int volume);

	IOCPMutex m_ContextLock;
 

	std::shared_ptr<spdlog::logger> _logger;
            
	char              m_ip[128];       //ip
	char              m_port[32];   // port
	char              m_ip_backup[128];       //ip
	char              m_port_backup[32];   // port
	char			  m_key[64];   // key
	char			  m_key_backup[64];   // key
	char	          m_plugin_id[32]; // plugin id
	HANDLE            m_threadServer;    // thread handle
	HANDLE            m_funcThread;    // thread handle
protected:
	virtual void      ThreadProcess(void);
	static UINT __stdcall ThreadWrapper(LPVOID pParam);


	virtual void      FuncProcess(void);
	static UINT __stdcall FuncWrapper(LPVOID pParam);
};
extern CProcessor ExtProcessor;
