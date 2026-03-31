/* test_11_operand.c */
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
    
    /* 5 outputs + 6 inputs = 11 operands total */
    for (int i = 0; i < 1000; i++) {
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "mul %3, %5, %6\n\t"
            "mul %4, %7, %8"
            : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=r" (o3), "=r" (o4)
            : "r" (i0 + i), "r" (i1 + i), "r" (i2 + i), 
              "r" (i3 + i), "r" (i4 + i), "r" (i5 + i)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
    }
}

/* Test 3: Vector shuffle with complex mask */
typedef int v8si __attribute__((vector_size(32)));
typedef int v4si __attribute__((vector_size(16)));

void test_vector_shuffle(void) {
    v8si vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    for (int i = 0; i < 1000; i++) {
        /* Complex shuffle that may expand to many RTL operands */
        v8si shuffled = __builtin_shufflevector(vec_a, vec_b, 
                                               0, 2, 4, 6, 8, 10, 12, 14);
        
        /* Use all elements to prevent optimization */
        for (int j = 0; j < 8; j++) {
            sink += shuffled[j];
        }
        
        /* Modify vectors to prevent constant folding */
        vec_a[0] += i;
        vec_b[7] -= i;
    }
}

/* Test 4: Custom multi-output asm with memory clobber */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6, in6 = 7;
    
    for (int i = 0; i < 1000; i++) {
        /* 4 outputs + 7 inputs = 11 operands */
        asm volatile (
            "# Custom 11-operand instruction\n\t"
            "mov %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "mov %1, %6\n\t"
            "sub %1, %1, %7\n\t"
            "mov %2, %8\n\t"
            "xor %2, %2, %9\n\t"
            "mov %3, %10\n\t"
            "or %3, %3, %11"
            : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3)
            : "r" (in0 + i), "r" (in1), "r" (in2), "r" (in3),
              "r" (in4), "r" (in5), "r" (in6)
            : "memory", "cc"
        );
        
        sink += out0 + out1 + out2 + out3;
        
        /* Modify inputs */
        in0 += out0;
        in1 += out1;
    }
}

/* Test 5: __sync_val_compare_and_swap on 16-byte struct */
struct large_struct {
    uint64_t a;
    uint64_t b;
};

void test_struct_cas(void) {
    struct large_struct atomic_struct = {0, 0};
    struct large_struct old_val = {0, 0};
    struct large_struct new_val = {0x12345678, 0x9ABCDEF0};
    
    for (int i = 0; i < 1000; i++) {
        /* This may generate complex RTL with many operands */
        __sync_val_compare_and_swap(&atomic_struct.a, old_val.a, new_val.a);
        __sync_val_compare_and_swap(&atomic_struct.b, old_val.b, new_val.b);
        
        sink += atomic_struct.a + atomic_struct.b;
        
        /* Modify values */
        new_val.a += i;
        new_val.b -= i;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests multiple times */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_atomic_128bit();
        test_multi_operand_asm();
        test_vector_shuffle();
        test_custom_multi_output();
        test_struct_cas();
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    printf("Test completed.\n");
    
    return 0;
}
