//
//   CommonStd.h
//   WIN32  -> MSVC 4.07
//   File関連 -> CommonFile.h
//                          2011.03.24
//                          2008.12.09
//                          k.Takeda
#ifndef COMMON_STD_HH_3AF0ABDA_F467_454f_B9DB_A487484A060F
#define COMMON_STD_HH_3AF0ABDA_F467_454f_B9DB_A487484A060F

#define BUFFERLENGTH 256


#include <stdint.h>
//-------------------
#include <vector>
#include <utility>
#include <string>
#include <algorithm>

#include "DeleteObject.h"// STL

#ifdef  MSVC
 #define _USE_MATH_DEFINES
#endif

#include <cmath>

using namespace std;
//--------------------

#endif // COMMON_STD_HH_3AF0ABDA_F467_454f_B9DB_A487484A060F
