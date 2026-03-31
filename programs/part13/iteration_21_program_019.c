/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_128bit(void) {
    static _Atomic __int128 large_atomic = 0;
    __int128 old_val, new_val, expected;
    
    /* Initialize with 128-bit values */
    old_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    new_val = ((__int128)0x0FEDCBA987654321ULL << 64) | 0x123456789ABCDEF0ULL;
    expected = old_val;
    
    for (int i = 0; i < 10000; i++) {
        /* This often expands to complex RTL with many operands */
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        
        /* Modify values slightly each iteration */
        old_val += 1;
        new_val += 2;
        expected = old_val;
    }
    
    sink += (uint64_t)large_atomic;
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0, i1, i2, i3, i4, i5;
    
    /* Initialize with distinct values */
    i0 = 0x1111111111111111ULL;
    i1 = 0x2222222222222222ULL;
    i2 = 0x3333333333333333ULL;
    i3 = 0x4444444444444444ULL;
    i4 = 0x5555555555555555ULL;
    i5 = 0x6666666666666666ULL;
    
    for (int i = 0; i < 10000; i++) {
        /* 11 operands total: 5 outputs + 6 inputs */
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "mov %0, %5\n\t"
            "mov %1, %6\n\t"
            "mov %2, %7\n\t"
            "mov %3, %8\n\t"
            "mov %4, %9"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : /* No clobbers to keep operand count at 11 */
        );
        
        /* Use results to prevent elimination */
        sink += o0 + o1 + o2 + o3 + o4;
        
        /* Modify inputs */
        i0 += 1; i1 += 2; i2 += 3; i3 += 4; i4 += 5; i5 += 6;
    }
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may expand to many RTL operands */
        v4si result = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Use result */
        for (int j = 0; j < 4; j++) {
            sink += result[j];
        }
        
        /* Modify vectors */
        vec_a += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
        vec_b += (v8si){2, 2, 2, 2, 2, 2, 2, 2};
        mask = (v4si){mask[3], mask[0], mask[1], mask[2]};
    }
}

/* Test 4: Multi-output custom instruction simulation */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0, in1, in2, in3, in4, in5, in6;
    
    in0 = 0xAAAAAAAAAAAAAAAAULL;
    in1 = 0xBBBBBBBBBBBBBBBBULL;
    in2 = 0xCCCCCCCCCCCCCCCCULL;
    in3 = 0xDDDDDDDDDDDDDDDDULL;
    in4 = 0xEEEEEEEEEEEEEEEEULL;
    in5 = 0xFFFFFFFFFFFFFFFFULL;
    in6 = 0x9999999999999999ULL;
    
    for (int i = 0; i < 10000; i++) {
        /* Simulate a 4-output, 7-input instruction (11 total operands) */
        asm volatile (
            "# Custom fused operation\n\t"
            "add %0, %4, %5\n\t"
            "sub %1, %6, %7\n\t"
            "and %2, %8, %9\n\t"
            "orr %3, %10, %4"
            : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
            : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
            : "cc"
        );
        
        sink += out0 ^ out1 ^ out2 ^ out3;
        
        /* Rotate inputs */
        uint64_t tmp = in0;
        in0 = in1; in1 = in2; in2 = in3; in3 = in4;
        in4 = in5; in5 = in6; in6 = tmp;
    }
}

/* Test 5: __sync_val_compare_and_swap on __int128 */
void test_sync_cas_128bit(void) {
    __int128 shared = 0;
    __int128 old_val = 0;
    __int128 new_val;
    
    for (int i = 0; i < 10000; i++) {
        new_val = old_val + 1;
        
        /* __sync_val_compare_and_swap often generates complex RTL */
        __int128 result = __sync_val_compare_and_swap(&shared, old_val, new_val);
        
        sink += (uint64_t)result;
        old_val = new_val;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to increase coverage chances */
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
