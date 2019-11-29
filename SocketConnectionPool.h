#pragma once
#include "StdAfx.h"

 
#include "MyIOCP.h"

#define POOL_MAX 4

#define MAN_POOL_SIZE POOL_MAX/2

#define MAN_POOL_SIZE_INIT POOL_MAX/4

#define MAX_BEAT_HEART_COUNT 2

 
class SocketConnectionPool
{
public:
	string server;
	int port;
	string server_backup;
	int port_backup;
	string key;
	IOCPMutex m_ContextLock;
	list<MyIOCP*> pool;
	int current_size;
public:
	SocketConnectionPool();
	~SocketConnectionPool();
	void SocketConnectionPool::setParams(string server, int port, string key);
	void SocketConnectionPool::initPool(int size);
	MyIOCP* SocketConnectionPool::CreateConnection();
	MyIOCP* SocketConnectionPool::GetConnection();
	void SocketConnectionPool::ReleaseConnection(MyIOCP* iocp);
	bool SocketConnectionPool::checkConnection();
	bool SocketConnectionPool::reNewPool();
	void SocketConnectionPool::DestoryConnPool();
	void SocketConnectionPool::DestoryConnection(MyIOCP* iocp);
	int SocketConnectionPool::GetPoolSize();
	bool SocketConnectionPool::checkParams(string server, int port, string key);
};

