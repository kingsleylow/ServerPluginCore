#include "Stdafx.h"
#include "MyIOCP.h"
#include "CProcessor.h"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
extern CServerInterface *ExtServer;
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
	 
		break;

	case CMD_REQUEST_ORDER_CROSS:
		this->HandleCrossTrade(tmp);
		break;
	default:
		break;
	}
	//this->receive_data = data;
	//this->receiveFlag = true;
	
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
	 
	   bool is_d = nlohmann::json::accept(data);
	   if (is_d==true) {
		   nlohmann::json j = nlohmann::json::parse(data);

		   if (j.contains("cmd")) {
			   cmd = j["cmd"];
		   }
	   }
	   else {
		   ExtProcessor.LOG(CmdTrade, "LifeByte::getCommand execption ", "LifeByte::getCommand execption");
	   }

	
	 
	//	ExtProcessor.LOG(CmdTrade, "LifeByte::getCommand execption ", "LifeByte::getCommand execption");

	 

	return cmd;
 }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

nlohmann::json MyIOCP::getData(string data,string key) {
// 


	bool is_d = nlohmann::json::accept(data);
	if (is_d == true) {
		nlohmann::json j = nlohmann::json::parse(data);
		if (j.contains(key)) {
			return   j[key];
		}
	}
	else {
		ExtProcessor.LOG(CmdTrade, "LifeByte::getData execption ", "LifeByte::getData execption");
	}
	 
	//

	 
	return NULL;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void MyIOCP::openOrderRequest(string data) {
	bool is_d = nlohmann::json::accept(data);
	if (is_d == true) {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains(CMD)
			|| !j["data"].contains(COMMENT)
			|| !j["data"].contains(LOGIN)
			|| !j["data"].contains(SERVER_ID)
			|| !j["data"].contains(SYMBOL)
			|| !j["data"].contains(VOLUMN)
			|| !j["data"].contains(MODE)
			|| !j["data"].contains(TASK_ID)
			|| !j.contains(ORDER)
			|| !j["data"].contains(STATE)
			) {
			return;
		}
		int login = j["data"][LOGIN];
		int server_id = j["data"][SERVER_ID];
		string symbol = j["data"][SYMBOL];
		int vol = j["data"][VOLUMN];
		int cmd = j["data"][CMD];
		string comment = j["data"][COMMENT];
		int   mode = j["data"][MODE];
		string task_id = j["data"][TASK_ID];
		int state = j[STATE].get<int>();;
		int order = atoi(j[ORDER].get<string>().c_str());;

		ExtProcessor.ExternalOpenOrder(server_id,login,symbol,vol,cmd, comment,mode, task_id,order,state);
	}
 

}
 
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void MyIOCP::closeOrderRequest(string data) {


	bool is_d = nlohmann::json::accept(data);
	if (is_d == true) {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains(CMD)
			|| !j["data"].contains(LOGIN)
			|| !j["data"].contains(SERVER_ID)
			|| !j["data"].contains(TASK_ID)
			|| !j["data"].contains(ORDER)
			|| !j["data"].contains(SYMBOL)
			|| !j["data"].contains(MODE)
			|| !j["data"].contains(STATE)
			|| !j["data"].contains(COMMENT)
			) {
			return;
		}
		int login = j["data"][LOGIN];
		int server_id = j["data"][SERVER_ID];
		int order = j["data"][ORDER];
		int vol = j["data"][VOLUMN];
		int cmd = j["data"][CMD];
		int mode = j["data"][MODE];
		int state = j["data"][STATE];
		string symbol = j["data"][SYMBOL];
		string task_id = j["data"][TASK_ID];
		string comment = j["data"][COMMENT];
  	//ExtProcessor.askLPtoCloseTrade(login,  order,  cmd,  symbol, comment, vol);

 
		ExtProcessor.ExternalCloseOrder(server_id, login, symbol, vol, cmd, comment, mode, order, task_id, state);
	}






	 
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::checkLogin(string data) {
	bool is_d = nlohmann::json::accept(data);
	if (is_d == true) {
		nlohmann::json j = nlohmann::json::parse(data);

		if (!j.contains("data")) {
			return;
		}
		if (!j["data"].contains("Level")) {
			return;
		}
		this->level = j["data"]["Level"];
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
	const int cmd, const int vol, const int& order, const int mode ,const int state, const string comment, const string task_id,const int trade_server) {
		nlohmann::json data = {
		{SERVER_ID,server_id},
		{LOGIN,login} ,
		{SYMBOL,symbol} ,
		{CMD,cmd},
		{COMMENT,comment},
		{VOLUMN,vol},
		{MODE,mode},
		{ORDER,order},
		{STATE,state},
		{TASK_ID,task_id},
		{TRADE_SERVER_ID,trade_server},
		};



	nlohmann::json j = this->GetJson(CMD_OPEN_ORDER, data);
	ExtProcessor.LOG(false, (j.dump()).c_str());
	ExtProcessor.LOG(CmdTrade, "LifeByte::send cross open ",(j.dump()).c_str());
	Send((j.dump()).c_str());
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void MyIOCP::closeOrderRequest(const int server_id, const string& login, const int order,   const int volumeInCentiLots,const string symbol,const int cmd,const int mode,const int state, const string comment, const string task_id, const int trade_server) {
	 
	nlohmann::json data = {
	{SERVER_ID,server_id},
	{LOGIN,login} ,
	{ORDER,order},
    {VOLUMN,volumeInCentiLots},
	{SYMBOL,symbol} ,
	{CMD,cmd} ,
	{MODE,mode} ,
	{STATE,state},
	{COMMENT,comment},
	{TASK_ID,task_id},
			{TRADE_SERVER_ID,trade_server},
	};



	nlohmann::json j = this->GetJson(CMD_CLOSE_ORDER, data);
	ExtProcessor.LOG(false, (j.dump()).c_str());
	ExtProcessor.LOG(CmdTrade, "LifeByte::send cross close ", (j.dump()).c_str());
	Send((j.dump()).c_str());
}

void MyIOCP::RequestCrossTradeRequest() {

	nlohmann::json data = {
	{SERVER_ID,ExtProcessor.plugin_id},
	 
	};

	nlohmann::json j = this->GetJson(CMD_REQUEST_ORDER_CROSS, data);
	ExtProcessor.LOG(false, (j.dump()).c_str());
	Send((j.dump()).c_str());
}



void MyIOCP::HandleCrossTrade(string data) {


	bool is_d = nlohmann::json::accept(data);
	if (is_d == false) {
		return;
	}
	nlohmann::json warp = nlohmann::json::parse(data);

	if (!warp.contains("data")) {
		return;
	}



	nlohmann::json res = warp["data"];
	if (!res.contains("data")) {
		return;
	}
	nlohmann::json j = res["data"];
	if (!j.contains("size") || !j.contains("list")) {
		return;
	}

	int size =  j["size"].get<int>();
	nlohmann::json list = j["list"];
	
	for (int i = 0; i < size; i++) {
		nlohmann::json jj = list[i];
		this->HandleCrossTrade(jj);
	}





}

void MyIOCP::HandleCrossTrade(nlohmann::json j) {
	if (!j.contains(TYPE)) {
		return;
	}
	int type = j[TYPE].get<int>();;

	if (!j.contains(CMD)
		|| !j.contains(LOGIN)
		|| !j.contains(SERVER_ID)
		|| !j.contains(VOLUMN)
		|| !j.contains(ORDER)
		|| !j.contains(SYMBOL)
		|| !j.contains(MODE)
		|| !j.contains(STATE)
		|| !j.contains(TASK_ID)
		|| !j.contains(COMMENT)
		||!j.contains(ORDER)
		|| !j.contains(TRADE_SERVER_ID)
		) {
		return;
	}
	int trade_server_id = j[TRADE_SERVER_ID].get<int>();;
	int server = j[SERVER_ID].get<int>();;
	int login = atoi(j[LOGIN].get<string>().c_str());;
	string symbol = j[SYMBOL].get<string>();
	int vol = j[VOLUMN].get<int>();;
	int cmd = j[CMD].get<int>();;
	string comment = j[COMMENT].get<string>();;
	int mode = j[MODE].get<int>();;
	string task_id = j[TASK_ID].get<string>();;
	int order = atoi(j[ORDER].get<string>().c_str());;
	int state = j[STATE].get<int>();;
	if (type == TS_OPEN_NORMAL) {


		ExtProcessor.ExternalOpenOrder(trade_server_id, login, symbol, vol, cmd, comment, mode, task_id,order, state);
	}
	else if (type == TS_CLOSED_NORMAL) {
	 
	
		ExtProcessor.ExternalCloseOrder(trade_server_id, login, symbol, vol, cmd, comment, mode, order, task_id, state);
	}
}