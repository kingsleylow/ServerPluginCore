#include "stdafx.h"
#include "SocketConnectionPool.h"
#include "CProcessor.h"

SocketConnectionPool::SocketConnectionPool()
{
	this->key = "";
	this->port = 0;
	this->server = "";
	this->current_size = 0;
	this->port_backup = 0;
	this->server_backup = "";
}


SocketConnectionPool::~SocketConnectionPool()
{
	this->DestoryConnPool();
}
// true : need to restart pool
//false: no need to

bool SocketConnectionPool::checkParams(string server, int port, string key) {
	m_ContextLock.Lock();
	if (this->server.empty() || this->port == 0 || this->key.empty()) {
		m_ContextLock.UnLock();
		return true;
	}

	if (server.compare(this->server) == 0 && port == this->port && key.compare(this->key)==0 ) {
		m_ContextLock.UnLock();
		return false;
	}
	m_ContextLock.UnLock();
	return true;
}



void SocketConnectionPool::setParams(string server, int port, string key) {
	m_ContextLock.Lock();
	this->server = server;
	this->port = port; 
	this->key = key;
	m_ContextLock.UnLock();
}

void SocketConnectionPool::initPool(int size) {
	if (this->server.empty() || this->port==0 || this->key.empty()) {
		return;
	}
	this->DestoryConnPool();

	MyIOCP* iocp;

	m_ContextLock.Lock();
	for (int i = 0; i < size; i++)
	{
		iocp = this->CreateConnection();

		if (iocp != NULL)
		{
			pool.push_back(iocp);
			++(this->current_size);
		}
		else
		{
			ExtProcessor.LOG("create conn error");
		}

	}
	m_ContextLock.UnLock();
}

MyIOCP* SocketConnectionPool::CreateConnection() {

	ExtProcessor.LOG( true,"CreateConnection");
	MyIOCP* iocp = new MyIOCP();
	try
	{
		char* server = new char(this->server.length() + 1);
		strcpy(server, (this->server).c_str());
		if (iocp != NULL && iocp->m_bIsConnected == FALSE) {
			BOOL res = iocp->Connect(server, (this->port));

			if (res == TRUE) {
				return iocp;
			}

		}
		if (iocp != NULL) {
			delete iocp;
			iocp = NULL;
		}
	}
	catch (const std::exception&)
	{
		return NULL;
	}

	 

	return NULL;
}

MyIOCP* SocketConnectionPool::GetConnection() {
	m_ContextLock.Lock();
	MyIOCP* iocp = NULL;
	if (pool.size() > 0)//the pool have a conn 
	{
		iocp = pool.front();
		pool.pop_front();//move the first conn 
		if (iocp == NULL) {
			m_ContextLock.UnLock();
			return NULL;
		}
		if (iocp->m_bIsConnected == FALSE) {
			if (iocp != NULL) {
				delete iocp;
				iocp = NULL;

				//man = NULL;
				iocp = this->CreateConnection();
			}

			if (iocp == NULL )
			{
				--this->current_size;
			}
		}
		m_ContextLock.UnLock();

		return iocp;
	}
	else
	{
		if (this->current_size <  POOL_MAX)//the pool no conn
		{
			iocp = this->CreateConnection();
			if (iocp != NULL)
			{
				 
				++this->current_size;
				m_ContextLock.UnLock();
				return iocp;
			}
			else
			{
				m_ContextLock.UnLock();
				return NULL;
			}
		}
		else //the conn count > maxSize
		{
			m_ContextLock.UnLock();
			return NULL;
		}
	}
}

void SocketConnectionPool::ReleaseConnection(MyIOCP* iocp)
{
	if (iocp !=NULL )
	{
		m_ContextLock.Lock();
		pool.push_back(iocp);
	 
		m_ContextLock.UnLock();

	}
	else {
		ExtProcessor.LOG(true,"ReleaseConnection");
		m_ContextLock.Lock();
		 
		if (iocp != NULL) {
			delete iocp;
			iocp = NULL;
		}
		--this->current_size;
		m_ContextLock.UnLock();
	}

}

bool SocketConnectionPool::checkConnection() {

	
	MyIOCP* iocp = GetConnection();

	if (iocp == NULL) {
		ReleaseConnection(iocp);
		return  false;
	}

	if (iocp->m_heart_count > MAX_BEAT_HEART_COUNT) {
		ExtProcessor.LOG(true,"m_heart_count false" );
		DestoryConnection(iocp);
		return false;
	}
	iocp->m_heart_count++;

	if (iocp->m_bIsConnected == FALSE) {
		ExtProcessor.LOG(true,"m_bIsConnected false"  );
		ReleaseConnection(iocp);
		return  false;
	}

	iocp->checkConnection();
	 
	ReleaseConnection(iocp);

	reNewPool();

	return true;
 }

bool SocketConnectionPool::reNewPool() {
	bool result = false;

	if (this->pool.size() < MAN_POOL_SIZE)//the pool no conn
	{
		m_ContextLock.Lock();
		MyIOCP* iocp = this->CreateConnection();
		if (iocp != NULL)
		{

			this->current_size = this->pool.size();

			ReleaseConnection(iocp);
			result = true;
		}
		else {
			DestoryConnection(iocp);
		}
		m_ContextLock.UnLock();
	}
	return result;
}

void SocketConnectionPool::DestoryConnPool()
{
	list<MyIOCP* >::iterator iter;
	m_ContextLock.Lock();
	for (iter = pool.begin(); iter != pool.end(); ++iter)
	{
		this->DestoryConnection(*iter);
	}
	current_size = 0;
	pool.clear();
	m_ContextLock.UnLock();
}

void SocketConnectionPool::DestoryConnection(MyIOCP* iocp) {
	if (iocp != NULL)
	{
		delete iocp;
		iocp = NULL;		 
	}
}

int SocketConnectionPool::GetPoolSize() {
	return this->pool.size();
}