//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once

 
//+------------------------------------------------------------------+
//| Simple configuration                                             |
//+------------------------------------------------------------------+
class CConfiguration
  {
private:
   CSync             m_sync;                 // ������������?
   char              m_filename[MAX_PATH];   // ��� ����?������������
   PluginCfg        *m_cfg;                  // ������ ������?
   int               m_cfg_total;            // ����?���������� ������?
   int               m_cfg_max;              // ������������ ���������� ������?

public:
                     CConfiguration();
                    ~CConfiguration();
   //--- ������������� ���� (������ ������ ����?
   void              Load(LPCSTR filename);
   //--- ������ ?����??
   int               Add(const PluginCfg* cfg);
   int               Set(const PluginCfg *values,const int total);
   int               Get(LPCSTR name,PluginCfg* cfg);
   int               Next(const int index,PluginCfg* cfg);
   int               Delete(LPCSTR name);
   inline int        Total(void) { m_sync.Lock(); int total=m_cfg_total; m_sync.Unlock(); return(total); }

   int               GetInteger(LPCSTR name,int *value,LPCSTR defvalue=NULL);
   int               GetString(LPCSTR name,LPTSTR value,const int maxlen,LPCSTR defvalue=NULL);

private:
   void              Save(void);
   PluginCfg*        Search(LPCSTR name);
   static int        SortByName(const void *left,const void *right);
   static int        SearchByName(const void *left,const void *right);
  };

extern CConfiguration ExtConfig;
//+------------------------------------------------------------------+
