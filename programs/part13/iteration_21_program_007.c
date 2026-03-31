/* test_11_operands.c - Test program to trigger 11-operand RTL generation */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This often expands to complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7;
    
    /* 11 total operands: 5 outputs + 7 inputs - 1 duplicate (i0 used twice) */
    for (int i = 0; i < 1000; i++) {
        asm volatile (
            "# Dummy 11-operand instruction\n\t"
            "add %0, %1, %6\n\t"
            "add %2, %3, %7\n\t"
            "add %4, %5, %8"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), "r"(i6)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++; i6++;
    }
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    for (int i = 0; i < 1000; i++) {
        /* __builtin_shufflevector can generate multi-operand RTL */
        v4si result = __builtin_shufflevector(vec1, vec2, 0, 8, 2, 10);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 4; j++) {
            sink += result[j];
        }
        
        /* Modify vectors slightly */
        vec1[0]++; vec2[0]++;
    }
}

/* Test 4: Multiple output constraints with early clobber */
void test_multi_output_asm(void) {
    uint64_t a, b, c, d, e, f;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* 13 operands total: 6 outputs + 7 inputs */
    for (int i = 0; i < 1000; i++) {
        asm volatile (
            "# Multi-output operation\n\t"
            "mov %0, %6\n\t"
            "add %1, %7, %8\n\t"
            "mul %2, %9, %10\n\t"
            "and %3, %11, %12\n\t"
            "orr %4, %6, %7\n\t"
            "eor %5, %8, %9"
            : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e), "=&r"(f)
            : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u), "r"(t)
            : "cc"
        );
        
        sink += a + b + c + d + e + f;
        x++; y++; z++; w++; v++; u++; t++;
    }
}

/* Test 5: Builtin with many arguments */
void test_many_arg_builtin(void) {
    /* __builtin_cpu_supports with many features to check */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2",
        "popcnt", "aes", "pclmul", "fma", "fma4", "xop", "avx512f"
    };
    
    for (int i = 0; i < 1000; i++) {
        int supports = 0;
        /* Check multiple features - each may expand to multi-operand RTL */
        for (int j = 0; j < 5 && j < sizeof(features)/sizeof(features[0]); j++) {
            supports += __builtin_cpu_supports(features[j]);
        }
        sink += supports;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests multiple times */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_atomic_128bit();
        test_multi_operand_asm();
        test_vector_shuffle();
        test_multi_output_asm();
        test_many_arg_builtin();
        
        printf("Iteration %d, sink = %lu\n", iteration, (unsigned long)sink);
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
