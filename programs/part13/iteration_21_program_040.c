/* test_11_operand.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
__attribute__((noinline))
uint64_t test_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c, 
                                uint64_t d, uint64_t e, uint64_t f,
                                uint64_t g, uint64_t h, uint64_t i,
                                uint64_t j, uint64_t k) {
    uint64_t o0, o1, o2, o3, o4, o5;
    
    /* 11 operands: 6 outputs + 5 inputs = 11 total operands in constraints */
    asm volatile (
        "# 11-operand asm\n\t"
        "add %0, %6, %7\n\t"
        "adc %1, %8, %9\n\t"
        "mul %2, %10, %11\n\t"
        "and %3, %12, %13\n\t"
        "or  %4, %14, %15\n\t"
        "xor %5, %16, %17"
        : "=&r" (o0), "=&r" (o1), "=&r" (o2), 
          "=&r" (o3), "=&r" (o4), "=&r" (o5)      /* 6 outputs */
        : "r" (a), "r" (b), "r" (c), "r" (d),     /* 11 inputs total */
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j), "r" (k)
        : "cc"
    );
    
    return o0 + o1 + o2 + o3 + o4 + o5;
}

/* Test 2: Atomic operations on 128-bit values */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
_Atomic int128_t large_atomic;

__attribute__((noinline))
int128_t test_atomic_128bit(int128_t new_val) {
    int128_t expected = 0;
    int128_t desired = new_val;
    
    /* __atomic_compare_exchange_n can generate many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* __atomic_exchange_n also generates complex RTL */
    return __atomic_exchange_n(&large_atomic, desired, __ATOMIC_SEQ_CST);
}
#endif

/* Test 3: Vector operations with many elements */
typedef int v8si __attribute__((vector_size(32)));

__attribute__((noinline))
v8si test_vector_shuffle(v8si a, v8si b, v8si mask) {
    /* __builtin_shufflevector with many elements */
    return __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
}

/* Test 4: Complex builtin with many arguments */
__attribute__((noinline))
uint64_t test_multi_arg_builtin(uint64_t a, uint64_t b, uint64_t c,
                                uint64_t d, uint64_t e, uint64_t f) {
    /* __builtin_add_overflow with many arguments (chained) */
    uint64_t sum1, sum2, sum3;
    unsigned char ov1, ov2, ov3;
    
    ov1 = __builtin_add_overflow(a, b, &sum1);
    ov2 = __builtin_add_overflow(sum1, c, &sum2);
    ov3 = __builtin_add_overflow(sum2, d, &sum3);
    
    /* Force use of all results */
    return sum3 + ov1 + ov2 + ov3 + e + f;
}

/* Test 5: Custom multi-output operation */
__attribute__((noinline))
void test_custom_multi_op(uint64_t in0, uint64_t in1, uint64_t in2,
                          uint64_t in3, uint64_t in4, uint64_t in5,
                          uint64_t *out0, uint64_t *out1, uint64_t *out2,
                          uint64_t *out3, uint64_t *out4) {
    /* Simulate a 5-input, 5-output operation */
    asm volatile (
        "# Custom 10-operand operation\n\t"
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %8\n\t"
        "mov %2, %9\n\t"
        "and %2, %10\n\t"
        "mov %3, %5\n\t"
        "xor %3, %7\n\t"
        "mov %4, %6\n\t"
        "or  %4, %8"
        : "=r" (*out0), "=r" (*out1), "=r" (*out2),
          "=r" (*out3), "=r" (*out4)
        : "r" (in0), "r" (in1), "r" (in2),
          "r" (in3), "r" (in4), "r" (in5)
        : "cc"
    );
}

int main() {
    uint64_t result = 0;
    
    /* Initialize many variables */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = i + 1;
    }
    
    /* Run tests in a hot loop */
    for (int iter = 0; iter < 100000; iter++) {
        /* Test 1: 11-operand inline asm */
        result ^= test_multi_operand_asm(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9],
            vars[10]
        );
        
        /* Test 2: 128-bit atomic operations */
        #ifdef __SIZEOF_INT128__
        int128_t val128 = ((int128_t)vars[11] << 64) | vars[12];
        test_atomic_128bit(val128);
        #endif
        
        /* Test 3: Vector shuffle */
        v8si vec_a = {vars[0], vars[1], vars[2], vars[3],
                      vars[4], vars[5], vars[6], vars[7]};
        v8si vec_b = {vars[8], vars[9], vars[10], vars[11],
                      vars[12], vars[13], vars[14], vars[15]};
        v8si vec_result = test_vector_shuffle(vec_a, vec_b, vec_a);
        
        /* Extract from vector */
        for (int i = 0; i < 8; i++) {
            result += vec_result[i];
        }
        
        /* Test 4: Multi-argument builtin */
        result += test_multi_arg_builtin(
            vars[0], vars[1], vars[2], vars[3], vars[4], vars[5]
        );
        
        /* Test 5: Custom multi-output operation */
        uint64_t outputs[5];
        test_custom_multi_op(
            vars[0], vars[1], vars[2], vars[3], vars[4], vars[5],
            &outputs[0], &outputs[1], &outputs[2], &outputs[3], &outputs[4]
        );
        
        for (int i = 0; i < 5; i++) {
            result += outputs[i];
        }
        
        /* Modify variables to prevent constant propagation */
        for (int i = 0; i < 20; i++) {
            vars[i] = vars[i] * 1103515245 + 12345;
        }
    }
    
    sink = result;
    printf("Result: %lu\n", (unsigned long)result);
    
    return 0;
}
