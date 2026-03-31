/* test_11_operand_optab.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-exchange on __int128 */
void test_atomic_128bit(void) {
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
        "# Dummy 11-operand instruction\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "mul %3, %5, %7\n\t"
        "sub %4, %9, %6"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=r"(o3), "=r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    /* Complex shuffle that may expand to many RTL operands */
    for (int i = 0; i < 1000; i++) {
        v8si result = __builtin_shufflevector(vec1, vec2, 
            0, 8, 1, 9, 2, 10, 3, 11);
        sink += result[0] + result[4];
        
        /* Modify mask to prevent constant folding */
        mask[0] = (mask[0] + 1) & 0xF;
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t x, y, z, w, v, u;
    
    /* Simulate a 6-input, 5-output operation (11 total) */
    asm volatile (
        "# Multi-output operation\n\t"
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "mul %2, %2, %10\n\t"
        "mov %3, %5\n\t"
        "xor %3, %3, %7\n\t"
        "mov %4, %6\n\t"
        "and %4, %4, %8"
        : "=&r"(x), "=&r"(y), "=&r"(z), "=&r"(w), "=&r"(v)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(sink)
        : "cc"
    );
    
    sink += x + y + z + w + v;
}

/* Test 5: Complex atomic exchange */
void test_complex_atomic_exchange(void) {
    struct LargeStruct {
        uint64_t a, b, c;
    };
    
    _Atomic(struct LargeStruct) atomic_struct = {0};
    struct LargeStruct new_val = {0x12345678, 0x9ABCDEF0, 0x55555555};
    
    /* This may generate many operand bindings */
    for (int i = 0; i < 500; i++) {
        struct LargeStruct old = __atomic_exchange_n(&atomic_struct, 
                                                    new_val, __ATOMIC_SEQ_CST);
        sink += old.a + old.b + old.c;
        new_val.a = (new_val.a * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

int main(void) {
    printf("Testing 11-operand optab coverage...\n");
    
    /* Run all tests multiple times */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_atomic_128bit();
        test_multi_operand_asm();
        test_vector_shuffle();
        test_multi_output_builtin();
        test_complex_atomic_exchange();
        
        printf("Iteration %d, sink = %lu\n", iteration, (unsigned long)sink);
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
