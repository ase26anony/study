/* test_optabs_10_operands.c
 * Program designed to trigger the 10-operand expansion case in optabs.cc
 * Compile with: gcc -O3 -ftree-vectorize -march=native -c test_optabs_10_operands.c
 */

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
    /* This complex shuffle pattern uses multiple operations that may expand
     * to a 10-operand internal function during RTL expansion */
    
    /* Step 1: Create multiple intermediate shuffles */
    v4si shuffle1 = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    v4si shuffle2 = __builtin_shuffle(c, d, (v4si){1, 0, 3, 2});
    
    /* Step 2: Perform bitwise operations with multiple constants */
    v4si masked1 = shuffle1 & (v4si){0xFF00FF00, 0x00FF00FF, 0xFF00FF00, 0x00FF00FF};
    v4si masked2 = shuffle2 | (v4si){0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000};
    
    /* Step 3: Complex blend with multiple control values */
    v4si result = __builtin_shuffle(masked1, masked2, 
                                   (v4si){0, 5, 2, 7});  /* 4 immediate values */
    
    /* Step 4: Additional operations with constants */
    result = result ^ (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    result = result + (v4si){1, 2, 3, 4};  /* 4 more immediate values */
    
    return result;
}

/* Multi-operand vector conversion with many arguments */
static v4sf complex_vector_conversion(v4si a, v4si b, v4si c, v4si d) {
    /* This creates a pattern that may require many operands during expansion */
    
    /* Combine vectors with different operations */
    v4si combined1 = a + b;
    v4si combined2 = c - d;
    
    /* Complex shuffle with immediate control */
    v4si shuffled = __builtin_shuffle(combined1, combined2, 
                                     (v4si){1, 6, 3, 4});  /* 4 immediate values */
    
    /* Convert to float with scaling factors */
    v4sf result = __builtin_convertvector(shuffled, v4sf);
    
    /* Apply multiple constant operations */
    result = result * (v4sf){1.5f, 2.5f, 3.5f, 4.5f};  /* 4 immediate float values */
    result = result + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};  /* 4 more immediate values */
    
    return result;
}

/* Custom 10-operand-like operation using GCC builtins */
static v8si simulate_10_operand_operation(v8si a, v8si b, v8si c, v8si d) {
    /* Attempt to create a pattern that might expand to 10 operands */
    
    /* Multiple shuffle operations with immediate controls */
    v8si shuffle1 = __builtin_shuffle(a, b, 
        (v8si){0, 9, 2, 11, 4, 13, 6, 15});  /* 8 immediate values */
    
    v8si shuffle2 = __builtin_shuffle(c, d,
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});     /* 8 immediate values */
    
    /* Complex blend with mask */
    v8si mask = (v8si){0, -1, 0, -1, 0, -1, 0, -1};  /* 8 immediate values */
    v8si result = (shuffle1 & mask) | (shuffle2 & ~mask);
    
    /* Additional operations with constants */
    result = result + (v8si){1, 2, 3, 4, 5, 6, 7, 8};  /* 8 immediate values */
    
    return result;
}

/* Atomic-like operation simulation with many parameters */
static long complex_atomic_style(long *ptr, int a, int b, int c, int d, 
                                 int e, int f, int g, int h, int i) {
    /* Simulate a complex operation that might require many operands */
    long result = *ptr;
    
    /* Multiple operations that could be combined */
    result += (long)a * b;
    result -= (long)c * d;
    result ^= (long)e << f;
    result |= (long)g << h;
    result &= ~(1L << i);
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    
    /* Initialize vector data */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v8si vec_a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si vec_b8 = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si vec_c8 = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si vec_d8 = {25, 26, 27, 28, 29, 30, 31, 32};
    
    long atomic_var = 1000;
    
    /* Loop to prevent dead code elimination */
    for (int i = 0; i < iterations; i++) {
        /* Execute operations that may trigger 10-operand expansions */
        
        /* Operation 1: Complex vector shuffle (potential 10-operand expansion) */
        v4si result1 = complex_vector_shuffle_10op(vec_a, vec_b, vec_c, vec_d);
        
        /* Operation 2: Vector conversion with many constants */
        v4sf result2 = complex_vector_conversion(vec_a, vec_b, vec_c, vec_d);
        
        /* Operation 3: 8-element vector operation */
        v8si result3 = simulate_10_operand_operation(vec_a8, vec_b8, vec_c8, vec_d8);
        
        /* Operation 4: Atomic-style with many parameters */
        long result4 = complex_atomic_style(&atomic_var, 
                                           i, i+1, i+2, i+3, i+4,
                                           i+5, i+6, i+7, i+8);
        
        /* Modify inputs to prevent constant propagation */
        vec_a += (v4si){1, 1, 1, 1};
        vec_b += (v4si){2, 2, 2, 2};
        atomic_var += result4;
        
        /* Print results to create side effects */
        if (i == iterations - 1) {
            printf("Result1: %d %d %d %d\n", 
                   result1[0], result1[1], result1[2], result1[3]);
            
            printf("Result2: %f %f %f %f\n", 
                   result2[0], result2[1], result2[2], result2[3]);
            
            printf("Result4: %ld\n", result4);
        }
    }
    
    return 0;
}
