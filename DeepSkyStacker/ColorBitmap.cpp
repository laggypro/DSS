#include "stdafx.h"
#include "ColorBitmap.h"
#include "ColorHelpers.h"
#include "ZExcept.h"
#include "ColorMultiBitmap.h"
#include "MedianFilterEngine.h"
#include "BitmapCharacteristics.h"


template <typename TType>
void CColorBitmapT<TType>::CheckXY(size_t x, size_t y) const
{
	ZASSERTSTATE(x >= 0 && x < m_lWidth&& y >= 0 && y < m_lHeight);
}

template <typename TType>
size_t CColorBitmapT<TType>::GetOffset(const size_t x, const size_t y) const
{
	return m_bTopDown ? static_cast<size_t>(m_lWidth) * y + x : static_cast<size_t>(m_lWidth) * (static_cast<size_t>(m_lHeight) - 1 - y) + x;
}
template <typename TType>
size_t CColorBitmapT<TType>::GetOffset(int x, int y) const
{
	CheckXY(x, y);
	return GetOffset(static_cast<size_t>(x), static_cast<size_t>(y));
}

template <typename TType>
CColorBitmapT<TType>::CColorBitmapT() :
	m_lWidth{ 0 },
	m_lHeight{ 0 },
	m_bWord{ std::is_same_v<TType, std::uint16_t> },
	m_bDouble{ std::is_same_v<TType, double> },
	m_bDWord{ std::is_same_v<TType, std::uint32_t> },
	m_bFloat{ std::is_same_v<TType, float> },
	m_fMultiplier{ (std::is_same_v<TType, std::uint16_t> || std::is_same_v<TType, double> || std::is_same_v<TType, float>) ? 256.0 : (std::is_same_v<TType, std::uint32_t> ? 256.0 * 65536.0 : 1.0) }
{
	m_bTopDown = true;
}

template <typename TType>
std::unique_ptr<CMemoryBitmap> CColorBitmapT<TType>::Clone(bool bEmpty/*=false*/) const
{
	auto pResult = std::make_unique<CColorBitmapT<TType>>();
	if (!bEmpty)
	{
		pResult->m_lHeight = m_lHeight;
		pResult->m_lWidth = m_lWidth;
		pResult->m_Red.m_vPixels = m_Red.m_vPixels;
		pResult->m_Green.m_vPixels = m_Green.m_vPixels;
		pResult->m_Blue.m_vPixels = m_Blue.m_vPixels;
	}
	pResult->m_bWord = m_bWord;
	pResult->m_bDouble = m_bDouble;
	pResult->m_bDWord = m_bDWord;
	pResult->m_bFloat = m_bFloat;

	pResult->CopyFrom(*this);

	return pResult;
}

template <typename TType>
bool CColorBitmapT<TType>::Init(int lWidth, int lHeight)
{
	m_lWidth = lWidth;
	m_lHeight = lHeight;

	const bool bResult = m_Red.Init(lWidth, lHeight) && m_Green.Init(lWidth, lHeight) && m_Blue.Init(lWidth, lHeight);
	return bResult;
}

template <typename TType>
void CColorBitmapT<TType>::SetValue(size_t i, size_t j, double fRed, double fGreen, double fBlue)
{
	CheckXY(i, j);

	const size_t lOffset = GetOffset(i, j);

	m_Red.m_vPixels[lOffset] = fRed;
	m_Green.m_vPixels[lOffset] = fGreen;
	m_Blue.m_vPixels[lOffset] = fBlue;
}

template <typename TType>
void CColorBitmapT<TType>::GetValue(size_t i, size_t j, double& fRed, double& fGreen, double& fBlue) const
{
	CheckXY(i, j);

	const size_t lOffset = GetOffset(i, j);

	fRed = m_Red.m_vPixels[lOffset];
	fGreen = m_Green.m_vPixels[lOffset];
	fBlue = m_Blue.m_vPixels[lOffset];
}

template <typename TType>
void CColorBitmapT<TType>::SetPixel(size_t i, size_t j, double fRed, double fGreen, double fBlue)
{
	CheckXY(i, j);

	const size_t lOffset = GetOffset(i, j);
	m_Red.m_vPixels[lOffset] = fRed * m_fMultiplier;
	m_Green.m_vPixels[lOffset] = fGreen * m_fMultiplier;
	m_Blue.m_vPixels[lOffset] = fBlue * m_fMultiplier;
}

template <typename TType>
void CColorBitmapT<TType>::SetPixel(size_t i, size_t j, double fGray)
{

	CheckXY(i, j);		// Throw if not

	const size_t lOffset = GetOffset(i, j);
	const double value = fGray * m_fMultiplier;
	m_Red.m_vPixels[lOffset] = value;
	m_Green.m_vPixels[lOffset] = value;
	m_Blue.m_vPixels[lOffset] = value;
}

template <typename TType>
void CColorBitmapT<TType>::GetPixel(size_t i, size_t j, double& fRed, double& fGreen, double& fBlue)
{
	fRed = fGreen = fBlue = 0.0;

	CheckXY(i, j);

	const size_t lOffset = GetOffset(i, j);

	fRed = m_Red.m_vPixels[lOffset] / m_fMultiplier;
	fGreen = m_Green.m_vPixels[lOffset] / m_fMultiplier;
	fBlue = m_Blue.m_vPixels[lOffset] / m_fMultiplier;
}

template <typename TType>
void CColorBitmapT<TType>::GetPixel(size_t i, size_t j, double& fGray)
{
	double fRed, fGreen, fBlue;
	fGray = 0.0;

	GetPixel(i, j, fRed, fGreen, fBlue);
	double H, S, L;

	ToHSL(fRed, fGreen, fBlue, H, S, L);
	fGray = L * 255.0;
}

template <typename TType>
bool CColorBitmapT<TType>::GetScanLine(size_t j, void* pScanLine)
{
	bool bResult = false;

	if (j < m_lHeight)
	{
		auto* const pTempScan = static_cast<std::uint8_t*>(pScanLine);
		const size_t w = static_cast<size_t>(m_lWidth);
		const size_t index = static_cast<size_t>(j) * w;

		memcpy(pTempScan, &(m_Red.m_vPixels[index]), sizeof(TType) * w);
		memcpy(pTempScan + sizeof(TType) * w, &(m_Green.m_vPixels[index]), sizeof(TType) * w);
		memcpy(pTempScan + sizeof(TType) * 2 * w, &(m_Blue.m_vPixels[index]), sizeof(TType) * w);
		bResult = true;
	}

	return bResult;
}

template <typename TType>
bool CColorBitmapT<TType>::SetScanLine(size_t j, void* pScanLine)
{
	bool bResult = false;

	if (j < m_lHeight)
	{
		const auto* const pTempScan = static_cast<const std::uint8_t*>(pScanLine);
		const size_t w = static_cast<size_t>(m_lWidth);
		const size_t index = static_cast<size_t>(j) * w;

		memcpy(&(m_Red.m_vPixels[index]), pTempScan, sizeof(TType) * w);
		memcpy(&(m_Green.m_vPixels[index]), pTempScan + sizeof(TType) * w, sizeof(TType) * w);
		memcpy(&(m_Blue.m_vPixels[index]), pTempScan + sizeof(TType) * 2 * w, sizeof(TType) * w);
		bResult = true;
	}

	return bResult;
}

template <typename TType>
void CColorBitmapT<TType>::Clear()
{
	m_lHeight = 0;
	m_lWidth = 0;
	m_Red.m_vPixels.clear();
	m_Green.m_vPixels.clear();
	m_Blue.m_vPixels.clear();
}

template <typename TType>
std::shared_ptr<CMultiBitmap> CColorBitmapT<TType>::CreateEmptyMultiBitmap() const
{
	std::shared_ptr<CColorMultiBitmapT<TType>> pResult = std::make_shared<CColorMultiBitmapT<TType>>();
	pResult->SetBitmapModel(this);
	return pResult;
}

template <typename TType>
std::shared_ptr<CMedianFilterEngine> CColorBitmapT<TType>::GetMedianFilterEngine() const
{
	std::shared_ptr<CColorMedianFilterEngineT<TType>> pFilter = std::make_shared<CColorMedianFilterEngineT<TType>>();
	pFilter->SetInputBitmap(this);
	return pFilter;
}

template <typename TType>
void CColorBitmapT<TType>::RemoveHotPixels(ProgressBase* pProgress/*=nullptr*/)
{
	m_Red.RemoveHotPixels(pProgress);
	m_Green.RemoveHotPixels(pProgress);
	m_Blue.RemoveHotPixels(pProgress);
}

template <typename TType>
void CColorBitmapT<TType>::GetCharacteristics(CBitmapCharacteristics& bc)
{
	bc.m_bFloat = m_bFloat;
	bc.m_dwHeight = m_lHeight;
	bc.m_dwWidth = m_lWidth;
	bc.m_lNrChannels = 3;
	bc.m_lBitsPerPixel = BitPerSample();
}

template <typename TType>
void CColorBitmapT<TType>::InitIterator(void*& pRed, void*& pGreen, void*& pBlue, size_t& elementSize, const size_t x, const size_t y)
{
	const size_t index = GetOffset(x, y);
	pRed = static_cast<void*>(this->m_Red.m_vPixels.data() + index);
	pGreen = static_cast<void*>(this->m_Green.m_vPixels.data() + index);
	pBlue = static_cast<void*>(this->m_Blue.m_vPixels.data() + index);
	elementSize = sizeof(TType);
}
template <typename TType>
void CColorBitmapT<TType>::InitIterator(const void*& pRed, const void*& pGreen, const void*& pBlue, size_t& elementSize, const size_t x, const size_t y) const
{
	const size_t index = GetOffset(x, y);
	pRed = static_cast<const void*>(this->m_Red.m_vPixels.data() + index);
	pGreen = static_cast<const void*>(this->m_Green.m_vPixels.data() + index);
	pBlue = static_cast<const void*>(this->m_Blue.m_vPixels.data() + index);
	elementSize = sizeof(TType);
}

template <typename TType>
std::tuple<double, double, double> CColorBitmapT<TType>::ConvertValue3(const void* pRed, const void* pGreen, const void* pBlue) const
{
	const double r = static_cast<double>(*static_cast<const TType*>(pRed)) / this->m_fMultiplier;
	const double g = static_cast<double>(*static_cast<const TType*>(pGreen)) / this->m_fMultiplier;
	const double b = static_cast<double>(*static_cast<const TType*>(pBlue)) / this->m_fMultiplier;
	return { r, g, b };
}

template <typename TType>
double CColorBitmapT<TType>::ConvertValue1(const void* pRed, const void* pGreen, const void* pBlue) const
{
	const double r = static_cast<double>(*static_cast<const TType*>(pRed)) / this->m_fMultiplier;
	const double g = static_cast<double>(*static_cast<const TType*>(pGreen)) / this->m_fMultiplier;
	const double b = static_cast<double>(*static_cast<const TType*>(pBlue)) / this->m_fMultiplier;
	double H, S, L;
	ToHSL(r, g, b, H, S, L);
	return L * 255.0;
}

template <typename TType>
void CColorBitmapT<TType>::ReceiveValue(void* pRed, void* pGreen, void* pBlue, const double red, const double green, const double blue) const
{
	*static_cast<TType*>(pRed) = static_cast<TType>(red * this->m_fMultiplier);
	*static_cast<TType*>(pGreen) = static_cast<TType>(green * this->m_fMultiplier);
	*static_cast<TType*>(pBlue) = static_cast<TType>(blue * this->m_fMultiplier);
}

template <typename TType>
void CColorBitmapT<TType>::ReceiveValue(void* pRed, void* pGreen, void* pBlue, const double gray) const
{
	const TType value = static_cast<TType>(gray * this->m_fMultiplier);
	*static_cast<TType*>(pRed) = value;
	*static_cast<TType*>(pGreen) = value;
	*static_cast<TType*>(pBlue) = value;
}

// Define these here so that we can simple headers. We only run with these types so this isn't too bad.
template class CColorBitmapT<std::uint8_t>;
template class CColorBitmapT<std::uint16_t>;
template class CColorBitmapT<std::uint32_t>;
template class CColorBitmapT<float>;
template class CColorBitmapT<double>;
template class CColorBitmapT<unsigned short>;