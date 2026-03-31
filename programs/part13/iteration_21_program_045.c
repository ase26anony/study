/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm\n\t"
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
}

/* Test 2: 128-bit atomic operations (often expand to many operands) */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t expected = 0;
    int128_t desired = ((int128_t)0x123456789ABCDEFULL << 64) | 0xFEDCBA987654321ULL;
    int128_t result;
    
    /* __atomic_compare_exchange_n often expands to complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result = __atomic_load_n(&large_atomic, __ATOMIC_RELAXED);
    sink += (uint64_t)(result >> 64) + (uint64_t)result;
}
#endif

/* Test 3: Vector shuffle with many operands */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    /* Complex shuffle that may generate many RTL operands */
    v4si shuffled = __builtin_shuffle(v1, v2, mask);
    
    for (int i = 0; i < 4; i++) {
        sink += shuffled[i];
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    unsigned long long a = 0x123456789ABCDEF0ULL;
    unsigned long long b = 0xFEDCBA9876543210ULL;
    unsigned long long c = 0;
    unsigned long long d = 0;
    
    /* __builtin_cpu_supports with many features checks */
    const char *features[] = {
        "avx", "avx2", "avx512f", "bmi", "bmi2",
        "sse", "sse2", "sse3", "ssse3", "sse4.1", "sse4.2"
    };
    
    for (int i = 0; i < 11; i++) {
        if (__builtin_cpu_supports(features[i])) {
            c |= (1ULL << i);
        }
    }
    
    /* Complex bit manipulation that may expand to many RTL operands */
    asm volatile (
        "# multi-output bit ops\n\t"
        "rorx %0, %2, 17\n\t"
        "rorx %1, %3, 23\n\t"
        "pdep %0, %0, %1\n\t"
        "pext %1, %2, %3"
        : "=&r" (c), "=&r" (d)
        : "r" (a), "r" (b), "0" (c), "1" (d)
        : "cc"
    );
    
    sink += c + d;
}

/* Test 5: Custom multi-operand instruction simulation */
void test_custom_multi_operand(void) {
    uint64_t inputs[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint64_t outputs[3] = {0};
    
    /* Simulate a 8-input, 3-output operation = 11 total operands */
    asm volatile (
        "# custom 11-operand operation\n\t"
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "mul %0, %0, %1\n\t"
        "add %2, %2, %0"
        : "=&r" (outputs[0]), "=&r" (outputs[1]), "=&r" (outputs[2])
        : "r" (inputs[0]), "r" (inputs[1]), "r" (inputs[2]),
          "r" (inputs[3]), "r" (inputs[4]), "r" (inputs[5]),
          "r" (inputs[6]), "r" (inputs[7])
        : "cc"
    );
    
    sink += outputs[0] + outputs[1] + outputs[2];
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        
#ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
        
        if (i % 500 == 0) {
            test_vector_shuffle();
        }
        
        if (i % 750 == 0) {
            test_multi_output_builtin();
        }
        
        if (i % 250 == 0) {
            test_custom_multi_operand();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
