/* test_11_operand_rtl.c */
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm\n\t"
        "mov %0, %5\n\t"
        "mov %1, %6\n\t"
        "mov %2, %7\n\t"
        "mov %3, %8\n\t"
        "mov %4, %9"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit types */
void test_large_atomic_ops(void) {
    /* Use __int128 for 16-byte atomic operations */
    _Atomic __int128 large_atomic = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    __int128 desired = new_val;
    int success;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        success = __atomic_compare_exchange_n(&large_atomic, &expected, desired, 
                                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        if (success) {
            sink += i;
        }
    }
}

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  /* Interleave elements */
    
    /* Complex shuffle operation that may expand to many RTL operands */
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output asm with 11 operands */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    /* 4 outputs + 7 inputs = 11 operands */
    asm volatile (
        "# Custom 11-operand instruction\n\t"
        "add %0, %4, %5\n\t"
        "add %1, %6, %7\n\t"
        "add %2, %8, %9\n\t"
        "mul %3, %10, %4"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Test 5: Complex builtin with many arguments */
void test_complex_builtin(void) {
    /* __builtin_cpu_supports with many features to check */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3", 
        "ssse3", "sse4.1", "sse4.2", "fma", "avx512f"
    };
    
    int supports = 0;
    for (int i = 0; i < 10; i++) {
        supports += __builtin_cpu_supports(features[i]);
    }
    
    sink += supports;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run tests many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        test_large_atomic_ops();
        test_vector_shuffle();
        test_custom_multi_output();
        test_complex_builtin();
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 1000 == 0) {
            sink += i;
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    printf("Test completed.\n");
    
    return 0;
}
