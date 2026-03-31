/* Test program to trigger 11-operand RTL generation in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
        sink += (uint64_t)expected;
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
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
            "add %3, %5, %7\n\t"
            "add %4, %6, %8"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3), "=&r" (o4)
            : "r" (i0 + i), "r" (i1 + i), "r" (i2 + i), 
              "r" (i3 + i), "r" (i4 + i), "r" (i5 + i)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
    }
}

/* Test 3: Vector shuffle with complex mask */
#ifdef __SSE2__
#include <xmmintrin.h>
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  // Interleave elements
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may generate multi-operand RTL */
        v8si result = __builtin_shufflevector(a, b, 
            0, 8, 1, 9, 2, 10, 3, 11);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs slightly */
        a[0] += i;
        b[7] -= i;
    }
}
#endif

/* Test 4: Custom multi-output fused operation via inline asm */
void test_fused_operation(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, 
             in5 = 6, in6 = 7, in7 = 8;
    
    for (int i = 0; i < 10000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Fused multiply-add-accumulate\n\t"
            "mov %0, %4\n\t"
            "imul %0, %5\n\t"
            "add %0, %6\n\t"
            "mov %1, %7\n\t"
            "imul %1, %8\n\t"
            "add %1, %9\n\t"
            "mov %2, %10\n\t"
            "add %2, %4\n\t"
            "mov %3, %5\n\t"
            "sub %3, %7"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0 + i), "r" (in1 + i), "r" (in2 + i), "r" (in3 + i),
              "r" (in4 + i), "r" (in5 + i), "r" (in6 + i)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Rotate inputs */
        uint64_t tmp = in7;
        in7 = in6; in6 = in5; in5 = in4; in4 = in3;
        in3 = in2; in2 = in1; in1 = in0; in0 = tmp + i;
    }
}

/* Test 5: __sync_val_compare_and_swap on __int128 */
void test_sync_cas(void) {
    __int128 shared = 0;
    __int128 old_val, new_val;
    
    for (int i = 0; i < 10000; i++) {
        old_val = shared;
        new_val = old_val + 1;
        
        /* __sync_val_compare_and_swap often expands to complex RTL */
        __int128 result = __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += (uint64_t)result;
        
        /* Occasionally reset */
        if (i % 1000 == 0) {
            shared = 0;
        }
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to maximize coverage chances */
    test_atomic_cmpxchg();
    printf("Atomic CMPXCHG test complete, sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Multi-operand ASM test complete, sink = %lu\n", (unsigned long)sink);
    
    #ifdef __SSE2__
    test_vector_shuffle();
    printf("Vector shuffle test complete, sink = %lu\n", (unsigned long)sink);
    #endif
    
    test_fused_operation();
    printf("Fused operation test complete, sink = %lu\n", (unsigned long)sink);
    
    test_sync_cas();
    printf("Sync CAS test complete, sink = %lu\n", (unsigned long)sink);
    
    printf("All tests completed. Final sink value: %lu\n", (unsigned long)sink);
    
    return 0;
}
