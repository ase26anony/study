/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);
        
        /* Group 2: More independent operations */
        t5 = t1 * t2;
        t6 = t3 / t4;
        t7 = t1 + t3;
        t8 = t2 - t4;
        
        /* Group 3: Creating data dependencies */
        t9 = t5 * vol_float1;
        t10 = t6 + vol_float2;
        t11 = t7 - vol_float1;
        t12 = t8 / vol_float2;
        
        /* Group 4: Mixed operations with dependencies */
        t13 = t9 + t10;
        t14 = t11 * t12;
        t15 = t13 - t14;
        t16 = t9 * t11;
        
        /* Group 5: Final computations with artificial delays */
        t17 = t15 / (t16 + 0.0001f);  /* Potential division delay */
        t18 = sqrtf(fabsf(t13) + 0.001f); /* Function call creates delay */
        t19 = t14 * t17;
        t20 = t18 + t19;
        
        /* Store results, creating memory pressure */
        c[i] = t20 + t15 + t10;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "mov $0, %%esi\n"
            "mov $0, %%edi\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                            int *restrict arr3, int size) {
    int i;
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    for (i = 0; i < size; i++) {
        /* Long latency chain */
        r1 = arr1[i] * vol_var1;      /* Volatile creates memory barrier */
        r2 = r1 + arr2[i];
        r3 = r2 / (vol_var2 + 1);     /* Division has latency */
        r4 = r3 - arr1[i];
        
        /* Independent chain that can be scheduled in parallel */
        r5 = arr2[i] * 3;
        r6 = arr3[i] + 5;
        r7 = r5 - r6;
        r8 = arr1[i] ^ arr2[i];       /* Bit operation */
        
        /* Create resource conflict with function call */
        r9 = abs(r4 - r7);            /* Function call latency */
        r10 = r8 * r9;
        
        /* Store with volatile to force ordering */
        arr3[i] = r10 + vol_var1;
        
        /* More independent operations to create scheduling candidates */
        arr1[i] = arr1[i] + 1;
        arr2[i] = arr2[i] - 1;
        
        /* Artificial delay through volatile */
        int delay_var = vol_var2;
        arr3[i] = arr3[i] * delay_var;
    }
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
static int control_flow_test(int x) {
    int result = 0;
    
    /* Complex control flow creates different instruction priorities */
    if (x < 100) {
        result = x * 2;
        /* Inline assembly to prevent optimization */
        __asm__ volatile ("nop" : : : "memory");
    } else if (x < 200) {
        result = x / 3;
        __asm__ volatile ("nop\nnop" : : : "memory");
    } else if (x < 300) {
        result = x + x;
        for (int i = 0; i < 3; i++) {
            result += vol_var1;  /* Volatile in loop */
        }
    } else {
        result = x - 50;
        /* Multiple nops to create different scheduling characteristics */
        __asm__ volatile ("nop\nnop\nnop" : : : "memory");
    }
    
    /* Switch statement for more control flow variation */
    switch (x % 4) {
        case 0:
            result += vol_var2 * 2;
            break;
        case 1:
            result -= vol_var1;
            break;
        case 2:
            result *= 3;
            break;
        case 3:
            result = result / 2;
            break;
    }
    
    return result;
}

/* Main computational kernel */
__attribute__((noinline))
static float compute_kernel(float *a, float *b, float *c, 
                           int *ia, int *ib, int *ic, int size) {
    float total = 0.0f;
    
    /* Mix different types of operations */
    high_pressure_loop(a, b, c, size);
    mixed_dependency(ia, ib, ic, size);
    
    /* Process results with control flow */
    for (int i = 0; i < size; i++) {
        int processed = control_flow_test(ia[i] + ib[i]);
        ic[i] = processed;
        total += c[i] + processed;
    }
    
    return total;
}

int main() {
    /* Allocate and initialize data */
    float *fa = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *fb = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *fc = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    int *ia = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *ib = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *ic = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
    }
    
    /* Perform many iterations to ensure scheduler sees hot code */
    float grand_total = 0.0f;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify volatile variables to create different scheduling contexts */
        vol_var1 = (iter % 100) + 1;
        vol_var2 = (iter % 50) + 1;
        vol_float1 = (iter % 10) + 0.5f;
        vol_float2 = (iter % 20) + 0.5f;
        
        /* Main computation */
        float result = compute_kernel(fa, fb, fc, ia, ib, ic, ARRAY_SIZE);
        grand_total += result;
        
        /* Prevent loop unrolling from simplifying too much */
        if (iter % 1000 == 0) {
            __asm__ volatile ("nop" : : : "memory");
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", grand_total);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(ia);
    free(ib);
    free(ic);
    
    return 0;
}
