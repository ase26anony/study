/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_user(void);
extern int hidden_visibility_func(int x) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_seed = 0;
static volatile int volatile_result = 0;

/* ========== Pattern 1: Direct built-in usage with attributes ========== */

/* Function with explicit hidden visibility attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
attributed_builtin_user(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function marked as used to ensure emission */
static int __attribute__((used, noinline, noclone))
test_builtin_arithmetic(int seed) {
    volatile int acc = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 5; i++) {
        /* Use math built-ins */
        int val = seed + i * 100;
        acc += __builtin_abs(val);
        
        /* Use sqrt built-in with float conversion */
        float fval = (float)val;
        acc += (int)__builtin_sqrtf(fval);
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return acc;
}

/* ========== Pattern 2: Bit manipulation built-ins ========== */

static int __attribute__((noinline))
test_builtin_bitops(unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Use various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed | 1);
    
    /* Use built-in with side effects */
    unsigned int rotated = __builtin_rotateleft32(seed, 3);
    result += __builtin_popcount(rotated);
    
    /* Store to volatile to ensure computation */
    volatile_result = result;
    
    return (int)result;
}

/* ========== Pattern 3: Overflow checking built-ins ========== */

static int __attribute__((noinline))
test_builtin_overflow(int a, int b) {
    volatile int overflow_detected = 0;
    int result;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        overflow_detected = 1;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &result)) {
        overflow_detected |= 2;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &result)) {
        overflow_detected |= 4;
    }
    
    /* Use the results to prevent dead code elimination */
    asm volatile("" : "+r" (result) : : "memory");
    
    return overflow_detected;
}

/* ========== Pattern 4: External linkage simulation ========== */

/* This function is declared extern earlier and defined here */
int external_builtin_user(void) {
    volatile int x = volatile_seed;
    
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use built-in with assembly barrier */
    int count = __builtin_popcount(x);
    asm volatile("" : : "r" (count) : "memory");
    
    return count;
}

/* Hidden visibility function definition */
int __attribute__((visibility("hidden"), nothrow))
hidden_visibility_func(int x) {
    /* Mix of built-ins and complex expressions */
    int result = __builtin_abs(x);
    
    /* Use __builtin_expect in loop */
    for (int i = 0; i < 3; i++) {
        if (__builtin_expect((x & (1 << i)) != 0, 0)) {
            result += __builtin_clz(x);
        }
    }
    
    /* Prevent tail call optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* ========== Pattern 5: Built-in in inline assembly context ========== */

static int __attribute__((noinline))
test_builtin_with_asm(int seed) {
    volatile int a = seed;
    volatile int b = seed * 2;
    int result;
    
    /* Use built-in between assembly barriers */
    asm volatile("" : : : "memory");
    
    if (__builtin_add_overflow(a, b, &result)) {
        result = __builtin_abs(a);
    }
    
    asm volatile("" : : "r" (result) : "memory");
    
    return result;
}

/* ========== Pattern 6: Built-in as function pointer argument ========== */

typedef int (*builtin_func_t)(int);

static int apply_builtin(builtin_func_t func, int x) {
    return func(x);
}

static int builtin_wrapper(int x) {
    /* This wrapper ensures the built-in is processed as a function */
    return __builtin_abs(x) + __builtin_popcount(x);
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    /* Initialize volatile seed from runtime data */
    volatile_seed = argc + (int)time(NULL);
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(volatile_seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)volatile_seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(volatile_seed, 1000);
    
    /* Test 4: External linkage functions */
    checksum += external_builtin_user();
    
    /* Test 5: Hidden visibility function */
    checksum += hidden_visibility_func(volatile_seed);
    
    /* Test 6: Function with attributes calling built-ins */
    checksum += attributed_builtin_user(volatile_seed);
    
    /* Test 7: Built-in with assembly barriers */
    checksum += test_builtin_with_asm(volatile_seed);
    
    /* Test 8: Built-in through function pointer */
    checksum += apply_builtin(builtin_wrapper, volatile_seed);
    
    /* Additional complex expression with multiple built-ins */
    volatile int complex_result = 0;
    for (volatile int i = 0; i < 3; i++) {
        int val = volatile_seed + i;
        complex_result += __builtin_abs(val);
        complex_result += __builtin_popcount(val);
        
        /* Use __builtin_expect to influence control flow */
        if (__builtin_expect((val % 2) == 0, 1)) {
            complex_result += __builtin_clz(val | 1);
        }
    }
    checksum += complex_result;
    
    /* Use __builtin_unreachable for impossible condition */
    if (checksum < 0) {
        __builtin_unreachable();
    }
    
    /* Final output to prevent entire program elimination */
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
