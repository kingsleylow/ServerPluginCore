#pragma once
#include "StdAfx.h"
#include "IOCPCommon.h"
#include "IOCPBuffer.h"
#include "IOCPBufferWriter.h"
#include "IOCPBufferReader.h"
#include "BaseIOCPServer.h"
#include "PackageIOCPClient.h"
#include "TextIOCPClient.h"
#include "TextIOCPServer.h"

#include "TaskManagement.h"
#define PARTNET_KEY "M7DyRzGaTBHJn8gWnaY7GF3VewbfEn2Z"
#define ADM_LEVEL 1
#define NOR_LEVEL 2
#define NO_RIGHT -1

#define CMD_INVALID -1
#define HEART_BEAT_TIMEOUT 500
#define ADM_PASSWORD "7CYfHAZaxeys"

#define CMD_PLUGIN_AUTH 123
#define CMD_HEART_BEAT 143

#define CMD_QUERY_PORTAL_IDS 926
#define CMD_QUERY_GOD_PORTFOLIO_MULTIPLE 913

#define CMD_QUERY_ALL_TASK 950  //first step
#define CMD_QUERY_START_REC_TASK 951
#define CMD_QUERY_FINISH_REC_TASK 952

#define CMD_QUERY_OPEN_ORDER 953
#define CMD_QUERY_CLOSE_ORDER 954


#define CMD_OPEN_ORDER 1000
#define CMD_CLOSE_ORDER 1001


#define CMD_REQUEST_TEST 1002
#define CMD_ORDER_TEST 1003

#define SERVER_ID "server_id"
#define LOGIN "login"
#define SYMBOL "symbol"
#define VOLUMN "vol"
#define CMD "cmd"
#define COMMENT "comment"
#define ORDER "order"
#define MODE "mode"
#define STATE "state"
class MyIOCP :
	public CTextIOCPClient
{
public:
	MyIOCP();
	 ~MyIOCP();
	 int m_heart_count;
	 int level;

	 //bool receiveFlag;
	 //std::string receive_data;
	virtual VOID NotifyConnectionStatus(IOCPClient_ConnectionType ConnectionType);
	VOID NotifyReceivedFormatPackage(const char* lpszBuffer);
	VOID MyIOCP::SendInitailRequest();
	VOID SendHeartBeat();
	nlohmann::json MyIOCP::GetJson(int cmd, nlohmann::json);
	void MyIOCP::checkConnection();
	void MyIOCP::SendRequest(int cmd, nlohmann::json data);
 
	int MyIOCP::getCommand(string data);
	nlohmann::json MyIOCP::getData(string data, string key);



	void MyIOCP::checkLogin(string data);
	void MyIOCP::refreshTaskData(string data);
	VOID MyIOCP::SendInitTask(int server_id);
	void MyIOCP::startRecTaskData(string data);
	void MyIOCP::finishRecTaskData(string data);
	void MyIOCP::openOrderRequest(const int server_id, const string& login, const string& symbol,
		const int cmd, const int vol, const string& comment, const int mode);
	void MyIOCP::closeOrderRequest(const int server_id, const string& login, const int order,   const int volumeInCentiLots,
		const string symbol,const int cmd,const int mode,const int state);
	void MyIOCP::openOrderRequest(string data);
	void MyIOCP::closeOrderRequest(string data);
	void MyIOCP::testRequest(string data);
	void MyIOCP::testOrder(string data);

};

