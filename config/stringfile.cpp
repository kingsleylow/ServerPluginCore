//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "../stdafx.h"
#include "stringfile.h"
//+------------------------------------------------------------------+
//| Конструкто?                                                     |
//+------------------------------------------------------------------+
CStringFile::CStringFile(const int nBufSize) :
             m_file(INVALID_HANDLE_VALUE),m_file_size(0),
             m_buffer(new BYTE[nBufSize]),m_buffer_size(nBufSize),
             m_buffer_index(0),m_buffer_readed(0),m_buffer_line(0)
  {
  }
//+------------------------------------------------------------------+
//| Деструктор                                                       |
//+------------------------------------------------------------------+
CStringFile::~CStringFile()
  {
//--- закрое?соединение
   Close();
//--- освободи?буфе?
   if(m_buffer!=NULL) { delete[] m_buffer; m_buffer=NULL; }
  }
//+------------------------------------------------------------------+
//| Открытие файл?для чтен?                                        |
//| dwAccess       -GENERIC_READ ил?GENERIC_WRITE                   |
//| dwCreationFlags-CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS        |
//+------------------------------------------------------------------+
bool CStringFile::Open(LPCTSTR lpFileName,const DWORD dwAccess,const DWORD dwCreationFlags)
  {
//--- закрое?на всяки?случай предыдущий файл
   Close();
//--- проверки
   if(lpFileName!=NULL)
     {
      //--- создадим файл
      m_file=CreateFile(lpFileName,dwAccess,FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,dwCreationFlags,FILE_ATTRIBUTE_NORMAL,NULL);
      //--- определи?размер файл?(не больше 4Gb)
      if(m_file!=INVALID_HANDLE_VALUE) m_file_size=GetFileSize(m_file,NULL);
     }
//--- вернем результа?
   return(m_file!=INVALID_HANDLE_VALUE);
  }
//+------------------------------------------------------------------+
//| Запись буфера указанно?длин??файл                             |
//+------------------------------------------------------------------+
int CStringFile::Read(void *buffer,const DWORD length)
  {
   DWORD readed=0;
//--- проверки
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- считае??вернем результа?
   if(ReadFile(m_file,buffer,length,&readed,NULL)==0) readed=0;
//--- вернем ко?во считанны?байт
   return(readed);
  }
//+------------------------------------------------------------------+
//| Чтение буфера указанно?длин?из файл?                          |
//+------------------------------------------------------------------+
int CStringFile::Write(const void *buffer,const DWORD length)
  {
   DWORD written=0;
//--- проверки
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- запише?данные
   if(WriteFile(m_file,buffer,length,&written,NULL)==0) written=0;
//--- вернем ко?во записанных байт
   return(written);
  }
//+------------------------------------------------------------------+
//| Выставим? на начало файл?                                      |
//+------------------------------------------------------------------+
void CStringFile::Reset()
  {
//--- сброси?счетчики
   m_buffer_index=0;
   m_buffer_readed=0;
   m_buffer_line=0;
//--- выставим? на начало файл?
   if(m_file!=INVALID_HANDLE_VALUE) SetFilePointer(m_file,0,NULL,FILE_BEGIN);
  }
//+------------------------------------------------------------------+
//| Заполняем строку ?возвращаем номе?строки. 0-ошибка           |
//+------------------------------------------------------------------+
int CStringFile::GetNextLine(char *line,const int maxsize)
  {
   char  *currsym=line,*lastsym=line+maxsize;
   BYTE  *curpos=m_buffer+m_buffer_index;
//--- проверки
   if(line==NULL || m_file==INVALID_HANDLE_VALUE || m_buffer==NULL) return(0);
//--- крутим? ?цикл?
   for(;;)
     {
      //--- перв? строка ил?прочитал?весь буфе?
      if(m_buffer_line==0 || m_buffer_index==m_buffer_readed)
        {
         //--- зану?ем счетчики
         m_buffer_index=0;
         m_buffer_readed=0;
         //--- читаем ?буфе?
         if(::ReadFile(m_file,m_buffer,m_buffer_size,(DWORD*)&m_buffer_readed,NULL)==0)
           {
            Close();
            return(0);
           }
         //--- считал?0 байт? коне?файл?
         if(m_buffer_readed<1) { *currsym=0; return(currsym!=line ? m_buffer_line:0); }
         curpos=m_buffer;
        }
      //--- анализируе?буфе?
      while(m_buffer_index<m_buffer_readed)
        {
         //--- дошл?до конц?
         if(currsym>=lastsym) { *currsym=0; return(m_buffer_line); }
         //--- проанализируем символ (нашл?коне?строки)
         if(*curpos=='\n')
           {
            //--- бы?ли пере?этим возвра?каретк?
            if(currsym>line && currsym[-1]=='\r') currsym--; // бы?вытираем ег?
            *currsym=0;
            //--- возвращаем номе?строки
            m_buffer_line++;
            m_buffer_index++;
            return(m_buffer_line);
           }
         //--- обычны?символ-копируем ег?
         *currsym++=*curpos++; m_buffer_index++;
        }
     }
//--- эт?невозможно...
   return(0);
  }
//+------------------------------------------------------------------+
