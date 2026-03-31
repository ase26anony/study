/* test_11_operand_optab.c */
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
    
    /* This should generate RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
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
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "mul %3, %0, %1\n\t"
            "add %4, %2, %3"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=r"(o4)
            : "r"(i0 + i), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
    }
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may generate many RTL operands */
        v8si shuffled = __builtin_shufflevector(vec1, vec2, 
            0, 8, 2, 10, 4, 12, 6, 14);
        
        /* Use the result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += shuffled[j];
        }
        
        /* Modify vectors slightly */
        vec1[0] += i;
        vec2[7] -= i;
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Custom 4-output, 7-input operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "mul %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "sub %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "xor %3, %3, %0"
            : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=r"(out3)
            : "r"(in0), "r"(in1 + i), "r"(in2), "r"(in3), 
              "r"(in4), "r"(in5), "r"(in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Modify inputs */
        in0 ^= out0;
        in1 += out1;
    }
}

/* Test 5: Builtin with many arguments */
void test_many_arg_builtin(void) {
    /* __builtin_cpu_supports with many features to check */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3", 
        "ssse3", "sse4.1", "sse4.2", "fma", "avx512f"
    };
    
    for (int i = 0; i < 10000; i++) {
        int supports = 0;
        /* Check multiple features - each may expand to RTL */
        for (int j = 0; j < 10; j++) {
            supports += __builtin_cpu_supports(features[j]);
        }
        sink += supports;
    }
}

int main(void) {
    printf("Testing 11-operand optab coverage...\n");
    
    test_atomic_128bit();
    printf("Test 1 (128-bit atomic) completed\n");
    
    test_multi_operand_asm();
    printf("Test 2 (11-operand asm) completed\n");
    
    test_vector_shuffle();
    printf("Test 3 (vector shuffle) completed\n");
    
    test_custom_multi_output();
    printf("Test 4 (custom multi-output) completed\n");
    
    test_many_arg_builtin();
    printf("Test 5 (many-arg builtin) completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
