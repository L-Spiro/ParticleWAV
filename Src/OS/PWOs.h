/**
 * Copyright L. Spiro 2022
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Includes all OS headers.
 */

#pragma once

#if defined( _WIN32 ) || defined( _WIN64 )
#include "PWWindows.h"
#elif defined( __APPLE__ )
#include "PWApple.h"

#include <pthread.h>
#else
#endif  // #if defined( _WIN32 ) || defined( _WIN64 )

#ifndef PW_FASTCALL
#define PW_FASTCALL
#endif	// PW_FASTCALL


#if defined( _MSC_VER )
    // Microsoft Visual Studio Compiler
    #define PW_ALIGN( n ) 						__declspec( align( n ) )
#elif defined( __GNUC__ ) || defined( __clang__ )
    // GNU Compiler Collection (GCC) or Clang
    #define PW_ALIGN( n ) 						__attribute__( (aligned( n )) )
#else
    #error "Unsupported compiler"
#endif


#if defined( _MSC_VER )
	#define PW_FORCEINLINE 					__forceinline
	#define PW_PREFETCH_LINE( ADDR )			_mm_prefetch( reinterpret_cast<const char *>(ADDR), _MM_HINT_T0 );
    #define PW_LIKELY(x) (x)
    #define PW_UNLIKELY(x) (x)
#elif defined( __GNUC__ ) || defined( __clang__ )
	#define PW_FORCEINLINE 					__inline__ __attribute__( (__always_inline__) )
	#define PW_PREFETCH_LINE( ADDR )			__builtin_prefetch( reinterpret_cast<const void *>(ADDR), 1, 1 );
    #define PW_LIKELY(x)                       __builtin_expect(!!(x), 1)
    #define PW_UNLIKELY(x)                     __builtin_expect(!!(x), 0)
    #define __assume(x)
#else
	#define PW_FORCEINLINE inline
#endif

#ifdef PW_WINDOWS
inline void SetThreadHighPriority() {
    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
}
inline void SetThreadNormalPriority() {
    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );
}
#else
inline void SetThreadHighPriority() {
    sched_param spSchParms;
    spSchParms.sched_priority = ::sched_get_priority_max( SCHED_FIFO );
    ::pthread_setschedparam( ::pthread_self(), SCHED_FIFO, &spSchParms );
}
void SetThreadNormalPriority() {
    sched_param spSchParms;
    spSchParms.sched_priority = 0;  // Normal priority
    ::pthread_setschedparam( ::pthread_self(), SCHED_OTHER, &spSchParms );
}
#endif  // PW_WINDOWS

inline void SetThreadAffinity( size_t sCoreId ) {
#ifdef PW_WINDOWS
	// Set thread affinity to the specified core on Windows
	DWORD_PTR dwptrMask = DWORD_PTR( 1 ) << sCoreId;
	::SetThreadAffinityMask( ::GetCurrentThread(), dwptrMask );
#elif defined( PW_APPLE ) || defined( __linux__ )
	// Set thread affinity on Linux
	cpu_set_t cpuset;
	CPU_ZERO( &cpuset );
	CPU_SET( sCoreId, &cpuset );
	::pthread_setaffinity_np( ::pthread_self(), sizeof( cpu_set_t ), &cpuset );
#endif
}