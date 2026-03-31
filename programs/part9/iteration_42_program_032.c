// Using function attributes (cleaner syntax)
void foo() __attribute__((optimize("O0")));
void foo() { /* complex code */ }

// Using compiler-specific macros for portability
#ifdef __GNUC__
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#else
#define NO_OPTIMIZE
#endif

NO_OPTIMIZE void foo() { /* complex code */ }
