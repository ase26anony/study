/* Test program to trigger 11-operand RTL generation in GCC's optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 old_val = 0;
    
    /* __atomic_compare_exchange_n can generate many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &old_val, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)old_val;
        old_val = new_val;
        new_val += 1;
    }
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 5 outputs + 6 inputs = 11 operands total */
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
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

/* Strategy 3: Vector operations with complex shuffles */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

void test_vector_shuffle(void) {
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  // Interleave elements
    
    for (int i = 0; i < 10000; i++) {
        /* __builtin_shufflevector can generate complex RTL */
        v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs */
        a += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
        b += (v8si){2, 2, 2, 2, 2, 2, 2, 2};
    }
}

/* Strategy 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Custom 4-output 7-input operation\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "mul %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "xor %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "sub %3, %3, %4"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5), "r" (in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++;
    }
}

/* Strategy 5: __sync_val_compare_and_swap on 16-byte object */
void test_sync_cas_128bit(void) {
    typedef struct { uint64_t a; uint64_t b; } uint128_t;
    volatile uint128_t shared = {0, 0};
    
    for (int i = 0; i < 10000; i++) {
        uint128_t old_val = shared;
        uint128_t new_val = {old_val.a + 1, old_val.b + 2};
        
        /* This may generate library call with many operands */
        __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += shared.a + shared.b;
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
