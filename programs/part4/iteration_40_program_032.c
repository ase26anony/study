/* test_builtin_hooks.c - Comprehensive built-in function test */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern int external_with_unreachable(int x) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_accumulator = 0;
static volatile int volatile_seed = 0;

/* Helper function with noinline to prevent optimization */
int __attribute__((noinline, noclone)) 
use_builtin_result(int x) {
    return x * 2 + 1;
}

/* Function with multiple attributes that may interact with the hook */
static int __attribute__((visibility("hidden"), nothrow, used))
attributed_builtin_user(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_abs(x) + 1;
    }
    return __builtin_abs(x);
}

/* Function using arithmetic built-ins */
int __attribute__((noinline, noclone))
test_builtin_arithmetic(int seed) {
    volatile int local_acc = 0;
    
    /* Loop with volatile counter to prevent optimization */
    volatile int i;
    for (i = 0; i < 5; i++) {
        /* Use different math built-ins */
        int val = seed + i * 10;
        int abs_val = __builtin_abs(val);
        
        /* Use sqrtf built-in with float conversion */
        float fval = (float)val;
        float sqrt_val = __builtin_sqrtf(fval);
        
        /* Combine results with optimization barrier */
        asm volatile("" : : "r"(sqrt_val) : "memory");
        local_acc += abs_val + (int)sqrt_val;
    }
    
    /* Use builtin with side effects */
    int result = __builtin_ffs(local_acc);  /* Find first set bit */
    volatile_accumulator += result;
    return result;
}

/* Function using bit operation built-ins */
int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int acc = 0;
    
    /* Use various bit manipulation built-ins */
    acc += __builtin_popcount(seed);
    acc += __builtin_clz(seed | 1);  /* Ensure non-zero */
    acc += __builtin_ctz(seed | 1);  /* Ensure non-zero */
    acc += __builtin_parity(seed);
    
    /* Rotate operations */
    unsigned int rotated = __builtin_rotateleft32(seed, 5);
    acc += __builtin_popcount(rotated);
    
    /* Store in volatile to prevent elimination */
    volatile_accumulator += acc;
    
    /* Use builtin with memory barrier */
    asm volatile("" : : "r"(acc) : "memory");
    return (int)acc;
}

/* Function using overflow checking built-ins */
int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result, overflow;
    
    /* Test addition overflow */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) {
        result = __builtin_abs(a);
    }
    
    /* Test multiplication overflow */
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) {
        result = __builtin_abs(b);
    }
    
    /* Test subtraction overflow */
    overflow = __builtin_sub_overflow(a, b, &result);
    volatile_accumulator += overflow;
    
    /* Use builtin for bounds checking */
    if (__builtin_expect(result < -1000 || result > 1000, 0)) {
        result = __builtin_clamp(result, -1000, 1000);
    }
    
    return result;
}

/* Function with external linkage simulation */
int external_builtin_user(int x) {
    /* Use unreachable built-in under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use prefetch built-in */
    __builtin_prefetch(&volatile_accumulator, 0, 3);
    
    /* Use synchronization built-in */
    int old = __atomic_fetch_add(&volatile_accumulator, x, __ATOMIC_SEQ_CST);
    
    return __builtin_abs(old);
}

/* Another external-like function */
int __attribute__((visibility("hidden")))
external_with_unreachable(int x) {
    /* Use expect with pointer built-in */
    int *ptr = &volatile_accumulator;
    if (__builtin_expect(ptr != NULL, 1)) {
        /* Use object size built-in */
        size_t sz = __builtin_object_size(ptr, 0);
        asm volatile("" : : "r"(sz) : "memory");
    }
    
    /* Conditional unreachable */
    if (x == 0xDEADBEEF) {
        __builtin_unreachable();
    }
    
    return __builtin_ffs(x);
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Initialize volatile seed from runtime sources */
    volatile_seed = argc;
    volatile_seed ^= (int)time(NULL);
    volatile_seed ^= (int)clock();
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(volatile_seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)volatile_seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(volatile_seed, volatile_seed / 2 + 1);
    
    /* Test 4: Attributed function with built-ins */
    checksum += attributed_builtin_user(volatile_seed);
    
    /* Test 5: External linkage simulation */
    checksum += external_builtin_user(volatile_seed);
    checksum += external_with_unreachable(volatile_seed);
    
    /* Use builtin for final result calculation */
    checksum = __builtin_bswap32(__builtin_abs(checksum));
    
    /* Optimization barrier for final result */
    asm volatile("" : : "r"(checksum) : "memory");
    
    /* Print result to ensure all code is live */
    printf("Built-in test checksum: %d (volatile accum: %d)\n", 
           checksum, volatile_accumulator);
    
    return checksum & 0xFF;
}

/* Additional static function with built-in declaration */
static void __attribute__((constructor))
init_builtin_test(void) {
    /* Declare and use built-in in constructor */
    extern int __builtin_ctzll(unsigned long long);
    volatile_seed += __builtin_ctzll(0x123456789ABCDEF0ULL);
}

/* Force emission of unused but attributed function */
static int __attribute__((used, visibility("hidden")))
unused_but_present(int x) {
    return __builtin_parity(x) + __builtin_clrsb(x);
}
