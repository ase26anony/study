/* test_target.c - Main source file with various patterns to trigger dump directory allocation */

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC push_options
#pragma GCC optimize("O3")

/* Pattern 2: Function with optimization attribute */
__attribute__((optimize("O2"), noinline, cold))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 3: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init(void) {
    /* Empty constructor - just to engage driver */
}

/* Pattern 4: Force coverage instrumentation */
#ifdef __COVERAGE__
__attribute__((noinline))
static void coverage_helper(void) {
    volatile int x = 0;
    (void)x;
}
#endif

/* Pattern 5: Benign error condition wrapped in #if 0 */
#if 0
/* This would cause a syntax error if compiled */
int syntax error here;
#endif

/* Pattern 6: Warning trigger that can be suppressed */
__attribute__((unused))
static int unused_variable = 42;

/* Pattern 7: Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Compile-time assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
#ifdef __COVERAGE__
    coverage_helper();
#endif
    
    return result == 120 ? 0 : 1;
}

/* Pattern 8: Additional function with different attributes */
__attribute__((hot, always_inline))
inline int double_value(int x) {
    return x * 2;
}

/* End of main source file */
