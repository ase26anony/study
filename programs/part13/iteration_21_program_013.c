/* Test program to trigger 11-operand RTL generation in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>

/* Global sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap (likely to generate many operands) */
void test_atomic_cmpxchg_128(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* This builtin often expands to complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7;
    
    for (int i = 0; i < 10000; i++) {
        /* 11 operands: 5 outputs + 6 inputs = 11 total */
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

/* Test 3: Vector operations with complex shuffling */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  /* Interleave elements */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may expand to many RTL operands */
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify inputs slightly */
        a[0]++; b[0]--;
    }
}

/* Test 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7, in7 = 8;
    
    for (int i = 0; i < 10000; i++) {
        /* 11 operands: 4 outputs + 7 inputs = 11 total */
        asm volatile (
            "# Custom fused operation\n\t"
            "mul %0, %4, %5\n\t"
            "mul %1, %5, %6\n\t"
            "mul %2, %6, %7\n\t"
            "mul %3, %7, %8\n\t"
            "add %0, %0, %9\n\t"
            "add %1, %1, %10"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), 
              "r" (in4), "r" (in5), "r" (in6)
            : "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        in0++; in1++; in2++; in3++; in4++; in5++; in6++; in7++;
    }
}

/* Test 5: Mixed operations that might combine into multi-operand RTL */
void test_mixed_operations(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    for (int iter = 0; iter < 10000; iter++) {
        /* Complex expression that might generate many operands during RTL expansion */
        uint64_t result = (
            (a * b) + (c * d) + (e * f) + (g * h) + (i * j) + 
            ((a + b) << 2) + ((c + d) << 3) + ((e + f) << 4) +
            __builtin_popcountll(g) + __builtin_clzll(h) + __builtin_ctzll(i)
        );
        
        sink += result;
        
        /* Rotate values to create varying patterns */
        uint64_t tmp = a;
        a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = i; i = j; j = k; k = tmp;
    }
}

int main(void) {
    printf("Starting 11-operand RTL generation tests...\n");
    
    /* Run all tests to increase chances of hitting the uncovered code */
    test_atomic_cmpxchg_128();
    printf("Test 1 complete\n");
    
    test_multi_operand_asm();
    printf("Test 2 complete\n");
    
    test_vector_shuffle();
    printf("Test 3 complete\n");
    
    test_custom_multi_output();
    printf("Test 4 complete\n");
    
    test_mixed_operations();
    printf("Test 5 complete\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
