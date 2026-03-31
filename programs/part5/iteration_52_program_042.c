/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Stress function with complex register pressure patterns */
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = n;
    volatile int v3 = seed * 2;
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call prevents constant propagation */
        int opaque = get_external_value() + i;
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque + 1);
        int t2 = t1 - v3 % (opaque ? opaque : 1);
        int t3 = t2 * t2 - t1;
        int t4 = t3 + (v1 ^ v2) | (v3 & opaque);
        int t5 = t4 << (opaque % 8);
        int t6 = t5 >> (v1 % 8);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = t6 * opaque - v2;
        
        /* Use base in multiple separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result -= base / 2;
        } else {
            result ^= base;
        }
        
        /* Address computation with multiple offsets */
        char buffer[256];
        char *ptr = &buffer[i % 128];
        
        /* Multiple uses of ptr with different offsets */
        ptr[0] = (char)(base & 0xFF);
        ptr[1] = (char)((base >> 8) & 0xFF);
        ptr[2] = (char)((base >> 16) & 0xFF);
        ptr[-1] = (char)(result & 0xFF);
        
        /* Inline assembly clobbering registers to increase pressure */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Dummy assembly clobbering registers\n"
            : 
            : "r"(opaque), "r"(base)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* Prevent loop unrolling with volatile loop counter update */
        v1 += (i & 1);
    }
    
    return result;
}

/* Another stress function with different pattern */
static int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = n * 0.75;
    double result = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Complex floating-point chain */
        double t1 = vd1 * i + vd2;
        double t2 = t1 / (vd1 + 1.0);
        double t3 = t2 - vd2 * 0.5;
        double t4 = t3 * t3 + t1;
        double t5 = t4 / (t2 + 1.0);
        
        /* Multi-use floating temporary */
        double base = t5 * vd1 - vd2;
        
        /* Use in different expressions */
        if (i % 2 == 0) {
            result += base * 2.0;
        } else {
            result -= base / 2.0;
        }
        
        /* More register pressure with integer conversions */
        int int_base = (int)base;
        result += (double)(int_base % 100);
        
        #ifdef __OPTIMIZE__
        /* Clobber floating point registers too */
        asm volatile (
            "# Clobber floating registers\n"
            : 
            : "f"(base), "f"(result)
            : "fr0", "fr1", "fr2", "fr3", "fr4", "memory"
        );
        #endif
    }
    
    return (int)result;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return rand() ^ (counter++);
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
        total_result += stress_computation2(seed + i * 2, iterations / 2);
        
        /* Vary the call pattern */
        if (i % 2 == 0) {
            total_result ^= stress_computation(seed - i, iterations / 3);
        }
    }
    
    /* Additional complex expression in main to increase overall pressure */
    volatile int vmain = total_result;
    for (int j = 0; j < 20; j++) {
        vmain = vmain * 1103515245 + 12345;
        vmain = (vmain / 65536) % 32768;
    }
    
    printf("Result: %d (seed: %d, iterations: %d)\n", 
           total_result ^ vmain, seed, iterations);
    
    return 0;
}
