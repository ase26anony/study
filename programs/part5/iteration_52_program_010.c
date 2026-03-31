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
        /* Force register pressure with complex expression */
        int t1 = v1 * v2 + v3 / (v4 + 1) - (v2 % (v3 + 1));
        int t2 = v2 * v3 + v4 / (v1 + 1) - (v3 % (v4 + 1));
        int t3 = v3 * v4 + v1 / (v2 + 1) - (v4 % (v1 + 1));
        int t4 = v4 * v1 + v2 / (v3 + 1) - (v1 % (v2 + 1));
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = t1 * t2 - t3 + t4;
        
        /* Use base in multiple different contexts */
        if (i % 4 == 0) {
            result += base * 3;
        } else if (i % 4 == 1) {
            result += base / 2;
        } else if (i % 4 == 2) {
            result -= base * 2;
        } else {
            result ^= base;
        }
        
        /* Address computation with multiple offsets */
        int array[8];
        for (int j = 0; j < 4; j++) {
            /* Base address computation that might be rematerialized */
            int* ptr = &array[j];
            result += *(ptr + 0) + *(ptr + 1) + *(ptr + 2);
        }
        
        /* Inline assembly to clobber registers and increase pressure */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber multiple registers\n\t"
            : 
            : "r"(result), "r"(t1), "r"(t2), "r"(t3), "r"(t4)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* Opaque function call prevents optimization */
        v1 = get_opaque_value(v1 + i);
        v2 = get_opaque_value(v2 + i);
        v3 = get_opaque_value(v3 + i);
        v4 = get_opaque_value(v4 + i);
    }
    
    return result;
}

/* Another stress function with different patterns */
#ifdef __OPTIMIZE__
__attribute__((noinline))
#endif
int stress_arithmetic(volatile int a, volatile int b, volatile int c, 
                      volatile int d, volatile int e, volatile int f) {
    /* Very complex expression creating dependency chain */
    int temp = a * b + c / (d + 1) - e % (f + 1);
    temp = temp * b - c / (a + 1) + d % (e + 1);
    temp = temp + a * c - b / (d + 1) + e % (f + 1);
    temp = temp - b * d + c / (e + 1) - a % (f + 1);
    temp = temp * c * d - e / (f + 1) + b % (a + 1);
    
    /* Use temp in switch with multiple cases */
    int result = 0;
    switch (temp % 5) {
        case 0:
            result = temp * 2 + a;
            break;
        case 1:
            result = temp / 2 + b;
            break;
        case 2:
            result = temp * 3 - c;
            break;
        case 3:
            result = temp + d * e;
            break;
        case 4:
            result = temp - f * a;
            break;
    }
    
    /* Additional complex computation */
    for (int i = 0; i < 3; i++) {
        result += (a * b + c * d - e * f) / (i + 1);
        result -= (b * c + d * e - f * a) % (i + 2);
    }
    
    return result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system time to prevent compile-time evaluation */
    static int counter = 0;
    return (seed * 1103515245 + 12345 + counter++) ^ (int)clock();
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    int total_result = 0;
    
    /* Call stress functions multiple times with varying inputs */
    for (int i = 0; i < iterations; i++) {
        volatile int v1 = rand() % 100 + 1;
        volatile int v2 = rand() % 100 + 1;
        volatile int v3 = rand() % 100 + 1;
        volatile int v4 = rand() % 100 + 1;
        
        /* Mix both stress functions */
        int r1 = stress_computation(v1, 10 + (i % 5));
        int r2 = stress_arithmetic(v1, v2, v3, v4, v1 + v2, v3 + v4);
        
        total_result += r1 + r2;
        
        /* Prevent loop optimization */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
