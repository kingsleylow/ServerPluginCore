#include "Stdafx.h"
#include "MyIOCP.h"
#include "CProcessor.h"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

MyIOCP::MyIOCP()
{
	this->m_heart_count = 0;
	this->level = NO_RIGHT;

 
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

MyIOCP::~MyIOCP()
{
	this->level = NO_RIGHT;
	this->m_heart_count = 0;
	


}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

VOID MyIOCP::NotifyReceivedFormatPackage(const char* lpszBuffer)
{
	 
	string tmp = lpszBuffer;
	int cmd = this->getCommand(tmp);
//	string data = this->getData( tmp, "data");
	if (cmd!= CMD_HEART_BEAT&&cmd!= CMD_PLUGIN_AUTH) {
//		ExtProcessor.LOG(false,tmp);
	}
	this->m_heart_count = 0;
	switch (cmd)
	{
	case CMD_PLUGIN_AUTH:
		this->checkLogin(tmp);
		break;
	case CMD_HEART_BEAT:
	
	 
		break;
	case CMD_QUERY_PORTAL_IDS:

		break;

	//case CMD_QUERY_GOD_PORTFOLIO_MULTIPLE:

	//	break;
	case CMD_QUERY_START_REC_TASK:
		this->startRecTaskData("");
		break;
	case CMD_QUERY_FINISH_REC_TASK:
		
		this->refreshTaskData(tmp);
		this->finishRecTaskData("");
		break;
	case CMD_QUERY_ALL_TASK:
		this->refreshTaskData(tmp);
		break;

	case CMD_OPEN_ORDER:
		this->openOrderRequest(tmp);
		break;
	case CMD_CLOSE_ORDER:
		this->closeOrderRequest(tmp);
		break;

	case CMD_REQUEST_TEST:
		this->testRequest(tmp);
		break;

	case CMD_ORDER_TEST:
		this->testOrder(tmp);
		break;
	default:
		break;
	}
	//this->receive_data = data;
	//this->receiveFlag = true;
	
}

void MyIOCP::testRequest(string data) {
	try {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains("num")) {
			return;
		}
		int num = j["data"]["num"];
		for (int i = 0; i < num; i++) {
			ExtProcessor.askLPtoOpenTrade(2000, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3000, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3001, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3002, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3003, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3004, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
			ExtProcessor.askLPtoOpenTrade(3005, "EURUSD", OP_BUY, 1, "place Test", 0, 0);
		}
	}
	catch (exception& e) {


	}
}
void MyIOCP::testOrder(string data) {
	try {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains("num")) {
			return;
		}
		int num = j["data"]["num"];
		for (int i = 0; i < num; i++) {
			ExtProcessor.OrdersOpen(2000, OP_BUY, "EURUSD", 1.0, 100, "open test");

		}
	}
	catch (exception& e) {


	}
}


//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

VOID MyIOCP::NotifyConnectionStatus(IOCPClient_ConnectionType ConnectionType) {
	if (ConnectionType == IOCPClient_ConnectionType_Connected) {
		this->m_heart_count = 0;
	}
	else if(ConnectionType == IOCPClient_ConnectionType_Disconnected){
 
	}
	else if (ConnectionType == IOCPClient_ConnectionType_ConnectFailed) {

 
	}	
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

VOID MyIOCP::SendInitTask(int server_id) {
	nlohmann::json data = {
	{"partner_key",PARTNET_KEY},
	{SERVER_ID,server_id},
	};
	nlohmann::json j = this->GetJson(CMD_QUERY_ALL_TASK, data);

	Send((j.dump()).c_str());
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

VOID MyIOCP::SendInitailRequest() {
	string loginTime = "";
	string userName = "";
	string devId = "";
	string text = ADM_PASSWORD + loginTime + userName + devId;
		 

	string ciphertext = Utils::MD5Crypt(text);
	nlohmann::json data = { 
		{"DevId",devId},
	{"LoginTime",loginTime} ,
	{"Token",ciphertext} ,
	{"UserName",userName}
	 
	};



	nlohmann::json j = this->GetJson(CMD_PLUGIN_AUTH, data);

	Send((j.dump()).c_str());
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

VOID MyIOCP::SendHeartBeat() {

	SendRequest(CMD_HEART_BEAT, NULL);
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

nlohmann::json MyIOCP::GetJson(int cmd, nlohmann::json data) {
	nlohmann::json j = { {"cmd",cmd},{"data",data} };
	return j;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::checkConnection() {




	if (this->level == NO_RIGHT) {
		this->SendInitailRequest();
	}
	else {
		this->SendHeartBeat();
	}
	
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::SendRequest(int cmd, nlohmann::json data) {
	nlohmann::json j = this->GetJson(cmd, data);

	Send((j.dump()).c_str());
}


 ///receive
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

int MyIOCP::getCommand(string data) {
	int cmd = CMD_INVALID;
	try {
		nlohmann::json j = nlohmann::json::parse(data);
		if (j.contains("cmd")) {
			cmd = j["cmd"];
		}
	}
	catch (exception& e) {
 

	}

	return cmd;
 }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

nlohmann::json MyIOCP::getData(string data,string key) {
	try {
	nlohmann::json j = nlohmann::json::parse(data);
	if (j.contains(key)) {
		return   j[key];
	}

	}
	catch (exception& e) {


	}
	return NULL;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void MyIOCP::openOrderRequest(string data) {
	try {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains("num")) {
			return;
		}
		int num = j["data"]["num"];
		for (int i = 0; i < num;i++) {
			ExtProcessor.askLPtoOpenTrade(2000, "EURUSD", OP_BUY, 1, "My Test", 0, 0);
		
		}
	}
	catch (exception& e) {


	}

}
extern CServerInterface *ExtServer;
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void MyIOCP::closeOrderRequest(string data) {
	UserInfo info = { 0 };
	int total = 0;
	if (ExtProcessor.UserInfoGet(2000, &info) == FALSE)
		return;
	TradeRecord* records = ExtServer->OrdersGetOpen(&info, &total);
	for (int i = 0; i < total; i++) {
		TradeRecord record = records[i];
		 
	 
		ExtProcessor.askLPtoCloseTrade(2000, record.order, record.cmd, record.symbol, "", record.volume);
		 
	}
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::checkLogin(string data) {
	try {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains("Level")) {
			return;
		}
		this->level = j["data"]["Level"];
		}
	catch (exception& e) {


	}
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::startRecTaskData(string data) {
//	ExtProcessor.LOG(false, "startRecTaskData");
	TaskManagement* man = TaskManagement::getInstance();
	man->startInit();
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::finishRecTaskData(string data) {
//	ExtProcessor.LOG(false, "finishRecTaskData");
	TaskManagement* man = TaskManagement::getInstance();
	man->finishInit();
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::refreshTaskData(string data) {

	TaskManagement* man = TaskManagement::getInstance();
	if (man->initialTask!= INITIAL_REC_BUFF) {
		return;
	}
//	ExtProcessor.LOG(false, "refreshTaskData");
	bool res = man->inital(data);
	
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::openOrderRequest(const int server_id, const string& login, const string& symbol,
	const int cmd, const int vol, const string& comment) {
		nlohmann::json data = {
		{SERVER_ID,server_id},
		{LOGIN,login} ,
		{SYMBOL,symbol} ,
		{CMD,cmd},
		{COMMENT,comment},
			{VOLUMN,vol},
		};



	nlohmann::json j = this->GetJson(CMD_QUERY_OPEN_ORDER, data);
	ExtProcessor.LOG(false, (j.dump()).c_str());
	Send((j.dump()).c_str());
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::closeOrderRequest(const int server_id, const string& login, const int order,   const int volumeInCentiLots) {
	 
	nlohmann::json data = {
	{SERVER_ID,server_id},
	{LOGIN,login} ,
	{ORDER,order},
    {VOLUMN,volumeInCentiLots},
	};



	nlohmann::json j = this->GetJson(CMD_QUERY_CLOSE_ORDER, data);
	ExtProcessor.LOG(false, (j.dump()).c_str());
	Send((j.dump()).c_str());
}


