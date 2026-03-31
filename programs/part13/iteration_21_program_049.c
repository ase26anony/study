/* test_11_operand_rtl.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm block\n\t"
        "mov %0, %5\n\t"
        "add %1, %6, %7\n\t"
        "sub %2, %8, %9\n\t"
        "mul %3, %10, %5\n\t"
        "and %4, %6, %7"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic;
    
    int128_t old_val = 0;
    int128_t new_val = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)expected;
}

void test_atomic_exchange_128bit(void) {
    typedef __int128 int128_t;
    _Atomic int128_t atomic_var = 0;
    int128_t new_val = ((int128_t)0xDEADBEEFULL << 96) | 0xCAFEBABEULL;
    int128_t result;
    
    result = __atomic_exchange_n(&atomic_var, new_val, __ATOMIC_SEQ_CST);
    sink += (uint64_t)result;
}
#endif

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle that may generate many RTL operands */
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent elimination */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Multiple output constraints with early clobber */
void test_multi_output_asm(void) {
    uint64_t out1, out2, out3, out4, out5;
    uint64_t in1 = 100, in2 = 200, in3 = 300, in4 = 400, in5 = 500, in6 = 600;
    
    /* 5 outputs + 6 inputs = 11 operands */
    asm volatile (
        "# Multi-output operation\n\t"
        "lea %0, [%5 + %6]\n\t"
        "lea %1, [%7 + %8]\n\t"
        "lea %2, [%9 + %10]\n\t"
        "mov %3, %5\n\t"
        "mov %4, %6"
        : "=&r"(out1), "=&r"(out2), "=&r"(out3), "=&r"(out4), "=&r"(out5)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    sink += out1 + out2 + out3 + out4 + out5;
}

/* Test 5: Builtin with many arguments */
void test_many_arg_builtin(void) {
    /* __builtin_cpu_supports with many feature checks */
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
    features += __builtin_cpu_supports("fma");
    
    sink += features;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run tests many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        
#ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic_cmpxchg();
            test_atomic_exchange_128bit();
        }
#endif
        
        if (i % 500 == 0) {
            test_vector_shuffle();
            test_multi_output_asm();
            test_many_arg_builtin();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    printf("Test completed.\n");
    
    return 0;
}
