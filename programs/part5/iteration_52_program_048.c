/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE_REGISTER_PRESSURE 1
#endif

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register pressure patterns */
int stress_computation(int seed, int n) {
    volatile int vol_a = seed;
    volatile int vol_b = seed * 2;
    volatile int vol_c = seed + 1;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function calls create unanalyzable values */
        int opaque1 = get_opaque_value(seed + i);
        int opaque2 = get_opaque_value(seed + i * 3);
        
        /* Long dependency chain with volatile accesses */
        int temp1 = vol_a * opaque1 + vol_b / (opaque2 + 1);
        int temp2 = temp1 - vol_c % (opaque1 + 2);
        int temp3 = temp2 * opaque2 - opaque1 / (vol_a + 1);
        int temp4 = temp3 + (opaque1 % 7) - (opaque2 % 5);
        
        /* Multi-use temporary value */
        int base = temp4 * 3 - opaque1;
        
        /* Use base in multiple, spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + opaque2;
        }
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 5; j++) {
            /* Base address computation that might be rematerialized */
            int* ptr = &array[j];
            result += *(ptr + 0) + *(ptr + 1) + *(ptr + 2);
        }
        
        /* Inline assembly with clobbers to reduce available hard registers */
        #ifdef AGGRESSIVE_REGISTER_PRESSURE
        asm volatile (
            "# Dummy assembly clobbering registers\n"
            : 
            : "r"(result), "r"(i)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* Prevent optimization with volatile loop variable update */
        vol_a += result % 17;
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_computation2(int seed, int n) {
    volatile double vol_x = seed * 1.5;
    volatile double vol_y = seed * 0.75;
    
    double result = 0.0;
    
    /* Floating-point arithmetic creates different register pressure */
    for (int i = 0; i < n; i++) {
        double opaque = get_opaque_value(seed + i) * 1.0;
        
        /* Complex FP expression chain */
        double t1 = vol_x * opaque + vol_y;
        double t2 = t1 / (opaque + 1.0);
        double t3 = t2 - vol_x * vol_y;
        double t4 = t3 * opaque - vol_y / (vol_x + 1.0);
        
        /* Multi-use value in switch statement */
        double base = t4 * 2.5 - opaque;
        
        switch (i % 4) {
            case 0:
                result += base * 1.1;
                break;
            case 1:
                result += base / 1.1;
                break;
            case 2:
                result += base + opaque;
                break;
            case 3:
                result += base - opaque;
                break;
        }
        
        /* More register pressure with nested loops */
        for (int j = 0; j < 3; j++) {
            result += (base * j) / (opaque + j + 1);
        }
    }
    
    return (int)result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use actual rand() to ensure compiler can't analyze the value */
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return rand() % 100 + seed % 23;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total_result = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 5; i++) {
        total_result += stress_computation(i * 17, iterations);
        total_result += stress_computation2(i * 23, iterations / 2 + 1);
        
        /* Alternate call pattern to create different control flow */
        if (i % 2 == 0) {
            total_result += stress_computation(total_result, 10);
        } else {
            total_result += stress_computation2(total_result, 15);
        }
    }
    
    printf("Result: %d\n", total_result);
    
    /* Ensure result is used to prevent optimization */
    volatile int *dummy = (volatile int*)malloc(sizeof(int));
    *dummy = total_result;
    printf("Final: %d\n", *dummy);
    free((void*)dummy);
    
    return total_result != 0 ? 0 : 1;
}
