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
    /* Inline asm to clobber specific x86 registers */
    asm volatile (
        "# Clobbering eax, ecx, edx\n"
        :
        : 
        : "eax", "ecx", "edx", "memory"
    );
    
    /* Opaque memory operations */
    if (p1) *p1 ^= 0x1234;
    if (p2) *p2 ^= 0x5678;
    if (p3) *p3 ^= 0x9ABC;
    if (p4) *p4 ^= 0xDEF0;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    /* Clobber x87/SSE registers on x86 */
    asm volatile (
        "# Clobbering xmm0, xmm1\n"
        :
        :
        : "xmm0", "xmm1", "memory"
    );
    
    if (f1) *f1 = *f1 * 2.0f;
    if (f2) *f2 = *f2 * 0.5f;
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x5555;
    int v4 = seed - 100;
    int v5 = seed * seed;
    int v6 = seed | 0xAAAA;
    int v7 = seed & 0x3333;
    int v8 = seed << 3;
    int v9 = seed >> 2;
    int v10 = ~seed;
    
    /* More variables for even more pressure */
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 - v8;
    int v15 = v9 & v10;
    int v16 = v11 | v12;
    int v17 = v13 << 1;
    int v18 = v14 >> 2;
    int v19 = v15 * v16;
    int v20 = v17 ^ v18;
    
    /* Use volatile read to prevent optimization */
    volatile int vol = global_seed;
    v1 += vol;
    v2 -= vol;
    
    /* Complex computation that keeps variables live */
    for (int i = 0; i < 3; i++) {
        v1 = v1 * v2 + v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 - v6 * v7;
        v4 = v4 + v8 ^ v9;
        v5 = v5 & v10 | v11;
    }
    
    /* Call that clobbers registers - variables v1-v20 are live across this call */
    clobber_callee(&v1, &v2, &v3, &v4);
    
    /* Use all variables after call to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return result;
}

/* Alternative path with less pressure */
int low_pressure_path(int seed) {
    int a = seed * 2;
    int b = seed + 5;
    return a + b;
}

/* Function with floating point pressure */
float float_pressure_path(float seed) {
    /* Many float variables */
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    float f3 = seed * 3.3f;
    float f4 = seed * 4.4f;
    float f5 = seed * 5.5f;
    float f6 = seed * 6.6f;
    float f7 = seed * 7.7f;
    float f8 = seed * 8.8f;
    float f9 = seed * 9.9f;
    float f10 = seed * 10.1f;
    
    /* Complex float computations */
    f1 = f1 * f2 + f3;
    f2 = f2 / f4 - f5;
    f3 = f3 * f6 + f7;
    f4 = f4 / f8 - f9;
    
    /* Call that clobbers float registers */
    clobber_callee2(&f1, &f2);
    
    /* Use results */
    return f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
}

/* Main function with multiple call sites and complex control flow */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc as seed for deterministic but varying behavior */
    int seed = argc;
    
    /* Multiple conditional paths to create different basic blocks */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary condition to hit different paths */
        if ((seed + iteration) % 3 == 0) {
            /* Path 1: High integer register pressure */
            total += high_pressure_path(seed + iteration);
        } 
        else if ((seed + iteration) % 3 == 1) {
            /* Path 2: High float register pressure */
            total += (int)float_pressure_path((float)(seed + iteration));
        }
        else {
            /* Path 3: Low pressure path */
            total += low_pressure_path(seed + iteration);
        }
        
        /* Nested condition to create more complex CFG */
        if (iteration % 2 == 0) {
            /* Another call site with moderate pressure */
            int a = iteration * 2;
            int b = iteration + 1;
            int c = a ^ b;
            int d = a & b;
            int e = a | b;
            
            /* Call with some variables live */
            clobber_callee(&a, &b, NULL, NULL);
            
            total += a + b + c + d + e;
        }
        
        /* Update volatile global to prevent optimizations */
        global_seed ^= iteration;
    }
    
    /* Switch statement for additional control flow complexity */
    switch (total % 4) {
        case 0: {
            /* Case with local variables and call */
            int x1 = total * 2;
            int x2 = total + 7;
            int x3 = total ^ 0xFF;
            clobber_callee(&x1, &x2, &x3, NULL);
            total = x1 + x2 + x3;
            break;
        }
        case 1: {
            /* Different case */
            total = low_pressure_path(total);
            break;
        }
        case 2: {
            /* Yet another case with pressure */
            total = high_pressure_path(total);
            break;
        }
        default: {
            /* Default with mixed operations */
            float f = float_pressure_path((float)total);
            total += (int)f;
            break;
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
