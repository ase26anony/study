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

/* Inline assembly to clobber registers */
static inline void clobber_registers(void) {
#if USE_AGGRESSIVE_PATTERNS
    /* Clobber multiple registers to increase pressure */
    asm volatile (
        "nop\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
#endif
}

/* Complex arithmetic with volatile variables and register clobbering */
static int complex_arithmetic(int seed, int iterations) {
    int result = seed;
    volatile int vol_counter = iterations; /* Prevent loop optimizations */
    
    for (int i = 0; i < vol_counter; i++) {
        /* Create long dependency chain with temporaries */
        int t1 = vol_a * vol_b + rand() % 100;
        int t2 = vol_c / (vol_d + 1) + rand() % 50;
        int t3 = t1 - t2 * (rand() % 10);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = t3 * vol_a + vol_b * vol_c - vol_d;
        
        /* Use base in multiple, spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
            clobber_registers();
        } else if (i % 3 == 1) {
            result -= base / 2;
            /* More complex expression chain */
            int t4 = base + vol_a * vol_b - vol_c / (vol_d + 1);
            result ^= t4;
        } else {
            result |= base & 0xFF;
            /* Another dependency chain */
            int t5 = (base << 2) | (vol_a & 0xF);
            int t6 = t5 * (vol_b + 1) % (vol_c + 1);
            result += t6;
        }
        
        /* Floating point chain to create FP virtual registers */
        float f1 = vol_f1 * vol_f2 + (float)rand() / RAND_MAX;
        float f2 = vol_f3 / (f1 + 1.0f) - (float)(rand() % 100) * 0.01f;
        float f3 = f1 * f2 - vol_f1 / vol_f2 + vol_f3;
        
        /* Use floating result in integer computation */
        result += (int)(f3 * 100.0f);
        
        /* Address computation with multiple offsets - candidate for base reg rematerialization */
        int array[16];
        for (int j = 0; j < 4; j++) {
            /* Compute base address once, use with different offsets */
            int *base_ptr = &array[j * 4];
            
            /* Multiple uses of base_ptr with different offsets */
            base_ptr[0] = result + j;
            base_ptr[1] = base_ptr[0] * 2;
            base_ptr[2] = base_ptr[1] - base;
            base_ptr[3] = base_ptr[2] ^ t3;
            
            result += array[j];
        }
        
        /* Prevent compiler from moving computations out of loop */
        vol_a = (vol_a + 1) % 100;
        vol_b = (vol_b * 3) % 100;
    }
    
    return result;
}

/* Another stress function with different patterns */
static int stress_address_computation(int n) {
    int result = 0;
    volatile int vol_n = n; /* Prevent optimization */
    
    int large_array[256];
    
    /* Initialize array with opaque values */
    for (int i = 0; i < 256; i++) {
        large_array[i] = rand() % 1000;
    }
    
    for (int i = 0; i < vol_n; i++) {
        /* Complex index computation creating many temporaries */
        int idx1 = (i * vol_a + vol_b) % 256;
        int idx2 = (i * vol_c - vol_d) % 256;
        int idx3 = (idx1 * idx2 + rand()) % 256;
        
        /* Multi-step address computation */
        int *ptr1 = &large_array[idx1];
        int *ptr2 = &large_array[idx2];
        int *ptr3 = &large_array[idx3];
        
        /* Use pointers with different offsets in separate basic blocks */
        if (i % 2 == 0) {
            result += ptr1[0] * 3;
            result -= ptr2[1] * 2;
            result |= ptr3[2] & 0xFF;
        } else {
            result ^= ptr1[3];
            result += ptr2[4] >> 2;
            result *= ptr3[5] | 1;
        }
        
        /* Nested complex expression */
        int temp = (ptr1[0] * ptr2[1]) / (ptr3[2] + 1);
        temp += (vol_a * vol_b - vol_c * vol_d) % 100;
        
        /* Switch with multiple cases using same computed value */
        switch (temp % 4) {
            case 0:
                result += temp * 2;
                break;
            case 1:
                result -= temp / 2;
                break;
            case 2:
                result ^= temp;
                /* Additional computation in this case */
                result += (temp << 3) | (vol_a & 0xF);
                break;
            case 3:
                result |= temp & 0xFF;
                /* Force another computation */
                result += (vol_b * temp) % (vol_c + 1);
                break;
        }
        
        clobber_registers();
    }
    
    return result;
}

/* Main test harness */
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
    
    srand(seed);
    
    /* Initialize volatile variables */
    vol_a = (rand() % 100) + 1;
    vol_b = (rand() % 100) + 1;
    vol_c = (rand() % 100) + 1;
    vol_d = (rand() % 100) + 1;
    
    vol_f1 = (float)(rand() % 100) / 10.0f;
    vol_f2 = (float)(rand() % 100) / 10.0f;
    vol_f3 = (float)(rand() % 100) / 10.0f;
    
    printf("Starting stress tests...\n");
    
    /* Call stress functions multiple times from different contexts */
    int total_result = 0;
    
    for (int run = 0; run < 3; run++) {
        /* Vary parameters slightly each run */
        int run_iterations = iterations + run * 10;
        
        /* First stress function */
        int result1 = complex_arithmetic(seed + run, run_iterations);
        printf("Run %d, complex_arithmetic result: %d\n", run, result1);
        total_result += result1;
        
        /* Second stress function */
        int result2 = stress_address_computation(run_iterations / 2);
        printf("Run %d, stress_address_computation result: %d\n", run, result2);
        total_result ^= result2;
        
        /* Modify volatile variables between calls */
        vol_a = (vol_a * 3) % 100;
        vol_b = (vol_b + 5) % 100;
        vol_c = (vol_c * 7) % 100;
        vol_d = (vol_d - 2) % 100;
        if (vol_d < 1) vol_d = 1;
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Ensure result is used */
    return total_result == 0 ? 1 : 0;
}
