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

#define CMD_QUERY_ALL_TASK 950
#define CMD_QUERY_START_REC_TASK 951
#define CMD_QUERY_FINISH_REC_TASK 952
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
	VOID MyIOCP::SendInitTask();
	void MyIOCP::startRecTaskData(string data);
	void MyIOCP::finishRecTaskData(string data);
};

