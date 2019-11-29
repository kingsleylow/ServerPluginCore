//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "stdafx.h"
#include "stringfile.h"
//+------------------------------------------------------------------+
//| �����������                                                      |
//+------------------------------------------------------------------+
CStringFile::CStringFile(const int nBufSize) :
             m_file(INVALID_HANDLE_VALUE),m_file_size(0),
             m_buffer(new BYTE[nBufSize]),m_buffer_size(nBufSize),
             m_buffer_index(0),m_buffer_readed(0),m_buffer_line(0)
  {
  }
//+------------------------------------------------------------------+
//| ����������                                                       |
//+------------------------------------------------------------------+
CStringFile::~CStringFile()
  {
//--- ������� ����������
   Close();
//--- ��������� �����
   if(m_buffer!=NULL) { delete[] m_buffer; m_buffer=NULL; }
  }
//+------------------------------------------------------------------+
//| �������� ����� ��� ������                                        |
//| dwAccess       -GENERIC_READ ��� GENERIC_WRITE                   |
//| dwCreationFlags-CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS        |
//+------------------------------------------------------------------+
bool CStringFile::Open(LPCTSTR lpFileName,const DWORD dwAccess,const DWORD dwCreationFlags)
  {
//--- ������� �� ������ ������ ���������� ����
   Close();
//--- ��������
   if(lpFileName!=NULL)
     {
      //--- �������� ����
      m_file=CreateFile(lpFileName,dwAccess,FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,dwCreationFlags,FILE_ATTRIBUTE_NORMAL,NULL);
      //--- ��������� ������ ����� (�� ������ 4Gb)
      if(m_file!=INVALID_HANDLE_VALUE) m_file_size=GetFileSize(m_file,NULL);
     }
//--- ������ ���������
   return(m_file!=INVALID_HANDLE_VALUE);
  }
//+------------------------------------------------------------------+
//| ������ ������ ��������� ����� � ����                             |
//+------------------------------------------------------------------+
int CStringFile::Read(void *buffer,const DWORD length)
  {
   DWORD readed=0;
//--- ��������
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- ������� � ������ ���������
   if(ReadFile(m_file,buffer,length,&readed,NULL)==0) readed=0;
//--- ������ ���-�� ��������� ����
   return(readed);
  }
//+------------------------------------------------------------------+
//| ������ ������ ��������� ����� �� �����                           |
//+------------------------------------------------------------------+
int CStringFile::Write(const void *buffer,const DWORD length)
  {
   DWORD written=0;
//--- ��������
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- ������� ������
   if(WriteFile(m_file,buffer,length,&written,NULL)==0) written=0;
//--- ������ ���-�� ���������� ����
   return(written);
  }
//+------------------------------------------------------------------+
//| ���������� �� ������ �����                                       |
//+------------------------------------------------------------------+
void CStringFile::Reset()
  {
//--- ������� ��������
   m_buffer_index=0;
   m_buffer_readed=0;
   m_buffer_line=0;
//--- ���������� �� ������ �����
   if(m_file!=INVALID_HANDLE_VALUE) SetFilePointer(m_file,0,NULL,FILE_BEGIN);
  }
//+------------------------------------------------------------------+
//| ��������� ������ � ���������� ����� ������. 0-������           |
//+------------------------------------------------------------------+
int CStringFile::GetNextLine(char *line,const int maxsize)
  {
   char  *currsym=line,*lastsym=line+maxsize;
   BYTE  *curpos=m_buffer+m_buffer_index;
//--- ��������
   if(line==NULL || m_file==INVALID_HANDLE_VALUE || m_buffer==NULL) return(0);
//--- �������� � �����
   for(;;)
     {
      //--- ������ ������ ��� ��������� ���� �����
      if(m_buffer_line==0 || m_buffer_index==m_buffer_readed)
        {
         //--- �������� ��������
         m_buffer_index=0;
         m_buffer_readed=0;
         //--- ������ � �����
         if(::ReadFile(m_file,m_buffer,m_buffer_size,(DWORD*)&m_buffer_readed,NULL)==0)
           {
            Close();
            return(0);
           }
         //--- ������� 0 ����? ����� �����
         if(m_buffer_readed<1) { *currsym=0; return(currsym!=line ? m_buffer_line:0); }
         curpos=m_buffer;
        }
      //--- ����������� �����
      while(m_buffer_index<m_buffer_readed)
        {
         //--- ����� �� �����?
         if(currsym>=lastsym) { *currsym=0; return(m_buffer_line); }
         //--- �������������� ������ (����� ����� ������)
         if(*curpos=='\n')
           {
            //--- ��� �� ����� ���� ������� �������?
            if(currsym>line && currsym[-1]=='\r') currsym--; // ���-�������� ���
            *currsym=0;
            //--- ���������� ����� ������
            m_buffer_line++;
            m_buffer_index++;
            return(m_buffer_line);
           }
         //--- ������� ������-�������� ���
         *currsym++=*curpos++; m_buffer_index++;
        }
     }
//--- ��� ����������...
   return(0);
  }
//+------------------------------------------------------------------+
