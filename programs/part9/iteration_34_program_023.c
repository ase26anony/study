/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle operation that conceptually requires many operands */
static v4si complex_vector_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern uses multiple source vectors and 
       immediate control values to create a result that requires
       many operands during RTL expansion */
    
    /* Step 1: Create intermediate shuffles with immediate masks */
    v4si t1 = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    v4si t2 = __builtin_shuffle(c, d, (v4si){3, 1, 2, 0});
    
    /* Step 2: Blend with immediate control */
    v4si t3 = __builtin_ia32_pblendw128(t1, t2, 0xAA);  /* 0b10101010 */
    
    /* Step 3: Another shuffle with complex pattern */
    v4si t4 = __builtin_shuffle(t3, (v4si){4, 5, 6, 7}, (v4si){2, 3, 0, 1});
    
    /* Step 4: Use multiple vector builtins with immediate arguments */
    /* This complex expression should require many operands */
    v4si result = __builtin_ia32_paddd128(
        __builtin_ia32_pmaddwd128(
            t1,
            __builtin_shuffle(t2, t4, (v4si){1, 0, 3, 2})
        ),
        __builtin_ia32_pslldi128(t3, 3)
    );
    
    return result;
}

/* Another approach: Complex vector conversion with many arguments */
static v8sf multi_operand_conversion(v8si a, v8si b, v8si c, v8si d) {
    /* Complex conversion pattern that may expand to many operands */
    
    /* Create intermediate vectors with various operations */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = a & b;
    v8si t4 = c | d;
    
    /* Complex shuffle/permute across 4 source vectors */
    /* This pattern conceptually needs to reference many operands */
    v8si shuffled = __builtin_shuffle(
        t1, t2, (v8si){0, 8, 2, 10, 4, 12, 6, 14}
    );
    
    /* Another shuffle mixing all 4 vectors */
    v8si mixed = __builtin_shuffle(
        t3, t4, (v8si){15, 7, 14, 6, 13, 5, 12, 4}
    );
    
    /* Convert to float with potential complex expansion */
    v8sf result = __builtin_convertvector(shuffled + mixed, v8sf);
    
    /* Scale with immediate constant */
    return result * (v8sf){1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f};
}

/* Target function that should trigger 10-operand expansion */
static v4si target_10_operand_expansion(v4si v1, v4si v2, v4si v3, v4si v4) {
    /* Complex expression designed to require exactly 10 operands
       during RTL expansion in optabs.cc */
    
    /* Use __builtin_ia32_pshufd which takes vector + immediate */
    v4si a = __builtin_ia32_pshufd(v1, 0x1B);  /* 0b00011011 = 27 */
    
    /* Multiple shuffle operations with different immediate controls */
    v4si b = __builtin_shuffle(v2, v3, (v4si){0, 4, 2, 6});
    v4si c = __builtin_shuffle(v3, v4, (v4si){1, 5, 3, 7});
    
    /* Complex blend with immediate mask */
    v4si d = __builtin_ia32_pblendw128(a, b, 0xF0);  /* 0b11110000 */
    
    /* Arithmetic with immediate shift count */
    v4si e = __builtin_ia32_psradi128(c, 4);
    
    /* Final complex operation that may expand to 10 operands:
       - 4 source vectors (a, b, d, e)
       - 4 immediate constants (from shuffles/blends)
       - 2 more operands for the final operation */
    v4si result = __builtin_ia32_pmaddwd128(
        __builtin_ia32_paddd128(d, e),
        __builtin_shuffle(a, b, (v4si){2, 3, 0, 1})
    );
    
    /* Additional operation with immediate */
    result = __builtin_ia32_pslldi128(result, 2);
    
    return result;
}

/* Function using atomic builtins with many arguments (conceptual) */
static long complex_atomic_operation(volatile long *ptr, int a, int b, int c, 
                                     int d, int e, int f, int g, int h) {
    /* While atomic builtins don't directly take 10 args, the expansion
       might need many operands for memory ordering, values, etc. */
    
    /* Complex atomic operation chain */
    long old = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    
    /* Complex compare-and-exchange with many parameters in expansion */
    long desired = old + a * b + c * d - e * f + g * h;
    long expected = old;
    
    /* __atomic_compare_exchange expands to many operands:
       - ptr, &expected, desired
       - weak, success_memorder, failure_memorder
       - Plus various temporaries and flags */
    __atomic_compare_exchange(ptr, &expected, &desired, 0,
                             __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    return expected;
}

int main(int argc, char *argv[]) {
    /* Prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with non-constant values to prevent
       compile-time evaluation */
    v4si v1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si v2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si v3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si v4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v8si w1 = {argc, argc+1, argc+2, argc+3, argc+4, argc+5, argc+6, argc+7};
    v8si w2 = {argc*2, argc*3, argc*4, argc*5, argc*6, argc*7, argc*8, argc*9};
    v8si w3 = {argc*10, argc*11, argc*12, argc*13, argc*14, argc*15, argc*16, argc*17};
    v8si w4 = {argc*18, argc*19, argc*20, argc*21, argc*22, argc*23, argc*24, argc*25};
    
    volatile v4si result1 = (v4si){0, 0, 0, 0};
    volatile v8sf result2 = (v8sf){0, 0, 0, 0, 0, 0, 0, 0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that should trigger complex expansions */
        result1 = target_10_operand_expansion(v1, v2, v3, v4);
        result2 = multi_operand_conversion(w1, w2, w3, w4);
        
        /* Mix in another complex operation */
        v1 = complex_vector_shuffle_10op(v1, v2, v3, v4);
        
        /* Modify inputs slightly to prevent loop invariant removal */
        v1[0] += i;
        v2[1] += i;
    }
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += result1[i];
    }
    
    printf("Result sum: %d\n", sum);
    
    /* Atomic operation test */
    volatile long atomic_var = 0;
    long atomic_result = complex_atomic_operation(&atomic_var, 
        argc, argc+1, argc+2, argc+3, argc+4, argc+5, argc+6, argc+7);
    printf("Atomic result: %ld\n", atomic_result);
    
    return sum != 0 ? 0 : 1;
}
