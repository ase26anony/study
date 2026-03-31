/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse test.c -o test */
/* Additional flags to try: -O3 -funroll-loops -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int iterations) {
    int i, result = 0;
    
    /* Complex arithmetic with volatile variables - creates many temporaries */
    for (i = 0; i < iterations; i++) {
        /* Long dependency chain with mixed operations */
        int t1 = vol_a * vol_b + seed;
        int t2 = vol_c / (vol_d + 1) - t1;
        int t3 = t2 % (vol_a + 2) * vol_b;
        int t4 = t3 + vol_c - (vol_d * t1) / (t2 + 1);
        
        /* Floating point chain - different register class */
        float f1 = vol_f1 * vol_f2 + (float)t1;
        float f2 = vol_f3 / (vol_f1 + 1.0f) - f1;
        float f3 = f2 * vol_f2 + (float)t2 / vol_f3;
        
        /* Multi-use temporary value used in different contexts */
        int base = t4 * (int)f3 + i;
        
        /* Use base in multiple, spatially separated ways */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + (int)f1;
        }
        
        /* Address computation with multiple offsets */
        char buffer[256];
        char *ptr = &buffer[0];
        
        /* Multiple uses of ptr with different offsets */
        ptr[i % 256] = (char)(base & 0xFF);
        ptr[(i + 1) % 256] = (char)((base >> 8) & 0xFF);
        ptr[(i + 2) % 256] = (char)((base >> 16) & 0xFF);
        
        /* Inline assembly to clobber hard registers */
        /* This increases register pressure significantly */
        asm volatile (
            "# Clobber multiple registers\n"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15"
        );
        
        /* Opaque function call prevents optimization */
        int opaque = get_external_value();
        
        /* More complex arithmetic with opaque value */
        result += (opaque % 100) * t3 + (int)(f2 * 100.0f);
        
        /* Loop-carried dependency with volatile */
        vol_a = (vol_a + 1) % 100;
        vol_b = (vol_b + opaque) % 100;
    }
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int iterations) {
    int i, result = seed;
    
    /* Switch statement to create multiple basic blocks */
    for (i = 0; i < iterations; i++) {
        int computed = (vol_c * vol_d + i) % 10;
        
        switch (computed) {
            case 0:
            case 1:
            case 2: {
                /* Complex expression in one branch */
                int temp = (vol_a * vol_b + vol_c * vol_d) / 
                          (vol_a + vol_b + 1);
                result += temp * 3;
                break;
            }
            case 3:
            case 4:
            case 5: {
                /* Different complex expression */
                int temp = (vol_c - vol_d) * (vol_a + vol_b) % 100;
                result += temp / 2;
                break;
            }
            default: {
                /* Yet another expression */
                int temp = (vol_a % vol_b) + (vol_c % vol_d) * i;
                result += temp + computed;
                break;
            }
        }
        
        /* More inline assembly with clobbers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# More register clobbering\n"
            :
            :
            : "memory", "cc",
              "r16", "r17", "r18", "r19", "r20"
        );
        #endif
        
        /* Volatile access in loop prevents optimizations */
        vol_c = (vol_c + result) % 256;
    }
    
    return result;
}

/* Function to simulate external value */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int seed = time(NULL);
    int result1, result2, final_result;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Initialize volatile variables with random values */
    vol_a = rand() % 100 + 1;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100 + 1;
    vol_d = rand() % 100 + 1;
    vol_f1 = (float)(rand() % 100) / 10.0f;
    vol_f2 = (float)(rand() % 100) / 10.0f;
    vol_f3 = (float)(rand() % 100) / 10.0f;
    
    /* Call stress functions multiple times from different contexts */
    result1 = stress_computation(seed, iterations);
    result2 = stress_computation2(seed, iterations / 2);
    
    /* Combine results to prevent elimination */
    final_result = result1 + result2 * 3;
    
    /* Additional calls to increase coverage */
    for (int i = 0; i < 5; i++) {
        final_result += stress_computation(seed + i, iterations / 10);
    }
    
    printf("Result: %d (seed: %d, iterations: %d)\n", 
           final_result, seed, iterations);
    
    return final_result != 0 ? 0 : 1;
}
