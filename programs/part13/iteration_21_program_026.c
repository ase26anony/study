/* Test program to trigger 11-operand RTL generation in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val, new_val, expected;
    
    /* Initialize values */
    old_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    new_val = ((__int128)0x0FEDCBA987654321ULL << 64) | 0x123456789ABCDEF0ULL;
    expected = old_val;
    
    /* This built-in often expands to complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        /* Modify values slightly each iteration */
        old_val += i;
        new_val += i;
        expected = old_val;
    }
    
    sink += (uint64_t)large_atomic;
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 
         * 11 operands total:
         * - 5 outputs (o0-o4)
         * - 6 inputs (i0-i5)
         * No clobbers to keep operand count exactly at 11
         */
        asm volatile (
            "# Multi-operand test with 11 operands\n\t"
            "mov %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "mov %1, %7\n\t"
            "add %1, %1, %8\n\t"
            "mov %2, %9\n\t"
            "add %2, %2, %10\n\t"
            "mov %3, %5\n\t"
            "sub %3, %3, %6\n\t"
            "mov %4, %7\n\t"
            "sub %4, %4, %8"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0 + i), "r" (i1 + i), "r" (i2 + i), 
              "r" (i3 + i), "r" (i4 + i), "r" (i5 + i)
            /* No clobbers - total operands = 5 outputs + 6 inputs = 11 */
        );
        
        /* Use all outputs to prevent dead code elimination */
        sink += o0 + o1 + o2 + o3 + o4;
        
        /* Modify inputs */
        i0 += o0; i1 += o1; i2 += o2; i3 += o3; i4 += o4;
    }
}

/* Strategy 3: Vector operations with complex shuffles */
#ifdef __SSE2__
#include <emmintrin.h>
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a, b, mask, result;
    
    /* Initialize vectors */
    for (int i = 0; i < 8; i++) {
        a[i] = i * 2;
        b[i] = i * 3 + 1;
        mask[i] = (i * 7) % 16;  /* Shuffle mask */
    }
    
    for (int i = 0; i < 10000; i++) {
        /* Complex operation that may generate many RTL operands */
        result = __builtin_shuffle(a, b, mask);
        
        /* Modify mask each iteration */
        for (int j = 0; j < 8; j++) {
            mask[j] = (mask[j] + i + j) % 16;
        }
        
        /* Use result */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
    }
}
#endif

/* Strategy 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 
         * 11 operands: 4 outputs + 7 inputs
         * Using early-clobber constraints to force separate registers
         */
        asm volatile (
            "# Custom 4-output, 7-input operation\n\t"
            "lea %0, [%4 + %5]\n\t"
            "lea %1, [%6 + %7]\n\t"
            "lea %2, [%8 + %9]\n\t"
            "lea %3, [%10 + %4]\n\t"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0 + i), "r" (in1 + i), "r" (in2 + i), "r" (in3 + i),
              "r" (in4 + i), "r" (in5 + i), "r" (in6 + i)
            /* Total: 4 + 7 = 11 operands */
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Rotate inputs */
        uint64_t temp = in6;
        in6 = in5; in5 = in4; in4 = in3; in3 = in2; in2 = in1; in1 = in0; in0 = temp;
    }
}

/* Strategy 5: __sync_val_compare_and_swap on 16-byte struct */
struct large_struct {
    uint64_t a;
    uint64_t b;
};

void test_struct_cas(void) {
    volatile struct large_struct shared = {0, 0};
    struct large_struct old_val, new_val;
    
    for (int i = 0; i < 10000; i++) {
        old_val.a = sink + i;
        old_val.b = sink - i;
        new_val.a = old_val.a + 1;
        new_val.b = old_val.b - 1;
        
        /* This may generate complex RTL with many operands */
        __sync_val_compare_and_swap(&shared.a, old_val.a, new_val.a);
        __sync_val_compare_and_swap(&shared.b, old_val.b, new_val.b);
        
        sink += shared.a + shared.b;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all test strategies */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed, sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed, sink = %lu\n", (unsigned long)sink);
    
    test_custom_multi_output();
    printf("Custom multi-output test completed, sink = %lu\n", (unsigned long)sink);
    
    test_struct_cas();
    printf("Struct CAS test completed, sink = %lu\n", (unsigned long)sink);
    
#ifdef __SSE2__
    test_vector_shuffle();
    printf("Vector shuffle test completed, sink = %lu\n", (unsigned long)sink);
#endif
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
