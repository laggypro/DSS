// ExplorerBar.cpp : implementation file
//

#include "stdafx.h"
#include "GraphView.h"
#include "ChartCtrl.h"
#include "ChartObject.h"
#include "ChartLegend.h"
#include "ChartAxis.h"
#include "ChartSerie.h"
#include "ChartPointsSerie.h"

/* ------------------------------------------------------------------- */
// CGraphViewTab dialog

IMPLEMENT_DYNAMIC(CGraphViewTab, CDialog)

CGraphViewTab::CGraphViewTab(CWnd* pParent /*=nullptr*/, bool bDarkMode /*=false*/)
	: CDialog(CGraphViewTab::IDD, pParent),
	m_bDarkMode(bDarkMode),
	m_Graph(bDarkMode)
{
}

/* ------------------------------------------------------------------- */

CGraphViewTab::~CGraphViewTab()
{
}

/* ------------------------------------------------------------------- */

void CGraphViewTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCORE, m_Score);
	DDX_Control(pDX, IDC_FWHM, m_FWHM);
	DDX_Control(pDX, IDC_STARS, m_Stars);
	DDX_Control(pDX, IDC_OFFSET, m_Offset);
	DDX_Control(pDX, IDC_ANGLE, m_Angle);
	DDX_Control(pDX, IDC_SKYBACKGROUND, m_SkyBackground);
	DDX_Control(pDX, IDC_GRAPH, m_Graph);
}

/* ------------------------------------------------------------------- */

BEGIN_MESSAGE_MAP(CGraphViewTab, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SCORE, OnScore)
	ON_BN_CLICKED(IDC_FWHM, OnFWHM)
	ON_BN_CLICKED(IDC_STARS, OnStars)
	ON_BN_CLICKED(IDC_OFFSET, OnOffset)
	ON_BN_CLICKED(IDC_ANGLE, OnAngle)
	ON_BN_CLICKED(IDC_SKYBACKGROUND, OnSkyBackground)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/* ------------------------------------------------------------------- */

void CGraphViewTab::ChangeVisibleGraph()
{
	if (m_Score.GetCheck())
	{
		m_csAngle.SetVisible(false);
		m_csScores.SetVisible(true);
		m_csStars.SetVisible(false);
		m_csdX.SetVisible(false);
		m_csdY.SetVisible(false);
		m_csFWHM.SetVisible(false);
		m_csSkyBackground.SetVisible(false);
		m_Graph.GetLegend()->SetVisible(FALSE);
	}
	else if (m_FWHM.GetCheck())
	{
		m_csAngle.SetVisible(false);
		m_csScores.SetVisible(false);
		m_csStars.SetVisible(false);
		m_csdX.SetVisible(false);
		m_csdY.SetVisible(false);
		m_csFWHM.SetVisible(true);
		m_csSkyBackground.SetVisible(false);
		m_Graph.GetLegend()->SetVisible(FALSE);
	}
	else if (m_Stars.GetCheck())
	{
		m_csAngle.SetVisible(false);
		m_csScores.SetVisible(false);
		m_csStars.SetVisible(true);
		m_csdX.SetVisible(false);
		m_csdY.SetVisible(false);
		m_csFWHM.SetVisible(false);
		m_csSkyBackground.SetVisible(false);
		m_Graph.GetLegend()->SetVisible(FALSE);
	}
	else if (m_Offset.GetCheck())
	{
		m_csAngle.SetVisible(false);
		m_csScores.SetVisible(false);
		m_csStars.SetVisible(false);
		m_csdX.SetVisible(true);
		m_csdY.SetVisible(true);
		m_csFWHM.SetVisible(false);
		m_csSkyBackground.SetVisible(false);
		m_Graph.GetLegend()->SetVisible(FALSE);
	}
	else if (m_Angle.GetCheck())
	{
		m_csAngle.SetVisible(true);
		m_csScores.SetVisible(false);
		m_csStars.SetVisible(false);
		m_csdX.SetVisible(false);
		m_csdY.SetVisible(false);
		m_csFWHM.SetVisible(false);
		m_csSkyBackground.SetVisible(false);
		m_Graph.GetLegend()->SetVisible(FALSE);
	}
	else if (m_SkyBackground.GetCheck())
	{
		m_csAngle.SetVisible(false);
		m_csScores.SetVisible(false);
		m_csStars.SetVisible(false);
		m_csdX.SetVisible(false);
		m_csdY.SetVisible(false);
		m_csFWHM.SetVisible(false);
		m_csSkyBackground.SetVisible(true);
		m_Graph.GetLegend()->SetVisible(FALSE);
	};

	m_Graph.GetLeftAxis()->SetAutomatic(true);
	m_Graph.GetLeftAxis()->SetAutoMargin(true, 5);
	m_Graph.RefreshCtrl();
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	m_ControlPos.MoveControls();
}

/* ------------------------------------------------------------------- */

BOOL CGraphViewTab::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ControlPos.SetParent(this);

	m_ControlPos.AddControl(IDC_GRAPH, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL);

	m_Score.SetCheck(TRUE);

	if (m_bDarkMode)
	{
		m_Graph.SetBackColor(RGB(51, 51, 51));
		m_csScores.Init(m_Graph, RGB(200, 0, 0));
		m_csFWHM.Init(m_Graph, RGB(200, 0, 0));
		m_csStars.Init(m_Graph, RGB(0, 100, 200));
		m_csdX.Init(m_Graph, RGB(200, 0, 0));
		m_csdY.Init(m_Graph, RGB(0, 100, 200));
		m_csAngle.Init(m_Graph, RGB(0, 200, 0));
		m_csSkyBackground.Init(m_Graph, RGB(0, 100, 200));
	}
	else
	{
		m_Graph.SetBackColor(RGB(255, 255, 255));
		m_csScores.Init(m_Graph, RGB(255, 0, 0));
		m_csFWHM.Init(m_Graph, RGB(128, 0, 0));
		m_csStars.Init(m_Graph, RGB(0, 0, 128));
		m_csdX.Init(m_Graph, RGB(255, 0, 0));
		m_csdY.Init(m_Graph, RGB(0, 0, 255));
		m_csAngle.Init(m_Graph, RGB(0, 255, 0));
		m_csSkyBackground.Init(m_Graph, RGB(0, 0, 128));
	}

	m_csdX.SetName(_T("dX"));
	m_csdY.SetName(_T("dY"));

	m_Graph.GetBottomAxis()->SetAutomatic(true);
	m_Graph.GetLeftAxis()->SetAutomatic(true);
	m_Graph.SetPanEnabled(false);
	m_Graph.SetZoomEnabled(false);

	ChangeVisibleGraph();

	return TRUE;
}

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnScore()
{
	if (m_Score.GetCheck())
	{
		m_FWHM.SetCheck(FALSE);
		m_Stars.SetCheck(FALSE);
		m_Offset.SetCheck(FALSE);
		m_Angle.SetCheck(FALSE);
		m_SkyBackground.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnFWHM()
{
	if (m_FWHM.GetCheck())
	{
		m_Score.SetCheck(FALSE);
		m_Stars.SetCheck(FALSE);
		m_Offset.SetCheck(FALSE);
		m_Angle.SetCheck(FALSE);
		m_SkyBackground.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnStars()
{
	if (m_Stars.GetCheck())
	{
		m_Score.SetCheck(FALSE);
		m_FWHM.SetCheck(FALSE);
		m_Offset.SetCheck(FALSE);
		m_Angle.SetCheck(FALSE);
		m_SkyBackground.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnOffset()
{
	if (m_Offset.GetCheck())
	{
		m_Score.SetCheck(FALSE);
		m_FWHM.SetCheck(FALSE);
		m_Stars.SetCheck(FALSE);
		m_Angle.SetCheck(FALSE);
		m_SkyBackground.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnAngle()
{
	if (m_Angle.GetCheck())
	{
		m_Score.SetCheck(FALSE);
		m_FWHM.SetCheck(FALSE);
		m_Stars.SetCheck(FALSE);
		m_Offset.SetCheck(FALSE);
		m_SkyBackground.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

HBRUSH CGraphViewTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (!m_bDarkMode)
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	CBrush Nothing;
	Nothing.CreateStockObject(NULL_BRUSH);
	return Nothing;
}

/* ------------------------------------------------------------------- */

BOOL CGraphViewTab::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bDarkMode)
		return CDialog::OnEraseBkgnd(pDC);

	CRect rect;
	GetClientRect(&rect);
	CBrush myBrush(RGB(80, 80, 80));    // dialog background color
	CBrush *pOld = pDC->SelectObject(&myBrush);
	BOOL bRes = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOld);    // restore old brush
	return bRes;                       // CDialog::OnEraseBkgnd(pDC);
}

/* ------------------------------------------------------------------- */

void CGraphViewTab::OnSkyBackground()
{
	if (m_SkyBackground.GetCheck())
	{
		m_Score.SetCheck(FALSE);
		m_FWHM.SetCheck(FALSE);
		m_Stars.SetCheck(FALSE);
		m_Offset.SetCheck(FALSE);
		m_Angle.SetCheck(FALSE);
		ChangeVisibleGraph();
	};
};

/* ------------------------------------------------------------------- */

void	CGraphViewTab::AddScoreFWHMStars(LPCTSTR szFileName, double fScore, double fFWHM, double fStars, double fSkyBackground)
{
	double				fX = m_csScores.m_pMain->GetPointsCount()+1;

	m_csScores.AddPoint(fX, fScore);
	m_csFWHM.AddPoint(fX, fFWHM);
	m_csStars.AddPoint(fX, fStars);
	m_csSkyBackground.AddPoint(fX, fSkyBackground);

	m_vFiles.push_back(szFileName);
};

/* ------------------------------------------------------------------- */

void	CGraphViewTab::AddOffsetAngle(LPCTSTR szFileName, double fdX, double fdY, double fAngle)
{
	BOOL				bFound = FALSE;

	for (LONG i = (LONG)m_vFiles.size()-1;i>=0 && !bFound;i--)
	{
		if (!m_vFiles[i].CompareNoCase(szFileName))
		{
			bFound = TRUE;
			m_csdX.AddPoint(i+1, fdX);
			m_csdY.AddPoint(i+1, fdY);
			m_csAngle.AddPoint(i+1, fAngle);
		};
	};
};

/* ------------------------------------------------------------------- */

void CGraphViewTab::SetPoint(LPCTSTR szFileName, POINTTYPE ptType, CHARTTYPE ctType)
{
	BOOL				bFound = FALSE;

	for (LONG i = (LONG)(m_vFiles.size())-1;i>=0 && !bFound;i--)
	{
		if (!m_vFiles[i].CompareNoCase(szFileName))
		{
			bFound = TRUE;
			switch (ctType)
			{
			case CT_SCORE :
				m_csScores.SetPoint(i+1, ptType);
				break;
			case CT_FWHM :
				m_csFWHM.SetPoint(i+1, ptType);
				break;
			case CT_STARS :
				m_csStars.SetPoint(i+1, ptType);
				break;
			case CT_DX :
				m_csdX.SetPoint(i+1, ptType);
				break;
			case CT_DY :
				m_csdY.SetPoint(i+1, ptType);
				break;
			case CT_ANGLE :
				m_csAngle.SetPoint(i+1, ptType);
				break;
			case CT_SKYBACKGROUND :
				m_csSkyBackground.SetPoint(i+1, ptType);
				break;
			};
		};
	};

	m_Graph.RefreshCtrl();
}

/* ------------------------------------------------------------------- */

void CGraphViewTab::ChangeImageInfo(LPCTSTR szFileName, STACKIMAGEINFO info)
{
	switch (info)
	{
	case II_SETREFERENCE :
		SetPoint(szFileName, PT_REFERENCE, CT_DX);
		SetPoint(szFileName, PT_REFERENCE, CT_DY);
		SetPoint(szFileName, PT_REFERENCE, CT_ANGLE);
		break;
	case II_DONTSTACK_NONE :
		SetPoint(szFileName, PT_OK, CT_SCORE);
		SetPoint(szFileName, PT_OK, CT_FWHM);
		SetPoint(szFileName, PT_OK, CT_STARS);
		SetPoint(szFileName, PT_OK, CT_DX);
		SetPoint(szFileName, PT_OK, CT_DY);
		SetPoint(szFileName, PT_OK, CT_ANGLE);
		SetPoint(szFileName, PT_OK, CT_SKYBACKGROUND);
		break;
	case II_DONTSTACK_SCORE :
		SetPoint(szFileName, PT_WRONG, CT_SCORE);
		break;
	case II_DONTSTACK_STARS :
		SetPoint(szFileName, PT_WRONG, CT_STARS);
		break;
	case II_DONTSTACK_FWHM :
		SetPoint(szFileName, PT_WRONG, CT_FWHM);
		break;
	case II_DONTSTACK_DX :
		SetPoint(szFileName, PT_WRONG, CT_DX);
		break;
	case II_DONTSTACK_DY :
		SetPoint(szFileName, PT_WRONG, CT_DY);
		break;
	case II_DONTSTACK_ANGLE :
		SetPoint(szFileName, PT_WRONG, CT_ANGLE);
		break;
	case II_DONTSTACK_SKYBACKGROUND :
		SetPoint(szFileName, PT_WRONG, CT_SKYBACKGROUND);
		break;
	case II_WARNING_NONE :
		break;
	case II_WARNING_SCORE :
		SetPoint(szFileName, PT_WARNING, CT_SCORE);
		break;
	case II_WARNING_STARS :
		SetPoint(szFileName, PT_WARNING, CT_STARS);
		break;
	case II_WARNING_FWHM :
		SetPoint(szFileName, PT_WARNING, CT_FWHM);
		break;
	case II_WARNING_DX :
		SetPoint(szFileName, PT_WARNING, CT_DX);
		break;
	case II_WARNING_DY :
		SetPoint(szFileName, PT_WARNING, CT_DY);
		break;
	case II_WARNING_ANGLE :
		SetPoint(szFileName, PT_WARNING, CT_ANGLE);
		break;
	case II_WARNING_SKYBACKGROUND :
		SetPoint(szFileName, PT_WARNING, CT_SKYBACKGROUND);
		break;
	};
};

/* ------------------------------------------------------------------- */

BOOL CChartSeries::IsPointInSerie(double fX, double fY, CChartSerie* pSerie)
{
	BOOL			bResult = FALSE;

	for (LONG i = (LONG)(pSerie->GetPointsCount()) - 1; i >= 0 && !bResult; i--)
	{
		if (fX == pSerie->GetXPointValue(i) &&
			fY == pSerie->GetYPointValue(i))
			bResult = TRUE;
	};

	return bResult;
}
void CChartSeries::Init(CChartCtrl& Chart, COLORREF crColor)
{
	m_pChart = &Chart;

	m_pMain = Chart.AddSerie(CChartSerie::stLineSerie);
	m_pMain->SetVerticalAxis(false);
	m_pMain->SetColor(crColor);

	m_pReference = Chart.AddSerie(CChartSerie::stPointsSerie);
	m_pWarning = Chart.AddSerie(CChartSerie::stPointsSerie);
	m_pOk = Chart.AddSerie(CChartSerie::stPointsSerie);
	m_pWrong = Chart.AddSerie(CChartSerie::stPointsSerie);

	m_pReference->SetColor(RGB(0, 180, 0));
	m_pOk->SetColor(RGB(0, 255, 0));
	m_pWrong->SetColor(RGB(255, 0, 0));
	m_pWarning->SetColor(RGB(255, 190, 75));

	CChartPointsSerie* pPointSerie;

	pPointSerie = dynamic_cast<CChartPointsSerie*>(m_pReference);
	pPointSerie->SetVerticalAxis(false);
	pPointSerie->SetPointSize(11, 11);
	pPointSerie->SetPointType(CChartPointsSerie::ptRectangle);

	pPointSerie = dynamic_cast<CChartPointsSerie*>(m_pOk);
	pPointSerie->SetVerticalAxis(false);
	pPointSerie->SetPointSize(11, 11);
	pPointSerie->SetPointType(CChartPointsSerie::ptEllipse);

	pPointSerie = dynamic_cast<CChartPointsSerie*>(m_pWrong);
	pPointSerie->SetVerticalAxis(false);
	pPointSerie->SetPointSize(11, 11);
	pPointSerie->SetPointType(CChartPointsSerie::ptTriangle);

	pPointSerie = dynamic_cast<CChartPointsSerie*>(m_pWarning);
	pPointSerie->SetVerticalAxis(false);
	pPointSerie->SetPointSize(17, 17);
	pPointSerie->SetPointType(CChartPointsSerie::ptEllipse);
}

void CChartSeries::SetName(LPCTSTR szName)
{
	if (m_pMain)
		m_pMain->SetName((LPCSTR)CT2CA(szName));
}

void CChartSeries::SetVisible(bool bShow)
{
	m_pMain->SetVisible(bShow);
	m_pReference->SetVisible(bShow);
	m_pOk->SetVisible(bShow);
	m_pWrong->SetVisible(bShow);
	m_pWarning->SetVisible(bShow);
}

void CChartSeries::AddPoint(double fX, double fY)
{
	m_pMain->AddPoint(fX, fY);
}

void CChartSeries::SetPoint(double fX, POINTTYPE ptType)
{
	// First search the point in the Main serie
	BOOL				bFound = FALSE;
	double				fY;

	for (LONG i = (LONG)(m_pMain->GetPointsCount()) - 1; i >= 0 && !bFound; i--)
	{
		if (m_pMain->GetXPointValue(i) == fX)
		{
			bFound = TRUE;
			fY = m_pMain->GetYPointValue(i);
			if (ptType == PT_REFERENCE)
			{
				if (!IsPointInSerie(fX, fY, m_pReference))
					m_pReference->AddPoint(fX, fY);
			}
			else if (ptType == PT_OK)
			{
				if (!IsPointInSerie(fX, fY, m_pOk))
					m_pOk->AddPoint(fX, fY);
			}
			else if (ptType == PT_WRONG)
			{
				if (!IsPointInSerie(fX, fY, m_pWrong))
					m_pWrong->AddPoint(fX, fY);
			}
			else if (ptType == PT_WARNING)
			{
				if (!IsPointInSerie(fX, fY, m_pWarning))
					m_pWarning->AddPoint(fX, fY);
			};
		};
	};
}