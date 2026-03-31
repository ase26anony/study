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
        "mov %0, %5\n\t"
        "mov %1, %6\n\t"
        "mov %2, %7\n\t"
        "mov %3, %8\n\t"
        "mov %4, %9"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit values */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t old_val = 0;
    int128_t new_val = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t expected = 0;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)large_atomic;
}
#endif

/* Test 3: Vector operations with many elements */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    /* Complex shuffle operation that may expand to many RTL operands */
    v4si result = __builtin_shuffle(v1, v2, mask);
    
    for (int i = 0; i < 4; i++) {
        sink += result[i];
    }
}

/* Test 4: Builtin with many arguments */
void test_multi_arg_builtin(void) {
    /* __builtin_cpu_supports with multiple features checked */
    int supports = 0;
    supports += __builtin_cpu_supports("mmx");
    supports += __builtin_cpu_supports("sse");
    supports += __builtin_cpu_supports("sse2");
    supports += __builtin_cpu_supports("sse3");
    supports += __builtin_cpu_supports("ssse3");
    supports += __builtin_cpu_supports("sse4.1");
    supports += __builtin_cpu_supports("sse4.2");
    supports += __builtin_cpu_supports("avx");
    supports += __builtin_cpu_supports("avx2");
    
    sink += supports;
}

/* Test 5: Custom multi-output operation */
void test_custom_multi_output(void) {
    uint64_t a, b, c, d, e;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* 5 outputs + 7 inputs = 12 operands (adjust as needed) */
    asm volatile (
        "# Custom multi-output\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "add %3, %11, %5\n\t"
        "add %4, %6, %7"
        : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e)
        : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u), "r"(t)
        : "cc"
    );
    
    sink += a + b + c + d + e;
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
        
        if (i % 10000 == 0) {
            test_multi_arg_builtin();
            test_custom_multi_output();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
