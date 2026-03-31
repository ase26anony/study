#ifndef COMMON_H
#define COMMON_H

// Extern TLS variable - declared here, defined in helper1.c
extern __thread int extern_tls;

// Weak TLS variable - weak linkage
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable - tentative definition
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);

// Visibility attribute test
#ifdef __GNUC__
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#else
#define HIDDEN_VIS
#endif

// DLL import/export simulation for Windows
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT
#define DLL_EXPORT
#endif

// DLL imported TLS variable simulation
extern DLL_IMPORT __thread int dll_tls_var;

#endif
