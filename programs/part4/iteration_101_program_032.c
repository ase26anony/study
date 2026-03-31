/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Use volatile for memory operations to prevent optimization */
static volatile v8si v8si_volatile;
static volatile v4df v4df_volatile;
static volatile v4si v4si_volatile;

/* Compiler barrier */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("avx2")))
v8si test_vector_operations(v8si a, v8si b, v8si c, v8si d, 
                           v4df e, v4df f, v4df g, v4df h,
                           v4si i, v4si j, v4si k, v4si l) {
    /* Operation 1: Complex shuffle with many operands */
    v8si shuffle_result;
    {
        /* Create a non-constant mask for shuffle */
        v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
        mask = mask + a; /* Make mask runtime-dependent */
        
        /* Shuffle that may expand to many operands */
        shuffle_result = __builtin_shuffle(a, b, mask);
        
        /* Force memory store to prevent optimization */
        v8si_volatile = shuffle_result;
        COMPILER_BARRIER();
    }
    
    /* Operation 2: Vector conditional expression with arithmetic */
    v8si cond_result;
    {
        v8si cmp_mask = a > b;
        v8si true_val = (c * d) + shuffle_result;
        v8si false_val = (c - d) * shuffle_result;
        
        /* VEC_COND_EXPR may expand to many operands */
        cond_result = cmp_mask ? true_val : false_val;
        
        v8si_volatile = cond_result;
        COMPILER_BARRIER();
    }
    
    /* Operation 3: Chain of operations that may require many temporaries */
    v8si chain_result;
    {
        /* Complex expression that may be expanded into multiple insns */
        v8si t1 = (a & b) | (c ^ d);
        v8si t2 = (a << 2) + (b >> 1);
        v8si t3 = t1 * t2;
        v8si t4 = cond_result + t3;
        
        /* Another shuffle with runtime mask */
        v8si mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
        mask2 = mask2 + (a & 3); /* Make it runtime-dependent */
        
        chain_result = __builtin_shuffle(t4, t3, mask2);
        
        v8si_volatile = chain_result;
        COMPILER_BARRIER();
    }
    
    /* Operation 4: Mixed-type operations that may require conversions */
    v8si mixed_result;
    {
        /* Convert v4df to v8si through memory/operations */
        v4df temp_df = e * f + g / h;
        v4df_volatile = temp_df;
        COMPILER_BARRIER();
        
        /* Load back and use in integer operations */
        v4df df_loaded = v4df_volatile;
        
        /* Convert through truncation (simulated) */
        v4si converted1 = { (int)df_loaded[0], (int)df_loaded[1], 
                           (int)df_loaded[2], (int)df_loaded[3] };
        
        /* More vector operations on v4si */
        v4si v4si_op = (i * j) + (k - l);
        v4si_volatile = v4si_op;
        COMPILER_BARRIER();
        
        v4si v4si_loaded = v4si_volatile;
        v4si combined_v4si = converted1 + v4si_loaded;
        
        /* Expand to v8si */
        mixed_result = chain_result + (v8si){combined_v4si[0], combined_v4si[1],
                                            combined_v4si[2], combined_v4si[3],
                                            combined_v4si[0], combined_v4si[1],
                                            combined_v4si[2], combined_v4si[3]};
    }
    
    /* Final combination */
    return shuffle_result + cond_result + chain_result + mixed_result;
}

/* Another test function focusing on exactly 10/11 operand patterns */
__attribute__((noinline, target("avx2")))
v4df test_many_operands(v4df a, v4df b, v4df c, v4df d,
                        v4df e, v4df f, v4df g, v4df h,
                        v4df i, v4df j) {
    /* Complex expression that may require many temporaries */
    v4df t1 = a * b + c;
    v4df t2 = d - e * f;
    v4df t3 = g / h + i;
    v4df t4 = j * a - b;
    
    /* Vector conditional with many operands */
    v4df cmp = a > b;
    v4df true_val = (t1 * t2) + (t3 / t4);
    v4df false_val = (t1 + t2) * (t3 - t4);
    
    v4df result = cmp ? true_val : false_val;
    
    /* Force use of all parameters */
    v4df_volatile = a + b + c + d + e + f + g + h + i + j;
    COMPILER_BARRIER();
    
    return result + v4df_volatile;
}

int main() {
    /* Initialize vectors with pattern values */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    v4df e = {1.0, 2.0, 3.0, 4.0};
    v4df f = {5.0, 6.0, 7.0, 8.0};
    v4df g = {9.0, 10.0, 11.0, 12.0};
    v4df h = {13.0, 14.0, 15.0, 16.0};
    
    v4si i = {10, 20, 30, 40};
    v4si j = {50, 60, 70, 80};
    v4si k = {90, 100, 110, 120};
    v4si l = {130, 140, 150, 160};
    
    /* Call test functions */
    v8si result1 = test_vector_operations(a, b, c, d, e, f, g, h, i, j, k, l);
    
    v4df m = {1.5, 2.5, 3.5, 4.5};
    v4df n = {5.5, 6.5, 7.5, 8.5};
    v4df o = {9.5, 10.5, 11.5, 12.5};
    v4df p = {13.5, 14.5, 15.5, 16.5};
    v4df q = {17.5, 18.5, 19.5, 20.5};
    v4df r = {21.5, 22.5, 23.5, 24.5};
    v4df s = {25.5, 26.5, 27.5, 28.5};
    v4df t = {29.5, 30.5, 31.5, 32.5};
    v4df u = {33.5, 34.5, 35.5, 36.5};
    v4df v = {37.5, 38.5, 39.5, 40.5};
    
    v4df result2 = test_many_operands(m, n, o, p, q, r, s, t, u, v);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    for (int idx = 0; idx < 8; idx++) {
        checksum1 += result1[idx];
    }
    
    double checksum2 = 0.0;
    for (int idx = 0; idx < 4; idx++) {
        checksum2 += result2[idx];
    }
    
    /* Print results to ensure execution */
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %f\n", checksum2);
    
    /* Return based on checksums to affect program exit code */
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
}
