#pragma once

#include "BitmapConstants.h"
#include "DSSCommon.h"
#include "BitmapExtraInfo.h"
#include "ColorRef.h"
#include "cfa.h"
#include "BitmapInfo.h"

namespace DSS { class ProgressBase; }
class CMemoryBitmap;
class CBitmapCharacteristics;

/* ------------------------------------------------------------------- */

void FormatFromMethod(QString& strText, MULTIBITMAPPROCESSMETHOD Method, double fKappa, int lNrIterations);
void FormatMethod(QString& strText, MULTIBITMAPPROCESSMETHOD Method, double fKappa, int lNrIterations);
bool Subtract(std::shared_ptr<CMemoryBitmap> pTarget, std::shared_ptr<const CMemoryBitmap> pSource, DSS::ProgressBase* pProgress = nullptr, double fRedFactor = 1.0, double fGreenFactor = 1.0, double fBlueFactor = 1.0);
bool Add(std::shared_ptr<CMemoryBitmap> pTarget, std::shared_ptr<const CMemoryBitmap> pSource, DSS::ProgressBase* pProgress = nullptr);
bool ShiftAndSubtract(std::shared_ptr<CMemoryBitmap> pTarget, std::shared_ptr<const CMemoryBitmap> pSource, DSS::ProgressBase* pProgress = nullptr, double fXShift = 0, double fYShift = 0);

bool FetchPicture(const fs::path filePath, std::shared_ptr<CMemoryBitmap>& rpBitmap, const bool ignoreBrightness, DSS::ProgressBase* const pProgress);
bool GetPictureInfo(LPCTSTR szFileName, CBitmapInfo& BitmapInfo);
std::shared_ptr<CMemoryBitmap> GetFilteredImage(const CMemoryBitmap* pInBitmap, const int lFilterSize, DSS::ProgressBase* pProgress = nullptr);

//////////////////////////////////////////////////////////////////////////

bool IsFITSRawBayer();		// From FITSUtil.h
bool IsFITSSuperPixels();	// From FITSUtil.h

using pByte = std::uint8_t*;
class C32BitsBitmap
{
private:
	HBITMAP m_hBitmap;
	void* m_lpBits;
	int m_lWidth;
	int m_lHeight;
	pByte* m_pLine;
	std::uint32_t m_dwByteWidth;

private:
	void InitInternals();

public:
	C32BitsBitmap();
	virtual ~C32BitsBitmap();

	virtual int	Width() { return m_lWidth; }
	virtual int	Height() { return m_lHeight; }
	virtual void Init(int lWidth, int lHeight);

	void* GetBits() { return m_lpBits; }
	int	ByteWidth() { return m_dwByteWidth; }
	HBITMAP GetHBITMAP() { return m_hBitmap; }
	HBITMAP	Create(int lWidth, int lHeight);
	bool InitFrom(CMemoryBitmap* pBitmap);
	bool CopyToClipboard();
	bool IsEmpty() { return (m_hBitmap == nullptr); }
	void Free();
	HBITMAP Detach();
	COLORREF GetPixel(int x, int y);
	pByte GetPixelBase(int x, int y);
	void SetPixel(int x, int y, COLORREF crColor);
};

class	CGammaTransformation
{
public:
	std::vector<std::uint8_t> m_vTransformation;

public:
	CGammaTransformation() {};
	~CGammaTransformation() {};

	void InitTransformation(double fBlackPoint, double fGrayPoint, double fWhitePoint);
	void InitTransformation(double fGamma);
	bool IsInitialized() { return (m_vTransformation.size() == 65537); }
};

class CAllDepthBitmap
{
public:
	bool m_bDontUseAHD;
	std::shared_ptr<CMemoryBitmap> m_pBitmap;
	std::shared_ptr<C32BitsBitmap> m_pWndBitmap;
	std::shared_ptr<QImage> m_Image;

    CAllDepthBitmap() : m_bDontUseAHD(false) {};
	~CAllDepthBitmap() {};
	CAllDepthBitmap(const CAllDepthBitmap& adb) = default;
	CAllDepthBitmap& operator=(const CAllDepthBitmap& adb) = default;

	void Clear();
	void SetDontUseAHD(bool bSet){m_bDontUseAHD = bSet;}
	bool initQImage();
};

void CopyBitmapToClipboard(HBITMAP hBitmap);
bool LoadPicture(LPCTSTR szFileName, CAllDepthBitmap & AllDepthBitmap, DSS::ProgressBase* pProgress = nullptr);
bool DebayerPicture(CMemoryBitmap* pInBitmap, std::shared_ptr<CMemoryBitmap>& rpOutBitmap, DSS::ProgressBase* pProgress);

bool	ApplyGammaTransformation(C32BitsBitmap* pOutBitmap, CMemoryBitmap* pInBitmap, CGammaTransformation& gammatrans);
bool	ApplyGammaTransformation(QImage* pImage, CMemoryBitmap* pInBitmap, CGammaTransformation& gammatrans);

