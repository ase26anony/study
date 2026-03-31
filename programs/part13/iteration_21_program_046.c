/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-exchange on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This often expands to complex RTL with many operands */
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
    
    /* 5 outputs + 6 inputs = 11 operands total */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "add %3, %5, %7\n\t"
            "add %4, %6, %8"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
            : "r"(i0 + i), "r"(i1 + i), "r"(i2 + i), 
              "r"(i3 + i), "r"(i4 + i), "r"(i5 + i)
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
        /* Complex shuffle that may generate multi-operand RTL */
        v8si shuffled = __builtin_shufflevector(vec1, vec2, 
            0, 8, 2, 10, 4, 12, 6, 14);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += shuffled[j];
        }
        
        /* Modify vectors slightly */
        vec1[0] += i;
        vec2[7] -= i;
    }
}

/* Test 4: Custom multi-output SIMD-like operation via inline asm */
void test_custom_simd_operation(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    /* 4 outputs + 7 inputs = 11 operands */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Custom SIMD operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "mul %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "xor %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "sub %3, %3, %4"
            : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
            : "r"(in0 + i), "r"(in1 + i), "r"(in2 + i), "r"(in3 + i),
              "r"(in4 + i), "r"(in5 + i), "r"(in6 + i)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Rotate inputs */
        uint64_t tmp = in6;
        in6 = in5; in5 = in4; in4 = in3; in3 = in2; in2 = in1; in1 = in0; in0 = tmp;
    }
}

/* Test 5: Builtin with many arguments - __builtin_cpu_supports */
void test_multi_string_builtin(void) {
    int result = 0;
    
    /* Chain multiple __builtin_cpu_supports calls */
    for (int i = 0; i < 10000; i++) {
        /* Each string argument becomes an operand during expansion */
        result |= __builtin_cpu_supports("sse") << 0;
        result |= __builtin_cpu_supports("sse2") << 1;
        result |= __builtin_cpu_supports("avx") << 2;
        result |= __builtin_cpu_supports("avx2") << 3;
        result |= __builtin_cpu_supports("fma") << 4;
        
        sink += result;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to increase coverage chances */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed\n");
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed\n");
    
    test_vector_shuffle();
    printf("Vector shuffle test completed\n");
    
    test_custom_simd_operation();
    printf("Custom SIMD operation test completed\n");
    
    test_multi_string_builtin();
    printf("Multi-string builtin test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
