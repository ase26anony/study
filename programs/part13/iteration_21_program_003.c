/* Test program to trigger 11-operand RTL generation in GCC's optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n can generate many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
        expected = large_atomic;
        new_val += 1;
    }
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7;
    
    /* 5 outputs + 7 inputs = 12 total operands in constraints */
    /* Adjust to get exactly 11 by removing one input */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %6, %7\n\t"
            "add %2, %7, %8\n\t"
            "add %3, %8, %9\n\t"
            "add %4, %9, %10"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++; i6++;
    }
}

/* Strategy 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex vector operation that might expand to many RTL operands */
        v8si result = __builtin_shuffle(a, b, 
            __builtin_shufflevector(mask, mask, 3, 2, 1, 0, 3, 2, 1, 0));
        
        /* Use result to prevent elimination */
        sink += result[0] + result[4];
        a += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
    }
}

/* Strategy 4: Custom multi-output asm with exactly 11 constraint operands */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5;
    uint64_t in5 = 6, in6 = 7, in7 = 8;
    
    /* 3 outputs + 8 inputs = 11 total constraint operands */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Custom 11-operand instruction\n\t"
            "mov %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "mov %1, %5\n\t"
            "add %1, %1, %6\n\t"
            "mov %2, %7\n\t"
            "add %2, %2, %8"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5)
            : "cc"
        );
        
        sink += out0 + out1 + out2;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++; in7++;
    }
}

/* Strategy 5: __sync_val_compare_and_swap on __int128 */
void test_sync_cas_128bit(void) {
    __int128 shared = 0;
    __int128 old_val, new_val, result;
    
    for (int i = 0; i < 10000; i++) {
        old_val = shared;
        new_val = old_val + 1;
        
        /* __sync_val_compare_and_swap expands to complex RTL */
        result = __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += (uint64_t)result;
        
        /* Also test __sync_bool_compare_and_swap */
        if (__sync_bool_compare_and_swap(&shared, new_val, new_val + 1)) {
            sink += 1;
        }
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all test strategies */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed\n");
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed\n");
    
    test_vector_shuffle();
    printf("Vector shuffle test completed\n");
    
    test_custom_multi_output();
    printf("Custom multi-output test completed\n");
    
    test_sync_cas_128bit();
    printf("Sync CAS 128-bit test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
