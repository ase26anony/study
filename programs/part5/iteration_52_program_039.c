/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Stress function with complex register patterns */
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = n;
    volatile int v3 = seed * 2;
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call creates unanalyzable value */
        int opaque = get_external_value() + i;
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque + 1);
        int t2 = t1 - v3 % (opaque | 1);
        int t3 = t2 * (v1 ^ v2) + (v3 & opaque);
        int t4 = t3 / (v2 + 1) - (v1 % (opaque + 2));
        int t5 = t4 * t3 - t2 / (t1 + 1);
        
        /* Multi-use temporary value */
        int base = t5 * opaque + v1 - v2;
        
        /* Use base in multiple contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + opaque;
        }
        
        /* Address computation with multiple offsets */
        char buffer[256];
        char *ptr = &buffer[opaque % 128];
        
        /* Multiple offset uses - potential for base address rematerialization */
        ptr[0] = (base >> 0) & 0xFF;
        ptr[1] = (base >> 8) & 0xFF;
        ptr[2] = (base >> 16) & 0xFF;
        ptr[3] = (base >> 24) & 0xFF;
        
        /* Inline assembly clobbering registers */
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
        
        /* Prevent loop unrolling with volatile */
        v1 += 1;
        if (v1 > 1000) v1 = seed;
    }
    
    return result;
}

/* Another stress function with different pattern */
static int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = n * 0.7;
    double result = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Complex floating-point chain */
        double d1 = vd1 * i + vd2 / (i + 1.0);
        double d2 = d1 - vd1 * vd2;
        double d3 = d2 * d1 + vd2 / d1;
        double d4 = d3 - d2 / d1 * vd1;
        
        /* Multi-use floating temporary */
        double base = d4 * vd1 + d3 * vd2 - d2;
        
        /* Use in multiple expressions */
        switch (i % 4) {
            case 0:
                result += base * 1.1;
                break;
            case 1:
                result += base / 1.1;
                break;
            case 2:
                result += base + vd1;
                break;
            case 3:
                result += base - vd2;
                break;
        }
        
        /* More register pressure */
        double t1 = base * 2.0;
        double t2 = base / 2.0;
        double t3 = base + t1;
        double t4 = base - t2;
        result += (t3 + t4) * 0.5;
        
        #ifdef __OPTIMIZE__
        /* Clobber floating point registers too */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7"
        );
        #endif
        
        /* Volatile update prevents optimization */
        vd1 += 0.1;
        if (vd1 > 100.0) vd1 = seed * 1.5;
    }
    
    return (int)result;
}

/* Function to simulate external opaque function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7FFF;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed <= 0) seed = 12345;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(seed);
    
    /* Initialize volatile data */
    volatile int init_val = seed;
    volatile int step = 3;
    
    int total = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 10; i++) {
        int n = 50 + (rand() % 50);
        
        if (i % 2 == 0) {
            total += stress_computation(init_val + i, n);
        } else {
            total += stress_computation2(init_val - i, n);
        }
        
        /* Change volatile to affect next iteration */
        init_val += step;
        if (init_val > 10000) init_val = seed;
    }
    
    /* Additional complex expression in main to increase pressure */
    int a = total, b = seed, c = iterations;
    int complex_result = ((a * b) + (b / c) - (a % (c + 1))) * 
                        ((b * c) - (c / (a + 1)) + (b % (a | 1)));
    
    printf("Result: %d (complex: %d)\n", total, complex_result);
    
    return total != 0 ? 0 : 1;
}
