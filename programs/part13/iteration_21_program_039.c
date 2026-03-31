/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Large atomic operations on __int128 */
void test_atomic_128bit(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    for (int i = 0; i < 10000; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
    }
}

/* Strategy 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 
     * 5 outputs + 6 inputs = 11 operands total
     * Using early-clobber (&) and specific register constraints
     * to increase complexity
     */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Dummy multi-operand instruction\n\t"
            "add %0, %5, %6\n\t"
            "adc %1, %7, %8\n\t"
            "mul %2, %9, %10\n\t"
            : "=&r"(o0), "=&r"(o1), "=r"(o2), "=r"(o3), "=r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Strategy 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    for (int i = 0; i < 10000; i++) {
        /* Complex vector operation that may expand to many RTL operands */
        v8si result = vec_a + vec_b;
        v4si shuffled = __builtin_shuffle(result, mask);
        
        /* Use the result to prevent elimination */
        sink += shuffled[0] + shuffled[1] + shuffled[2] + shuffled[3];
        
        vec_a += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
        vec_b += (v8si){2, 2, 2, 2, 2, 2, 2, 2};
    }
}

/* Strategy 4: Custom multi-output operation via inline asm */
void test_custom_multi_output(void) {
    uint64_t a, b, c, d, e;
    uint64_t x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7;
    
    /* 
     * Custom "fused" operation: 5 outputs + 7 inputs = 12 operands
     * We'll use 11 by adjusting constraints
     */
    for (int i = 0; i < 10000; i++) {
        asm volatile (
            "# Custom fused operation\n\t"
            "mov %0, %5\n\t"
            "add %1, %6, %7\n\t"
            "sub %2, %8, %9\n\t"
            "mul %3, %10, %11\n\t"
            "and %4, %5, %6\n\t"
            : "=&r"(a), "=&r"(b), "=&r"(c), "=r"(d), "=r"(e)
            : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u), "r"(t)
            : "cc"
        );
        
        sink += a + b + c + d + e;
        x++; y++; z++; w++; v++; u++; t++;
    }
}

/* Strategy 5: Complex builtin with many arguments */
void test_complex_builtin(void) {
    /* __builtin_cpu_supports with multiple features */
    const char *features[] = {
        "avx", "avx2", "sse", "sse2", "sse3", 
        "ssse3", "sse4.1", "sse4.2", "fma", "aes"
    };
    
    for (int i = 0; i < 10000; i++) {
        int supports = 0;
        /* Check multiple features - each may generate RTL */
        for (int j = 0; j < 10; j++) {
            supports += __builtin_cpu_supports(features[j]);
        }
        sink += supports;
    }
}

/* Strategy 6: Memory barrier with many operands */
void test_memory_barrier(void) {
    uint64_t vars[11];
    
    for (int i = 0; i < 11; i++) {
        vars[i] = i + 1;
    }
    
    for (int iter = 0; iter < 10000; iter++) {
        /* Complex memory operation that may use many address operands */
        asm volatile (
            "# Memory barrier with many addresses\n\t"
            "mfence\n\t"
            : "+m"(vars[0]), "+m"(vars[1]), "+m"(vars[2]), 
              "+m"(vars[3]), "+m"(vars[4]), "+m"(vars[5]),
              "+m"(vars[6]), "+m"(vars[7]), "+m"(vars[8]),
              "+m"(vars[9]), "+m"(vars[10])
            :
            : "memory"
        );
        
        /* Use all variables */
        uint64_t sum = 0;
        for (int i = 0; i < 11; i++) {
            sum += vars[i];
            vars[i] += 1;
        }
        sink += sum;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all test strategies */
    test_atomic_128bit();
    printf("Atomic 128-bit test completed\n");
    
    test_multi_operand_asm();
    printf("Multi-operand asm test completed\n");
    
    test_vector_shuffle();
    printf("Vector shuffle test completed\n");
    
    test_custom_multi_output();
    printf("Custom multi-output test completed\n");
    
    test_complex_builtin();
    printf("Complex builtin test completed\n");
    
    test_memory_barrier();
    printf("Memory barrier test completed\n");
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
