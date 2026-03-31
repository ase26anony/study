/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_counter = 0;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    long long ll1, ll2, ll3, ll4, ll5, ll6;
    
    /* Local array for address calculation candidate */
    int local_array[100];
    volatile int result = 0;
    
    /* Initialize some values */
    for (i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (i = 0; i < arg1; i++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile argument */
        int cand1 = arg2 + 5;  /* arg2 + 5 is cheap to recompute */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg3 * 2;  /* arg3 * 2 is cheap to recompute */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4];  /* &local_array[arg4] is recomputable */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 << 2) + arg3;
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* BLOCK B: High register pressure region */
            
            /* Dense sequence of independent arithmetic operations */
            j = i * 2 + 1;
            k = j * 3 - 2;
            l = k + j * 4;
            m = l - k * 2;
            n = m * m + j;
            o = n / (i + 1) + 2;
            p = o << 3;
            q = p ^ 0x55AA55AA;
            r = q * 7;
            s = r % 31;
            t = s | 0xFF00;
            u = t & 0x0F0F0F0F;
            v = u + t;
            w = v - u * 2;
            x = w * w;
            y = x >> 4;
            z = y ^ x;
            
            /* Floating point operations */
            f1 = i * 1.1f;
            f2 = f1 * 2.2f;
            f3 = f2 + f1;
            f4 = f3 - f2 * 0.5f;
            f5 = f4 * f3;
            f6 = f5 / (f4 + 1.0f);
            f7 = f6 + f5;
            f8 = f7 - f6;
            f9 = f8 * f7;
            f10 = f9 / 3.14f;
            
            /* Double precision */
            d1 = i * 1.234567;
            d2 = d1 * 2.345678;
            d3 = d2 + d1;
            d4 = d3 - d2;
            d5 = d4 * d3;
            d6 = d5 / (d4 + 1.0);
            d7 = d6 + d5;
            d8 = d7 - d6;
            
            /* Long long operations */
            ll1 = i * 1000LL;
            ll2 = ll1 * 2000LL;
            ll3 = ll2 + ll1;
            ll4 = ll3 - ll2;
            ll5 = ll4 * ll3;
            ll6 = ll5 / (ll4 + 1LL);
            
#ifdef __SSE2__
            /* Vector operations to consume more registers */
            v4si vec1 = {i, i+1, i+2, i+3};
            v4si vec2 = {arg1, arg2, arg3, arg4};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            v4si vec6 = vec5 << 2;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f6, f7, f8};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
            
            /* Use vectors to prevent elimination */
            int *vp = (int*)&vec6;
            result += vp[0] + vp[1] + vp[2] + vp[3];
#endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after memory clobber */
            j = k + l + m + n + o + p + q + r + s + t;
            f1 = f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
            d1 = d2 + d3 + d4 + d5 + d6 + d7 + d8;
            ll1 = ll2 + ll3 + ll4 + ll5 + ll6;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This should trigger filter_old_remats as the original
               rematerialization may no longer be profitable */
            result += cand1 * 2;
            result += cand2 / 2;
            result += *cand3 + 1;
            result += cand4 - 1;
            
            /* Use all temporary variables to prevent elimination */
            result += j + z;
            result += (int)f1;
            result += (int)d1;
            result += (int)ll1;
        }
        
        /* Alternate path to create more control flow complexity */
        if (global_counter++ % 100 == 0) {
            /* Another use of candidates in different block */
            result -= cand1;
            result -= cand2;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    volatile int result = 0;
    
    /* Read iteration count from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create different volatile arguments for each call */
    volatile int arg1 = 50;
    volatile int arg2 = 23;
    volatile int arg3 = 47;
    volatile int arg4 = 15;
    
    /* Call test function multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly to prevent complete optimization */
        result += test_remat(arg1 + (i % 3), 
                            arg2 + (i % 5),
                            arg3 + (i % 7),
                            arg4 + (i % 11));
        
        /* Modify global volatile to affect control flow */
        global_counter++;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
