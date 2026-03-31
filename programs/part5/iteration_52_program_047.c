/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE 1
#else
#define AGGRESSIVE 0
#endif

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register pressure patterns */
int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic with long dependency chain */
    for (int i = 0; i < n; i++) {
        /* Prevent optimization with volatile */
        int a = v1 + i;
        int b = v2 * (i + 1);
        int c = v3 / (i + 2);
        int d = v4 % (i + 3);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = a * b + c - d;
        
        /* Use base in multiple spatially separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base % 100;
        }
        
        /* More complex arithmetic creating many temporaries */
        int e = get_opaque_value(i);
        int f = get_opaque_value(i + seed);
        
        /* Long expression chain - many virtual registers needed */
        int temp1 = a * b * c * d;
        int temp2 = e * f * base;
        int temp3 = temp1 % (temp2 + 1);
        int temp4 = (temp1 * temp2) / (temp3 + 1);
        int temp5 = temp3 ^ temp4;
        
        /* Use temporaries in address computations */
        int* dummy_array = (int*)malloc(100 * sizeof(int));
        if (dummy_array) {
            /* Multiple offsets from computed base */
            int offset1 = temp1 % 50;
            int offset2 = temp2 % 50;
            int offset3 = temp3 % 50;
            
            dummy_array[offset1] = temp4;
            dummy_array[offset2] = temp5;
            dummy_array[offset3] = base;
            
            result += dummy_array[offset1] + dummy_array[offset2];
            free(dummy_array);
        }
        
        /* Inline assembly with clobbers to reduce available registers */
        #if AGGRESSIVE
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            "mov %0, %0\n"
            :
            : "r" (result)
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        #endif
        
        /* Loop-carried dependency with volatile */
        v1 += i % 5;
        v2 -= i % 3;
        v3 *= (i % 7) + 1;
        v4 /= (i % 4) + 1;
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 0.75;
    
    double result = 0.0;
    
    /* Floating-point complex expressions */
    for (int i = 0; i < n; i++) {
        double a = vd1 + i * 0.1;
        double b = vd2 - i * 0.2;
        double c = get_opaque_value(i) * 0.01;
        
        /* Complex FP expression - many temporary FP registers */
        double temp = (a * b) + (c / (a + 1.0)) - (b * c) + 
                     (a / (b + 1.0)) * (c * c) - 
                     ((a + b) / (c + 1.0)) + 
                     (a * a * b * c) / 1000.0;
        
        /* Multi-use of computed value */
        if (temp > 0) {
            result += temp * 2.0;
        } else {
            result += temp / 2.0;
        }
        
        /* Address computation with multiple offsets */
        static double array[100];
        int idx1 = get_opaque_value(i) % 100;
        int idx2 = get_opaque_value(i + seed) % 100;
        int idx3 = (i * seed) % 100;
        
        array[idx1] = temp;
        array[idx2] = result;
        array[idx3] = a + b + c;
        
        result += array[idx1] + array[idx2] - array[idx3];
        
        /* Update volatiles to prevent optimization */
        vd1 *= 1.01;
        vd2 /= 1.01;
    }
    
    return (int)result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use rand() to create compiler-opaque values */
    static int initialized = 0;
    if (!initialized) {
        srand(seed);
        initialized = 1;
    }
    return rand() % 1000;
}

/* Main test harness */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    srand(seed);
    
    int total_result = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 5; i++) {
        total_result += stress_computation(seed + i, iterations);
        total_result += stress_computation2(seed + i * 100, iterations / 2);
        
        /* Vary the call pattern */
        if (i % 2 == 0) {
            total_result += stress_computation(rand() % 1000, iterations / 3);
        } else {
            total_result += stress_computation2(rand() % 1000, iterations / 4);
        }
    }
    
    /* Additional complex expression in main to increase pressure */
    volatile int main_volatile = total_result;
    for (int i = 0; i < 10; i++) {
        int a = main_volatile * i;
        int b = get_opaque_value(i) * a;
        int c = (a % 17) * (b % 23);
        int d = (c ^ a) | (b & 0xFF);
        
        /* Multi-use temporary in switch */
        int computed = a * b + c - d;
        switch (i % 4) {
            case 0: total_result += computed * 3; break;
            case 1: total_result += computed / 3; break;
            case 2: total_result += computed % 100; break;
            case 3: total_result += computed ^ 0x55; break;
        }
        
        main_volatile += i;
    }
    
    printf("Result: %d\n", total_result % 1000000);
    return 0;
}
