/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-exchange on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    for (int i = 0; i < 10000; i++) {
        /* __atomic_compare_exchange_n can generate complex RTL with many operands */
        __atomic_compare_exchange_n(&large_atomic, &old_val, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink ^= (uint64_t)old_val;
        old_val = new_val;
        new_val = new_val + 1;
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 5 outputs + 6 inputs = 11 operands total */
        asm volatile (
            "# Dummy 11-operand instruction\n\t"
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
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex vector operations can generate multi-operand RTL */
        v8si result = __builtin_shuffle(vec_a, vec_b, 
            __builtin_convertvector(mask, v8si));
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink ^= result[j];
        }
        
        vec_a += 1;
        vec_b += 1;
        mask = (v4si){mask[3], mask[0], mask[1], mask[2]};
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Custom multi-output operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "add %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "add %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "add %3, %3, %4"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5), "r" (in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++;
    }
}

/* Test 5: __sync_val_compare_and_swap on 16-byte object */
void test_sync_cas_128bit(void) {
    __int128 shared = 0;
    __int128 old = 0;
    __int128 new_val = 1;
    
    for (int i = 0; i < 10000; i++) {
        /* This builtin often expands to complex RTL */
        __int128 result = __sync_val_compare_and_swap(&shared, old, new_val);
        sink ^= (uint64_t)result;
        
        old = new_val;
        new_val = new_val + 1;
        shared = result + 1;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to maximize coverage chances */
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
