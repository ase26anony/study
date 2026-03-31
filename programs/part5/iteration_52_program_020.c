/* Test program to trigger virtual register creation in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to prevent optimization */
extern int opaque_func(int x);

/* Stress function with complex register patterns */
static int __attribute__((noinline)) 
stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Multi-use temporary value - candidate for rematerialization */
        int base = v1 * v2 + opaque_func(v3) - (v1 % (v2 + 1));
        
        /* Use base in multiple, spatially separated contexts */
        if (i & 1) {
            /* First use with address computation */
            int *ptr = (int*)&v1;
            result += base + *ptr + (i * 3);
            
            /* Inline assembly clobbering registers */
            asm volatile (
                "# Clobber hard registers\n"
                "mov r0, %0\n"
                "mov r1, %1\n"
                :
                : "r" (base), "r" (result)
                : "r0", "r1", "r2", "r3", "memory"
            );
        } else {
            /* Second use with different computation */
            result += base - (v2 / (v3 + 1)) + (i << 2);
            
            /* More complex arithmetic forcing register pressure */
            int temp1 = v1 * v3 + opaque_func(base);
            int temp2 = v2 * base - opaque_func(temp1);
            int temp3 = temp1 % (temp2 + 1) + v3;
            result ^= temp3;
        }
        
        /* Address computation with multiple offsets */
        int offsets[] = {1, 3, 7, 15};
        for (int j = 0; j < 4; j++) {
            /* Base address recomputation pattern */
            int *addr = (int*)&v1 + offsets[j];
            result += *addr + base * j;
            
            /* Prevent optimization with volatile */
            asm volatile ("" : : "r" (addr) : "memory");
        }
        
        /* Loop-carried dependency with volatile */
        v1 += result & 0xFF;
        v2 ^= opaque_func(v1);
        v3 = v3 * 1103515245 + 12345;
        
        /* Control flow to obstruct optimization */
        switch (i % 4) {
            case 0:
                result += base * 2;
                break;
            case 1:
                result += base / 3;
                break;
            case 2:
                result += base + opaque_func(result);
                break;
            case 3:
                result += (base << 1) | (result & 1);
                break;
        }
    }
    
    return result;
}

/* Another stress function with different patterns */
static int __attribute__((noinline))
stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 2.5;
    double result = 0.0;
    
    /* Floating-point arithmetic for different register types */
    for (int i = 0; i < n; i++) {
        /* Complex FP expression creating temporaries */
        double temp = vd1 * vd2 + (vd1 / (vd2 + 1.0)) - 
                     fmod(vd1, vd2 + 2.0);
        
        /* Multi-use in different contexts */
        if (opaque_func(i) & 1) {
            result += temp * sin(i * 0.1);
        } else {
            result += temp / cos(i * 0.05);
        }
        
        /* More register pressure */
        vd1 = vd1 * 1.1 + result;
        vd2 = vd2 * 0.9 - result;
        
        /* Inline assembly with FP clobbers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# FP register clobber\n"
            "fld d0, %0\n"
            "fld d1, %1\n"
            :
            : "m" (vd1), "m" (vd2)
            : "d0", "d1", "d2", "d3", "memory"
        );
        #endif
    }
    
    return (int)result;
}

/* Opaque function implementation */
int opaque_func(int x) {
    /* Use system time to prevent constant propagation */
    static int counter = 0;
    return (x ^ counter++) + 1;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int seed = 12345;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 12345;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(seed);
    
    /* Initialize volatile data */
    volatile int init_val = rand();
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    /* First call site */
    total += stress_computation(init_val, iterations);
    
    /* Change volatile values between calls */
    init_val = rand();
    
    /* Second call site with different parameters */
    total += stress_computation(init_val, iterations / 2);
    
    /* Third call site with different function */
    init_val = rand();
    total += stress_computation2(init_val, iterations);
    
    /* Fourth call site in loop */
    for (int i = 0; i < 3; i++) {
        init_val = rand();
        total += stress_computation(init_val, 10 + i * 5);
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (seed: %d, iterations: %d)\n", 
           total, seed, iterations);
    
    /* Use result to affect control flow */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return total & 0xFF;
}
