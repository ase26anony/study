/* test_11_operands.c */
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This often expands to complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
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

/* Test 3: Vector shuffle with many operands */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

void test_vector_shuffle(void) {
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11}; /* Interleave elements */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may expand to many RTL operands */
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs slightly */
        a[0]++; b[7]--;
    }
}

/* Test 4: Multiple built-in functions combined */
void test_builtin_combination(void) {
    unsigned long long x = 0x123456789ABCDEF0ULL;
    unsigned long long y = 0xFEDCBA9876543210ULL;
    unsigned long long z = 0;
    
    for (int i = 0; i < 10000; i++) {
        /* Chain multiple builtins that may create complex RTL */
        z = __builtin_bswap64(x);
        z = __builtin_rotateleft64(z, 13);
        z = __sync_fetch_and_add(&z, y);
        z = __builtin_clzll(z) | (__builtin_ctzll(y) << 32);
        
        sink += z;
        x = (x << 1) | (x >> 63); /* Rotate */
        y = (y >> 1) | (y << 63);
    }
}

/* Test 5: Custom multi-output asm simulating a fused operation */
void test_fused_operation(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Simulated fused multiply-add-accumulate\n\t"
            "mov %0, %4\n\t"
            "mov %1, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %1, %1, %7\n\t"
            "mul %2, %0, %1\n\t"
            "add %2, %2, %8\n\t"
            "mul %3, %2, %9\n\t"
            "add %3, %3, %10"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5), "r" (in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++;
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
    
    test_builtin_combination();
    printf("Built-in combination test completed\n");
    
    test_fused_operation();
    printf("Fused operation test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
