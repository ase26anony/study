#ifndef TLS_DEFS_H
#define TLS_DEFS_H

// Visibility attributes
#define HIDDEN __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))

// Force usage
#define USED __attribute__((used))

// Weak linkage
#define WEAK __attribute__((weak))

// DLL import simulation (works on non-Windows too with GCC)
#define DLLIMPORT __attribute__((dllimport))
#define DLLEXPORT __attribute__((dllexport))

// Prevent optimization
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&var))

#endif
