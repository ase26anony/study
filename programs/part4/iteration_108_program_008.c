/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4, int *p5) {
    /* Inline asm to explicitly clobber registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : "memory");
    #endif
    
    /* Do some actual work to prevent removal */
    *p1 = *p2 + *p3;
    *p4 = *p5 ^ 0x1234;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) 
                 : "eax", "ecx", "edx", "st", "st(1)", "st(2)", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    *f1 = *f2 * 2.0f;
    *i1 += (int)(*f1);
}

/* Function to create register pressure with many live variables */
__attribute__((noinline))
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0xABCD;
    int v4 = v1 + v2;
    int v5 = v3 - v1;
    int v6 = v2 * v3;
    int v7 = v4 ^ v5;
    int v8 = v6 + global_seed;  /* Use volatile global */
    int v9 = v7 * v8;
    int v10 = v9 - seed;
    
    /* More variables to increase pressure */
    int v11 = v10 >> 2;
    int v12 = v11 | 0xFF;
    int v13 = v12 & v9;
    int v14 = v13 + v8;
    int v15 = v14 * 3;
    int v16 = v15 ^ v7;
    int v17 = v16 - v6;
    int v18 = v17 + v5;
    int v19 = v18 * v4;
    int v20 = v19 ^ v3;
    
    /* Use getchar to create side effect and prevent moving computations */
    int c = getchar();
    v1 += c;
    
    /* Call function that clobbers registers - many variables are live here */
    clobber_callee(&v1, &v2, &v3, &v4, &v5);
    
    /* Use results after call - keeps variables live across call */
    int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
              v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return sum;
}

/* Alternative path with different register pressure pattern */
__attribute__((noinline))
int medium_pressure_path(int seed) {
    /* Mix of int and float variables for different register classes */
    int i1 = seed;
    int i2 = seed * 3;
    int i3 = i1 ^ i2;
    int i4 = global_seed + i3;
    
    float f1 = (float)seed / 2.0f;
    float f2 = f1 * 3.14f;
    float f3 = f2 + (float)i1;
    float f4 = f3 - f1;
    
    /* Call function that clobbers float and int registers */
    clobber_callee2(&f1, &f2, &i1);
    
    /* Use results */
    return i1 + i2 + i3 + i4 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
}

/* Low pressure path for contrast */
__attribute__((noinline))
int low_pressure_path(int seed) {
    int a = seed;
    int b = a * 2;
    return a + b + global_seed;
}

int main(int argc, char **argv) {
    /* Deterministic but input-dependent seed */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    srand(seed);
    
    int result = 0;
    
    /* Loop to create multiple call sites with varying pressure */
    for (int i = 0; i < 10; i++) {
        /* Varying condition to create different basic blocks */
        int condition = rand() % 100;
        
        if (condition < 40) {
            /* High pressure path - should trigger caller-save */
            result += high_pressure_path(seed + i);
        } 
        else if (condition < 80) {
            /* Medium pressure with mixed register types */
            result += medium_pressure_path(seed - i);
        }
        else {
            /* Low pressure for contrast */
            result += low_pressure_path(i);
        }
        
        /* Additional computation between iterations to create more
           opportunities for register pressure */
        volatile int temp = getchar();
        seed ^= temp;
        global_seed += i;
    }
    
    /* Complex switch statement to create varied control flow */
    switch (result % 5) {
        case 0: {
            /* Another high-pressure block inside switch */
            int x1 = result, x2 = result * 2, x3 = result ^ 0x1234;
            int x4 = x1 + x2, x5 = x3 - x1, x6 = x2 * x3;
            clobber_callee(&x1, &x2, &x3, &x4, &x5);
            result = x1 + x2 + x3 + x4 + x5 + x6;
            break;
        }
        case 1:
        case 2: {
            /* Different pattern */
            float f1 = result / 100.0f, f2 = f1 * 2.0f;
            int i1 = result, i2 = i1 * 3;
            clobber_callee2(&f1, &f2, &i1);
            result = i1 + i2 + (int)f1 + (int)f2;
            break;
        }
        default:
            result ^= 0xDEADBEEF;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
