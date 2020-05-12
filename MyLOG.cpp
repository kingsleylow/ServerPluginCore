#include "stdafx.h"
#include "MyLOG.h"
#include "glog/logging.h"


using namespace google;
MyLOG::MyLOG()
{
   
	 InitGoogleLogging("");
   FLAGS_log_dir = LOG_DIR;
	FLAGS_logbufsecs = 0;
}


MyLOG::~MyLOG()
{
	google::ShutdownGoogleLogging();
}


void MyLOG::LOG_I(string msg) {
 LOG(INFO) << msg;

	 
}
MyLOG* MyLOG::instance;
MyLOG* MyLOG::getInstance() {
	if (instance == NULL) {
		instance = new MyLOG();
	}

	return instance;
}