
// Circle_DrawDlgDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Circle_DrawDlg.h"
#include "Circle_DrawDlgDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.



UINT WorkThread(LPVOID pParam)
{
	CCircleDrawDlgDlg* pMainDlg = (CCircleDrawDlgDlg*)pParam;

	DWORD		dwResult;
	bool		m_bTerminate = false;

	while (1)
	{
		dwResult = WaitForSingleObject(pMainDlg->m_evtTerminateThread, 1);
		if (dwResult == WAIT_OBJECT_0) {
			TRACE("WorkThread 종료 Event 발생\n");
			break;
		}
		
		if (pMainDlg->m_bWorkThreadStart)
		{
			auto start = high_resolution_clock::now();

			while (true) {

				if (!pMainDlg->m_bWorkThreadStart) break;

				dwResult = WaitForSingleObject(pMainDlg->m_evtTerminateThread, 1);
				if (dwResult == WAIT_OBJECT_0) {
					m_bTerminate = true;
					TRACE("WorkThread 종료 Event 발생\n");
					break;
				}
				auto now = high_resolution_clock::now();
				auto duration = duration_cast<seconds>(now - start);

				if (duration.count() >= 2) {

					pMainDlg->strCnt.Format(_T("%d"), pMainDlg->m_nRandomCount);
					pMainDlg->MakeRandomCircle();
					pMainDlg->m_nRandomCount--;
					if (pMainDlg->m_nRandomCount == 0)pMainDlg->m_bWorkThreadStart = false;
					break;
				}
			}
		}

		Sleep(1);
		if (m_bTerminate) break;
	}
	pMainDlg->m_evtTerminateThreadDone.SetEvent();
	return 0;
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCircleDrawDlgDlg 대화 상자



CCircleDrawDlgDlg::CCircleDrawDlgDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CIRCLE_DRAWDLG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pWorkThreadHandle = NULL;
}

void CCircleDrawDlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCircleDrawDlgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_INIT, &CCircleDrawDlgDlg::OnBnClickedButtonInit)
	ON_CBN_SELCHANGE(IDC_COMBO_PENSIZE, &CCircleDrawDlgDlg::OnCbnSelchangeComboPensize)
	ON_BN_CLICKED(IDC_BUTTON_RANDOMCIRCLE, &CCircleDrawDlgDlg::OnBnClickedButtonRandomcircle)
	ON_BN_CLICKED(IDC_BUTTON_RANDOMCIRCLE2, &CCircleDrawDlgDlg::OnBnClickedButtonRandomcircle2)
	ON_CBN_SELCHANGE(IDC_COMBO_POINTSIZE, &CCircleDrawDlgDlg::OnSelchangeComboPointsize)
END_MESSAGE_MAP()


// CCircleDrawDlgDlg 메시지 처리기

BOOL CCircleDrawDlgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	((CComboBox*)GetDlgItem(IDC_COMBO_POINTSIZE))->SetCurSel(m_nPointSize - 3);
	((CComboBox*)GetDlgItem(IDC_COMBO_PENSIZE))->SetCurSel(m_nPenSize - 1);

	CRect ChecRectSize;
	this->GetWindowRect(ChecRectSize);

	strCnt = _T("");
	m_CanvSize.x = 640;
	m_CanvSize.y = 480;
	
	int nBpp = 32; // Make color buffer
	m_ImageBuf.Create(m_CanvSize.x, -m_CanvSize.y, nBpp);
	m_nPitch = m_ImageBuf.GetPitch();
	BufferClear();

	m_nRandomCount = 0;
	m_bWorkThreadStart = false;
	m_evtTerminateThread.ResetEvent();
	m_evtTerminateThreadDone.ResetEvent();
	m_pWorkThreadHandle = ::AfxBeginThread(WorkThread, this);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CCircleDrawDlgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCircleDrawDlgDlg::BufferClear()
{
	unsigned char* fm = (unsigned char*)m_ImageBuf.GetBits();
	memset(fm, 0xff, m_CanvSize.x * m_CanvSize.y * 4);
}

void CCircleDrawDlgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		if (m_ImageBuf)
		{
			m_ImageBuf.BitBlt(dc.m_hDC, 0, 0, SRCCOPY);

			if (strCnt.GetLength()>0)
			{
				strCnt.Format(_T("%d"), m_nRandomCount);
				dc.DrawText(strCnt, CRect(10, 10, 100, 50), TA_TOP);
			}
		}
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCircleDrawDlgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCircleDrawDlgDlg::DrawPoint(unsigned char* fm, int nCenterX, int nCenterY, int nRadius, int nBlue, int nGreen, int nRed)
{
	for (int y = -nRadius; y <= nRadius; y++) {
		for (int x = -nRadius; x <= nRadius; x++) {
			if (x * x + y * y <= nRadius * nRadius) {
				int px = nCenterX + x;
				int py = nCenterY + y;

				if (px >= 0 && px < m_CanvSize.x && py >= 0 && py < m_CanvSize.y) {
					BYTE* pixel = fm + (py * m_nPitch) + (px * 4);
					pixel[0] = nBlue;		// B Channel
					pixel[1] = nGreen;		// G Channel
					pixel[2] = nRed;		// R Channel
					pixel[3] = 255;			// A Channel
				}
			}
		}
	}
}

void CCircleDrawDlgDlg::DrawCircle(unsigned char* fm, int nCenterX, int nCenterY, int nRadius, int nBorder, int nBlue, int nGreen, int nRed)
{
	for (int y = -nRadius - nBorder; y <= nRadius + nBorder; y++) {
		for (int x = -nRadius - nBorder; x <= nRadius + nBorder; x++) {
			int distance = x * x + y * y;
			int outerRadius = (nRadius + nBorder) * (nRadius + nBorder);
			int innerRadius = (nRadius - nBorder) * (nRadius - nBorder);

			// Border 범위 점검.
			if (distance <= outerRadius && distance >= innerRadius)	{
				int px = nCenterX + x;
				int py = nCenterY + y;

				if (px >= 0 && px < m_CanvSize.x && py >= 0 && py < m_CanvSize.y) {
					BYTE* pixel = fm + (py * m_nPitch) + (px * 4);
					pixel[0] = nBlue;		// B Channel
					pixel[1] = nGreen;		// G Channel
					pixel[2] = nRed;		// R Channel
					pixel[3] = 255;			// A Channel
				}
			}
		}
	}
}

void CCircleDrawDlgDlg::BufferDraw()
{
	BufferClear();

	unsigned char* fm = (unsigned char*)m_ImageBuf.GetBits();

	if (m_vtPoints.size() == 3)
	{
		CPoint center = CalculateCircleCenter();
		double radius = CalculateRadius(center);
		DrawCircle(fm, center.x, center.y, radius, m_nPenSize);

		DrawPoint(fm, center.x, center.y, m_nPointSize, 0, 0, 255);
	}

	if (m_vtPoints.size() < 4)
	{
		for(int n=0; n< m_vtPoints.size(); n++) {
			DrawPoint(fm, m_vtPoints[n].x, m_vtPoints[n].y, m_nPointSize, 255, 0, 0);
		}
	}
}

void CCircleDrawDlgDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_vtPoints.size() < 3) {
		m_vtPoints.push_back(point);
	}
	else {
		// 포인트 범위 클릭위치 체크
		for (int i = 0; i < 3; ++i) {
			if (abs(m_vtPoints[i].x - point.x) < m_nPointSize && abs(m_vtPoints[i].y - point.y) < m_nPointSize) {
				m_bIsDragging = true;
				m_nDragIndex = i;
				break;
			}
		}
	}
	BufferDraw();
	Invalidate();

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CCircleDrawDlgDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsDragging && m_nDragIndex != -1) {
		m_vtPoints[m_nDragIndex] = point;
		BufferDraw();
		Invalidate();
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

double CCircleDrawDlgDlg::CalculateRadius(const CPoint& center) {
	if (m_vtPoints.size() < 3) return 0.0;
	return sqrt(pow(m_vtPoints[0].x - center.x, 2) + pow(m_vtPoints[0].y - center.y, 2));
}

CPoint CCircleDrawDlgDlg::CalculateCircleCenter()
{
	if (m_vtPoints.size() < 3) return CPoint(0, 0);

	int x1 = m_vtPoints[0].x, y1 = m_vtPoints[0].y;
	int x2 = m_vtPoints[1].x, y2 = m_vtPoints[1].y;
	int x3 = m_vtPoints[2].x, y3 = m_vtPoints[2].y;

	double A = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
	if (A == 0) return CPoint(0, 0);

	double B = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
	double C = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);

	int cx = int(-B / (2 * A));
	int cy = int(-C / (2 * A));

	return CPoint(cx, cy);
}

void CCircleDrawDlgDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsDragging = false;

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CCircleDrawDlgDlg::OnBnClickedButtonInit()
{
	strCnt = _T("");
	m_vtPoints.clear();
	BufferClear();
	Invalidate();
}

void CCircleDrawDlgDlg::OnSelchangeComboPointsize()
{
	m_nPointSize = ((CComboBox*)GetDlgItem(IDC_COMBO_POINTSIZE))->GetCurSel() + 3;
	BufferDraw();
	Invalidate();
}

void CCircleDrawDlgDlg::OnCbnSelchangeComboPensize()
{
	m_nPenSize = ((CComboBox*)GetDlgItem(IDC_COMBO_PENSIZE))->GetCurSel()+1;
	BufferDraw();
	Invalidate();
}

void CCircleDrawDlgDlg::MakeRandomCircle()
{
	m_vtPoints.clear();

	for (int n = 0; n < 3; n++)
	{
		CPoint RdPoint;
		RdPoint.x = rand() % 641;
		RdPoint.y = rand() % 481;
		m_vtPoints.push_back(RdPoint);

		BufferDraw();
	}
	Invalidate();
}

void CCircleDrawDlgDlg::OnBnClickedButtonRandomcircle()
{
	strCnt = _T("");
	MakeRandomCircle();
}

void CCircleDrawDlgDlg::OnBnClickedButtonRandomcircle2()
{
	if (m_bWorkThreadStart) {

		SetDlgItemText(IDC_BUTTON_RANDOMCIRCLE2, _T("무작위 생성 시작"));
		m_bWorkThreadStart = false;
	}
	else {
		SetDlgItemText(IDC_BUTTON_RANDOMCIRCLE2, _T("무작위 생성 멈춤"));
		strCnt = _T("");
		m_nRandomCount = 10;
		m_bWorkThreadStart = true;
	}
}

void CCircleDrawDlgDlg::OnOK()
{
	m_bWorkThreadStart = false;

	if (m_ImageBuf) m_ImageBuf.Destroy();

	if (m_pWorkThreadHandle) {
		m_evtTerminateThread.SetEvent();
		DWORD	dwResult = WaitForSingleObject(m_evtTerminateThreadDone, 500);
		if (dwResult == WAIT_TIMEOUT) TRACE("WorkThread 비정상 종료\n");
	}

	CDialogEx::OnOK();
}