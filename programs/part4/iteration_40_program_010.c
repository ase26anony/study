/* test_builtin_hooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ========== ATTRIBUTE-DRIVEN DECLARATIONS ========== */

/* Function with explicit hidden visibility and other attributes */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
hidden_attributed_function(int x, int y) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > y, 0)) {
        return __builtin_abs(x - y);
    }
    return __builtin_abs(y - x);
}

/* Function marked as used to force emission */
int __attribute__((used, noinline))
force_used_function(volatile int a) {
    /* Use multiple built-ins in sequence */
    int b = __builtin_popcount(a);
    int c = __builtin_clz(a | 1);  /* Ensure non-zero */
    return b + c;
}

/* ========== EXTERNAL LINKAGE SIMULATION ========== */

/* Forward declaration with extern */
extern int external_builtin_user(int val) __attribute__((visibility("hidden")));

/* Static function calling built-ins */
static int __attribute__((noinline))
static_builtin_wrapper(volatile int seed) {
    int result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        /* Use different built-ins based on loop iteration */
        switch (i) {
            case 0:
                result += __builtin_ffs(seed + i);
                break;
            case 1:
                result += __builtin_parity(seed + i);
                break;
            case 2:
                result += __builtin_ctz((seed + i) | 1);
                break;
        }
        
        /* Optimization barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* ========== BUILT-IN TEST FUNCTIONS ========== */

/* Test arithmetic built-ins */
int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    volatile int accumulator = 0;
    
    /* Use floating-point built-in */
    float fval = (float)seed + 0.5f;
    int sqrt_val = (int)__builtin_sqrtf(fval);
    
    /* Use abs built-in with volatile to prevent constant folding */
    volatile int abs_val = __builtin_abs(seed - 100);
    
    /* Complex expression with built-in */
    accumulator = sqrt_val + abs_val + __builtin_abs(seed);
    
    /* Use built-in in conditional */
    if (__builtin_expect(accumulator > 50, 1)) {
        accumulator += __builtin_abs(seed * -1);
    }
    
    return accumulator;
}

/* Test bit operation built-ins */
int __attribute__((noinline, noclone))
test_builtin_bitops(volatile unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Chain multiple bit built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_parity(seed);
    result += __builtin_ffs(seed | 1);
    
    /* Use in loop with side effects */
    for (volatile int i = 0; i < 2; i++) {
        result += __builtin_bswap32(seed + i);
        asm volatile("" : : : "memory");
    }
    
    return (int)result;
}

/* Test overflow built-ins */
int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) {
    volatile int result = 0;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1000;
    } else {
        result += overflow;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 2000;
    } else {
        result += overflow;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 3000;
    } else {
        result += overflow;
    }
    
    return result;
}

/* Test unreachable built-in */
void __attribute__((noinline, noclone))
test_builtin_unreachable(volatile int x, volatile int *output) {
    if (x < 0) {
        *output = x * 2;
    } else if (x == 0) {
        *output = 0;
    } else if (x > 1000) {
        /* This should never happen in our test */
        __builtin_unreachable();
    } else {
        *output = x + 1;
    }
}

/* ========== EXTERNAL FUNCTION DEFINITION ========== */

/* Definition of previously extern-declared function */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int val) {
    volatile int result = 0;
    
    /* Use built-in with optimization barrier */
    result = __builtin_bswap32(val);
    asm volatile("" : : : "memory");
    
    /* Conditional with built-in */
    if (__builtin_expect(val > 100, 0)) {
        __builtin_unreachable();
    }
    
    return result;
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    volatile int checksum = 0;
    
    /* Use argc as volatile seed to prevent compile-time computation */
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)argc;
    
    /* Initialize with time for more randomness */
    srand(time(NULL));
    volatile int rand_val = rand();
    
    printf("Testing GCC built-in hooks with seed: %d\n", seed);
    
    /* 1. Test arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* 2. Test bit operation built-ins */
    checksum += test_builtin_bitops(useed);
    
    /* 3. Test overflow built-ins */
    checksum += test_builtin_overflow(seed, rand_val % 100);
    
    /* 4. Test hidden attributed function */
    checksum += hidden_attributed_function(seed, rand_val % 50);
    
    /* 5. Test static wrapper with built-ins */
    checksum += static_builtin_wrapper(seed);
    
    /* 6. Test force-used function */
    checksum += force_used_function(seed);
    
    /* 7. Test external linkage function */
    checksum += external_builtin_user(seed);
    
    /* 8. Test unreachable built-in */
    volatile int unreachable_output = 0;
    test_builtin_unreachable(seed, &unreachable_output);
    checksum += unreachable_output;
    
    /* Additional complex expression with multiple built-ins */
    volatile int complex_result = 0;
    for (volatile int i = 0; i < 3; i++) {
        complex_result += __builtin_popcount(seed + i);
        complex_result += __builtin_clz((seed + i) | 1);
        
        /* Mix with inline assembly barrier */
        asm volatile("" : : : "memory");
        
        /* Use built-in in pointer arithmetic */
        int *ptr = &complex_result;
        ptr += __builtin_ffs(seed + i) % 2;
    }
    checksum += complex_result;
    
    /* Final result depends on all computations */
    printf("Final checksum: %d\n", checksum);
    
    /* Use built-in in return value computation */
    return checksum + __builtin_abs(checksum) % 256;
}
