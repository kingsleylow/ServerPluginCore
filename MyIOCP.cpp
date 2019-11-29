#include "Stdafx.h"
#include "MyIOCP.h"
#include "CProcessor.h"

MyIOCP::MyIOCP()
{
	this->m_heart_count = 0;
	this->level = NO_RIGHT;

 
}


MyIOCP::~MyIOCP()
{
	this->level = NO_RIGHT;
	this->m_heart_count = 0;
 


}


VOID MyIOCP::NotifyReceivedFormatPackage(const char* lpszBuffer)
{
	 
	string data = lpszBuffer;
	int cmd = this->getCommand(data);
 
	this->m_heart_count = 0;
	switch (cmd)
	{
	case CMD_PLUGIN_AUTH:
		this->checkLogin(data);
		break;
	case CMD_HEART_BEAT:
		this->refreshTaakData(data);
	 
		break;
	case CMD_QUERY_PORTAL_IDS:

		break;

	//case CMD_QUERY_GOD_PORTFOLIO_MULTIPLE:

	//	break;
	default:
		break;
	}
	//this->receive_data = data;
	//this->receiveFlag = true;
	
}

VOID MyIOCP::NotifyConnectionStatus(IOCPClient_ConnectionType ConnectionType) {
	if (ConnectionType == IOCPClient_ConnectionType_Connected) {
		//Send("hello");
	//	ExtProcessor.LOG("New Connection");
	//	ExtProcessor.LOG(CmdOK, "IOCPClient_ConnectionType_Connected", "IOCPClient_ConnectionType_Connected");
	//	SendInitailRequest();
	}
	else if(ConnectionType == IOCPClient_ConnectionType_Disconnected){
 
	}
	else if (ConnectionType == IOCPClient_ConnectionType_ConnectFailed) {

 
	}	
}

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

VOID MyIOCP::SendHeartBeat() {

	SendRequest(CMD_HEART_BEAT, NULL);
}


nlohmann::json MyIOCP::GetJson(int cmd, nlohmann::json data) {
	nlohmann::json j = { {"cmd",cmd},{"data",data} };
	return j;
}

void MyIOCP::checkConnection() {

	if (this->level == NO_RIGHT) {
		this->SendInitailRequest();
	}
	else {
		this->SendHeartBeat();
	}
	
}


void MyIOCP::SendRequest(int cmd, nlohmann::json data) {
	nlohmann::json j = this->GetJson(cmd, data);
	Send((j.dump()).c_str());
}


 ///receive

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

nlohmann::json MyIOCP::getData(string data,string key) {
	nlohmann::json j = nlohmann::json::parse(data);
	if (j.contains(key)) {
		return   j[key];
	}
	return NULL;
}

void MyIOCP::checkLogin(string data) {
	nlohmann::json j = nlohmann::json::parse(data);
	if (j.contains("Level")) {
		this->level = j["Level"];
	}
}


void MyIOCP::refreshTaakData(string data) {

}