#pragma once
#ifndef _IMPL_DEFINES_H_
#define _IMPL_DEFINES_H_ 1

// Export definitions
#ifdef _PSTL_DLL
#define _EXP_IMPL __declspec(dllexport)
#else
#define _EXP_IMPL __declspec(dllimport)
#endif

#endif