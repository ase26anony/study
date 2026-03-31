/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7;
    
    /* 11 operands: 4 outputs + 7 inputs = 11 total */
    asm volatile (
        "# 11-operand asm\n\t"
        "add %0, %4, %5\n\t"
        "add %1, %6, %7\n\t"
        "add %2, %8, %9\n\t"
        "add %3, %10, %11"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3)  /* 4 outputs */
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3),         /* 7 inputs */
          "r"(i4), "r"(i5), "r"(i6)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3;
}

/* Test 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t old_val = 0;
    int128_t new_val = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t expected = 0;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
        sink += (uint64_t)expected;
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
    
    /* Complex shuffle that may expand to many RTL operands */
    v8si result = __builtin_shufflevector(v1, v2, 0, 2, 4, 6, 8, 10, 12, 14);
    
    /* Use result to prevent elimination */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output asm with memory clobber */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3, out4;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6;
    
    /* 11 operands: 5 outputs + 6 inputs = 11 total */
    asm volatile (
        "# Custom 11-operand instruction\n\t"
        "mov %0, %5\n\t"
        "add %1, %6, %7\n\t"
        "sub %2, %8, %9\n\t"
        "mul %3, %10, %11\n\t"
        "and %4, %5, %6"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), 
          "=&r"(out3), "=&r"(out4)                    /* 5 outputs */
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3),     /* 6 inputs */
          "r"(in4), "r"(in5)
        : "memory", "cc"
    );
    
    sink += out0 + out1 + out2 + out3 + out4;
}

/* Test 5: Mixed constraints with early clobber */
void test_mixed_constraints(void) {
    uint64_t a, b, c, d, e;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* 11 operands: 5 outputs + 6 inputs = 11 total */
    asm volatile (
        "# Mixed constraints with 11 operands\n\t"
        "lea (%5, %6, 1), %0\n\t"
        "lea (%7, %8, 1), %1\n\t"
        "lea (%9, %10, 1), %2\n\t"
        "mov %3, %11\n\t"
        "mov %4, %5"
        : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e)  /* 5 outputs */
        : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u), "0"(t)  /* 7 inputs, but '0' reuses a */
        : "cc"
    );
    
    sink += a + b + c + d + e;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 10000; i++) {
        test_multi_operand_asm();
        test_vector_shuffle();
        test_custom_multi_output();
        test_mixed_constraints();
        
#ifdef __SIZEOF_INT128__
        if (i % 100 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
