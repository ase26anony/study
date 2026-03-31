/* test_11_operand_optab.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap (likely to generate multi-operand RTL) */
void test_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    /* This builtin often expands to complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &old_val, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    sink += (uint64_t)large_atomic;
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm block\n\t"
        "mov %0, %5\n\t"
        "add %1, %6, %7\n\t"
        "sub %2, %8, %9\n\t"
        "and %3, %10, %5\n\t"
        "or %4, %6, %8\n\t"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 3: Vector shuffle with many operands */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle that may generate many RTL operands */
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};
    v8si result = __builtin_shufflevector(v1, v2, 
                                          0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use all elements to prevent optimization */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    /* 4 outputs + 7 inputs = 11 operands */
    asm volatile (
        "# Custom 11-operand instruction\n\t"
        "lea %0, [%4 + %5]\n\t"
        "imul %1, %6, %7\n\t"
        "xor %2, %8, %9\n\t"
        "add %3, %10, %4\n\t"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Test 5: Atomic exchange on 16-byte object */
void test_atomic_exchange(void) {
    _Atomic __int128 atomic_var = 0;
    __int128 new_val = ((__int128)sink << 64) | sink;
    
    /* This may generate RTL with many operands */
    __int128 old = __atomic_exchange_n(&atomic_var, new_val, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)old + (uint64_t)(old >> 64);
}

int main(void) {
    printf("Testing 11-operand optab coverage...\n");
    
    /* Run each test many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_atomic_cmpxchg();
        test_multi_operand_asm();
        test_vector_shuffle();
        test_custom_multi_output();
        test_atomic_exchange();
        
        /* Prevent loop unrolling from simplifying things */
        if (i % 1000 == 0) {
            sink = sink % 1000000;
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
