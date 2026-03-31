/* test_builtin_hooks.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_test(int x);
extern void unreachable_test(int x) __attribute__((nothrow));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int checksum = 0;

/* Helper with memory barrier */
static void memory_barrier(void) {
    __asm__ volatile("" : : : "memory");
}

/* Function with multiple attributes that calls builtins */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
attributed_builtin_function(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        /* Use __builtin_abs with volatile store */
        volatile int result = __builtin_abs(x);
        memory_barrier();
        return result;
    }
    return 0;
}

/* Function using arithmetic builtins */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int sum = 0;
    volatile int i;
    
    for (i = 0; i < 5; i++) {
        /* Use different arithmetic builtins */
        int val = seed + i;
        sum += __builtin_abs(val);
        
        /* Use __builtin_sqrtf with float conversion */
        float fval = (float)val;
        if (fval >= 0) {
            sum += (int)__builtin_sqrtf(fval);
        }
        
        /* Prevent loop unrolling */
        memory_barrier();
    }
    
    checksum += sum;
    return sum;
}

/* Function using bit operation builtins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* Use various bit operation builtins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed | 1);
    
    /* Use __builtin_bswap32 for endian conversion */
    result += __builtin_bswap32(seed) & 0xFF;
    
    volatile unsigned int stored = result;
    checksum += stored;
    return (int)result;
}

/* Function using overflow checking builtins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        checksum += 1000;
    } else {
        checksum += result;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &result)) {
        checksum += 2000;
    } else {
        checksum += result;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &result)) {
        checksum += 3000;
    }
    
    return result;
}

/* Function with external linkage that uses builtins */
int __attribute__((visibility("default"), noinline))
external_builtin_test(int x) {
    /* Use __builtin_assume_aligned */
    int* ptr = &x;
    ptr = (int*)__builtin_assume_aligned(ptr, sizeof(int));
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return x * 2;
    } else {
        /* Use __builtin_prefetch */
        __builtin_prefetch(ptr, 0, 3);
        return __builtin_abs(x) + 1;
    }
}

/* Function using __builtin_unreachable */
static void __attribute__((noinline, noclone))
unreachable_test(int x) {
    switch (x) {
        case 0:
            checksum += 10;
            break;
        case 1:
            checksum += 20;
            break;
        case 2:
            checksum += 30;
            break;
        default:
            /* Mark default case as unreachable for certain inputs */
            if (x < 0 || x > 100) {
                __builtin_unreachable();
            }
            checksum += 40;
    }
}

/* Function using builtins with complex expressions */
static int __attribute__((noinline, noclone))
test_complex_builtins(int seed) {
    int result = 0;
    
    /* Chain builtins together */
    result = __builtin_abs(__builtin_abs(seed) - 50);
    
    /* Use __builtin_choose_expr */
    result = __builtin_choose_expr(
        seed > 0,
        __builtin_clz(seed),
        __builtin_ctz(-seed)
    );
    
    /* Use __builtin_types_compatible_p */
    if (__builtin_types_compatible_p(typeof(seed), int)) {
        result += 100;
    }
    
    volatile int stored = result;
    checksum += stored;
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize volatile seed from runtime sources */
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)time(NULL);
    
    if (argc > 1) {
        seed = atoi(argv[1]);
        useed = (unsigned int)seed;
    }
    
    printf("Starting builtin tests with seed=%d, useed=%u\n", seed, useed);
    
    /* Test 1: Arithmetic builtins */
    int r1 = test_builtin_arithmetic(seed);
    printf("Arithmetic test result: %d\n", r1);
    
    /* Test 2: Bit operation builtins */
    int r2 = test_builtin_bitops(useed);
    printf("Bitops test result: %d\n", r2);
    
    /* Test 3: Overflow builtins */
    int r3 = test_builtin_overflow(seed, 1000);
    printf("Overflow test result: %d\n", r3);
    
    /* Test 4: Attributed function with builtins */
    int r4 = attributed_builtin_function(seed);
    printf("Attributed function result: %d\n", r4);
    
    /* Test 5: External linkage function */
    int r5 = external_builtin_test(seed);
    printf("External function result: %d\n", r5);
    
    /* Test 6: Unreachable builtin */
    unreachable_test(seed % 5);
    
    /* Test 7: Complex builtin expressions */
    int r7 = test_complex_builtins(seed);
    printf("Complex builtins result: %d\n", r7);
    
    /* Additional builtin usage in main */
    
    /* Use __builtin_expect for branch prediction */
    if (__builtin_expect(checksum > 0, 1)) {
        printf("Checksum is positive: %d\n", checksum);
    }
    
    /* Use __builtin_trap for extreme cases */
    if (seed == 0xDEADBEEF) {
        __builtin_trap();
    }
    
    /* Use __builtin_return_address */
    void* return_addr = __builtin_return_address(0);
    printf("Main return address: %p\n", return_addr);
    
    /* Final result based on all tests */
    int final_result = r1 + r2 + r3 + r4 + r5 + r7 + checksum;
    
    /* Use __builtin_parity */
    if (__builtin_parity(final_result)) {
        printf("Final result has odd parity\n");
    } else {
        printf("Final result has even parity\n");
    }
    
    printf("Total checksum: %d\n", checksum);
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
