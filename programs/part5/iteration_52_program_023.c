/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define USE_AGGRESSIVE_PATTERNS 1
#else
#define USE_AGGRESSIVE_PATTERNS 0
#endif

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* External function to create opaque values */
extern int rand(void);

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int iterations) {
    int result = 0;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base_computation = seed * vol_a + vol_b / (vol_c + 1);
    
    /* Complex floating-point chain creating many temporaries */
    float fp_chain = (vol_f1 * vol_f2) + (vol_f3 / vol_f1) - 
                     (vol_f2 * vol_f3) + (vol_f1 / vol_f2);
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int loop_bound = iterations;
    for (volatile int i = 0; i < loop_bound; i++) {
        /* Complex integer arithmetic creating dependency chain */
        int temp1 = base_computation * i;
        int temp2 = temp1 + vol_d;
        int temp3 = temp2 / (vol_a + 1);
        int temp4 = temp3 % (vol_b + 1);
        int temp5 = temp4 - vol_c;
        
        /* Use the temporary in multiple contexts */
        if (temp5 > 100) {
            result += temp5 * 2;  /* First use of temp5 */
        } else if (temp5 < -50) {
            result -= temp5 / 2;  /* Second use of temp5 */
        } else {
            result += temp5 + base_computation;  /* Third use with base */
        }
        
        /* Address computation with multiple offsets */
        int array[100];
        int *base_addr = &array[i % 50];
        
        /* Multiple uses of base address with different offsets */
        base_addr[0] = temp5;
        base_addr[5] = temp5 * 2;
        base_addr[10] = temp5 / 2;
        base_addr[15] = temp5 + base_computation;
        
        /* More complex arithmetic with function calls */
        int opaque_val = rand() % 100;
        int complex_val = (opaque_val * temp5) + (base_computation / (opaque_val + 1)) -
                         (temp5 % (opaque_val + 2)) + (opaque_val * base_computation);
        
        result += complex_val;
        
        /* Floating-point operations to create FP register pressure */
        fp_chain = fp_chain * 1.01f + (float)complex_val * 0.5f;
        result += (int)fp_chain;
        
#if USE_AGGRESSIVE_PATTERNS
        /* Inline assembly clobbering registers */
        __asm__ volatile (
            "# Register clobber\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
#endif
        
        /* Switch with multi-use temporaries */
        switch (i % 4) {
            case 0:
                result += base_computation * 3;  /* Use in case 0 */
                break;
            case 1:
                result += base_computation / 2;  /* Use in case 1 */
                break;
            case 2:
                result -= base_computation;      /* Use in case 2 */
                break;
            case 3:
                result += base_computation + temp5;  /* Combined use */
                break;
        }
    }
    
    /* Final complex expression */
    int final = (result * vol_a) + (vol_b * base_computation) - 
                (vol_c % (result + 1)) + (vol_d / (base_computation + 1));
    
    return final;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    int total = seed;
    
    /* Create long dependency chain */
    for (int i = 0; i < n; i++) {
        volatile int vi = i;
        
        /* Complex expression with many intermediates */
        int a = total + vi;
        int b = a * vol_a;
        int c = b - vol_b;
        int d = c / (vol_c + 1);
        int e = d % (vol_d + 1);
        int f = e + vol_a;
        int g = f * vol_b;
        int h = g - vol_c;
        
        /* Use value in multiple places */
        if (h > 0) {
            total += h * 2;
        } else {
            total -= h / 2;
        }
        
        /* Another use in address calculation */
        int arr[50];
        for (int j = 0; j < 10; j++) {
            arr[j] = h + j * 3;  /* h used with different offsets */
        }
        
        total += arr[5];
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result1 = 0, result2 = 0;
    
    /* Use command line or random for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    
    /* Call stress functions multiple times from different contexts */
    for (int call = 0; call < 5; call++) {
        int seed = rand() % 1000;
        
        /* First call site */
        result1 += stress_computation(seed, iterations);
        
        /* Second call site with different arguments */
        result2 += stress_computation2(seed + 1, iterations / 2);
        
        /* Third call site with inverted arguments */
        result1 -= stress_computation(iterations, seed % 50);
    }
    
    /* Combine and output results to prevent elimination */
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
