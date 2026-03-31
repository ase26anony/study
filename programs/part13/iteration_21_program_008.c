/* Test program to trigger 11-operand RTL generation in GCC's optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    static _Atomic __int128 large_atomic = 0;
    __int128 old_val, new_val, expected;
    
    /* Initialize with 64-bit parts to avoid constant folding */
    old_val = ((__int128)rand() << 64) | rand();
    new_val = ((__int128)rand() << 64) | rand();
    expected = old_val;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Use the result to prevent dead code elimination */
    sink ^= (uint64_t)expected ^ (uint64_t)(expected >> 64);
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0, i1, i2, i3, i4, i5;
    
    /* Initialize with non-constant values */
    i0 = rand(); i1 = rand(); i2 = rand(); 
    i3 = rand(); i4 = rand(); i5 = rand();
    
    /* 11 operands: 5 outputs + 6 inputs = 11 total */
    asm volatile (
        "# Multi-operand asm with 11 operands\n\t"
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %5\n\t"
        "sub %3, %6\n\t"
        "mov %4, %7\n\t"
        "sub %4, %8"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)  /* 5 outputs */
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)   /* 6 inputs */
        : "cc"  /* clobbers */
    );
    
    /* Use all outputs */
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Strategy 3: Vector operations with complex shuffles */
#ifdef __SSE2__
#include <emmintrin.h>
void test_vector_shuffle(void) {
    /* Use SSE2 intrinsics which may generate multi-operand RTL */
    __m128i a = _mm_set_epi32(rand(), rand(), rand(), rand());
    __m128i b = _mm_set_epi32(rand(), rand(), rand(), rand());
    __m128i mask = _mm_set_epi32(3, 2, 1, 0);
    
    /* Complex operation that might use many operands */
    __m128i result = _mm_add_epi32(a, b);
    result = _mm_shuffle_epi32(result, _MM_SHUFFLE(0, 1, 2, 3));
    
    /* Extract and use results */
    int res_arr[4];
    _mm_storeu_si128((__m128i*)res_arr, result);
    for (int i = 0; i < 4; i++) {
        sink ^= res_arr[i];
    }
}
#endif

/* Strategy 4: Custom multi-output operation using inline asm */
void test_custom_multi_output(void) {
    uint64_t out0, out1, out2, out3;
    uint64_t in0, in1, in2, in3, in4, in5, in6;
    
    in0 = rand(); in1 = rand(); in2 = rand(); in3 = rand();
    in4 = rand(); in5 = rand(); in6 = rand();
    
    /* 11 operands: 4 outputs + 7 inputs = 11 total */
    asm volatile (
        "# Custom 11-operand operation\n\t"
        "lea %0, [%4 + %5]\n\t"
        "lea %1, [%6 + %7]\n\t"
        "lea %2, [%8 + %9]\n\t"
        "lea %3, [%4 + %6 + %8]\n\t"
        "add %0, %10"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in0), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3;
}

/* Strategy 5: Builtin with many arguments */
void test_builtin_many_args(void) {
    /* __builtin_cpu_supports with many features checks */
    int features = 0;
    
    /* Each of these may expand to multiple RTL operands */
    features |= __builtin_cpu_supports("sse");
    features |= __builtin_cpu_supports("sse2") << 1;
    features |= __builtin_cpu_supports("avx") << 2;
    features |= __builtin_cpu_supports("avx2") << 3;
    features |= __builtin_cpu_supports("fma") << 4;
    
    sink ^= features;
}

int main(void) {
    int i;
    
    /* Seed RNG for non-constant values */
    srand(42);
    
    /* Run each test many times to increase coverage probability */
    for (i = 0; i < 100000; i++) {
        test_atomic_128bit();
        test_multi_operand_asm();
        test_custom_multi_output();
        test_builtin_many_args();
        
#ifdef __SSE2__
        test_vector_shuffle();
#endif
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 1000 == 0) {
            sink = sink % 1000;
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
