/* test_11_operand.c - Test program to trigger 11-operand RTL generation */
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
        "mov %4, %9\n\t"
        "add %0, %10\n\t"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit values */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    for (int i = 0; i < 1000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
        sink += (uint64_t)expected;
    }
}
#endif

/* Test 3: Vector operations with many elements */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    int mask[8] = {1, 3, 5, 7, 0, 2, 4, 6};
    
    /* Complex shuffle that may generate many RTL operands */
    v8si result = __builtin_shuffle(a, b, *(v8si*)mask);
    
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output operation */
void test_custom_multi_output(void) {
    uint64_t a, b, c, d, e;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* 5 outputs + 7 inputs = 12 operands (adjust to 11 as needed) */
    asm volatile (
        "# Custom multi-output\n\t"
        "lea %0, [%5 + %6]\n\t"
        "lea %1, [%7 + %8]\n\t"
        "lea %2, [%9 + %10]\n\t"
        "lea %3, [%5 + %7]\n\t"
        "lea %4, [%6 + %8]\n\t"
        : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e)
        : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u)
        : "cc"
    );
    
    sink += a + b + c + d + e;
}

/* Test 5: Mixed constraints with memory clobber */
void test_mixed_constraints(void) {
    uint64_t o0, o1, o2;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7, i7 = 8;
    
    /* 3 outputs + 8 inputs = 11 operands */
    asm volatile (
        "# Mixed constraints\n\t"
        "mov %0, %3\n\t"
        "imul %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %6\n\t"
        "mov %2, %7\n\t"
        "sub %2, %8\n\t"
        "add %0, %9\n\t"
        "add %1, %10\n\t"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), "r"(i6), "r"(i7)
        : "memory", "cc"
    );
    
    sink += o0 + o1 + o2;
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
