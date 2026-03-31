/* test_11_operand_rtl.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 
     * 11 operands total:
     * - 5 output operands (o0-o4)
     * - 6 input operands (i0-i5)
     * Total: 11
     */
    asm volatile (
        "# 11-operand asm\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "add %3, %5, %7\n\t"
        "add %4, %6, %8"
        : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t old_val = 0;
    int128_t new_val = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t expected = 0;
    
    /* __atomic_compare_exchange_n often expands to complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)(expected >> 64) + (uint64_t)expected;
    }
}
#endif

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    /* Complex vector operation that may generate multi-operand RTL */
    for (int i = 0; i < 1000; i++) {
        v8si temp = v1 + v2;
        v4si shuffled = __builtin_shuffle(temp, mask);
        
        /* Use results to prevent elimination */
        for (int j = 0; j < 4; j++) {
            sink += shuffled[j];
        }
    }
}

/* Test 4: Mixed constraints inline assembly */
void test_mixed_constraints_asm(void) {
    uint64_t a, b, c, d, e;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /*
     * 12 operands total:
     * - 5 outputs (a-e)
     * - 7 inputs (x, y, z, w, v, u, t)
     * Total: 12 (will hit case 11 or higher)
     */
    asm volatile (
        "# Mixed constraints asm\n\t"
        "mov %0, %5\n\t"
        "add %1, %6, %7\n\t"
        "mul %2, %8, %9\n\t"
        "and %3, %10, %11\n\t"
        "orr %4, %12, %5"
        : "=&r" (a), "=&r" (b), "=&r" (c), "=&r" (d), "=r" (e)
        : "r" (x), "r" (y), "r" (z), "r" (w), "r" (v), "r" (u), "r" (t)
        : "cc", "memory"
    );
    
    sink += a + b + c + d + e;
}

/* Test 5: Builtin with many arguments */
void test_multi_arg_builtin(void) {
    /* __builtin_cpu_supports with many features */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3", "ssse3", 
        "sse4.1", "sse4.2", "popcnt", "aes", "pclmul"
    };
    
    for (int i = 0; i < 11; i++) {
        if (__builtin_cpu_supports(features[i])) {
            sink += i;
        }
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test multiple times to increase coverage probability */
    for (int i = 0; i < 10000; i++) {
        test_multi_operand_asm();
        test_mixed_constraints_asm();
        test_vector_shuffle();
        test_multi_arg_builtin();
        
#ifdef __SIZEOF_INT128__
        if (i % 100 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
