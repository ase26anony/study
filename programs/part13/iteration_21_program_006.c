/* test_11_operands.c */
/* Compile with: gcc -O2 -fdump-rtl-expand test_11_operands.c -o test_11_operands */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
     * No clobbers (they don't count as ops in this context)
     */
    asm volatile (
        "# 11-operand asm\n\t"
        "mov %0, %5\n\t"
        "add %1, %5, %6\n\t"
        "add %2, %5, %7\n\t"
        "add %3, %5, %8\n\t"
        "add %4, %5, %9"
        : "=r"(o0), "=r"(o1), "=r"(o2), "=r"(o3), "=r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit values */
#ifdef __SIZEOF_INT128__
void test_large_atomic(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 old_val = 0;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
    }
}
#endif

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    /* Complex vector operation that may expand to many RTL operands */
    v8si result = __builtin_shufflevector(a, b, 0, 2, 4, 6, 8, 10, 12, 14);
    
    /* Use result to prevent elimination */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    /*
     * 11 operands:
     * - 4 outputs
     * - 7 inputs
     */
    asm volatile (
        "# Custom 11-operand instruction\n\t"
        "lea %0, [%4 + %5]\n\t"
        "lea %1, [%4 + %6]\n\t"
        "lea %2, [%4 + %7]\n\t"
        "lea %3, [%4 + %8]"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Test 5: Memory barrier with many address operands */
void test_memory_barrier(void) {
    volatile uint64_t vars[7] = {1, 2, 3, 4, 5, 6, 7};
    uint64_t results[4];
    
    /* Simulate a complex memory operation with many address calculations */
    asm volatile (
        "# Memory barrier with many addresses\n\t"
        "mfence\n\t"
        "mov %0, [%4]\n\t"
        "mov %1, [%4 + 8]\n\t"
        "mov %2, [%4 + 16]\n\t"
        "mov %3, [%4 + 24]"
        : "=r"(results[0]), "=r"(results[1]), "=r"(results[2]), "=r"(results[3])
        : "r"(&vars[0]), "r"(&vars[1]), "r"(&vars[2]), "r"(&vars[3]), 
          "r"(&vars[4]), "r"(&vars[5]), "r"(&vars[6])
        : "memory"
    );
    
    for (int i = 0; i < 4; i++) {
        sink += results[i];
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test multiple times to increase coverage probability */
    for (int i = 0; i < 10000; i++) {
        test_multi_operand_asm();
        test_custom_multi_output();
        test_memory_barrier();
        test_vector_shuffle();
        
        #ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic();
        }
        #endif
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
