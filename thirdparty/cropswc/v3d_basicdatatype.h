//
// Created by 黄磊 on 27.12.21.
//

#ifndef UNTITLED_V3D_BASICDATATYPE_H
#define UNTITLED_V3D_BASICDATATYPE_H
typedef unsigned short       v3d_uint16;
typedef unsigned char        v3d_uint8;
typedef unsigned int         v3d_uint32;
typedef unsigned long long   v3d_uint64;
typedef          char        v3d_sint8;
typedef          short       v3d_sint16;
typedef          int         v3d_sint32;
typedef          long long   v3d_sint64;
typedef          float       v3d_float32;
typedef          double      v3d_float64;

typedef void* v3dhandle;

//2010-05-19: by Hanchuan Peng. add the MSVC specific version # (vc 2008 has a _MSC_VER=1500) and win64 macro.
//Note that _WIN32 seems always defined for any windows application.
//For more info see page for example: http://msdn.microsoft.com/en-us/library/b0084kay%28VS.80%29.aspx

#if defined(_MSC_VER) && (_WIN64)
//#if defined(_MSC_VER) && defined(_WIN64) //correct?

#define V3DLONG long long

#else

#define V3DLONG long

#endif

#if defined (_MSC_VER)

#define strcasecmp strcmpi

#endif

enum TimePackType {TIME_PACK_NONE,TIME_PACK_Z,TIME_PACK_C};

#endif //UNTITLED_V3D_BASICDATATYPE_H
