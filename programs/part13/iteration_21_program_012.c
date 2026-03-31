/* test_11_operand.c - Test program to trigger 11-operand RTL generation */

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
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 1000; i++) {
        /* 5 outputs + 6 inputs = 11 operands total */
        asm volatile (
            "# Dummy 11-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "mul %3, %0, %1\n\t"
            "add %4, %2, %3"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle mask using all elements */
    int mask[16] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
    
    for (int i = 0; i < 1000; i++) {
        /* This may expand to many RTL operands */
        v8si result = __builtin_shufflevector(a, b, 
            mask[0], mask[1], mask[2], mask[3], 
            mask[4], mask[5], mask[6], mask[7]);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs */
        for (int j = 0; j < 8; j++) {
            a[j] += 1;
            b[j] += 2;
        }
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3, out4;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, 
             in5 = 6, in6 = 7, in7 = 8, in8 = 9;
    
    for (int i = 0; i < 1000; i++) {
        /* 5 outputs + 9 inputs = 14 operands (more than 11) */
        asm volatile (
            "# Custom multi-output operation\n\t"
            "mov %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "mov %1, %7\n\t"
            "add %1, %1, %8\n\t"
            "mov %2, %9\n\t"
            "add %2, %2, %10\n\t"
            "mov %3, %11\n\t"
            "add %3, %3, %12\n\t"
            "mov %4, %13\n\t"
            "add %4, %4, %14"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), 
              "=&r" (out3), "=&r" (out4)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5), "r" (in6), "r" (in7), "r" (in8)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3 + out4;
        
        /* Rotate inputs */
        uint64_t temp = in8;
        in8 = in7; in7 = in6; in6 = in5; in5 = in4;
        in4 = in3; in3 = in2; in2 = in1; in1 = in0;
        in0 = temp;
    }
}

/* Test 5: __sync_val_compare_and_swap on __int128 */
void test_sync_cas_128bit(void) {
    __int128 shared = 0;
    __int128 old_val, new_val;
    
    for (int i = 0; i < 1000; i++) {
        old_val = shared;
        new_val = old_val + (((__int128)i << 64) | i);
        
        /* This builtin often generates complex RTL */
        __int128 result = __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += (uint64_t)result;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to increase coverage chances */
    test_atomic_128bit();
    printf("  test_atomic_128bit complete\n");
    
    test_multi_operand_asm();
    printf("  test_multi_operand_asm complete\n");
    
    test_vector_shuffle();
    printf("  test_vector_shuffle complete\n");
    
    test_custom_multi_output();
    printf("  test_custom_multi_output complete\n");
    
    test_sync_cas_128bit();
    printf("  test_sync_cas_128bit complete\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
