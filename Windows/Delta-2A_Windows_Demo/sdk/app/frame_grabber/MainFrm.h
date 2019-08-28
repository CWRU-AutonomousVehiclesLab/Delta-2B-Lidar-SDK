/*
*  RSLIDAR System
*  Driver Interface
*
*  Copyright 2015 RS Team
*  All rights reserved.
*
*	Author: ruishi, Data:2015-12-25
*
*/

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
    enum {
        WORKING_MODE_IDLE       = 0,
        WORKING_MODE_SCAN       = 3,
    };

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
    CScanView         m_scanview;
	CCommandBarCtrl m_CmdBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_CMD_STOP, UPDUI_MENUPOPUP| UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_CMD_SCAN, UPDUI_MENUPOPUP| UPDUI_TOOLBAR)
     //   UPDATE_ELEMENT(ID_OPT_FORCESCAN, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		//COMMAND_ID_HANDLER(ID_FILE_DUMPDATA, OnFileDumpdata)
		//COMMAND_ID_HANDLER(ID_OPT_FORCESCAN, OnOptForcescan)
		COMMAND_ID_HANDLER(ID_CMD_SCAN, OnCmdScan)
		COMMAND_ID_HANDLER(ID_CMD_STOP, OnCmdStop)
		//COMMAND_ID_HANDLER(ID_CMD_RESET, OnCmdReset)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

    void    onUpdateTitle();
    void    onSwitchMode(int newMode);
    void    onRefreshScanData();
    void    checkDeviceHealth();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnCmdReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCmdStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCmdScan(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOptForcescan(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileDumpdata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    int     workingMode; // 0 - idle 1 - framegrabber
    bool    forcescan;
};
