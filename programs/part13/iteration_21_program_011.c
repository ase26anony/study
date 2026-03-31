/* test_11_operand_rtl.c */
#include <stdint.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This often expands to complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)expected + (uint64_t)(expected >> 64);
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 5 outputs + 6 inputs = 11 operands total */
        asm volatile (
            "# Dummy 11-operand asm\n\t"
            "mov %0, %5\n\t"
            "add %1, %6, %7\n\t"
            "sub %2, %8, %9\n\t"
            "mul %3, %10, %11\n\t"
            "and %4, %5, %6"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
              "r"(i0 + i1)  /* 11th operand */
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Test 3: Vector shuffle with many operands */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

void test_vector_shuffle(void) {
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  /* Interleave elements */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may generate multi-operand RTL */
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify mask slightly each iteration */
        mask = mask + (v8si){1, 1, 1, 1, 1, 1, 1, 1};
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    unsigned long long o0, o1, o2;
    unsigned long long i0 = 0x12345678, i1 = 0x9ABCDEF0;
    unsigned long long i2 = 0x13579BDF, i3 = 0x2468ACE0;
    unsigned long long i4 = 0x55555555, i5 = 0xAAAAAAAA;
    
    for (int i = 0; i < 10000; i++) {
        /* Simulate a multi-output operation using inline asm */
        asm volatile (
            "# Multi-output operation\n\t"
            "mov %0, %3\n\t"
            "ror %0, %0, #13\n\t"
            "mov %1, %4\n\t"
            "eor %1, %1, %5\n\t"
            "mov %2, %6\n\t"
            "add %2, %2, %7\n\t"
            "mul %2, %2, %8"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2;
        i0 = (i0 << 1) | (i0 >> 63);
        i1 = (i1 << 2) | (i1 >> 62);
        i2 = (i2 << 3) | (i2 >> 61);
    }
}

/* Test 5: Complex atomic exchange on 16-byte struct */
struct large_struct {
    uint64_t a;
    uint64_t b;
};

void test_struct_atomic(void) {
    _Atomic struct large_struct atomic_struct = {0, 0};
    struct large_struct new_val = {0x12345678, 0x9ABCDEF0};
    
    for (int i = 0; i < 10000; i++) {
        struct large_struct old = __atomic_exchange_n(&atomic_struct, new_val, __ATOMIC_SEQ_CST);
        sink += old.a + old.b;
        
        new_val.a = (new_val.a << 1) | (new_val.a >> 63);
        new_val.b = (new_val.b << 2) | (new_val.b >> 62);
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    test_atomic_cmpxchg();
    printf("Atomic CMPXCHG test complete, sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Multi-operand asm test complete, sink = %lu\n", (unsigned long)sink);
    
    test_vector_shuffle();
    printf("Vector shuffle test complete, sink = %lu\n", (unsigned long)sink);
    
    test_multi_output_builtin();
    printf("Multi-output builtin test complete, sink = %lu\n", (unsigned long)sink);
    
    test_struct_atomic();
    printf("Struct atomic test complete, sink = %lu\n", (unsigned long)sink);
    
    printf("All tests completed. Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
