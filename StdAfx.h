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
 

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <vector>
#include "Utils.h"

//+------------------------------------------------------------------+
#define LOG_NAME "daily_log"
#define LOG_FOLDER ".\\log\\LifeByteTrader\\"
#define COPY_STR(dst,src) { strncpy(dst,src,sizeof(dst)-1); dst[sizeof(dst)-1]=0; }