/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5;
    uint64_t i5 = 6, i6 = 7, i7 = 8, i8 = 9;
    
    /* 11 operands: 4 outputs + 9 inputs = 13 total constraints
       but we need 11 operands in the RTL pattern.
       Let's use 3 outputs and 8 inputs = 11 operands */
    asm volatile (
        "# Multi-operand asm with 11 operands\n\t"
        "add %0, %3, %4\n\t"
        "add %1, %5, %6\n\t"
        "add %2, %7, %8\n\t"
        : "=&r" (o0), "=&r" (o1), "=&r" (o2)  /* 3 outputs */
        : "r" (i0), "r" (i1), "r" (i2),       /* 9 inputs */
          "r" (i3), "r" (i4), "r" (i5),
          "r" (i6), "r" (i7), "r" (i8)
        : "cc"
    );
    
    sink += o0 + o1 + o2;
}

/* Test 2: Atomic operations on 128-bit type */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t expected = 0;
    int128_t desired = 0x123456789ABCDEF0;
    desired = (desired << 64) | 0xFEDCBA9876543210;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    for (int i = 0; i < 100; i++) {
        int128_t new_val = desired + i;
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val,
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)expected + (uint64_t)(expected >> 64);
    }
}
#endif

/* Test 3: Vector shuffle with many operands */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle that might generate many RTL operands */
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};
    
    /* __builtin_shufflevector can generate complex RTL patterns */
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output instruction simulation */
void test_custom_multi_output(void) {
    uint64_t a, b, c, d, e;
    uint64_t x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    uint64_t x6 = 6, x7 = 7, x8 = 8;
    
    /* Simulate a fused operation with 5 outputs and 8 inputs = 13 operands
       Adjust to get exactly 11 operands in RTL */
    asm volatile (
        "# Custom multi-output instruction\n\t"
        "mov %0, %5\n\t"
        "add %1, %5, %6\n\t"
        "mul %2, %6, %7\n\t"
        "and %3, %7, %8\n\t"
        "orr %4, %8, %9\n\t"
        : "=&r" (a), "=&r" (b), "=&r" (c), "=&r" (d), "=&r" (e)
        : "r" (x1), "r" (x2), "r" (x3), "r" (x4), 
          "r" (x5), "r" (x6), "r" (x7), "r" (x8)
        : "cc"
    );
    
    sink += a + b + c + d + e;
}

/* Test 5: Complex builtin with many arguments */
void test_complex_builtin(void) {
    /* __builtin_cpu_supports with many features */
    const char *features[] = {
        "avx", "avx2", "fma", "sse", "sse2", 
        "sse3", "ssse3", "sse4.1", "sse4.2",
        "popcnt", "aes", "pclmul"
    };
    
    int supports = 0;
    for (int i = 0; i < 12; i++) {
        supports += __builtin_cpu_supports(features[i]);
    }
    sink += supports;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 10000; i++) {
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
        
        if (i % 1000 == 0) {
            test_complex_builtin();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
