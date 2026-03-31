/* Test program to trigger virtual register creation in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int iterations) {
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile variables to force loads/stores */
        int v1 = vol_a + i;
        int v2 = vol_b * seed;
        int v3 = vol_c % (v1 + 1);
        int v4 = vol_d - v2;
        
        /* Long dependency chain with floating point */
        float f_base = vol_f1 * vol_f2 + vol_f3 / (v1 + 1.0f);
        float f_chain = f_base * 2.0f - vol_f1 / f_base + 
                       (vol_f2 * vol_f3) / (f_base + 1.0f);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base_computation = v1 * v2 + v3 / (v4 + 1) - v2 % (v3 + 1);
        
        /* Use base_computation in multiple contexts */
        if (i % 3 == 0) {
            result += base_computation * 2;
        } else if (i % 3 == 1) {
            result += base_computation / 2;
        } else {
            result += base_computation + (int)f_chain;
        }
        
        /* Inline assembly that clobbers registers */
        /* This increases register pressure significantly */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber multiple registers to increase pressure\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            :
            : "r" (v1), "r" (v2), "r" (v3)
            : "r0", "r1", "r2", "memory"
        );
        #endif
        
        /* Address computation with multiple offsets */
        /* This can trigger register recreation patterns */
        static int array[100];
        int *base_ptr = &array[i % 50];
        
        /* Use base pointer with different offsets */
        int val1 = base_ptr[0] + base_computation;
        int val2 = base_ptr[5] - base_computation;
        int val3 = base_ptr[10] * (base_computation % 7);
        
        result += val1 + val2 + val3;
        
        /* Opaque function call to prevent analysis */
        int opaque = get_opaque_value(seed + i);
        
        /* More complex arithmetic with opaque value */
        result += (opaque * v1) / (v2 + 1) - (v3 % (opaque + 1)) + 
                 (int)(f_chain * opaque);
    }
    
    return result;
}

/* Another stress function with different patterns */
int stress_control_flow(int seed, int n) {
    int total = 0;
    volatile int vol_counter = n;
    
    /* Loop with volatile bound to prevent optimization */
    while (vol_counter > 0) {
        int x = get_opaque_value(seed + vol_counter);
        
        /* Complex switch with multi-use temporaries */
        switch (x % 5) {
            case 0: {
                int temp = x * vol_a + vol_b / (x + 1);
                total += temp * 2;
                total += temp / 4;  /* Same temp used twice */
                break;
            }
            case 1: {
                int temp = x % vol_c + vol_d * x;
                total += temp - (x % 7);
                total += temp + (x % 3);  /* Same temp used twice */
                break;
            }
            case 2: {
                float f_temp = vol_f1 * x + vol_f2 / (x + 1.0f);
                total += (int)(f_temp * 3.0f);
                total += (int)(f_temp / 2.0f);  /* Same temp used twice */
                break;
            }
            case 3: {
                /* Very complex expression chain */
                int a = x * vol_a;
                int b = vol_b % (x + 1);
                int c = vol_c + x;
                int d = vol_d - x;
                
                int chain1 = a * b + c / (d + 1) - b % (c + 1);
                int chain2 = chain1 * 2 - a / (b + 1) + c % (d + 1);
                int chain3 = chain2 / 3 + b * (c % 5) - d / (a + 1);
                
                total += chain1 + chain2 + chain3;
                break;
            }
            default: {
                total += x * 2;
                break;
            }
        }
        
        /* More inline assembly clobbering */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# More register clobbering\n"
            "mov r3, %0\n"
            "mov r4, %1\n"
            :
            : "r" (x), "r" (total)
            : "r3", "r4", "memory"
        );
        #endif
        
        vol_counter--;
    }
    
    return total;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use actual rand() to ensure it's truly opaque to compiler */
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return rand() % 100 + seed % 50;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    /* Use command line arguments to vary inputs */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Starting stress tests with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Call stress functions multiple times */
    int result1 = 0, result2 = 0;
    
    for (int i = 0; i < 3; i++) {
        result1 += stress_computation(seed + i, iterations / 3);
        result2 += stress_control_flow(seed + i * 7, iterations / 4);
    }
    
    /* Mix results to prevent optimization */
    int final_result = result1 * 3 - result2 * 2;
    
    printf("Final result: %d\n", final_result);
    
    /* Return non-zero to indicate success */
    return final_result != 0 ? 0 : 1;
}
