/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with many temporaries */
static int complex_arithmetic(int seed) {
    /* Use seed to prevent constant propagation */
    int a = seed + vol_a;
    int b = vol_b * seed;
    int c = vol_c ^ seed;
    int d = vol_d | seed;
    
    /* Long dependency chain with many temporaries */
    int t1 = a * b + c;
    int t2 = t1 / (d + 1);
    int t3 = t2 % (b + 1);
    int t4 = t3 ^ (c * 2);
    int t5 = t4 - (a % 7);
    int t6 = t5 << (seed & 3);
    int t7 = t6 >> (vol_a & 1);
    int t8 = t7 | (vol_b & 0xFF);
    int t9 = t8 & (vol_c * 2);
    int t10 = t9 ^ (vol_d ^ seed);
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = t10 * 31 + 17;
    
    /* Use base in multiple, spatially separated contexts */
    int result = 0;
    
    /* Control flow to separate uses */
    if (seed % 2) {
        result += base * 2;  /* First use */
    } else {
        result += base / 2;  /* Second use */
    }
    
    /* Another use in different context */
    for (int i = 0; i < (seed & 3); i++) {
        result += base + i;  /* Third use */
    }
    
    /* Address computation with multiple offsets */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * seed;
    }
    
    /* Multiple offset computations from same base */
    int *ptr = &array[0];
    result += *(ptr + (base & 7));    /* Offset 1 */
    result += *(ptr + ((base >> 3) & 7)); /* Offset 2 */
    result += *(ptr + ((base >> 6) & 7)); /* Offset 3 */
    
    return result;
}

/* Floating point stress with volatile */
static float fp_stress(int iterations) {
    volatile float v1 = vol_f1;
    volatile float v2 = vol_f2;
    volatile float v3 = vol_f3;
    
    float acc = 0.0f;
    
    /* Loop with volatile variables prevents optimization */
    for (int i = 0; i < iterations; i++) {
        /* Complex FP expression with many temporaries */
        float t1 = v1 * v2 + v3;
        float t2 = t1 / (v2 + 1.0f);
        float t3 = t2 - v1;
        float t4 = t3 * v3;
        float t5 = t4 / (v2 * 2.0f);
        
        /* Multi-use value */
        float base = t5 * 1.5f;
        
        /* Use in different ways */
        if (i % 2) {
            acc += base * 2.0f;
        } else {
            acc += base / 2.0f;
        }
        
        /* Force register pressure with inline asm clobbers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* Modify volatiles to prevent loop invariant removal */
        v1 += 0.1f;
        v2 -= 0.05f;
    }
    
    return acc;
}

/* Main stress function with register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    
    /* Use external function for opaque values */
    int opaque = get_external_value() + seed;
    
    /* Loop with complex arithmetic and control flow */
    for (int i = 0; i < n; i++) {
        int iter_seed = seed + i + opaque;
        
        /* Branch to create multiple basic blocks */
        if (iter_seed % 4 == 0) {
            result += complex_arithmetic(iter_seed);
        } else if (iter_seed % 4 == 1) {
            result += (int)fp_stress(iter_seed % 10 + 1);
        } else if (iter_seed % 4 == 2) {
            /* More complex expression chains */
            int a = iter_seed * vol_a;
            int b = vol_b + iter_seed;
            int c = vol_c ^ iter_seed;
            
            /* Chain of operations */
            int val = a * b + c;
            val = val / (a % 7 + 1);
            val = val ^ (b << 2);
            val = val | (c & 0xFF);
            
            /* Multi-use temporary */
            int temp = val * 3 - 1;
            
            /* Spatially separated uses */
            result += temp;
            if (i % 3 == 0) {
                result += temp / 2;
            }
            if (i % 5 == 0) {
                result += temp * 2;
            }
        } else {
            /* Default case with inline asm */
            #ifdef __OPTIMIZE__
            asm volatile (
                "# More register clobbering\n"
                :
                :
                : "r6", "r7", "r8", "r9", "r10", "memory"
            );
            #endif
            
            result += iter_seed * vol_d;
        }
        
        /* Call external function periodically */
        if (i % 7 == 0) {
            opaque = get_external_value() + i;
        }
    }
    
    return result;
}

/* Dummy implementation for external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ & 0xFF;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    int iterations = 100;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Initialize random for more variability */
    srand(seed);
    
    /* Modify volatile variables */
    vol_a = rand() % 100;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100;
    vol_d = rand() % 100;
    
    /* Call stress function multiple times */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        int result = stress_computation(seed + i, iterations);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total: %d\n", total);
    
    /* Ensure result is used */
    return total == 0 ? 1 : 0;
}
