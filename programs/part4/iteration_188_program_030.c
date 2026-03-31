/* caller-save-test.c
 * Designed to trigger instruction movement into caller-save/restore sequences
 * and test the uncovered lines in caller-save.cc (lines 905-913)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization */
volatile int global_counter = 0;

/* Non-inline helper function - forces actual call */
__attribute__((noinline)) 
void helper_func(int a, double b, float c, long d) {
    /* Simple side effect to prevent optimization */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    
    /* Memory clobber to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Worker function with high register pressure */
__attribute__((noinline))
int worker_function(int seed1, int seed2, int seed3, int iterations) {
    /* Declare many variables of different types to create register pressure */
    int v1 = seed1;
    int v2 = seed2 * 2;
    int v3 = seed3 + 1;
    long v4 = (long)seed1 * seed2;
    long v5 = v4 + seed3;
    float v6 = (float)seed1 / 3.14f;
    float v7 = (float)seed2 * 2.718f;
    float v8 = v6 + v7;
    double v9 = (double)seed3 * 1.414;
    double v10 = v9 / 2.0;
    double v11 = v10 + (double)seed1;
    int v12 = v1 + v2;
    long v13 = v4 - v5;
    float v14 = v7 * v8;
    double v15 = v10 - v11;
    
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Loop to increase pressure and force repeated save/restore */
    for (int i = 0; i < iterations; i++) {
        /* Perform computations before call - values must survive */
        int t1 = v1 + v2 + i;
        long t2 = v4 * v5 - i;
        float t3 = v6 * v7 + (float)i;
        double t4 = v9 / v10 - (double)i;
        
        /* Mix computations to use both integer and FP registers */
        v1 = v2 ^ t1;
        v2 = v3 + t1;
        v4 = v5 | t2;
        v6 = v7 * t3;
        v9 = v10 + t4;
        
        /* Additional computations whose results are used after call */
        int critical_value = v1 * v2 + v3;  /* This may be moved into save sequence */
        long another_value = v4 ^ v5;
        
        /* Memory barrier to limit reordering */
        asm volatile("" : : : "memory");
        
        /* Function call - many registers are live across this */
        helper_func(critical_value, v9, v6, another_value);
        
        /* Use values after call - ensures liveness across call */
        v3 = critical_value + 1;  /* Use critical_value here */
        v5 = another_value >> 2;
        v8 = v6 + v7;
        v11 = v9 * 2.0;
        
        /* More computations after call */
        v12 = v12 + v1;
        v13 = v13 - v4;
        v14 = v14 * v6;
        v15 = v15 + v9;
        
        /* Accumulate to volatile to prevent dead code elimination */
        result += v3 + (int)v8 + (int)v11 + v12;
    }
    
    /* Final computation using all variables */
    int final = v1 + v2 + v3 + (int)v4 + (int)v5 + 
                (int)v6 + (int)v7 + (int)v8 + (int)v9 + 
                (int)v10 + (int)v11 + v12 + (int)v13 + 
                (int)v14 + (int)v15;
    
    return final + result;
}

/* Another non-inline function to create more call sites */
__attribute__((noinline))
int secondary_worker(int base, int multiplier) {
    int a = base * 3;
    long b = (long)base * multiplier;
    float c = (float)base / 1.5f;
    double d = (double)multiplier * 3.14159;
    
    /* Create register pressure */
    int x1 = a + 1;
    int x2 = a * 2;
    long x3 = b + 100;
    float x4 = c * 2.0f;
    double x5 = d / 2.0;
    
    /* Call with live values */
    helper_func(x1, d, c, b);
    
    /* Use values after call */
    x2 = x1 + x2;
    x3 = x3 | b;
    x4 = x4 + c;
    x5 = x5 * d;
    
    return x1 + x2 + (int)x3 + (int)x4 + (int)x5;
}

int main(int argc, char *argv[]) {
    int seed1, seed2, seed3;
    int iterations = 5;
    
    /* Get initial values from command line or stdin */
    if (argc >= 4) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
        if (argc >= 5) {
            iterations = atoi(argv[4]);
        }
    } else {
        printf("Enter three seed values: ");
        if (scanf("%d %d %d", &seed1, &seed2, &seed3) != 3) {
            seed1 = 42;
            seed2 = 17;
            seed3 = 99;
        }
    }
    
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;  /* Prevent excessive runtime */
    
    volatile int total = 0;  /* volatile accumulator */
    
    /* Multiple calls to increase chance of triggering the code path */
    for (int i = 0; i < 3; i++) {
        int r1 = worker_function(seed1 + i, seed2 - i, seed3 * (i + 1), iterations);
        int r2 = secondary_worker(seed1 + r1, seed2);
        
        total += r1 + r2;
        
        /* Modify seeds to create variation */
        seed1 = (seed1 * 13 + 7) & 0xFF;
        seed2 = (seed2 * 17 + 11) & 0xFF;
        seed3 = (seed3 * 19 + 13) & 0xFF;
    }
    
    /* Print result to create observable side effect */
    printf("Result: %d (global_counter: %d)\n", total, global_counter);
    
    return total != 0 ? 0 : 1;
}
