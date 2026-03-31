/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg_base = 100;
static volatile int vol_iterations = 1000;

/* Vector extensions for register pressure */
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
    
    /* Result accumulator */
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    for (i = 0; i < vol_iterations; i++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = arg1 + 5;  /* arg1 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;  /* arg2 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 10];  /* &local_array[arg3 + 10] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg4) + (arg2 << 2);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        result += cand4;
        
        /* Control flow: conditional jump to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense independent arithmetic operations */
            j = arg1 + i;
            k = arg2 + j;
            l = arg3 + k;
            m = arg4 + l;
            n = i * j;
            o = j * k;
            p = k * l;
            q = l * m;
            r = m * n;
            s = n * o;
            t = o * p;
            u = p * q;
            v = q * r;
            w = r * s;
            x = s * t;
            y = t * u;
            z = u * v;
            
            /* Floating point operations */
            f1 = arg1 * 1.1f;
            f2 = arg2 * 2.2f;
            f3 = arg3 * 3.3f;
            f4 = arg4 * 4.4f;
            f5 = f1 + f2;
            f6 = f3 + f4;
            f7 = f5 * f6;
            f8 = f7 / 2.0f;
            f9 = f8 - f1;
            f10 = f9 + f2;
            
            /* Double precision operations */
            d1 = arg1 * 1.111;
            d2 = arg2 * 2.222;
            d3 = arg3 * 3.333;
            d4 = arg4 * 4.444;
            d5 = d1 + d2 + d3 + d4;
            d6 = d5 * 0.5;
            d7 = d6 / 3.14159;
            d8 = d7 - d1;
            
            /* Long long operations */
            ll1 = (long long)arg1 * i;
            ll2 = (long long)arg2 * j;
            ll3 = (long long)arg3 * k;
            ll4 = (long long)arg4 * l;
            ll5 = ll1 + ll2 + ll3 + ll4;
            ll6 = ll5 >> 2;
            
#ifdef __SSE2__
            /* Vector operations for additional register pressure */
            v4si vec1 = {arg1, arg2, arg3, arg4};
            v4si vec2 = {i, j, k, l};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f6, f7, f8};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
#endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This forces compiler to either rematerialize or replace */
            result += cand1 * 2;
            result += cand2 / 2;
            result += (int)(cand3 - local_array);
            result += cand4 << 1;
            
            /* Use some of the high-pressure variables to prevent elimination */
            result += j + k + l + m;
            result += (int)f10;
            result += (int)d8;
            result += (int)ll6;
#ifdef __SSE2__
            result += vec5[0] + vec5[1];
#endif
        }
        
        /* Update array element to make address calculation meaningful */
        local_array[arg3 + 10] = i;
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    vol_iterations = iterations;
    
    /* Call test function multiple times with different args */
    volatile int total = 0;
    for (int i = 0; i < 10; i++) {
        total += test_remat(vol_arg_base + i, 
                           vol_arg_base + i * 2,
                           vol_arg_base + i * 3,
                           vol_arg_base + i * 4);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
