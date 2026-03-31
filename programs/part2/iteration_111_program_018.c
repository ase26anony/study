/* test_target.c - Main test file */
/* Pattern: Use #pragma GCC to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC target("arch=x86-64")

/* Force generation of auxiliary files via attributes */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init(void) {
    /* This ensures the driver goes through all compilation phases */
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Compile-time assertion to engage driver's error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return result == 120 ? 0 : 1;
}
