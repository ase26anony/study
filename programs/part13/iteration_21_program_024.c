/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    
    /* 5 outputs + 7 inputs = 12 total operands in constraints */
    /* The asm template itself has 11 operand references (%0-%10) */
    for (int i = 0; i < 1000; i++) {
        asm volatile (
            "# 11-operand dummy instruction\n\t"
            "add %0, %1, %2\n\t"
            "adc %3, %4, %5\n\t"
            "mul %6, %7, %8\n\t"
            "add %9, %10, %0"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5), "r" (i6)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++; i6++;
    }
}

/* Test 3: Vector shuffle with many operands */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  /* Interleave elements */
    
    for (int i = 0; i < 1000; i++) {
        /* __builtin_shufflevector can generate complex RTL */
        v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs slightly */
        a[0]++; b[0]++;
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    unsigned long long a = 1, b = 2, c = 3, d = 4;
    unsigned long long carry = 0;
    
    /* Simulate a multi-output arithmetic operation */
    for (int i = 0; i < 1000; i++) {
        /* Use inline asm to simulate 4 outputs + 4 inputs + clobber = 9+ */
        /* Add more operands to reach 11 */
        asm volatile (
            "# Multi-output operation\n\t"
            "umulh %0, %4, %5\n\t"    /* high part */
            "mul   %1, %4, %5\n\t"    /* low part */
            "adds  %2, %6, %7\n\t"    /* sum with flags */
            "adc   %3, xzr, xzr"      /* capture carry */
            : "=&r" (a), "=&r" (b), "=&r" (c), "=&r" (carry)
            : "r" (a), "r" (b), "r" (c), "r" (d)
            : "cc"
        );
        
        sink += a + b + c + carry;
        d++;
    }
}

/* Test 5: Complex memory operation with many address components */
void test_complex_addressing(void) {
    uint64_t array[16] = {0};
    uint64_t index1 = 1, index2 = 2, index3 = 3, offset = 4;
    uint64_t scale = 8;
    
    for (int i = 0; i < 1000; i++) {
        /* Complex addressing mode that might expand to multiple operands */
        uint64_t *addr = &array[(index1 + index2 * scale + offset) % 16];
        
        /* Operation with the computed address */
        asm volatile (
            "# Complex addressing\n\t"
            "ldr %0, [%1, %2, lsl #3]\n\t"
            "add %0, %0, %3\n\t"
            "str %0, [%1, %2, lsl #3]"
            : "+&r" (array[0])
            : "r" (array), "r" (index3), "r" (i)
            : "memory"
        );
        
        sink += *addr;
        index1 = (index1 * 3) % 16;
        index2 = (index2 * 5) % 16;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to maximize chance of hitting the uncovered code */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed\n");
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed\n");
    
    test_vector_shuffle();
    printf("Vector shuffle test completed\n");
    
    test_multi_output_builtin();
    printf("Multi-output builtin test completed\n");
    
    test_complex_addressing();
    printf("Complex addressing test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
