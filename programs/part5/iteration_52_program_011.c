/* Test program to stress early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE_ASM 1
#else
#define AGGRESSIVE_ASM 0
#endif

/* Volatile variables to prevent optimization */
volatile int vol_a = 1;
volatile int vol_b = 2;
volatile int vol_c = 3;
volatile int vol_d = 4;
volatile int vol_e = 5;
volatile int vol_f = 6;
volatile int vol_g = 7;

/* External function to create opaque values */
extern int rand(void);

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base_computation = 0;
    
    /* Complex arithmetic expression creating dependency chain */
    base_computation = (vol_a * vol_b) + (vol_c / vol_d) - (vol_e % vol_f);
    base_computation = base_computation * vol_g - seed;
    
    /* Inline assembly to clobber hard registers (increases register pressure) */
    #if AGGRESSIVE_ASM
    asm volatile (
        "# Clobber multiple registers to increase pressure\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        :
        : "r" (vol_a), "r" (vol_b), "r" (vol_c)
        : "r0", "r1", "r2", "memory"
    );
    #endif
    
    /* Loop with volatile variables to prevent optimization */
    for (i = 0; i < n; i++) {
        volatile int loop_var = i;
        int temp1, temp2, temp3;
        
        /* Complex expression using volatile and function calls */
        temp1 = (rand() % 100) * loop_var + base_computation;
        temp2 = (vol_d * temp1) / (vol_e + 1);
        temp3 = (temp2 % vol_f) * vol_g;
        
        /* Multi-use of base_computation in different contexts */
        if (i % 3 == 0) {
            result += temp1 * base_computation;
        } else if (i % 3 == 1) {
            result += temp2 + base_computation;
        } else {
            result += temp3 - base_computation;
        }
        
        /* Nested loop for additional pressure */
        for (j = 0; j < (i % 5) + 1; j++) {
            volatile int inner_vol = j;
            int inner_temp = (base_computation * inner_vol) % 256;
            result ^= inner_temp;
        }
    }
    
    /* Address computation with multiple offsets - pattern for rematerialization */
    int array[100];
    for (i = 0; i < 10; i++) {
        int *base_addr = &array[i * 10];
        
        /* Multiple uses of base_addr with different offsets */
        base_addr[0] = result + i;
        base_addr[1] = result - i;
        base_addr[2] = result * i;
        base_addr[3] = result / (i + 1);
        
        /* Complex expression using the array values */
        result = (base_addr[0] + base_addr[1]) * (base_addr[2] - base_addr[3]);
    }
    
    /* Switch statement with multi-use temporaries */
    switch (result % 4) {
        case 0:
            result += base_computation * 2;
            break;
        case 1:
            result -= base_computation / 2;
            break;
        case 2:
            result *= base_computation + 1;
            break;
        case 3:
            result /= (base_computation % 10) + 1;
            break;
    }
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    int result = seed;
    volatile int vol_counter = 0;
    
    while (vol_counter < n) {
        /* Long dependency chain with floating point */
        float f1 = (float)(rand() % 1000) / 100.0f;
        float f2 = (float)(rand() % 1000) / 100.0f;
        float f3 = (float)(rand() % 1000) / 100.0f;
        
        float temp = f1 * f2 + f3 / f1 - f2 * f3;
        result += (int)(temp * 100);
        
        /* More inline assembly pressure */
        #if AGGRESSIVE_ASM
        asm volatile (
            "# More register clobbering\n\t"
            :
            :
            : "r3", "r4", "r5", "r6", "memory"
        );
        #endif
        
        vol_counter++;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result1 = 0, result2 = 0;
    int i;
    
    /* Use command line or default iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Seed random for reproducibility */
    srand(time(NULL));
    
    printf("Starting stress computations with %d iterations...\n", iterations);
    
    /* Multiple calls to stress functions */
    for (i = 0; i < 5; i++) {
        result1 += stress_computation(rand() % 100, iterations / 5);
        result2 += stress_computation2(rand() % 100, iterations / 10);
        
        /* Mix in some direct complex expressions */
        int direct_result = (vol_a * vol_b + vol_c) / (vol_d - vol_e % vol_f);
        result1 ^= direct_result;
        result2 |= direct_result;
    }
    
    /* Final complex expression using both results */
    int final_result = (result1 * result2) / ((result1 % 100) + 1) 
                       - (result2 % 50) * 2;
    
    printf("Final result: %d\n", final_result);
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    return final_result != 0 ? 0 : 1;
}
