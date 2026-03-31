/* test_11_operand.c - Test program to trigger 11-operand RTL generation */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm\n\t"
        "mov %0, %5\n\t"
        "mov %1, %6\n\t"
        "mov %2, %7\n\t"
        "mov %3, %8\n\t"
        "mov %4, %9"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : /* No clobbers to keep operand count at 11 */
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on __int128 (16 bytes) */
void test_large_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA987654321ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)large_atomic;
}

/* Test 3: Vector shuffle with many operands */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle mask - may generate many operand RTL */
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output asm with 11 operands */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, 
             in5 = 6, in6 = 7;
    
    /* 4 outputs + 7 inputs = 11 operands */
    asm volatile (
        "# Custom 11-operand instruction\n\t"
        "add %0, %4, %5\n\t"
        "add %1, %6, %7\n\t"
        "add %2, %8, %9\n\t"
        "add %3, %10, %4"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6)
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Test 5: Complex builtin with many arguments */
void test_multi_arg_builtin(void) {
    /* __builtin_cpu_supports with many features to check */
    int features = 0;
    features += __builtin_cpu_supports("mmx");
    features += __builtin_cpu_supports("sse");
    features += __builtin_cpu_supports("sse2");
    features += __builtin_cpu_supports("sse3");
    features += __builtin_cpu_supports("ssse3");
    features += __builtin_cpu_supports("sse4.1");
    features += __builtin_cpu_supports("sse4.2");
    features += __builtin_cpu_supports("avx");
    features += __builtin_cpu_supports("avx2");
    
    sink += features;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test many times to increase coverage probability */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        test_large_atomic_cmpxchg();
        test_vector_shuffle();
        test_custom_multi_output();
        test_multi_arg_builtin();
        
        /* Prevent loop unrolling from simplifying things */
        if (i % 1000 == 0) {
            sink = sink % 1000;
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    printf("Test completed.\n");
    
    return 0;
}
