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

// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "scanView.h"
#include "MainFrm.h"
#include "drvlogic\lidarmgr.h"

const int REFRESEH_TIMER = 0x800;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;
    if (m_hWndClient == m_scanview.m_hWnd){
        return m_scanview.PreTranslateMessage(pMsg);
    } else {
        return FALSE;
    }
	
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();
    m_hWndClient =m_scanview.Create(m_hWnd, rcDefault, NULL, WS_CHILD  | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

    workingMode = WORKING_MODE_IDLE;
    onUpdateTitle();
    // setup timer
    this->SetTimer(REFRESEH_TIMER, 1000/30);

#ifdef feidangong
	checkDeviceHealth();
#endif // feidangong

   
    UISetCheck(ID_CMD_STOP, 1);
    forcescan = 0;
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
    this->KillTimer(REFRESEH_TIMER);
    
    CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;

	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
    switch (workingMode)
    {
    case WORKING_MODE_SCAN:
        onRefreshScanData();
        break;
    }
}

LRESULT CMainFrame::OnCmdReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    //if (MessageBox("The device will reboot.", "Are you sure?",
    //    MB_OKCANCEL|MB_ICONQUESTION) != IDOK) {
    //        return 0;
    //}


    //onSwitchMode(WORKING_MODE_IDLE);
    //LidarMgr::GetInstance().lidar_drv->reset();

	system("cls");
	return 0;
}
LRESULT CMainFrame::OnCmdStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    onSwitchMode(WORKING_MODE_IDLE);
	return 0;
}


LRESULT CMainFrame::OnCmdScan(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    onSwitchMode(WORKING_MODE_SCAN);
	return 0;
}

LRESULT CMainFrame::OnOptForcescan(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    forcescan=!forcescan;
   // UISetCheck(ID_OPT_FORCESCAN, forcescan?1:0);
	return 0;
}
LRESULT CMainFrame::OnFileDumpdata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    switch (workingMode) {
    case WORKING_MODE_SCAN:
        {
            //capture the snapshot
            std::vector<scanDot> snapshot = m_scanview.getScanList();

            //prompt
            CFileDialog dlg(FALSE);
            if (dlg.DoModal()==IDOK) {
                FILE * outputfile = fopen(dlg.m_szFileName, "w");
                //fprintf(outputfile, "#RPLIDAR SCAN DATA\n#COUNT=%d\n#Angule\tDistance\tQuality\n",snapshot.size());
				fprintf(outputfile, "#RPLIDAR SCAN DATA\n#COUNT=%d\n#Angule\tDistance\n", snapshot.size());
                for (int pos = 0; pos < (int)snapshot.size(); ++pos) {
                    //fprintf(outputfile, "%.4f %.1f %d\n", snapshot[pos].angle, snapshot[pos].dist, snapshot[pos].quality);
					fprintf(outputfile, "%.4f %.1f\n", snapshot[pos].angle, snapshot[pos].dist,1);
                }
                fclose(outputfile);
            }
        }
        break;

    }
	return 0;
}


void    CMainFrame::onRefreshScanData()
{
	RSLIDAR_SIGNAL_DISTANCE_UNIT_T nodes[360 * 2];
    size_t cnt = _countof(nodes);
	RSlidarDriver * lidar_drv = LidarMgr::GetInstance().lidar_drv;

	if (IS_OK(lidar_drv->grabScanData(nodes, cnt)))
    {
		//_cprintf(">>>>>>%d\n", cnt);
		m_scanview.setScanData(nodes, cnt);
    }
}

void    CMainFrame::onUpdateTitle()
{
    char titleMsg[200];
    const char * workingmodeDesc;
    switch (workingMode) {
    case WORKING_MODE_IDLE:
        workingmodeDesc = "IDLE";
        break;
    case WORKING_MODE_SCAN:
        workingmodeDesc = "SCAN";
        break;
    default:
        assert(!"should not come here");
    }

	RSLIDAR_RESPONSE_DEVICE_INFO_T & devinfo = LidarMgr::GetInstance().devinfo;

    sprintf(titleMsg, "[%s] GearNumber: %d"
        , workingmodeDesc
		,16);

    this->SetWindowTextA(titleMsg);
}

void    CMainFrame::onSwitchMode(int newMode)
{
    
    // switch mode
    if (newMode == workingMode) return;

    

    switch (newMode) {
    case WORKING_MODE_IDLE:
        {
            // stop the previous operation
            LidarMgr::GetInstance().lidar_drv->stop();
            UISetCheck(ID_CMD_STOP, 1);
            UISetCheck(ID_CMD_GRAB_PEAK, 0);
            UISetCheck(ID_CMD_GRAB_FRAME, 0);
            UISetCheck(ID_CMD_SCAN, 0);
            UISetCheck(ID_CMD_GRABFRAMENONEDIFF, 0);
        }
        break;
    case WORKING_MODE_SCAN:
        {
            CWindow  hwnd = m_hWndClient;
            hwnd.ShowWindow(SW_HIDE);
            m_hWndClient = m_scanview;
            m_scanview.ShowWindow(SW_SHOW);

#ifdef feidangong
			checkDeviceHealth();
#endif // feidangong

            LidarMgr::GetInstance().lidar_drv->startScan();
            UISetCheck(ID_CMD_STOP, 0);
            UISetCheck(ID_CMD_GRAB_PEAK, 0);
            UISetCheck(ID_CMD_GRAB_FRAME, 0);
            UISetCheck(ID_CMD_SCAN, 1);
            UISetCheck(ID_CMD_GRABFRAMENONEDIFF, 0);
        }
        break;
    default:
        assert(!"unsupported mode");
    }
    
    UpdateLayout();
    workingMode = newMode;
    onUpdateTitle();
}

void    CMainFrame::checkDeviceHealth()
{
    int errorcode;
    if (!LidarMgr::GetInstance().checkDeviceHealth(&errorcode)){
        char msg[200];
        sprintf(msg, "The device is in unhealthy status.\n"
                   "You need to reset it.\n"
                   "Errorcode: 0x%08x", errorcode);
        
        MessageBox(msg, "Warning", MB_OK);

    }
}
