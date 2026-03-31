/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations with various attributes */
extern int external_builtin_user(unsigned int x) 
    __attribute__((visibility("hidden"), nothrow));

static int static_builtin_helper(int a, int b) 
    __attribute__((noinline, noclone, used));

/* Function with explicit hidden visibility and nothrow */
int __attribute__((visibility("hidden"), nothrow, used)) 
attributed_function(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* External function definition using builtins */
int external_builtin_user(unsigned int x) {
    int result = 0;
    volatile int vol = x;
    
    /* Use multiple builtins in sequence */
    result += __builtin_popcount(vol);
    result += __builtin_clz(vol | 1);  /* Ensure non-zero */
    
    /* __builtin_unreachable under specific condition */
    if (vol == 0xFFFFFFFF) {
        __builtin_unreachable();
    }
    
    return result;
}

/* Static function with builtin usage */
static int static_builtin_helper(int a, int b) 
    __attribute__((noinline, noclone, used)) {
    volatile int vol_a = a;
    volatile int vol_b = b;
    int sum = 0;
    
    /* Arithmetic builtins with volatile operands */
    sum += __builtin_abs(vol_a);
    sum += __builtin_abs(vol_b);
    
    /* Overflow checking builtins */
    int overflow;
    if (__builtin_add_overflow(vol_a, vol_b, &overflow)) {
        sum += 1000;
    }
    
    return sum;
}

/* Another static function with different builtins */
static float __attribute__((used)) 
float_builtins(float f) {
    volatile float vol_f = f;
    
    /* Use math builtins */
    if (vol_f >= 0.0f) {
        return __builtin_sqrtf(vol_f);
    }
    return 0.0f;
}

/* Function using builtin with assembly barrier */
int __attribute__((noinline)) 
builtin_with_barrier(int x) {
    volatile int vol_x = x;
    int result;
    
    /* Memory barrier prevents optimization */
    asm volatile("" : : : "memory");
    
    result = __builtin_ctz(vol_x | 1);  /* Ensure non-zero */
    
    asm volatile("" : : : "memory");
    
    return result;
}

/* Test arithmetic builtins */
int test_builtin_arithmetic(int seed) {
    volatile int accum = 0;
    volatile int counter = seed % 10 + 1;
    
    for (volatile int i = 0; i < counter; i++) {
        accum += __builtin_abs(seed + i * 7);
        accum += __builtin_abs(seed - i * 3);
        
        /* Use sqrtf builtin */
        float fval = (float)(seed + i);
        if (fval >= 0.0f) {
            accum += (int)__builtin_sqrtf(fval);
        }
    }
    
    return accum;
}

/* Test bit operation builtins */
int test_builtin_bitops(unsigned int seed) {
    volatile unsigned int vol_seed = seed;
    int result = 0;
    
    result += __builtin_popcount(vol_seed);
    result += __builtin_clz(vol_seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(vol_seed | 1);  /* Ensure non-zero */
    result += __builtin_ffs(vol_seed | 1);  /* Ensure non-zero */
    
    /* Parity builtin */
    result += __builtin_parity(vol_seed);
    
    return result;
}

/* Test overflow builtins */
int test_builtin_overflow(int a, int b) {
    volatile int vol_a = a;
    volatile int vol_b = b;
    int result = 0;
    int overflow;
    
    /* Test various overflow operations */
    if (__builtin_add_overflow(vol_a, vol_b, &overflow)) {
        result += 1;
    }
    
    if (__builtin_sub_overflow(vol_a, vol_b, &overflow)) {
        result += 2;
    }
    
    if (__builtin_mul_overflow(vol_a, vol_b, &overflow)) {
        result += 4;
    }
    
    /* Use the overflow result */
    result += overflow;
    
    return result;
}

/* Test function with multiple attributes */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(int x) {
    volatile int vol_x = x;
    
    /* Use __builtin_expect in loop */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        if (__builtin_expect((vol_x & (1 << i)) != 0, 0)) {
            sum += __builtin_popcount(vol_x >> i);
        }
    }
    
    /* Use __builtin_unreachable for impossible case */
    if (vol_x < 0 && vol_x > 1000) {
        __builtin_unreachable();
    }
    
    return sum;
}

/* Test external linkage simulation */
int test_external_linkage(unsigned int x) {
    /* Call externally declared function */
    int result = external_builtin_user(x);
    
    /* Additional builtin usage */
    result += __builtin_bswap16(x & 0xFFFF);
    result += __builtin_bswap32(x);
    
    return result;
}

/* Complex expression with multiple builtins */
int complex_builtin_expression(int x, int y) {
    volatile int vol_x = x;
    volatile int vol_y = y;
    int result = 0;
    
    /* Nested builtin calls */
    result = __builtin_abs(__builtin_abs(vol_x) - __builtin_abs(vol_y));
    
    /* Builtin in conditional */
    if (__builtin_expect(result > 100, 0)) {
        result = __builtin_clz(result | 1);
    }
    
    /* Builtin with assembly barrier */
    asm volatile("" : : : "memory");
    result += __builtin_ffs(vol_x | vol_y | 1);
    asm volatile("" : : : "memory");
    
    return result;
}

/* Main function tying everything together */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed from various sources */
    volatile unsigned int seed = (unsigned int)time(NULL);
    if (argc > 1) {
        seed += (unsigned int)atoi(argv[1]);
    }
    seed += (unsigned int)argc;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic builtins */
    checksum += test_builtin_arithmetic((int)seed);
    
    /* Test 2: Bit operation builtins */
    checksum += test_builtin_bitops(seed);
    
    /* Test 3: Overflow builtins */
    checksum += test_builtin_overflow((int)seed, (int)(seed >> 16));
    
    /* Test 4: Attributed function */
    checksum += test_attributed_function((int)seed);
    
    /* Test 5: External linkage */
    checksum += test_external_linkage(seed);
    
    /* Test 6: Static helper with builtins */
    checksum += static_builtin_helper((int)seed, (int)(seed >> 8));
    
    /* Test 7: Float builtins */
    checksum += (int)float_builtins((float)seed);
    
    /* Test 8: Builtin with barrier */
    checksum += builtin_with_barrier((int)seed);
    
    /* Test 9: Complex expression */
    checksum += complex_builtin_expression((int)seed, (int)(seed >> 4));
    
    /* Test 10: Direct attributed function call */
    checksum += attributed_function((int)seed);
    
    /* Use checksum to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
