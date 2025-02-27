#include <stdafx.h>
#include "FrameInfo.h"
#include "BitmapInfo.h"
#include "BitmapExt.h"


/* ------------------------------------------------------------------- */

bool CFrameInfo::InitFromFile(const fs::path& file, PICTURETYPE Type)
{
	bool				bResult = false;

	if (Type == PICTURETYPE_REFLIGHTFRAME)
		m_PictureType  = PICTURETYPE_LIGHTFRAME;
	else
		m_PictureType  = Type;
	filePath = file;

	CBitmapInfo			bmpInfo;

	bResult = GetPictureInfo(file.c_str() ,bmpInfo);

	if (bResult)
	{
		m_lWidth			= bmpInfo.m_lWidth;
		m_lHeight			= bmpInfo.m_lHeight;
		m_lBitPerChannels	= bmpInfo.m_lBitPerChannel;
		m_lNrChannels		= bmpInfo.m_lNrChannels;
		m_CFAType			= bmpInfo.m_CFAType;
		m_bFITS16bit		= bmpInfo.m_bFITS16bit;
		m_DateTime			= bmpInfo.m_DateTime;
		m_strDateTime		= bmpInfo.m_strDateTime.toStdWString().c_str();
		m_ExtraInfo			= bmpInfo.m_ExtraInfo;

		QString strInfo;
		bmpInfo.GetDescription(strInfo);
		m_strInfos = strInfo.toStdWString().c_str();

		m_lISOSpeed			= bmpInfo.m_lISOSpeed;
		m_lGain				= bmpInfo.m_lGain;
		m_fExposure			= bmpInfo.m_fExposure;
		m_fAperture			= bmpInfo.m_fAperture;

		m_bMaster			= bmpInfo.IsMaster();
		m_filterName		= bmpInfo.m_filterName;

		RefreshSuperPixel();
	};

	return bResult;
}

void CFrameInfo::CopyFrom(const CFrameInfo& cfi)
{
	m_lWidth = cfi.m_lWidth;
	m_lHeight = cfi.m_lHeight;
	filePath = cfi.filePath;
	m_lISOSpeed = cfi.m_lISOSpeed;
	m_lGain = cfi.m_lGain;
	m_fExposure = cfi.m_fExposure;
	m_fAperture = cfi.m_fAperture;
	m_PictureType = cfi.m_PictureType;
	m_CFAType = cfi.m_CFAType;
	m_lBitPerChannels = cfi.m_lBitPerChannels;
	m_lNrChannels = cfi.m_lNrChannels;
	m_strDateTime = cfi.m_strDateTime;
	m_DateTime = cfi.m_DateTime;
	m_bMaster = cfi.m_bMaster;
	m_strInfos = cfi.m_strInfos;
	m_bFITS16bit = cfi.m_bFITS16bit;
	m_bSuperPixel = cfi.m_bSuperPixel;
	m_ExtraInfo = cfi.m_ExtraInfo;
	m_filterName = cfi.m_filterName;
}

void CFrameInfo::Reset()
{
	m_lISOSpeed = 0;
	m_lGain = -1;
	m_fExposure = 0.0;
	m_fAperture = 0.0;
	m_PictureType = PICTURETYPE_LIGHTFRAME;
	m_CFAType = CFATYPE_NONE;
	m_lNrChannels = 3;
	m_lBitPerChannels = 16;
	m_bMaster = false;
	m_bFITS16bit = false;
	m_bSuperPixel = false;
	m_ExtraInfo.Clear();
	m_lWidth = 0;
	m_lHeight = 0;
}

bool CFrameInfo::IsCompatible(int lWidth, int lHeight, int lBitPerChannels, int lNrChannels, CFATYPE CFAType) const
{
	bool			result = true;

	if (m_lWidth != lWidth)
	{
		incompatibilityReason = QCoreApplication::translate("DSS::StackingDlg", "Width mismatch");
		return false;
	}
	if (m_lHeight != lHeight)
	{
		incompatibilityReason = QCoreApplication::translate("DSS::StackingDlg", "Height mismatch");
		return false;
	}
	if (m_lBitPerChannels != lBitPerChannels)
	{
		incompatibilityReason = QCoreApplication::translate("DSS::StackingDlg", "Colour depth mismatch");
		return false;
	}

	if (result)
	{
		result = (m_lNrChannels == lNrChannels) && (m_CFAType == CFAType);
		if (!result)
		{
			// Check that if CFA if Off then the number of channels may be
			// 3 instead of 1 if BayerDrizzle and SuperPixels are off
			if (!IsRawBayer() && !IsSuperPixels() && !IsFITSRawBayer() && !IsFITSSuperPixels())
			{
				if (m_CFAType != CFAType)
				{
					if ((m_CFAType != CFATYPE_NONE) && (m_lNrChannels == 1))
						result = (CFAType != CFATYPE_NONE) && (lNrChannels == 3);
					else if ((CFAType == CFATYPE_NONE) && (lNrChannels == 1))
						result = (m_CFAType == CFATYPE_NONE) && (m_lNrChannels == 3);
				};
				if (false == result)
					incompatibilityReason = QCoreApplication::translate("DSS::StackingDlg", "Number of channels mismatch");
			};
		};
	};

	return  result;
}

CFATYPE	CFrameInfo::GetCFAType() const
{
	if (m_bFITS16bit)
	{
		//
		// On the old days this was always determined by the values read from the 
		// workspace by GetFITSCFATYPE().  Now however GetFITSInfo() may auto-detect
		// the CFA pattern and pre-populate CFAType, which we should now use.  If it's
		// not set then do it the old way.
		//
		if (m_CFAType != CFATYPE_NONE)
			return m_CFAType;
		else
			return m_CFAType = GetFITSCFATYPE();
	}
	else
		return m_CFAType;
}

bool CFrameInfo::IsSuperPixel() const
{
	if (m_bFITS16bit)
		m_bSuperPixel = IsFITSSuperPixels() && (GetCFAType() != CFATYPE_NONE);

	return m_bSuperPixel;
}

void CFrameInfo::RefreshSuperPixel()
{
	m_bSuperPixel = false;
	if (m_bFITS16bit)
		m_bSuperPixel = IsFITSSuperPixels() && (GetCFAType() != CFATYPE_NONE);
	else if (m_CFAType != CFATYPE_NONE)
		m_bSuperPixel = ::IsSuperPixels();
}