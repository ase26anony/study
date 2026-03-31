/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -march=i386 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    asm volatile("" 
                 : 
                 : "r"(*p1), "r"(*p2), "r"(*p3), "r"(*p4)
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    
    /* Additional memory clobber to prevent reordering */
    *p1 = *p1 + 1;
    *p2 = *p2 + 1;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1, int *i2) {
    asm volatile(""
                 :
                 : "r"(*f1), "r"(*f2), "r"(*i1), "r"(*i2)
                 : "eax", "ebx", "ecx", "edx", "xmm0", "xmm1", "xmm2", "xmm3", "memory");
    *f1 = *f1 + 1.0f;
    *i1 = *i1 + 1;
}

/* Function to create complex control flow */
__attribute__((noinline))
int complex_condition(int seed) {
    return (seed % 7) > 3;
}

int main(int argc, char **argv) {
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    volatile int vol_seed = seed; /* Prevent constant propagation */
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Declare MANY local variables to create register pressure */
        int v1 = vol_seed + iteration * 1;
        int v2 = vol_seed + iteration * 2;
        int v3 = vol_seed + iteration * 3;
        int v4 = vol_seed + iteration * 4;
        int v5 = vol_seed + iteration * 5;
        int v6 = vol_seed + iteration * 6;
        int v7 = vol_seed + iteration * 7;
        int v8 = vol_seed + iteration * 8;
        int v9 = vol_seed + iteration * 9;
        int v10 = vol_seed + iteration * 10;
        int v11 = vol_seed + iteration * 11;
        int v12 = vol_seed + iteration * 12;
        int v13 = vol_seed + iteration * 13;
        int v14 = vol_seed + iteration * 14;
        int v15 = vol_seed + iteration * 15;
        
        /* Float variables to use floating point registers */
        float f1 = (float)v1 * 1.1f;
        float f2 = (float)v2 * 1.2f;
        float f3 = (float)v3 * 1.3f;
        float f4 = (float)v4 * 1.4f;
        
        /* Perform complex computations that cannot be optimized away */
        v1 = v1 * 3 + v2;
        v2 = v2 * 5 + v3;
        v3 = v3 * 7 + v4;
        v4 = v4 * 11 + v5;
        v5 = v5 * 13 + v6;
        v6 = v6 * 17 + v7;
        v7 = v7 * 19 + v8;
        v8 = v8 * 23 + v9;
        v9 = v9 * 29 + v10;
        v10 = v10 * 31 + v11;
        v11 = v11 * 37 + v12;
        v12 = v12 * 41 + v13;
        v13 = v13 * 43 + v14;
        v14 = v14 * 47 + v15;
        
        /* Use volatile global to prevent reordering */
        v15 = v15 * global_seed;
        
        /* Complex floating point computations */
        f1 = f1 * 2.0f + f2;
        f2 = f2 * 3.0f + f3;
        f3 = f3 * 4.0f + f4;
        f4 = f4 * 5.0f + f1;
        
        /* Create conditional branch where one path has high register pressure */
        if (complex_condition(seed + iteration)) {
            /* HIGH REGISTER PRESSURE PATH - call at end of basic block */
            
            /* More computations to increase live ranges */
            int t1 = v1 + v2 + v3;
            int t2 = v4 + v5 + v6;
            int t3 = v7 + v8 + v9;
            int t4 = v10 + v11 + v12;
            
            float ft1 = f1 + f2;
            float ft2 = f3 + f4;
            
            /* Call that clobbers many registers - this should be BB_END */
            clobber_callee(&t1, &t2, &t3, &t4);
            
            /* Additional call with different register types */
            clobber_callee2(&ft1, &ft2, &v13, &v14);
            
            /* Use results after call to keep variables live */
            result += t1 + t2 + t3 + t4 + (int)ft1 + (int)ft2 + v13 + v14 + v15;
        } else {
            /* LOW PRESSURE PATH - simpler computations */
            result += v1 + v2 + v3 + v4;
        }
        
        /* Switch statement to create additional basic blocks */
        switch ((seed + iteration) % 4) {
            case 0: {
                /* Another high pressure block */
                int s1 = v1 * v2;
                int s2 = v3 * v4;
                int s3 = v5 * v6;
                int s4 = v7 * v8;
                
                clobber_callee(&s1, &s2, &s3, &s4);
                result += s1 + s2;
                break;
            }
            case 1:
                result += v9 + v10;
                break;
            case 2: {
                /* Yet another high pressure scenario */
                float fs1 = f1 * f2;
                float fs2 = f3 * f4;
                int is1 = v11 * v12;
                int is2 = v13 * v14;
                
                clobber_callee2(&fs1, &fs2, &is1, &is2);
                result += (int)fs1 + (int)fs2 + is1 + is2;
                break;
            }
            default:
                result += v15;
                break;
        }
        
        /* Mix in some I/O to prevent optimization */
        if (iteration == 1) {
            volatile char c = getchar();
            if (c != EOF) {
                v1 += (int)c;
            }
        }
        
        /* Update seed for next iteration */
        seed = seed * 1103515245 + 12345;
    }
    
    /* Final computation using all accumulated results */
    int final_result = result % 1000;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result == 0 ? 0 : 1;
}
