/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - x86 specific */
__attribute__((noinline, noclone))
void clobber_callee_x86(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific x86 registers */
    asm volatile (
        "movl %0, %%eax\n"
        "movl %1, %%ecx\n"
        "movl %2, %%edx\n"
        "addl %%ecx, %%eax\n"
        "subl %%edx, %%eax\n"
        "movl %%eax, %3\n"
        : 
        : "m"(*p1), "m"(*p2), "m"(*p3), "m"(*p4)
        : "eax", "ecx", "edx", "memory"
    );
}

/* Alternative for non-x86 architectures */
__attribute__((noinline, noclone))
void clobber_callee_generic(int *p1, int *p2, int *p3, int *p4) {
    /* Use memory clobber to force spills */
    asm volatile ("" : : "r"(*p1), "r"(*p2), "r"(*p3), "r"(*p4) : "memory");
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_more(int *arr, int n) {
    asm volatile ("" : : "r"(arr), "r"(n) : "memory");
}

/* Function with high register pressure around calls */
__attribute__((noinline, noclone))
int high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int result = 0;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed ^ 0x1234;
    v4 = seed - iter;
    v5 = (seed << 3) | (seed >> 29);
    v6 = v1 + v2;
    v7 = v3 * v4;
    v8 = v5 ^ v6;
    v9 = v7 - v8;
    v10 = v9 * 0x5678;
    
    v11 = v10 + global_seed;
    v12 = v11 * v1;
    v13 = v12 / (v2 + 1);
    v14 = v13 | v3;
    v15 = v14 & v4;
    v16 = v15 ^ v5;
    v17 = v16 + v6;
    v18 = v17 - v7;
    v19 = v18 * v8;
    v20 = v19 / (v9 + 1);
    
    v21 = v20 | v10;
    v22 = v21 & v11;
    v23 = v22 ^ v12;
    v24 = v23 + v13;
    v25 = v24 - v14;
    v26 = v25 * v15;
    v27 = v26 / (v16 + 1);
    v28 = v27 | v17;
    v29 = v28 & v18;
    v30 = v29 ^ v19;
    
    /* Create conditional basic block where call is at the end */
    if ((seed & 0x3) == 0) {
        /* High pressure path - many variables live across call */
        
        /* More computations to increase live range */
        v1 = v1 + v30;
        v2 = v2 * v29;
        v3 = v3 ^ v28;
        v4 = v4 - v27;
        
        /* Call that clobbers registers - this should be BB_END before save insertion */
        clobber_callee_x86(&v1, &v2, &v3, &v4);
        
        /* Use results after call to keep variables live */
        v5 = v1 + v2;
        v6 = v3 * v4;
    } else if ((seed & 0x3) == 1) {
        /* Medium pressure path */
        clobber_callee_generic(&v5, &v6, &v7, &v8);
        v9 = v5 + v6 + v7 + v8;
    } else {
        /* Low pressure path - still some live variables */
        int temp[4] = {v10, v11, v12, v13};
        clobber_more(temp, 4);
        v14 = temp[0] + temp[1];
    }
    
    /* Use all variables in final computation to keep them live */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
             v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    /* Another conditional with call at block end */
    if ((result & 0x1) && (iter > 0)) {
        int a = result * 2;
        int b = result / 2;
        int c = result ^ 0xABCD;
        int d = result + iter;
        
        /* This call should also trigger caller-save */
        clobber_callee_x86(&a, &b, &c, &d);
        
        result = a + b + c + d;
    }
    
    return result;
}

/* Function with loop creating multiple call sites */
__attribute__((noinline, noclone))
int loop_with_calls(int base_seed, int iterations) {
    int total = 0;
    volatile int vol_counter = 0; /* Prevent loop optimizations */
    
    for (int i = 0; i < iterations; i++) {
        /* Varying condition creates different basic blocks */
        int seed = base_seed + i + vol_counter;
        
        /* Each iteration creates new register pressure scenario */
        int res = high_pressure_function(seed, i);
        
        /* Use volatile to prevent moving computations across iterations */
        asm volatile ("" : : "r"(res) : "memory");
        
        total += res;
        
        /* Update volatile to create data dependency */
        vol_counter += (res & 0xFF);
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Initialize with argc for deterministic but input-dependent behavior */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    global_seed = seed;
    
    printf("Starting caller-save test with seed: %d\n", seed);
    
    /* Create multiple scenarios with varying register pressure */
    int result1 = high_pressure_function(seed, 0);
    printf("Result 1: %d\n", result1);
    
    int result2 = high_pressure_function(seed ^ 0x1234, 1);
    printf("Result 2: %d\n", result2);
    
    int result3 = loop_with_calls(seed, 5);
    printf("Result 3 (loop): %d\n", result3);
    
    /* Final computation using all results */
    int final = result1 + result2 + result3;
    printf("Final result: %d\n", final);
    
    return final & 0xFF;
}
