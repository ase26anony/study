/* Test program to trigger 11-operand RTL generation in GCC's optabs */
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
    
    /* This should generate complex RTL with many operands */
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
    asm volatile (
        "# Dummy multi-operand instruction\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "add %3, %5, %7\n\t"
        "add %4, %6, %8"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 3: Vector shuffle with complex mask */
typedef int v8si __attribute__((vector_size(32)));
typedef int v4si __attribute__((vector_size(16)));

void test_vector_shuffle(void) {
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  // Interleave elements
    
    /* Complex vector operation that may expand to many RTL operands */
    v8si result = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    
    /* Use result to prevent elimination */
    for (int i = 0; i < 8; i++) {
        sink += result[i];
    }
}

/* Test 4: Custom multi-output SIMD-like operation via inline asm */
void test_custom_simd_operation(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0 = 0x1111, in1 = 0x2222, in2 = 0x3333, in3 = 0x4444;
    uint64_t in4 = 0x5555, in5 = 0x6666, in6 = 0x7777, in7 = 0x8888;
    
    /* 4 outputs + 8 inputs = 12 operands (more than 11) */
    asm volatile (
        "# Custom SIMD operation\n\t"
        "mov %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %2, %9\n\t"
        "mov %3, %10\n\t"
        "add %3, %3, %11"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6), "r"(in7)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Test 5: __sync_val_compare_and_swap on __int128 */
void test_sync_cas_128bit(void) {
    __int128 shared = 0;
    __int128 old_val = 0;
    __int128 new_val = 1;
    __int128 result;
    
    /* This builtin often expands to complex RTL */
    result = __sync_val_compare_and_swap(&shared, old_val, new_val);
    
    sink += (uint64_t)result;
}

/* Test 6: Multiple atomic operations in sequence */
void test_multi_atomic_ops(void) {
    _Atomic uint64_t atoms[6];
    for (int i = 0; i < 6; i++) {
        atoms[i] = i;
    }
    
    /* Chain of atomic operations that might be combined */
    for (int i = 0; i < 100; i++) {
        uint64_t val0 = __atomic_fetch_add(&atoms[0], 1, __ATOMIC_SEQ_CST);
        uint64_t val1 = __atomic_fetch_add(&atoms[1], val0, __ATOMIC_SEQ_CST);
        uint64_t val2 = __atomic_fetch_add(&atoms[2], val1, __ATOMIC_SEQ_CST);
        uint64_t val3 = __atomic_fetch_add(&atoms[3], val2, __ATOMIC_SEQ_CST);
        uint64_t val4 = __atomic_fetch_add(&atoms[4], val3, __ATOMIC_SEQ_CST);
        uint64_t val5 = __atomic_fetch_add(&atoms[5], val4, __ATOMIC_SEQ_CST);
        
        sink += val0 + val1 + val2 + val3 + val4 + val5;
    }
}

int main(void) {
    printf("Starting 11-operand RTL test...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_atomic_cmpxchg();
        test_multi_operand_asm();
        test_vector_shuffle();
        test_custom_simd_operation();
        test_sync_cas_128bit();
        test_multi_atomic_ops();
        
        printf("Iteration %d, sink = %lu\n", iteration, (unsigned long)sink);
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
