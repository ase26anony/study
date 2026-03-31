/* test_11_operand_rtl.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm() {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 
     * 11 operands total:
     * - 5 output operands (o0-o4)
     * - 6 input operands (i0-i5)
     * Total: 11 operands
     */
    asm volatile (
        "# Dummy 11-operand assembly\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "mul %3, %5, %6\n\t"
        "mul %4, %7, %8"
        : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5)
        : "cc"
    );
    
    /* Use all outputs to prevent dead code elimination */
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Strategy 2: Atomic operations on __int128 */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg() {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t expected = 0;
    int128_t desired = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t result;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Another atomic operation */
    result = __atomic_exchange_n(&large_atomic, desired * 2, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)(result >> 64) + (uint64_t)result;
}
#endif

/* Strategy 3: Vector operations with complex shuffling */
void test_vector_shuffle() {
    /* 256-bit vector (8 ints) */
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si result;
    
    /* Complex shuffle operation */
    result = __builtin_shuffle(a, b, mask);
    
    /* Use result to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Strategy 4: Custom multi-output asm with memory clobber */
void test_custom_multi_output() {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 100, in1 = 200, in2 = 300, in3 = 400, in4 = 500;
    uint64_t in5 = 600, in6 = 700, in7 = 800;
    
    /*
     * 12 operands total:
     * - 4 outputs
     * - 8 inputs
     * Total: 12 operands (more than 11)
     */
    asm volatile (
        "# Custom multi-output operation\n\t"
        "mov %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %2, %9\n\t"
        "mov %3, %10\n\t"
        "add %3, %3, %11"
        : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
          "r" (in4), "r" (in5), "r" (in6), "r" (in7)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Strategy 5: Complex builtin with many arguments */
void test_complex_builtin() {
    /* __builtin_cpu_supports with many features */
    int features = 0;
    
    /* Check multiple CPU features (each string is an operand) */
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

/* Main function with hot loop */
int main() {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test many times in a hot loop */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        
#ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
        
        if (i % 500 == 0) {
            test_vector_shuffle();
        }
        
        if (i % 200 == 0) {
            test_custom_multi_output();
        }
        
        if (i % 10000 == 0) {
            test_complex_builtin();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    printf("Test completed.\n");
    
    return 0;
}
