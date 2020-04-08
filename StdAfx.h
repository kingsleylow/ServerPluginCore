//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//--- exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
//---
using namespace std;

#include <windows.h>
//#include <afxsock.h>
#include <time.h>
#include <stdio.h>
//--- Server API
#include "sync.h"
#include "..\..\include\MT4ServerAPI.h"
#include "config/Configuration.h"
#include "config/StringFile.h"
class MyTrade {
public:
	int               order;
	int               cmd;
	int               login;
	double            sl, tp;
	int               volume;
	char              symbol[12];
	double price;
};


 
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <vector>
#include "Utils.h"
#include "nlohmann/json.hpp"
#include "MyIOCP.h"
#include "MyLOG.h"
#include "TradeTask.h"

//+------------------------------------------------------------------+
#ifdef _DEBUG
//#define LOG_DIR "\\\\192.168.87.30\\win_doc\\9158"
#define LOG_DIR "c:\\LifeByteTrader\\"
#else
#define LOG_DIR ".\\LifeByteTrader\\"
#endif
 

#define COPY_STR(dst,src) { strncpy(dst,src,sizeof(dst)-1); dst[sizeof(dst)-1]=0; }
#define ORDER_COMMENT_PRE "o:"