#include <stdafx.h>
#include "MasterFrames.h"
#include "DSSProgress.h"
#include "DeBloom.h"

/* ------------------------------------------------------------------- */

bool CMasterFrames::LoadMasters(const CStackingInfo* pStackingInfo, CDSSProgress * pProgress)
{
	ZFUNCTRACE_RUNTIME();
	bool bResult = true;

	if (pStackingInfo->m_pOffsetTask != nullptr)
		bResult = GetTaskResult(pStackingInfo->m_pOffsetTask, pProgress, m_pMasterOffset);

	if (pStackingInfo->m_pDarkTask != nullptr)
	{
		std::shared_ptr<CMemoryBitmap> pMasterDark;
		bResult = bResult && GetTaskResult(pStackingInfo->m_pDarkTask, pProgress, pMasterDark);

		if (bResult)
			m_MasterDark.SetMasterDark(pMasterDark);
	}

	if (pStackingInfo->m_pDarkFlatTask)
	{
		std::shared_ptr<CMemoryBitmap> pMasterDarkFlat;
		bResult = bResult && GetTaskResult(pStackingInfo->m_pDarkFlatTask, pProgress, pMasterDarkFlat);
		// ### WTF??? Nothing is done with pMasterDarkFlat!
	}

	if (pStackingInfo->m_pFlatTask)
	{
		bResult = bResult && GetTaskResult(pStackingInfo->m_pFlatTask, pProgress, m_MasterFlat.m_pFlatFrame);
		if (bResult)
			m_MasterFlat.ComputeFlatNormalization(pProgress);
	}

	return bResult;
}

/* ------------------------------------------------------------------- */

void	CMasterFrames::ApplyMasterOffset(std::shared_ptr<CMemoryBitmap> pBitmap, CDSSProgress * pProgress)
{
	ZFUNCTRACE_RUNTIME();
	CString				strText;

	ZTRACE_RUNTIME("Subtracting Offset Frame");

	if (m_pMasterOffset && m_pMasterOffset->IsOk())
	{
		if (pProgress)
		{
			strText.LoadString(IDS_SUBSTRACTINGOFFSET);
			pProgress->Start2(strText, 0);
		};
		Subtract(pBitmap, m_pMasterOffset, pProgress);
	};
}

void CMasterFrames::ApplyMasterDark(std::shared_ptr<CMemoryBitmap> pBitmap, const STARVECTOR*, CDSSProgress * pProgress)
{
	ZFUNCTRACE_RUNTIME();
	ZTRACE_RUNTIME("Subtracting Dark Frame");

	if (m_MasterDark.IsOk())
		m_MasterDark.Subtract(pBitmap, pProgress);
}

void	CMasterFrames::ApplyMasterFlat(std::shared_ptr<CMemoryBitmap> pBitmap, CDSSProgress * pProgress)
{
	ZFUNCTRACE_RUNTIME();
	CString				strText;

	if (m_MasterFlat.IsOk())
	{
		m_MasterFlat.ComputeFlatNormalization(pProgress);
		if (pProgress)
		{
			strText.LoadString(IDS_APPLYINGFLAT);
			pProgress->Start2(strText, 0);
		};
		m_MasterFlat.ApplyFlat(pBitmap, pProgress);
	}
}

void	CMasterFrames::ApplyHotPixelInterpolation(std::shared_ptr<CMemoryBitmap> pBitmap, CDSSProgress * pProgress)
{
	ZFUNCTRACE_RUNTIME();
	if (m_MasterDark.IsOk())
		m_MasterDark.InterpolateHotPixels(pBitmap, pProgress);
}

void CMasterFrames::ApplyAllMasters(std::shared_ptr<CMemoryBitmap> pBitmap, const STARVECTOR* pStars, CDSSProgress* pProgress)
{
	ZFUNCTRACE_RUNTIME();
	CDeBloom debloom;

	if (m_fDebloom)
		debloom.CreateBloomMask(pBitmap.get(), pProgress);

	ApplyMasterOffset(pBitmap, pProgress);
	ApplyMasterDark(pBitmap, pStars, pProgress);
	ApplyMasterFlat(pBitmap, pProgress);
	ApplyHotPixelInterpolation(pBitmap, pProgress);

	debloom.DeBloomImage(pBitmap.get(), pProgress);
}
