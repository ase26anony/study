/* Test program to trigger virtual register creation in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with long dependency chain */
static int complex_arithmetic(int seed) {
    /* Use volatile variables to force loads */
    int a = vol_a + seed;
    int b = vol_b * a;
    int c = vol_c / (b + 1);
    int d = vol_d % (c + 1);
    
    /* Long chain of arithmetic operations */
    int t1 = a * b + c - d;
    int t2 = b * c + d / (a + 1);
    int t3 = c * d + a % (b + 1);
    int t4 = d * a + b - c;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = t1 * t2 - t3 + t4;
    
    /* Use base in multiple separated contexts */
    int result = 0;
    switch (seed % 4) {
        case 0:
            result = base + t1 * 2;
            break;
        case 1:
            result = base - t2 / 2;
            break;
        case 2:
            result = base * 3 + t3;
            break;
        case 3:
            result = base / 4 - t4;
            break;
    }
    
    /* Another complex expression using all temporaries */
    result += (t1 * t2) / (t3 + 1) - (t4 % (a + 1)) + (b * c) - (d / (seed + 1));
    
    return result;
}

/* Floating point version with more register pressure */
static float fp_complex_arithmetic(float seed) {
    volatile float v1 = vol_f1, v2 = vol_f2, v3 = vol_f3;
    
    /* Complex FP expressions creating many virtual registers */
    float f1 = v1 * seed + v2;
    float f2 = v2 / seed - v3;
    float f3 = v3 * seed + v1;
    float f4 = v1 / (seed + 1.0f) - v2;
    
    /* Long dependency chain */
    float t1 = f1 * f2 + f3 / f4;
    float t2 = f2 * f3 - f4 / f1;
    float t3 = f3 * f4 + f1 / f2;
    float t4 = f4 * f1 - f2 / f3;
    
    /* Multi-use base value */
    float base = (t1 + t2) * (t3 - t4);
    
    /* Use base with different offsets in address-like pattern */
    float results[4];
    results[0] = base + 0.0f;
    results[1] = base + 1.0f;
    results[2] = base + 2.0f;
    results[3] = base + 3.0f;
    
    /* Complex expression using all values */
    return results[0] * results[1] - results[2] / results[3] + 
           t1 * t2 - t3 / t4 + f1 + f2 - f3 * f4;
}

/* Function with inline assembly to clobber registers */
static int with_asm_clobber(int x, int y) {
    int result;
    
    /* Complex computation before asm */
    int a = x * y + vol_a;
    int b = y * vol_b - x;
    int c = a % (b + 1) + vol_c;
    
#ifdef __OPTIMIZE__
    /* Inline assembly that clobbers multiple registers */
    __asm__ volatile (
        "mov %1, %%eax\n\t"
        "mov %2, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (c)
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
#else
    result = a + c;
#endif
    
    /* More computations after asm */
    int d = result * vol_d;
    int e = d / (x + 1) + y;
    
    return result + d - e;
}

/* Main stress function with loop-carried dependencies */
int stress_computation(int seed, int iterations) {
    int total = 0;
    volatile int vol_counter = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (vol_counter = 0; vol_counter < iterations; vol_counter++) {
        /* Mix different types of computations */
        int val1 = complex_arithmetic(seed + vol_counter);
        
        /* Use opaque function call */
        int opaque = get_external_value();
        
        /* More complex expressions with opaque values */
        float val2 = fp_complex_arithmetic((float)(seed + opaque) / 100.0f);
        
        /* Computation with inline assembly */
        int val3 = with_asm_clobber(val1, opaque);
        
        /* Address computation pattern with multiple offsets */
        int array[8];
        int base_idx = (val1 + val3) % 8;
        
        /* Use base with multiple offsets - potential for rematerialization */
        total += array[(base_idx + 0) % 8];
        total += array[(base_idx + 1) % 8];
        total += array[(base_idx + 2) % 8];
        total += array[(base_idx + 3) % 8];
        
        /* Complex arithmetic chain */
        total += (val1 * val3) / (opaque + 1) - 
                 (val1 % (opaque + 1)) + 
                 ((int)val2 * 100);
    }
    
    return total;
}

/* Simulated external function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7fffffff;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    int iterations = 100;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Initialize volatile variables with some pattern */
    vol_a = seed;
    vol_b = seed * 2;
    vol_c = seed * 3;
    vol_d = seed * 4;
    vol_f1 = (float)seed / 10.0f;
    vol_f2 = (float)seed / 5.0f;
    vol_f3 = (float)seed / 3.0f;
    
    /* Call stress function multiple times from different contexts */
    int result1 = stress_computation(seed, iterations);
    int result2 = stress_computation(seed + 1, iterations / 2);
    int result3 = stress_computation(seed + 2, iterations / 4);
    
    /* Use results to prevent elimination */
    int final_result = result1 + result2 * 2 - result3;
    
    printf("Result: %d\n", final_result);
    
    /* Additional calls with different patterns */
    for (int i = 0; i < 10; i++) {
        int temp = stress_computation(seed + i * 100, 5);
        printf("Iteration %d: %d\n", i, temp);
    }
    
    return final_result != 0 ? 0 : 1;
}
