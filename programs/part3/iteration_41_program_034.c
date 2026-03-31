/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_arg4 = 40;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates that become "old remats" */
static volatile int test_remat(volatile int a, volatile int b, volatile int c, volatile int d) {
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Local array for address calculation candidate */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = a + 5;  /* a + 5 is cheap to recompute */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = b * 2 - 3;
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[a % 50];  /* &local_array[i] where i is loop-invariant-ish */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (c << 2) | (d & 0xFF);
        
        /* Immediate use of candidates in Block A */
        result += cand1;
        result += *cand3;
        result += cand2 + cand4;
        
        /* Conditional jump based on volatile to split control flow */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many independent arithmetic operations */
            temp1 = a * b + c;
            temp2 = b * c + d;
            temp3 = c * d + a;
            temp4 = d * a + b;
            temp5 = a + b + c + d;
            temp6 = a - b + c - d;
            temp7 = b - c + d - a;
            temp8 = c - d + a - b;
            temp9 = (a << 1) | (b >> 1);
            temp10 = (b << 2) | (c >> 2);
            temp11 = (c << 3) | (d >> 3);
            temp12 = (d << 4) | (a >> 4);
            temp13 = a ^ b ^ c ^ d;
            temp14 = ~a + ~b;
            temp15 = a * c - b * d;
            
            /* Long operations */
            ltemp1 = (long)a * b * 1000L;
            ltemp2 = (long)b * c * 2000L;
            ltemp3 = (long)c * d * 3000L;
            ltemp4 = (long)d * a * 4000L;
            ltemp5 = ltemp1 + ltemp2 + ltemp3 + ltemp4;
            
            /* Float operations */
            ftemp1 = (float)a / 3.14f;
            ftemp2 = (float)b / 2.71f;
            ftemp3 = ftemp1 * ftemp2;
            ftemp4 = (float)c * 1.414f;
            ftemp5 = ftemp3 + ftemp4;
            
            /* Double operations */
            dtemp1 = (double)a / 3.1415926535;
            dtemp2 = (double)b / 2.7182818284;
            dtemp3 = dtemp1 * dtemp2;
            dtemp4 = (double)c * 1.4142135623;
            dtemp5 = dtemp3 + dtemp4;
            
            /* Vector operations if available */
#ifdef __SSE2__
            v4si vec1 = {a, b, c, d};
            v4si vec2 = {b, c, d, a};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {(float)a, (float)b, (float)c, (float)d};
            v4sf fvec2 = {(float)b, (float)c, (float)d, (float)a};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
            v4sf fvec5 = fvec3 - fvec4;
            
            /* Use vector results */
            int *vptr = (int*)&vec5;
            temp1 += vptr[0];
            temp2 += vptr[1];
            temp3 += vptr[2];
            temp4 += vptr[3];
#endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after clobber */
            temp1 = temp1 * 2 - temp2;
            temp2 = temp2 * 3 - temp3;
            temp3 = temp3 * 4 - temp4;
            temp4 = temp4 * 5 - temp5;
            
            /* Use all temporaries to prevent elimination */
            result += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
            result += temp9 + temp10 + temp11 + temp12 + temp13 + temp14 + temp15;
            result += (int)(ltemp5 % 1000);
            result += (int)(ftemp5 * 100);
            result += (int)(dtemp5 * 100);
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This should trigger filter_old_remats as the original
               rematerialization may no longer be profitable */
            result += cand1 * 2;      /* Use cand1 again */
            result += cand2 - 5;      /* Use cand2 again */
            result += *cand3 + 10;    /* Use cand3 again */
            result += cand4 << 1;     /* Use cand4 again */
            
            /* More uses with different contexts */
            if (cand1 > 0) {
                result += cand2;
            }
            if (cand3 != NULL) {
                result += cand4;
            }
        }
        
        /* Additional control flow to further split live ranges */
        if (vol_cond) {
            /* Another use of candidates */
            result += cand1 + cand2 + *cand3 + cand4;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile long long total = 0;
    
    /* Multiple calls to increase analysis opportunities */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly to prevent complete optimization */
        vol_arg1 = 10 + (i % 5);
        vol_arg2 = 20 + (i % 7);
        vol_arg3 = 30 + (i % 11);
        vol_arg4 = 40 + (i % 13);
        
        total += test_remat(vol_arg1, vol_arg2, vol_arg3, vol_arg4);
        
        /* Alternate condition to affect control flow */
        vol_cond = (i % 3) != 0;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    return 0;
}
