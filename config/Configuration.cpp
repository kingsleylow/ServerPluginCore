//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "Stdafx.h"
#include "Configuration.h"
#include "stringfile.h"

CConfiguration ExtConfig;
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CConfiguration::CConfiguration() : m_cfg(NULL),m_cfg_total(0),m_cfg_max(0)
  {
   m_filename[0]=0;
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CConfiguration::~CConfiguration()
  {
   m_sync.Lock();
//---
   if(m_cfg!=NULL) { delete[] m_cfg; m_cfg=NULL; }
   m_cfg_total=m_cfg_max=0;
//---
   m_sync.Unlock();
  }
//+------------------------------------------------------------------+
//| Мы только читаем конфигурацию, но ничего не ме?ем ?не?        |
//+------------------------------------------------------------------+
void CConfiguration::Load(LPCSTR filename)
  {
   char        tmp[MAX_PATH],*cp,*start;
   CStringFile in;
   PluginCfg   cfg,*buf;
//--- проверка
   if(filename==NULL) return;
//--- сохраним имя конфигурационног?файл?
   m_sync.Lock();
   COPY_STR(m_filename,filename);
//--- открое?файл
   if(in.Open(m_filename,GENERIC_READ,OPEN_EXISTING))
     {
      while(in.GetNextLine(tmp,sizeof(tmp)-1)>0)
        {
         if(tmp[0]==';') continue;
         //--- пропусти?пробел?
         start=tmp; while(*start==' ') start++;
         if((cp=strchr(start,'='))==NULL) continue;
         *cp=0;
         //--- занули? бере?имя параметр?
         ZeroMemory(&cfg,sizeof(cfg));
         COPY_STR(cfg.name,start);
         //--- опять пропусти?пробел?
         cp++; while(*cp==' ') cp++;
         COPY_STR(cfg.value,cp);
         //--- проверяем
         if(cfg.name[0]==0 || cfg.value[0]==0) continue;
         //--- добавляем
         if(m_cfg==NULL || m_cfg_total>=m_cfg_max) // мест?есть?
           {
            //--- перевыдели?новы?буфе?
            if((buf=new PluginCfg[m_cfg_total+64])==NULL) break;
            //--- скопируе?остатк?из старог?
            if(m_cfg!=NULL)
              {
               if(m_cfg_total>0) memcpy(buf,m_cfg,sizeof(PluginCfg)*m_cfg_total);
               delete[] m_cfg;
              }
            //--- подменим буфе?
            m_cfg    =buf;
            m_cfg_max=m_cfg_total+64;
           }
         //--- копируем
         memcpy(&m_cfg[m_cfg_total++],&cfg,sizeof(PluginCfg));
        }
      //--- закрываемся
      in.Close();
     }
//--- теперь возьмё??отсортируе?по имен?(чтоб?искать быстро)
   if(m_cfg!=NULL && m_cfg_total>0) qsort(m_cfg,m_cfg_total,sizeof(PluginCfg),SortByName);
   m_sync.Unlock();
  }
//+------------------------------------------------------------------+
//| Сбро?конфигов на диск                                           |
//+------------------------------------------------------------------+
void CConfiguration::Save(void)
  {
   CStringFile out;
   char        tmp[512];
//--- запише?вс?на диск
   m_sync.Lock();
   if(m_filename[0]!=0)
      if(out.Open(m_filename,GENERIC_WRITE,CREATE_ALWAYS))
        {
         if(m_cfg!=NULL)
            for(int i=0;i<m_cfg_total;i++)
              {
               _snprintf(tmp,sizeof(tmp)-1,"%s=%s\n",m_cfg[i].name,m_cfg[i].value);
               if(out.Write(tmp,strlen(tmp))<1) break;
              }
         //--- закрое?файл
         out.Close();
        }
   m_sync.Unlock();
//---
  }
//+------------------------------------------------------------------+
//| Неблокируемы?поис?по имен?                                    |
//+------------------------------------------------------------------+
PluginCfg* CConfiguration::Search(LPCSTR name)
  {
   PluginCfg *config=NULL;
//---
   if(m_cfg!=NULL && m_cfg_total>0)
      config=(PluginCfg*)bsearch(name,m_cfg,m_cfg_total,sizeof(PluginCfg),SearchByName);
//---
   return(config);
  }
//+------------------------------------------------------------------+
//| Добавление/модификация плагин?                                  |
//+------------------------------------------------------------------+
int CConfiguration::Add(const PluginCfg *cfg)
  {
   PluginCfg *config,*buf;
//--- проверки
   if(cfg==NULL || cfg->name[0]==0) return(FALSE);
//---
   m_sync.Lock();
   if((config=Search(cfg->name))!=NULL) memcpy(config,cfg,sizeof(PluginCfg));
   else
     {
      //--- мест?есть?
      if(m_cfg==NULL || m_cfg_total>=m_cfg_max)
        {
         //--- выдели?мест?
         if((buf=new PluginCfg[m_cfg_total+64])==NULL) { m_sync.Unlock(); return(FALSE); }
         //--- скопируе?остатк?из старог?буфера
         if(m_cfg!=NULL)
           {
            if(m_cfg_total>0) memcpy(buf,m_cfg,sizeof(PluginCfg)*m_cfg_total);
            delete[] m_cfg;
           }
         //--- замени?буфе?
         m_cfg    =buf;
         m_cfg_max=m_cfg_total+64;
        }
      //--- добавляем ?коне?
      memcpy(&m_cfg[m_cfg_total++],cfg,sizeof(PluginCfg));
      //--- отсортируемся
      qsort(m_cfg,m_cfg_total,sizeof(PluginCfg),SortByName);
     }
   m_sync.Unlock();
//--- сохраним?, перезачитаем?
   Save();
//--- выходи?
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Выстав?ем набо?настроек                                        |
//+------------------------------------------------------------------+
int CConfiguration::Set(const PluginCfg *values,const int total)
  {
//--- проверки
   if(total<0) return(FALSE);
//---
   m_sync.Lock();
   if(values!=NULL && total>0)
     {
      //--- мест?есть?
      if(m_cfg==NULL || total>=m_cfg_max)
        {
         //--- удалим старый ?выдели?новы?буфе?
         if(m_cfg!=NULL) delete[] m_cfg;
         if((m_cfg=new PluginCfg[total+64])==NULL)
           {
            m_cfg_max=m_cfg_total=0;
            m_sync.Unlock();
            return(FALSE);
           }
         //--- выставим новы?предел
         m_cfg_max=total+64;
        }
      //--- скопируе?всем скопом
      memcpy(m_cfg,values,sizeof(PluginCfg)*total);
     }
//--- выставим обще?количество ?отсортируемся
   m_cfg_total=total;
   if(m_cfg!=NULL && m_cfg_total>0) qsort(m_cfg,m_cfg_total,sizeof(PluginCfg),SortByName);
   m_sync.Unlock();
//--- сохраним?
   Save();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Ищем конфиг по имен?                                            |
//+------------------------------------------------------------------+
int CConfiguration::Get(LPCSTR name,PluginCfg *cfg)
  {
   PluginCfg *config=NULL;
//--- проверки
   if(name!=NULL && cfg!=NULL)
     {
      m_sync.Lock();
      if((config=Search(name))!=NULL) memcpy(cfg,config,sizeof(PluginCfg));
      m_sync.Unlock();
     }
//--- вернем результа?
   return(config!=NULL);
  }
//+------------------------------------------------------------------+
//| Ищем конфиг                                                      |
//+------------------------------------------------------------------+
int CConfiguration::Next(const int index,PluginCfg *cfg)
  {
//--- проверки
   if(cfg!=NULL && index>=0)
     {
      m_sync.Lock();
      if(m_cfg!=NULL && index<m_cfg_total)
        {
         memcpy(cfg,&m_cfg[index],sizeof(PluginCfg));
         m_sync.Unlock();
         return(TRUE);
        }
      m_sync.Unlock();
     }
//--- неудач?
   return(FALSE);
  }
//+------------------------------------------------------------------+
//| Удаляем конфиг                                                   |
//+------------------------------------------------------------------+
int CConfiguration::Delete(LPCSTR name)
  {
   PluginCfg *config=NULL;
//--- проверки
   if(name!=NULL)
     {
      m_sync.Lock();
      if((config=Search(name))!=NULL)
        {
         int index=config-m_cfg;
         if((index+1)<m_cfg_total) memmove(config,config+1,sizeof(PluginCfg)*(m_cfg_total-index-1));
         m_cfg_total--;
        }
      //--- отсортируемся
      if(m_cfg!=NULL && m_cfg_total>0) qsort(m_cfg,m_cfg_total,sizeof(PluginCfg),SortByName);
      m_sync.Unlock();
     }
//--- вернем результа?
   return(config!=NULL);
  }
//+------------------------------------------------------------------+
//| Сортировка по имен?                                             |
//+------------------------------------------------------------------+
int CConfiguration::SortByName(const void *left,const void *right)
  {
   return strcmp( ((PluginCfg*)left)->name,((PluginCfg*)right)->name );
  }
//+------------------------------------------------------------------+
//| Поис?по имен?                                                  |
//+------------------------------------------------------------------+
int CConfiguration::SearchByName(const void *left,const void *right)
  {
   return strcmp( (char*)left, ((PluginCfg*)right)->name);
  }
//+------------------------------------------------------------------+
//| Ищем конфиг по имен?                                            |
//+------------------------------------------------------------------+
int CConfiguration::GetInteger(LPCSTR name,int *value,LPCSTR defvalue)
  {
   PluginCfg *config=NULL;
//--- проверки
   if(name!=NULL && value!=NULL)
     {
      m_sync.Lock();
      if((config=Search(name))!=NULL) *value=atoi(config->value);
      else
         if(defvalue!=NULL)
           {
            m_sync.Unlock();
            //--- создадим нову?запись
            PluginCfg cfg={0};
            COPY_STR(cfg.name,name);
            COPY_STR(cfg.value,defvalue);
            Add(&cfg);
            //--- выставим значение по умолчани??вернем?
            *value=atoi(cfg.value);
            return(TRUE);
           }
      m_sync.Unlock();
     }
//--- вернем результа?
   return(config!=NULL);
  }
//+------------------------------------------------------------------+
//| Ищем конфиг по имен?                                            |
//+------------------------------------------------------------------+
int CConfiguration::GetString(LPCSTR name,LPTSTR value,const int maxlen,LPCSTR defvalue)
  {
   PluginCfg *config=NULL;
//--- проверки
   if(name!=NULL && value!=NULL)
     {
      m_sync.Lock();
      if((config=Search(name))!=NULL)
        {
         strncpy(value,config->value,maxlen);
         value[maxlen]=0;
        }
      else
         if(defvalue!=NULL)
           {
            m_sync.Unlock();
            //--- создадим нову?запись
            PluginCfg cfg={0};
            COPY_STR(cfg.name,name);
            COPY_STR(cfg.value,defvalue);
            Add(&cfg);
            //--- выставим значение по умолчани??вернем?
            strncpy(value,cfg.value,maxlen);
            value[maxlen]=0;
            return(TRUE);
           }
      m_sync.Unlock();
     }
//--- вернем результа?
   return(config!=NULL);
  }
//+------------------------------------------------------------------+
