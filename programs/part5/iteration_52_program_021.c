/* test-early-remat.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops test-early-remat.c -o test-early-remat
 * Also try: gcc -O3 -funroll-loops -fno-gcse test-early-remat.c -o test-early-remat
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
static volatile int vol_a = 1;
static volatile int vol_b = 2;
static volatile int vol_c = 3;
static volatile int vol_d = 4;
static volatile int vol_e = 5;
static volatile int vol_f = 6;
static volatile int vol_g = 7;

/* Complex arithmetic stress function with register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base_computation = seed * vol_a + vol_b / (vol_c + 1);
    
    /* Complex arithmetic chain creating many temporaries */
    for (i = 0; i < n; i++) {
        /* Volatile loop bound to prevent unrolling optimization */
        volatile int loop_bound = n;
        
        /* Long dependency chain of floating-point operations */
        double a = (double)(seed + i) * vol_a;
        double b = (double)vol_b / (vol_c + 0.5);
        double c = a * b + (double)vol_d / (vol_e + 0.3);
        double d = c - (double)(vol_f % (vol_g + 1));
        double e = d * d - c * b + a / (b + 1.0);
        
        /* Integer arithmetic with modulo creating more temporaries */
        int temp1 = (i * vol_a + vol_b) % (vol_c + 1);
        int temp2 = (temp1 * vol_d - vol_e) % (vol_f + 1);
        int temp3 = (temp2 * vol_g + vol_a) % (vol_b + 1);
        
        /* Multi-use of base_computation in different contexts */
        int use1 = base_computation + temp1;
        int use2 = base_computation - temp2;
        int use3 = base_computation * temp3;
        
        /* Address computation with multiple offsets - pattern for rematerialization */
        int array[8];
        for (j = 0; j < 8; j++) {
            array[j] = j * j;
        }
        
        /* Multiple uses of computed addresses with different offsets */
        int sum = 0;
        sum += array[(use1 % 8)];      /* Base address + offset 0 */
        sum += array[(use2 % 8) ^ 1];  /* Different offset pattern */
        sum += array[(use3 % 8) & 3];  /* Another offset pattern */
        
        /* Inline assembly to clobber registers and increase pressure */
#ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
#endif
        
        /* Opaque function call to create unanalyzable values */
        int opaque = get_external_value();
        
        /* More complex arithmetic mixing all values */
        result += (int)(e * 100.0) + use1 + use2 + use3 + sum + opaque + temp1 * temp2 - temp3;
        
        /* Control flow to create multiple basic blocks */
        if (result % 7 == 0) {
            /* Different computation in this path */
            result += base_computation * 3;
        } else if (result % 5 == 0) {
            /* Another path with different register usage */
            result -= base_computation / 2;
        } else {
            /* Default path */
            result ^= base_computation;
        }
        
        /* Switch statement to create more control flow */
        switch (i % 4) {
            case 0:
                result += array[0] * base_computation;
                break;
            case 1:
                result += array[1] * (base_computation + 1);
                break;
            case 2:
                result += array[2] * (base_computation - 1);
                break;
            case 3:
                result += array[3] * (base_computation * 2);
                break;
        }
    }
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    int result = seed;
    
    /* Create many virtual registers with complex expressions */
    for (int i = 0; i < n; i++) {
        /* Chain of interdependent calculations */
        int a = (result * 1103515245 + 12345) & 0x7fffffff;
        int b = (a * 1103515245 + 12345) & 0x7fffffff;
        int c = (b * 1103515245 + 12345) & 0x7fffffff;
        int d = (c * 1103515245 + 12345) & 0x7fffffff;
        
        /* Use all temporaries in a complex expression */
        result = (a % 1000) * (b % 100) + (c % 10) * (d % 1000) - 
                 (a % 100) * (c % 100) + (b % 10) * (d % 100);
        
        /* More inline assembly for register pressure */
#ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "cc",
              "r0", "r1", "r2", "r3", "r4", "r5"
        );
#endif
    }
    
    return result;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7fff;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result1 = 0, result2 = 0;
    
    /* Use command line or default iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    printf("Starting stress computations with %d iterations...\n", iterations);
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 3; i++) {
        int seed = rand() % 1000;
        
        /* First stress function */
        result1 += stress_computation(seed, iterations);
        
        /* Second stress function */
        result2 += stress_computation2(seed, iterations / 2);
        
        /* Mix results to prevent optimization */
        result1 ^= result2;
        result2 ^= result1;
        result1 ^= result2;
    }
    
    /* Final computation mixing both results */
    int final_result = result1 * 3 - result2 * 2;
    
    printf("Result: %d\n", final_result);
    
    /* Use result to prevent dead code elimination */
    if (final_result == 0x12345678) {  /* Extremely unlikely */
        printf("Impossible condition reached!\n");
    }
    
    return final_result & 0xff;  /* Return non-zero to indicate execution */
}
