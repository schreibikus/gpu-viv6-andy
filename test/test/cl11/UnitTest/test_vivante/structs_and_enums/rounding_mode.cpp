/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/



#include "rounding_mode.h"

#if !(defined(_WIN32) && defined(_MSC_VER))
RoundingMode set_round( RoundingMode r, Type outType )
{
    static const int flt_rounds[ kRoundingModeCount ] = { FE_TONEAREST, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
    static const int int_rounds[ kRoundingModeCount ] = { FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
    const int *p = int_rounds;
    if( outType == kfloat || outType == kdouble )
        p = flt_rounds;
    int oldRound = fegetround();
    fesetround( p[r] );

    switch( oldRound )
    {
        case FE_TONEAREST:
            return kRoundToNearestEven;
        case FE_UPWARD:
            return kRoundUp;
        case FE_DOWNWARD:
            return kRoundDown;
        case FE_TOWARDZERO:
            return kRoundTowardZero;
        default:
            abort();    // ??!
    }
    return kDefaultRoundingMode;    //never happens
}

RoundingMode get_round( void )
{
    int oldRound = fegetround();

    switch( oldRound )
    {
        case FE_TONEAREST:
            return kRoundToNearestEven;
        case FE_UPWARD:
            return kRoundUp;
        case FE_DOWNWARD:
            return kRoundDown;
        case FE_TOWARDZERO:
            return kRoundTowardZero;
    }

    return kDefaultRoundingMode;
}

#else
RoundingMode set_round( RoundingMode r, Type outType )
{
    static const int flt_rounds[ kRoundingModeCount ] = { _RC_NEAR, _RC_NEAR, _RC_UP, _RC_DOWN, _RC_CHOP };
    static const int int_rounds[ kRoundingModeCount ] = { _RC_CHOP, _RC_NEAR, _RC_UP, _RC_DOWN, _RC_CHOP };
    const int *p = ( outType == kfloat || outType == kdouble )? flt_rounds : int_rounds;
    unsigned int oldRound;

    int err = _controlfp_s(&oldRound, 0, 0); //get rounding mode into oldRound
    if (err) {
        //vlog_error("\t\tERROR: -- cannot get rounding mode in %s:%d\n", __FILE__, __LINE__);
        return kDefaultRoundingMode;    //what else never happens
    }

    oldRound &= _MCW_RC;

    RoundingMode old =
        (oldRound == _RC_NEAR)? kRoundToNearestEven :
        (oldRound == _RC_UP)?   kRoundUp :
        (oldRound == _RC_DOWN)? kRoundDown :
        (oldRound == _RC_CHOP)? kRoundTowardZero:
        kDefaultRoundingMode;

    _controlfp_s(&oldRound, p[r], _MCW_RC); //setting new rounding mode
    return old;    //returning old rounding mode
}

RoundingMode get_round( void )
{
    unsigned int oldRound;

    int err = _controlfp_s(&oldRound, 0, 0); //get rounding mode into oldRound
    oldRound &= _MCW_RC;
    return
        (oldRound == _RC_NEAR)? kRoundToNearestEven :
        (oldRound == _RC_UP)?   kRoundUp :
        (oldRound == _RC_DOWN)? kRoundDown :
        (oldRound == _RC_CHOP)? kRoundTowardZero:
        kDefaultRoundingMode;
}

#endif

//
// FlushToZero() sets the host processor into ftz mode.  It is intended to have a remote effect on the behavior of the code in
// basic_test_conversions.c. Some host processors may not support this mode, which case you'll need to do some clamping in
// software by testing against FLT_MIN or DBL_MIN in that file.
//
// Note: IEEE-754 says conversions are basic operations.  As such they do *NOT* have the behavior in section 7.5.3 of
// the OpenCL spec. They *ALWAYS* flush to zero for subnormal inputs or outputs when FTZ mode is on like other basic
// operators do (e.g. add, subtract, multiply, divide, etc.)
//
// Configuring hardware to FTZ mode varies by platform.
// CAUTION: Some C implementations may also fail to behave properly in this mode.
//
//  On PowerPC, it is done by setting the FPSCR into non-IEEE mode.
//  On Intel, you can do this by turning on the FZ and DAZ bits in the MXCSR -- provided that SSE/SSE2
//          is used for floating point computation! If your OS uses x87, you'll need to figure out how
//          to turn that off for the conversions code in basic_test_conversions.c so that they flush to
//          zero properly.  Otherwise, you'll need to add appropriate software clamping to basic_test_conversions.c
//          in which case, these function are at liberty to do nothing.
//
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( __e2k__ ) || defined (_WIN32)
    #include <xmmintrin.h>
#elif defined( __PPC__ )
    #include <fpu_control.h>
#endif
void *FlushToZero( void )
{
#if defined( __APPLE__ ) || defined(__linux__) || defined (_WIN32) || defined(__QNXNTO__)
    #if defined( __i386__ ) || defined( __x86_64__ ) || defined( __e2k__ ) || defined(_MSC_VER)
        union{ int i;  void *p; }u = { _mm_getcsr() };
        _mm_setcsr( u.i | 0x8040 );
        return u.p;
    #elif defined( __arm__ ) || defined( __aarch64__ )
        // processor is already in FTZ mode -- do nothing
        return NULL;
    #elif defined( __PPC__ )
        fpu_control_t flags = 0;
        _FPU_GETCW(flags);
        flags |= _FPU_MASK_NI;
        _FPU_SETCW(flags);
        return NULL;
        #else
        #error Unknown arch
    #endif
#else
    #error  Please configure FlushToZero and UnFlushToZero to behave properly on this operating system.
#endif
}

// Undo the effects of FlushToZero above, restoring the host to default behavior, using the information passed in p.
void UnFlushToZero( void *p)
{
#if defined( __APPLE__ ) || defined(__linux__) || defined (_WIN32) || defined(__QNXNTO__)
    #if defined( __i386__ ) || defined( __x86_64__ ) || defined( __e2k__ ) || defined(_MSC_VER)
        union{ void *p; int i;  }u = { p };
        _mm_setcsr( u.i );
    #elif defined( __arm__ ) || defined( __aarch64__ )
        // processor is already in FTZ mode -- do nothing
    #elif defined( __PPC__)
        fpu_control_t flags = 0;
        _FPU_GETCW(flags);
        flags &= ~_FPU_MASK_NI;
        _FPU_SETCW(flags);
        #else
        #error Unknown arch
    #endif
#else
    #error  Please configure FlushToZero and UnFlushToZero to behave properly on this operating system.
#endif
}
