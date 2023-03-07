#pragma once
#define NOMINMAX

// Qt
#include <QCoreApplication>
#include <QString>
#include <QPoint>
#include <QElapsedTimer>

// Std
#include <vector>
#include <algorithm>
#include <mutex>
#include <deque>
#include <set>
#include <numeric>
#include <immintrin.h>
#include <limits>

using std::min;
using std::max;

// Misc global headers.
#include <omp.h>

#include "../ZCLass/zdefs.h"
#include "../ZCLass/Ztrace.h"
#include "../ZCLass/zexcept.h"


// Windows (to go!)
#include <WTypesbase.h>
#include <atlstr.h>
#include <cassert>
#include <atltypes.h>
