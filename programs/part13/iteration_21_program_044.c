/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 old_val = 0;
    
    /* __atomic_compare_exchange_n can generate many operands */
    for (int i = 0; i < 10000; i++) {
        __int128 expected = old_val;
        __int128 desired = new_val + i;
        
        /* This builtin often expands to complex RTL with many operands */
        if (__atomic_compare_exchange_n(&large_atomic, &expected, desired,
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            old_val = desired;
        }
        
        /* Mix in __atomic_exchange for variety */
        __int128 tmp = __atomic_exchange_n(&large_atomic, new_val - i, __ATOMIC_ACQ_REL);
        sink += (uint64_t)(tmp >> 64) + (uint64_t)tmp;
    }
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 11 operands total: 5 outputs + 6 inputs = 11 */
        asm volatile (
            "# 11-operand asm\n\t"
            "add %0, %5, %6\n\t"
            "adc %1, %7, %8\n\t"
            "mul %2, %9, %10\n\t"
            "add %3, %0, %1\n\t"
            "sub %4, %2, %3"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=r"(o3), "=r"(o4)
            : "r"(i0 + i), "r"(i1 + i), "r"(i2 + i), 
              "r"(i3 + i), "r"(i4 + i), "r"(i5 + i)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0 = o0; i1 = o1; i2 = o2; /* Chain dependencies */
    }
}

/* Strategy 3: Vector operations with complex shuffles */
#ifdef __SSE2__
#include <emmintrin.h>
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may generate many RTL operands */
        v8si mask = {i & 7, (i+1) & 7, (i+2) & 7, (i+3) & 7,
                     (i+4) & 7, (i+5) & 7, (i+6) & 7, (i+7) & 7};
        
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        a = result + b;
        b = a + mask;
    }
}
#endif

/* Strategy 4: Custom multi-output "fused" operation */
void test_fused_operation(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7, in7 = 8;
    
    for (int i = 0; i < 10000; i++) {
        /* 11 operands: 4 outputs + 7 inputs = 11 */
        asm volatile (
            "# Fused operation with 11 operands\n\t"
            "mov %0, %4\n\t"
            "add %0, %5\n\t"
            "mov %1, %6\n\t"
            "sub %1, %7\n\t"
            "mov %2, %8\n\t"
            "xor %2, %9\n\t"
            "mov %3, %10\n\t"
            "or %3, %11"
            : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=r"(out3)
            : "r"(in0 + i), "r"(in1 + i), "r"(in2 + i), "r"(in3 + i),
              "r"(in4 + i), "r"(in5 + i), "r"(in6 + i), "r"(in7 + i)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Create dependency chain */
        in0 = out0; in1 = out1; in2 = out2; in3 = out3;
        in4 = out0 ^ out1; in5 = out2 ^ out3;
        in6 = out0 + out2; in7 = out1 + out3;
    }
}

/* Strategy 5: __sync builtins with 16-byte types */
void test_sync_builtins(void) {
    __int128 shared = 0;
    __int128 old_val, new_val;
    
    for (int i = 0; i < 10000; i++) {
        old_val = shared;
        new_val = old_val + i;
        
        /* __sync_val_compare_and_swap on __int128 */
        __int128 result = __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += (uint64_t)(result >> 64) + (uint64_t)result;
        
        /* Also test __sync_fetch_and_add if available */
        if (i % 2 == 0) {
            shared = __sync_fetch_and_add(&shared, 1);
        }
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    test_atomic_128bit();
    printf("Atomic 128-bit test completed, sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed, sink = %lu\n", (unsigned long)sink);
    
#ifdef __SSE2__
    test_vector_shuffle();
    printf("Vector shuffle test completed, sink = %lu\n", (unsigned long)sink);
#endif
    
    test_fused_operation();
    printf("Fused operation test completed, sink = %lu\n", (unsigned long)sink);
    
    test_sync_builtins();
    printf("Sync builtins test completed, sink = %lu\n", (unsigned long)sink);
    
    printf("All tests completed. Final sink = %lu\n", (unsigned long)sink);
    return 0;
}
