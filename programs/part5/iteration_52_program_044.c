/* Compile with: gcc -O2 -fno-omit-frame-pointer -o test test.c */
/* For more aggressive testing: gcc -O3 -funroll-loops -fno-gcse -o test test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with volatile variables and long dependency chains */
static int complex_volatile_chain(int seed) {
    /* Force many virtual registers with complex expression */
    int t1 = vol_a * seed + vol_b;
    int t2 = vol_c / (seed | 1) - vol_d;  /* Avoid division by zero */
    int t3 = t1 % (t2 + 1);
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = t1 * t2 - t3;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "# Clobber hard registers to force virtual register usage\n"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    /* Use base in multiple different contexts */
    int result = 0;
    switch (seed % 4) {
        case 0:
            result = base + vol_a * 2;
            break;
        case 1:
            result = base - vol_b / 2;
            break;
        case 2:
            result = base * (vol_c + 1);
            break;
        case 3:
            result = (base % 17) + vol_d;
            break;
    }
    
    /* More complex arithmetic creating register pressure */
    float f1 = vol_f1 * seed;
    float f2 = vol_f2 / (seed + 1);
    float f3 = vol_f3 + seed;
    float f4 = f1 * f2 - f3 / (f1 + 0.5f);
    
    /* Convert float result to affect integer return */
    return result + (int)f4;
}

/* Function with address computation patterns */
static int address_computation_pattern(int *array, int size, int idx) {
    /* Compute base address - candidate for rematerialization */
    int *base_ptr = &array[idx % size];
    
    /* Use base with multiple offsets in separate expressions */
    int sum = 0;
    sum += base_ptr[0] * 2;      /* offset 0 */
    sum += base_ptr[1] / 3;      /* offset 1 */
    sum += base_ptr[-1] % 5;     /* offset -1 */
    sum += base_ptr[2] + 7;      /* offset 2 */
    sum += base_ptr[-2] - 11;    /* offset -2 */
    
    /* Complex expression using the base multiple times */
    int temp = (base_ptr[0] * base_ptr[1]) / (base_ptr[-1] + 1);
    
    return sum + temp;
}

/* Loop with volatile dependencies to prevent optimization */
static int loop_with_volatile_deps(int iterations) {
    volatile int vol_counter = iterations;
    volatile int vol_mod = 7;
    
    int result = 0;
    
    /* Loop-carried dependency with volatile */
    for (volatile int i = 0; i < vol_counter; i = i + 1) {
        /* Complex expression inside loop - creates register pressure */
        int a = get_external_value() + i;
        int b = vol_mod * a;
        int c = b % (vol_mod + 1);
        int d = (a * c) / (b + 1);
        
        /* Multi-use temporary */
        int temp = a + b - c * d;
        
        /* Use temp in different ways based on condition */
        if (i % 3 == 0) {
            result += temp * 2;
        } else if (i % 3 == 1) {
            result -= temp / 2;
        } else {
            result ^= temp;
        }
        
        /* More register pressure with floating point */
        float f1 = vol_f1 * i;
        float f2 = vol_f2 / (i + 1.0f);
        result += (int)(f1 * f2);
    }
    
    return result;
}

/* Main stress function combining all patterns */
static int stress_computation(int seed, int n) {
    int array[100];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = (i * seed) % 97;
    }
    
    /* Combine multiple patterns to increase register pressure */
    int r1 = complex_volatile_chain(seed);
    int r2 = address_computation_pattern(array, 100, seed % 90);
    int r3 = loop_with_volatile_deps(n % 50 + 10);
    
    /* Final complex expression using all results */
    int result = (r1 * r2) / ((r3 | 1) + 1);
    result += (r1 % 13) * (r2 % 17) - (r3 % 19);
    
    /* Another inline assembly to clobber registers */
    asm volatile (
        "# Final register clobber\n"
        : 
        : 
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5"
    );
    
    return result;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7fffffff;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    /* Use command line argument for variability if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    int total_result = 0;
    
    /* Call stress function multiple times from different contexts */
    for (int i = 0; i < 5; i++) {
        int n = rand() % 100;
        int result = stress_computation(seed + i, n);
        total_result += result;
        
        /* Vary the call pattern */
        if (i % 2 == 0) {
            /* Additional computation at some call sites */
            total_result ^= complex_volatile_chain(result);
        }
    }
    
    /* Ensure result is used to prevent dead code elimination */
    printf("Result: %d\n", total_result % 1000000);
    
    return 0;
}
