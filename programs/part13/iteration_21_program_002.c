/* test_11_operands.c */
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Strategy 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    int o0, o1, o2, o3, o4;
    int i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm\n\t"
        "mov %0, %5\n\t"
        "add %1, %5, %6\n\t"
        "add %2, %6, %7\n\t"
        "add %3, %7, %8\n\t"
        "add %4, %8, %9"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Strategy 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    for (int i = 0; i < 100; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (int)(expected & 0xFFFFFFFF);
    }
}
#endif

/* Strategy 3: Vector operations with complex patterns */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

void test_vector_shuffle(void) {
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  // Interleave elements
    
    /* Complex shuffle that may generate many RTL operands */
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Strategy 4: Custom multi-output operation simulation */
void test_custom_multi_output(void) {
    long long a, b, c, d, e;
    long long x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* Simulate a fused operation with many inputs/outputs */
    asm volatile (
        "# Custom multi-output\n\t"
        "imul %0, %5, %6\n\t"
        "imul %1, %6, %7\n\t"
        "imul %2, %7, %8\n\t"
        "imul %3, %8, %9\n\t"
        "imul %4, %9, %10"
        : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e)
        : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u), "r"(t)
        : "cc"
    );
    
    sink += (int)(a + b + c + d + e);
}

/* Strategy 5: Complex builtin with many arguments */
void test_complex_builtin(void) {
    /* __builtin_cpu_supports with many features checks */
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

/* Main function that runs all tests in hot loops */
int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        
#ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
        
        if (i % 500 == 0) {
            test_vector_shuffle();
            test_custom_multi_output();
            test_complex_builtin();
        }
    }
    
    printf("Final sink value: %d\n", sink);
    printf("Test completed. Check coverage with:\n");
    printf("  gcc -O2 -fdump-rtl-expand test_11_operands.c\n");
    printf("  gcc -O3 -march=native test_11_operands.c -o test && ./test\n");
    
    return 0;
}
