/* Test program to trigger virtual register creation stress in early-remat.cc */
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
    
    /* Complex arithmetic with volatile variables - creates many temporaries */
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain with volatile accesses */
        int temp1 = vol_a * vol_b + vol_c / (vol_d + 1);
        int temp2 = vol_b * vol_c - vol_d % (vol_a + 1);
        int temp3 = vol_c * vol_d + vol_a / (vol_b + 1);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = temp1 * temp2 - temp3;
        
        /* Use base in multiple, spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + seed;
        }
        
        /* Complex floating-point chain - creates FP virtual registers */
        float f_temp = vol_f1 * vol_f2 + vol_f3 / vol_f1 - vol_f2;
        result += (int)f_temp;
        
        /* Inline assembly that clobbers registers - increases pressure */
        #ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            : 
            : "r"(temp1), "r"(temp2), "r"(temp3)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* Address computation with multiple offsets */
        int array[10];
        int *ptr = &array[i % 10];
        
        /* Use base address with different offsets - may trigger rematerialization */
        int val1 = *(ptr + 0);
        int val2 = *(ptr + 1);
        int val3 = *(ptr + 2);
        
        result += val1 + val2 + val3;
        
        /* Opaque function call prevents optimization */
        int opaque = get_opaque_value(seed + i);
        
        /* More complex arithmetic with opaque value */
        result += (opaque * base) / (temp1 + 1) - (temp2 % (temp3 + 1));
    }
    
    return result;
}

/* Another stress function with different patterns */
int stress_control_flow(int seed, int n) {
    int total = 0;
    volatile int vol_counter = seed;
    
    /* Loop with volatile bound prevents optimization */
    for (volatile int i = 0; i < n; i++) {
        /* Complex expression with many temporaries */
        int a = vol_counter * 3;
        int b = vol_counter / 2;
        int c = vol_counter % 7;
        
        /* Multi-use value */
        int complex_val = (a * b) + (c << 2) - (a / (b + 1));
        
        /* Switch with spatially separated uses */
        switch (i % 4) {
            case 0:
                total += complex_val * 2;
                break;
            case 1:
                total += complex_val / 2;
                break;
            case 2:
                total += complex_val + a;
                break;
            case 3:
                total += complex_val - b;
                break;
        }
        
        /* More register pressure */
        int d = complex_val * vol_counter;
        int e = complex_val / (vol_counter + 1);
        int f = d % (e + 1);
        
        total += f;
        
        vol_counter++;
    }
    
    return total;
}

/* External function definition */
int get_opaque_value(int seed) {
    /* Use rand() to create unpredictable values */
    return rand() % 100 + seed;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result = 0;
    
    /* Use command line or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    
    /* Call stress functions multiple times from different contexts */
    for (int j = 0; j < 5; j++) {
        result += stress_computation(j * 10, iterations);
        result += stress_control_flow(j * 20, iterations / 2);
        
        /* Vary inputs to prevent constant propagation */
        vol_a = (vol_a * 3) % 17;
        vol_b = (vol_b * 5) % 19;
        vol_c = (vol_c * 7) % 23;
        vol_d = (vol_d * 11) % 29;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
