/* test_builtin_hooks.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimization of helper functions */
#define NO_OPT __attribute__((noinline, noclone))

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Function with multiple attributes that may interact with builtin processing */
static int hidden_function(int x) __attribute__((visibility("hidden"), nothrow, used));
static int hidden_function(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Another static function using builtins with volatile barrier */
static NO_OPT int test_builtin_arithmetic(volatile int seed) {
    volatile int result = 0;
    for (volatile int i = 0; i < 5; i++) {
        /* Use math builtins with volatile operands */
        int val = seed + i;
        result += __builtin_abs(val);
        
        /* Use sqrt builtin with type conversion */
        float fval = (float)val;
        result += (int)__builtin_sqrtf(fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    return result;
}

/* Function using bit manipulation builtins */
static NO_OPT int test_builtin_bitops(unsigned int seed) {
    volatile unsigned int accum = 0;
    
    /* Use various bit operation builtins */
    accum += __builtin_popcount(seed);
    accum += __builtin_clz(seed | 1);  /* Ensure non-zero */
    accum += __builtin_ctz(seed | 1);
    accum += __builtin_ffs(seed | 1);
    
    /* Rotate operations */
    accum += __builtin_rotateleft32(seed, 3);
    accum += __builtin_rotateright32(seed, 2);
    
    return accum;
}

/* Function using overflow checking builtins */
static NO_OPT int test_builtin_overflow(int a, int b) {
    volatile int result = 0;
    int overflow;
    
    /* Test various overflow builtins */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1;
    }
    
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 2;
    }
    
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 4;
    }
    
    /* Use builtin for type conversion with overflow check */
    long long ll = (long long)a * b;
    if (__builtin_saddll_overflow(a, b, &overflow)) {
        result += 8;
    }
    
    return result;
}

/* Function with explicit builtin prototype declaration */
static NO_OPT int test_builtin_declaration(volatile int x) {
    /* Declare builtin function prototype */
    int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
    
    /* Use the declared builtin */
    int count = __builtin_popcount((unsigned int)x);
    
    /* Use other builtins with explicit attributes */
    int __builtin_expect(long, long) __attribute__((nothrow, used));
    
    if (__builtin_expect(count > 0, 1)) {
        return count + __builtin_clz((unsigned int)x | 1);
    }
    
    return count;
}

/* External function definition (simulating separate compilation unit) */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable for impossible paths */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Complex builtin usage */
    int result = __builtin_bswap32((unsigned int)x);
    
    /* Use builtin for memory operations */
    int buffer[4] = {0};
    __builtin_memcpy(buffer, &x, sizeof(x));
    
    /* Use builtin for constant propagation barrier */
    result += __builtin_constant_p(x) ? 0 : 1;
    
    return result;
}

/* Function combining multiple builtin patterns */
static NO_OPT int test_complex_builtin_usage(volatile int seed) {
    int result = 0;
    
    /* Loop with volatile counter and builtins */
    for (volatile int i = 0; i < 3; i++) {
        int val = seed + i * 7;
        
        /* Chain builtin calls */
        result += __builtin_abs(__builtin_abs(val) - 10);
        
        /* Use builtin in conditional */
        result += __builtin_parity((unsigned int)val) ? 1 : 0;
        
        /* Use builtin for alignment */
        result += (int)__builtin_align_up(val, 8);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    
    /* Initialize with time for more variability */
    seed += (int)time(NULL) & 0xFF;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic builtins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation builtins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow builtins */
    checksum += test_builtin_overflow(seed, seed + 100);
    
    /* Test 4: Function with hidden visibility attribute */
    checksum += hidden_function(seed);
    
    /* Test 5: Builtin declaration with attributes */
    checksum += test_builtin_declaration(seed);
    
    /* Test 6: External linkage simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Complex builtin usage */
    checksum += test_complex_builtin_usage(seed);
    
    /* Additional direct builtin calls in main */
    checksum += __builtin_ffs(checksum | 1);
    checksum += __builtin_popcount((unsigned int)checksum);
    
    /* Use __builtin_expect for branch prediction */
    if (__builtin_expect(checksum > 0, 1)) {
        checksum += __builtin_clz((unsigned int)checksum);
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}
