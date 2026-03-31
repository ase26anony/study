/* test_11_operand_optab.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_cmpxchg_128(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This often expands to complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink ^= (uint64_t)large_atomic;
        sink ^= (uint64_t)(large_atomic >> 64);
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 5 outputs + 6 inputs = 11 operands total */
        asm volatile (
            "# Dummy 11-operand assembly\n\t"
            "add %0, %5, %6\n\t"
            "adc %1, %7, %8\n\t"
            "mul %2, %9, %10\n\t"
            "add %3, %0, %1\n\t"
            "sub %4, %2, %3"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=r"(o3), "=r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : "cc"
        );
        
        sink ^= o0 ^ o1 ^ o2 ^ o3 ^ o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Test 3: Vector shuffle with complex mask */
#ifdef __SSE2__
#include <xmmintrin.h>
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may generate many RTL operands */
        v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink ^= result[j];
        }
        
        /* Modify inputs */
        a[0]++; b[0]--;
    }
}
#endif

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    /* Simulate a fused multiply-add with multiple outputs */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t out1, out2, out3, out4;
    
    for (int i = 0; i < 10000; i++) {
        /* Custom asm with 4 outputs + 5 inputs = 9 operands */
        /* Add dummy clobbers to reach 11 total operands */
        asm volatile (
            "# Multi-output operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %5\n\t"
            "mov %1, %6\n\t"
            "sub %1, %7\n\t"
            "mov %2, %8\n\t"
            "xor %2, %4\n\t"
            "mov %3, %0\n\t"
            "add %3, %1"
            : "=&r"(out1), "=&r"(out2), "=&r"(out3), "=r"(out4)
            : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e)
            : "memory", "cc"  /* 2 clobbers + 9 operands = 11 total */
        );
        
        sink ^= out1 ^ out2 ^ out3 ^ out4;
        a++; b++; c++; d++; e++;
    }
}

/* Test 5: Complex atomic exchange */
void test_complex_atomic_exchange(void) {
    struct pair {
        uint64_t a;
        uint64_t b;
    };
    
    _Atomic(struct pair) atomic_pair = {0, 0};
    struct pair new_val = {0x12345678, 0x9ABCDEF0};
    
    for (int i = 0; i < 10000; i++) {
        struct pair old = __atomic_exchange_n(&atomic_pair, new_val, __ATOMIC_SEQ_CST);
        sink ^= old.a ^ old.b;
        new_val.a++; new_val.b--;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    test_atomic_cmpxchg_128();
    printf("Test 1 complete (atomic 128-bit). sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Test 2 complete (11-operand asm). sink = %lu\n", (unsigned long)sink);
    
#ifdef __SSE2__
    test_vector_shuffle();
    printf("Test 3 complete (vector shuffle). sink = %lu\n", (unsigned long)sink);
#endif
    
    test_multi_output_builtin();
    printf("Test 4 complete (multi-output). sink = %lu\n", (unsigned long)sink);
    
    test_complex_atomic_exchange();
    printf("Test 5 complete (atomic struct). sink = %lu\n", (unsigned long)sink);
    
    printf("All tests completed. Final sink = %lu\n", (unsigned long)sink);
    return 0;
}
