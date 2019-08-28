/******************************************************************************
//����:	�����־��Ϣ  logfile.h
//  
//�汾��:	1.
//
//����:	��С��hixi@21cn.com
//ʱ��:	2013/5/15
******************************************************************************/

/******************************************************************************
	1,	ʹ�ü򵥷���.ֻ��һ��ͷ�ļ�logfile.h include��,ֱ�ӵ��ú�������
	2,	VC6,VC7(VS2008) ����VC�汾
	3,	������ļ������к�
	4,	֧�ֶ��߳�Ӧ��

  ����:
  ��cppԴ�����ļ���ֻҪ#include "logfile.h"�󣬾Ϳ���ֱ�ӵ������º��������־��Ϣ

	CLogout("I am CLogout \r\n");
	CLogout(AT"I am CLogoutAT \r\n");
	CLoglevelout(3,"I am CLoglevelout");

	CString test = " i am  wangxiaoding!";
	int n = 8;
	CLogout("CString = %s \r\n",test);
	CLogout("CString = %d \r\n",n);


******************************************************************************/
//��ֹ���includeͷ�ļ�
#if !defined(AFX_LOGFILE_H__EF4BC4B2_3BB6_46E8_B936_0F3A61E20BF0__INCLUDED_)
#define AFX_LOGFILE_H__EF4BC4B2_3BB6_46E8_B936_0F3A61E20BF0__INCLUDED_

#pragma once

//-----------------------------------------------------------------------------

// Debug�汾��1
#if _DEBUG
#ifndef _FLAG_OUTLOG_ENABLE
//#define _FLAG_OUTLOG_ENABLE TRUE
#endif // _FLAG_OUTLOG_ENABLE
#endif // _DEBUG

// ���ÿ���̨��2
//#define  _DEBUGCONSOLE

// �����ļ�����3
//#define  _SETFILENAME
#ifdef _SETFILENAME
#define  FILENAME "log.txt"
#endif	//_SETFILENAME

// ���õȼ���4
#define  _LOGLEVEL
#ifdef _LOGLEVEL
#define	MIN_LEVEL	1
#define	MAX_LEVEL	5
#endif	// _LOGLEVEL


//-----------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h> 
#include <stdarg.h>

// ��־����࣬��̬��
struct CLog
{    
    
	// ȡ����ִ���ļ�����
    static void GetProcessFileName(char* lpName)
    {
        if ( ::GetModuleFileNameA(NULL, lpName, MAX_PATH) > 0)
        {
            char* pBegin = lpName;
            char* pTemp  = lpName;
            while ( *pTemp != 0 )
            {
                if ( *pTemp == '\\' )
                {
                    pBegin = pTemp + 1;
                }
                pTemp++;
            }

            memcpy(lpName, pBegin, strlen(pBegin)+1);
        }

    }

	// ������ļ�
    // lpFile   : Դ�ļ���
    // nLine    : Դ�ļ��к�
    // lpFormat : ���������
    static void logout(LPCSTR lpFile, int nLine,LPCSTR lpFormat, ...)
    {
		static CRITICAL_SECTION  m_crit;
		if (m_crit.DebugInfo==NULL)
		::InitializeCriticalSection(&m_crit); 
		/*-----------------------�����ٽ���(�����Ϣ)------------------------------*/   
		::EnterCriticalSection(&m_crit);   

        if ( NULL == lpFormat )
            return;

        //��ǰʱ��
        SYSTEMTIME st;
        ::GetLocalTime(&st);

        //������Ϣͷ
        const DWORD BufSize = 2048;
        char szMsg[BufSize];

		if (nLine==0)
		{	
			//��nLine==0 ʱ,��CLogout("xxx")ֻ��ӡ��Ϣ
			sprintf(szMsg, "[%02d:%02d:%02d.%03d]:", 
				st.wHour, st.wMinute, st.wSecond, 
									st.wMilliseconds);
		}
		else
		{
			//��nLine������0 ʱ,��CLogout(AT"xxx")��ӡ�ļ����кż���Ϣ
			sprintf(szMsg, "[%02d:%02d:%02d.%03d]�ļ�%s��%04d��:", 
				st.wHour, st.wMinute, st.wSecond, 
									  st.wMilliseconds, lpFile, nLine);

		}

		//��ʽ����Ϣ,������������Ϣ
        char* pTemp = szMsg;
        pTemp += strlen(szMsg);
        va_list args; 
        va_start(args, lpFormat);    
        wvsprintfA(pTemp,  lpFormat, args);	 //vsprintf_s BufSize - strlen(szMsg),
        va_end(args);  

        DWORD dwMsgLen = (DWORD)strlen(szMsg);

        //��ȡ��־�ļ���
		char szFileName[MAX_PATH];
        char szExeName[MAX_PATH];
        GetProcessFileName(szExeName);
        sprintf(szFileName, "Log(%s)%d-%d-%d.txt", szExeName, //sprintf_s MAX_PATH
                                        st.wYear, st.wMonth, st.wDay);

        // �ж��ļ������Ƿ���ͬ,����Ƿ���Ч.
        // �����ͬ����Ч,��رյ�ǰ�ļ�,�������ļ�
        static char   s_szFileName[MAX_PATH] = {0};
        static HANDLE s_hFile = INVALID_HANDLE_VALUE; 

		//�����Զ�����־�ļ���
#ifdef _SETFILENAME
		strcpy(szFileName,FILENAME);
#endif	//_SETFILENAME

        BOOL bNew = ((strcmp(s_szFileName, szFileName) != 0) || (s_hFile == INVALID_HANDLE_VALUE));
        

#ifdef _DEBUGCONSOLE	//����̨���
		static BOOL bOpenConsole = FALSE;

		if (!bOpenConsole)
		{
			bOpenConsole = ::AllocConsole();
			if (bOpenConsole)
			{
				freopen("CONOUT$","w+t",stdout);  
				freopen("CONIN$","r+t",stdin);
				freopen("CONERR", "w", stderr);
				
				HANDLE handle= GetStdHandle(STD_OUTPUT_HANDLE);	
				SetConsoleTitle("DebugCosole");
				SetConsoleTextAttribute((HANDLE)handle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				
				HWND hwnd=NULL; 
				while(NULL==hwnd) 
					hwnd=::FindWindow(NULL,(LPCTSTR)"DebugCosole"); 
				
				HMENU hmenu = ::GetSystemMenu ( hwnd, FALSE ); 
				DeleteMenu ( hmenu, SC_CLOSE, MF_BYCOMMAND );
			}
		}
#endif	//_DEBUGCONSOLE

        printf("%s", szMsg);

        if ( bNew ) // �رվ��ļ����������ļ�
        {
            if ( s_hFile != INVALID_HANDLE_VALUE)
            {
                ::CloseHandle(s_hFile);
                s_hFile = INVALID_HANDLE_VALUE;
            }
			
			//������־�ļ�. ���ļ�ʱ׷�ӷ�ʽ��,û��ʱ����.
            s_hFile = ::CreateFileA( szFileName, 
                                     GENERIC_WRITE, 
                                     FILE_SHARE_WRITE | FILE_SHARE_READ, 
                                     0, 
                                     OPEN_ALWAYS, 
                                     FILE_ATTRIBUTE_NORMAL, 
                                     0);

            if ( s_hFile == INVALID_HANDLE_VALUE)
            {
                printf("::CreateFile Error: %d", ::GetLastError());
                return;
            }
        }



        //����Ϣд���ļ�
        if ( s_hFile != INVALID_HANDLE_VALUE) 
        {
            DWORD dwWrite = 0;
            ::SetFilePointer(s_hFile, 0, NULL, FILE_END);
            ::WriteFile(s_hFile, szMsg, dwMsgLen, &dwWrite, NULL);
            
			//���ݴ����ɹ������ļ���
			strcpy(s_szFileName,szFileName);
        }

		::LeaveCriticalSection(&m_crit);    
		/*----------------------------�˳��ٽ���---------------------------------*/
    }

}; // CLog


//�궨���ļ������к�
#define AT __FILE__, __LINE__,



#if (_FLAG_OUTLOG_ENABLE) 

//��־����ӿں���1
static void CLogout(LPCSTR lpFormat, ...)
{
	const DWORD BufSize = 2048;
    char szMsg[BufSize];
	
	va_list args; 	//��ʽ����Ϣ

	va_start(args, lpFormat);    
	wvsprintfA(szMsg,  lpFormat, args);	 //vsprintf_s BufSize - strlen(szMsg),
	va_end(args);  
		
	//�����Ϣ
	CLog::logout("0",0,szMsg);
}

//��־����ӿں���2  ʹ����Clogout(AT"xxxx")��ʽ
static void CLogout(LPCSTR lpFile, int nLine,LPCSTR lpFormat, ...)
{
	const DWORD BufSize = 2048;
    char szMsg[BufSize];
	
	char* pTemp = szMsg;
	
	va_list args; //��ʽ����Ϣ
	
	va_start(args, lpFormat);    
	wvsprintfA(szMsg,  lpFormat, args);	 //vsprintf_s BufSize - strlen(szMsg),
	va_end(args);  
	
	//������ļ������кŵ���Ϣ
	CLog::logout(lpFile, nLine,szMsg);
}

#ifdef _LOGLEVEL
//��־����ӿں���3
static void CLoglevelout(int nshowlevel,LPCSTR lpFormat, ...)
{

#ifdef _SETFILENAME
	if (MIN_LEVEL<=nshowlevel && nshowlevel<= MAX_LEVEL)
#endif
	{
		const DWORD BufSize = 2048;
		char szMsg[BufSize];
		
		va_list args; 	//��ʽ����Ϣ
		
		va_start(args, lpFormat);    
		wvsprintfA(szMsg,  lpFormat, args);	 //vsprintf_s BufSize - strlen(szMsg),
		va_end(args);  
		
		char buffer[20];
   		_itoa(nshowlevel, buffer, 10 );
		strcat(szMsg,"......Level=");
        strcat(szMsg,buffer);
		strcat(szMsg,"\r\n");

		//�����Ϣ
		CLog::logout("0",0,szMsg);
	}

}
#endif //_LOGLEVEL

#ifdef _DEBUGCONSOLE
//�رտ���̨�ӿں���4 �� ����Release�汾
static void CLogconsole_close()
{
	FreeConsole();
}

static void CLogcconsole_win(BOOL pSHWinConsole = FALSE)
{
	static BOOL bGetWinConsole = FALSE;
	HWND wincmd = NULL;

	if (!bGetWinConsole)
		{
			typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
			PROCGETCONSOLEWINDOW GetConsoleWindow;
			
			HMODULE hKernel32 = GetModuleHandle("kernel32");
			GetConsoleWindow = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");

			wincmd=GetConsoleWindow();
		}
	if (pSHWinConsole)
	{
		ShowWindowAsync(wincmd, SW_SHOWNORMAL);
	}
	else
	{
		ShowWindowAsync(wincmd, SW_HIDE );
	}
}
#endif //_DEBUGCONSOLE

#else  //_FLAG_OUTLOG_ENABLE

//��־����ӿں���1 �� ����Release�汾
static void CLogout(LPCSTR lpFormat, ...)
{
}
//��־����ӿں���2 �� ����Release�汾
static void CLogout(LPCSTR lpFile, int nLine,LPCSTR lpFormat, ...)
{
}

#ifdef _LOGLEVEL
//��־����ӿں���3 �� ����Release�汾
static void CLoglevelout(int nshowlevel,LPCSTR lpFormat, ...)
{
}
#endif //_LOGLEVEL

#ifdef _DEBUGCONSOLE
//�رտ���̨�ӿں���4 �� ����Release�汾
static void CLogconsole_close()
{
}

static void CLogcconsole_win(BOOL pSHWinConsole = FALSE)
{
}
#endif //_DEBUGCONSOLE

#endif //_FLAG_OUTLOG_ENABLE

#endif //!defined(AFX_LOGFILE_H__EF4BC4B2_3BB6_46E8_B936_0F3A61E20BF0__INCLUDED_)
