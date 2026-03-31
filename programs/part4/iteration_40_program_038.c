/* test_targhooks.c - Comprehensive built-in function usage to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void unreachable_helper(int x) __attribute__((nothrow));

/* Volatile variables to prevent optimization */
static volatile int global_volatile_counter = 0;
static volatile int optimization_barrier = 0;

/* Function with explicit hidden visibility and used attribute */
static int hidden_used_function(int x) 
    __attribute__((visibility("hidden"), used, nothrow, noinline, noclone));

/* Function with artificial-like properties */
static void artificial_like_function(void) 
    __attribute__((constructor, noinline));

/* Test 1: Arithmetic built-ins with volatile context */
static int test_builtin_arithmetic(volatile int seed) {
    volatile int result = 0;
    volatile float fseed = (float)seed;
    
    for (volatile int i = 0; i < 5; i++) {
        /* Use various arithmetic built-ins */
        int abs_val = __builtin_abs(seed + i - 2);
        float sqrt_val = __builtin_sqrtf(fseed + (float)i);
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r" (abs_val) : : "memory");
        
        result += abs_val + (int)sqrt_val;
        
        /* Use expect built-in */
        if (__builtin_expect((seed & 1), 0)) {
            result += __builtin_ffs(seed) * 2;
        }
    }
    
    return result;
}

/* Test 2: Bit operation built-ins */
static int test_builtin_bitops(unsigned int pattern) {
    volatile unsigned int v = pattern;
    int total = 0;
    
    /* Multiple bit built-in calls */
    total += __builtin_popcount(v);
    total += __builtin_clz(v | 1);  /* Ensure non-zero */
    total += __builtin_ctz(v | 1);
    total += __builtin_parity(v);
    
    /* Use in conditional context */
    if (__builtin_constant_p(pattern)) {
        total += 100;
    }
    
    /* Store to volatile to prevent elimination */
    global_volatile_counter = total;
    
    return total;
}

/* Test 3: Overflow checking built-ins */
static int test_builtin_overflow(int a, int b) {
    volatile int res;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &res)) {
        res = __builtin_abs(a);
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &res)) {
        res = __builtin_clz((unsigned int)__builtin_abs(a));
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &res)) {
        res = __builtin_popcount((unsigned int)b);
    }
    
    /* Use asm barrier */
    asm volatile("" : : "r" (res) : "memory");
    
    return res;
}

/* Test 4: Hidden visibility function with built-ins */
static int hidden_used_function(int x) {
    /* This function should be marked with hidden visibility */
    volatile int y = x;
    
    /* Use expect built-in with artificial context */
    if (__builtin_expect(y > 100, 0)) {
        return __builtin_abs(y) + __builtin_clz((unsigned int)y);
    }
    
    /* Use unreachable in certain paths */
    if (y < 0) {
        __builtin_unreachable();
    }
    
    return __builtin_popcount((unsigned int)y) + 
           __builtin_parity((unsigned int)y);
}

/* Test 5: External linkage simulation */
int external_builtin_user(int x) {
    /* External declaration, defined here */
    volatile int r = x;
    
    /* Use multiple built-ins */
    r += __builtin_bswap16((short)r);
    r += __builtin_bswap32(r);
    
    /* Use in loop with volatile */
    for (volatile int i = 0; i < 3; i++) {
        r += __builtin_ffs(r + i);
    }
    
    /* Call unreachable helper */
    if (r > 1000) {
        unreachable_helper(r);
    }
    
    return r;
}

void unreachable_helper(int x) {
    /* Function with nothrow attribute */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use sync built-in */
    __sync_fetch_and_add(&global_volatile_counter, 1);
}

/* Test 6: Constructor function with built-ins */
static void artificial_like_function(void) {
    /* This runs before main, testing constructor context */
    volatile int init_val = 42;
    
    /* Use built-ins in constructor */
    int bits = __builtin_popcount(init_val);
    int leading = __builtin_clz(init_val);
    
    /* Store to global volatile */
    optimization_barrier = bits + leading;
    
    /* Use prefetch built-in */
    __builtin_prefetch(&global_volatile_counter, 0, 3);
}

/* Test 7: Complex expression with built-ins */
static int complex_builtin_expr(volatile int base) {
    int a = base;
    int b = base * 2;
    int c = base / 3;
    
    /* Complex expression mixing multiple built-ins */
    int result = __builtin_abs(a) + 
                 (__builtin_add_overflow(b, c, &a) ? 
                  __builtin_clz((unsigned int)b) : 
                  __builtin_popcount((unsigned int)c));
    
    /* Use in switch with built-in constant */
    switch (__builtin_constant_p(base) ? 1 : 2) {
        case 1:
            result += __builtin_ffs(base);
            break;
        case 2:
            result += __builtin_parity((unsigned int)base);
            break;
        default:
            __builtin_unreachable();
    }
    
    return result;
}

/* Main function tying everything together */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    volatile int seed = argc;
    seed += time(NULL) % 100;
    
    /* Run constructor-like function */
    artificial_like_function();
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operations */
    checksum += test_builtin_bitops((unsigned int)seed * 0x9e3779b9);
    
    /* Test 3: Overflow checking */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Hidden visibility function */
    checksum += hidden_used_function(seed);
    
    /* Test 5: External linkage */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Complex expressions */
    checksum += complex_builtin_expr(seed);
    
    /* Additional direct built-in usage in main */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(checksum);
    direct_result += __builtin_popcount((unsigned int)checksum);
    
    /* Use expect to influence branching */
    if (__builtin_expect(direct_result > 1000, 0)) {
        direct_result += __builtin_clz((unsigned int)direct_result);
    }
    
    /* Final result with optimization barrier */
    asm volatile("" : : "r" (direct_result) : "memory");
    
    /* Return checksum to prevent dead code elimination */
    return (checksum + direct_result) & 0xFF;
}
