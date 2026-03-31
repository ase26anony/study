/* test_optabs_11_operands.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operation on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
        expected = large_atomic;
        new_val = new_val + 1;
    }
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "mul %3, %5, %7\n\t"
            "mul %4, %6, %8"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0 + i), "r" (i1 + i), "r" (i2 + i), 
              "r" (i3 + i), "r" (i4 + i), "r" (i5 + i)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Strategy 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex vector operation that may expand to many RTL operands */
        v8si result = a + b;
        
        /* Shuffle operation with runtime mask - may generate complex RTL */
        v4si shuffled = __builtin_shuffle(a, b, mask);
        
        /* Use results to prevent elimination */
        for (int j = 0; j < 4; j++) {
            sink += shuffled[j];
        }
        
        a = result;
        b = result + 1;
        mask = (v4si){mask[1], mask[2], mask[3], mask[0]};
    }
}

/* Strategy 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    /* 4 outputs + 7 inputs = 11 operands */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Custom fused operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "add %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "add %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "add %3, %3, %4"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3),
              "r" (in4), "r" (in5), "r" (in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++;
    }
}

/* Strategy 5: Complex builtin with many arguments */
void test_complex_builtin(void) {
    /* __builtin_cpu_supports with multiple features checked */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3",
        "ssse3", "sse4.1", "sse4.2", "fma", "avx512f"
    };
    
    for (int i = 0; i < 10000; i++) {
        int supports = 0;
        /* Check multiple features - each may become an operand */
        for (int j = 0; j < 10; j++) {
            supports += __builtin_cpu_supports(features[j]);
        }
        sink += supports;
    }
}

int main(void) {
    printf("Testing optabs with 11 operands...\n");
    
    /* Run all test strategies */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed\n");
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed\n");
    
    test_vector_shuffle();
    printf("Vector shuffle test completed\n");
    
    test_custom_multi_output();
    printf("Custom multi-output test completed\n");
    
    test_complex_builtin();
    printf("Complex builtin test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
