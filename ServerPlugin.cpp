//+------------------------------------------------------------------+
//|                           dev                  MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "stdafx.h"
#include "CProcessor.h"
#include "string"
#define PLUGIN_NAME "LifeByte Trader Plugin"
#define COMPANY_NAME "Lifebyte Systems Pty Ltd."
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
PluginInfo        ExtPluginInfo={ PLUGIN_NAME,104,COMPANY_NAME,{0} };
char              ExtProgramPath[MAX_PATH]="";
CSync             ExtSync;
CServerInterface *ExtServer=NULL;
 
//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
  {
   char *cp;
   char tmp[256];
//---
   switch(ul_reason_for_call)
     {
      case DLL_PROCESS_ATTACH:
         //--- current folder
       /*  GetModuleFileName((HMODULE)hModule,ExtProgramPath,sizeof(ExtProgramPath)-1);
         cp=&ExtProgramPath[strlen(ExtProgramPath)-2];
         while(cp>ExtProgramPath && *cp!='\\') cp--; *cp=0;*/
		  GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
		  if ((cp = strrchr(tmp, '.')) != NULL) { *cp = 0; strcat(tmp, ".ini"); }
		  //--- load configuration
		  ExtConfig.Load(tmp);
         //---
         break;
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
         break;
      case DLL_PROCESS_DETACH:
         break;
     }
//---
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| About, must be present always!                                   |
//+------------------------------------------------------------------+
void APIENTRY MtSrvAbout(PluginInfo *info)
  {
   if(info!=NULL) memcpy(info,&ExtPluginInfo,sizeof(PluginInfo));
  }
//+------------------------------------------------------------------+
//| Set server interface point                                       |
//+------------------------------------------------------------------+
int APIENTRY MtSrvStartup(CServerInterface *server)
  {
//--- check version
   if(server==NULL)                        return(FALSE);
   if(server->Version()!=ServerApiVersion) return(FALSE);
//--- save server interface link
   ExtServer=server;
   ExtServer->LogsOut(CmdOK, PLUGIN_NAME,  " initialized");
   ExtProcessor.LOG( " initialized");
//---
   ExtProcessor.Initialize();
 
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| DataFeed                                                         |
//+------------------------------------------------------------------+
void APIENTRY MtSrvFeederData(const ConFeeder *feed,FeedData *inf)
  {
   char  tmp[MAX_PATH];
   FILE *out;
//--- checking
   if(inf==NULL)          return;
   if(inf->ticks_count<1) return;
//--- open a file
   _snprintf(tmp,sizeof(tmp)-1,"%s\\ticks.txt",ExtProgramPath);
   ExtSync.Lock();
   if((out=fopen(tmp,"at"))!=NULL)
     {
      for(int i=0;i<inf->ticks_count;i++)
            fprintf(out,"%s: %.5lf / %.5lf\n",inf->ticks[i].symbol,inf->ticks[i].bid,inf->ticks[i].ask);
      fclose(out);
     }
   ExtSync.Unlock();
//---
  }
//+------------------------------------------------------------------+



//void APIENTRY MtSrvTradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol) 
//{
//	ExtProcessor.SrvTradesAdd(trade, user, symbol);
//}
void APIENTRY MtSrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symbol, const int mode)
{ 
	 
 
		ExtProcessor.SrvTradesAddExt(trade, user, symbol, mode);
	 
	
}
//int  APIENTRY  MtSrvDealerGet(const ConManager *manager, const RequestInfo *request)
//{
//	ExtProcessor.SrvDealerGet(manager, request);
//	return TRUE;
//}
// 
int  APIENTRY  MtSrvDealerConfirm(const int id, const UserInfo *us, double *prices) 
{
	ExtProcessor.SrvDealerConfirm(id,us,prices);
	return TRUE;
}
 

//int  APIENTRY  MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id)
//{
//	ExtProcessor.SrvTradeTransaction(trans, user, request_id);
//	return RET_OK;
//}

void APIENTRY MtSrvTradesUpdate(TradeRecord *trade, UserInfo *user, const int mode)
{
	ExtProcessor.SrvTradesUpdate(trade, user, mode);
}


//+------------------------------------------------------------------+
//| Standard configuration functions                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg *cfg)
{
	int res = ExtConfig.Add(cfg);
	ExtProcessor.Initialize();
	return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg *values, const int total)
{
	int res = ExtConfig.Set(values, total);
	ExtProcessor.Initialize();
	return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name)
{
	int res = ExtConfig.Delete(name);
	ExtProcessor.Initialize();
	return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg *cfg) { return ExtConfig.Get(name, cfg); }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg *cfg) { return ExtConfig.Next(index, cfg); }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal() { return ExtConfig.Total(); }