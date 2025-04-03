
// Circle_DrawDlgDlg.h: 헤더 파일
//

#pragma once

#include <vector>
#include <thread>
#include <chrono>
using namespace std::chrono;

// CCircleDrawDlgDlg 대화 상자
class CCircleDrawDlgDlg : public CDialogEx
{
// 생성입니다.
public:
	CCircleDrawDlgDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CIRCLE_DRAWDLG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	int		m_nPenSize = 1;
	int		m_nPointSize = 5;
	CPoint	m_CanvSize;
	CImage	m_ImageBuf;

	std::vector<CPoint> m_vtPoints;

	bool m_bIsDragging = false;
	int m_nDragIndex = -1;

	void DrawCircle();
	void DrawPoint();
	void BufferClear();
	CPoint CalculateCircleCenter();
	double CalculateRadius(const CPoint& center);


	CWinThread* m_pWorkThreadHandle;
	CEvent		m_evtTerminateThread;
	CEvent		m_evtTerminateThreadDone;
	bool		m_bWorkThreadStart;


	int			m_nRandomCount;
	void		MakeRandomCircle();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonInit();
	afx_msg void OnCbnSelchangeComboPensize();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonRandomcircle();
	afx_msg void OnBnClickedButtonRandomcircle2();
	afx_msg void OnSelchangeComboPointsize();
};
