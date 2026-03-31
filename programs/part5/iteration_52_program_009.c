/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register patterns */
#ifdef __OPTIMIZE__
__attribute__((noinline))
#endif
int stress_computation(volatile int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 7;
    volatile int v4 = seed - 3;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call creates unanalyzable value */
        int opaque = get_opaque_value(seed + i);
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque ? opaque : 1);
        int t2 = t1 - v3 % (opaque + 1);
        int t3 = t2 * v4 + opaque * opaque;
        int t4 = t3 / (v1 + 1) - t2 % (v2 + 1);
        int t5 = t4 * t3 + t1 / (t2 ? t2 : 1);
        
        /* Multi-use temporary value used in different contexts */
        int base = t5 * 31 + 17;
        
        /* Address computation with multiple offsets */
        static int array[256];
        int *ptr1 = &array[base % 256];
        int *ptr2 = &array[(base + 8) % 256];
        int *ptr3 = &array[(base + 16) % 256];
        int *ptr4 = &array[(base + 24) % 256];
        
        /* Use the pointers in different expressions */
        if (i % 3 == 0) {
            result += *ptr1 + t1;
        } else if (i % 3 == 1) {
            result += *ptr2 - t2;
        } else {
            result += *ptr3 * t3 - *ptr4;
        }
        
        /* Inline assembly clobbering registers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber many registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            : 
            : "r" (t1), "r" (t2), "r" (t3)
            : "r0", "r1", "r2", "memory"
        );
        #endif
        
        /* Update volatiles to prevent optimization */
        v1 += i;
        v2 -= opaque % 5;
        v3 ^= t4;
        v4 |= t5;
    }
    
    return result;
}

/* Another stress function with different pattern */
#ifdef __OPTIMIZE__
__attribute__((noinline))
#endif
int stress_computation2(volatile int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 0.75;
    double result = 0.0;
    
    /* Floating point chain */
    for (int i = 0; i < n; i++) {
        double opaque = get_opaque_value(seed + i) * 0.01;
        
        /* Complex FP expressions */
        double t1 = vd1 * opaque + vd2 / (opaque + 0.001);
        double t2 = t1 - vd1 * vd2;
        double t3 = t2 * opaque + t1 / opaque;
        double t4 = t3 * t2 - t1 * t3;
        
        /* Multi-use value in switch */
        double base = t4 * 3.14159;
        
        switch (i % 4) {
            case 0:
                result += base * 2.0;
                break;
            case 1:
                result -= base / 2.0;
                break;
            case 2:
                result *= base + 1.0;
                break;
            case 3:
                result = result / (base + 0.5);
                break;
        }
        
        vd1 += opaque;
        vd2 -= t4 * 0.1;
    }
    
    return (int)result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand to prevent compiler analysis */
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return rand() ^ seed;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int seed = 42;
    int total = 0;
    
    /* Call stress functions from multiple sites */
    for (int i = 0; i < iterations; i++) {
        seed += i;
        total += stress_computation(seed, 50);
        
        if (i % 3 == 0) {
            total -= stress_computation2(seed * 2, 30);
        } else if (i % 3 == 1) {
            total += stress_computation(seed ^ 0x55, 40);
        } else {
            total ^= stress_computation2(seed + 100, 35);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
