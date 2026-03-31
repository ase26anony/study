/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Stress function with complex register patterns */
#ifdef __OPTIMIZE__
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = n;
    volatile int v3 = seed * 2;
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Force register pressure with long dependency chain */
        int t1 = v1 * v2 + v3 / (v2 + 1);
        int t2 = t1 % (v3 + 1) - v1;
        int t3 = t2 * t2 + v2;
        int t4 = t3 / (v1 + 1) - t2;
        int t5 = t4 % (v3 + 2) * t1;
        int t6 = t5 + t3 - t4;
        
        /* Multi-use temporary value used in different contexts */
        int base = t6 * v1 - v2;
        
        /* Use base in multiple separated computations */
        if (i % 3 == 0) {
            result += base * 2 + t1;
        } else if (i % 3 == 1) {
            result += base / 2 - t2;
        } else {
            result += base % 256 + t3;
        }
        
        /* Inline assembly clobbering registers */
        asm volatile (
            "# Clobber registers to increase pressure\n\t"
            : 
            : "r"(t1), "r"(t2), "r"(t3)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        
        /* Address computation with multiple offsets */
        int array[8];
        for (int j = 0; j < 4; j++) {
            /* Base address computation that might be rematerialized */
            int* ptr = &array[j];
            result += *(ptr + 0) + *(ptr + 1) + *(ptr + 2);
        }
        
        /* Opaque function call preventing optimization */
        v1 = get_external_value() % 100;
    }
    
    return result;
}
#else
/* Simple version for -O0 */
static int stress_computation(int seed, int n) {
    return seed + n;
}
#endif

/* Another stress function with different pattern */
static int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = n * 0.7;
    double result = 0.0;
    
    /* Floating-point complex expressions */
    for (int i = 0; i < n; i++) {
        double d1 = vd1 * vd2 + vd1 / (vd2 + 1.0);
        double d2 = d1 * d1 - vd2;
        double d3 = d2 / (vd1 + 1.0) * d1;
        double d4 = d3 - d2 + vd1;
        
        /* Multi-use value in switch statement */
        double base = d4 * vd1 - vd2;
        
        switch (i % 4) {
            case 0:
                result += base * 2.0;
                break;
            case 1:
                result += base / 2.0;
                break;
            case 2:
                result += base + d1;
                break;
            case 3:
                result += base - d2;
                break;
        }
        
        /* More register pressure */
        asm volatile (
            "# More clobbering\n\t"
            : 
            : "r"((int)d1), "r"((int)d2)
            : "r6", "r7", "r8", "r9", "r10", "memory"
        );
    }
    
    return (int)result;
}

/* Function with loop-carried volatile dependencies */
static int volatile_loop_computation(int iterations) {
    volatile int counter = 0;
    volatile int increment = 1;
    int sum = 0;
    
    for (volatile int i = 0; i < iterations; i = i + increment) {
        /* Complex expression with volatile operands */
        int temp = (counter * 3 + i) % 256;
        temp = temp * temp - counter;
        temp = temp / (i + 1) + counter;
        
        /* Use in multiple ways */
        sum += temp;
        sum -= temp / 2;
        sum += temp % 16;
        
        counter = counter + increment;
        
        /* Prevent optimization with external call */
        if (i % 7 == 0) {
            sum += rand() % 10;
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 100;
    int seed = time(NULL);
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    printf("Starting stress tests...\n");
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    for (int i = 0; i < 3; i++) {
        total += stress_computation(seed + i, n / 3);
        total += stress_computation2(seed * (i + 1), n / 4);
        total += volatile_loop_computation(n / 5);
    }
    
    /* Additional calls with different parameters */
    if (n > 50) {
        total += stress_computation(total, 25);
        total += stress_computation2(total * 2, 20);
    }
    
    printf("Result: %d\n", total);
    printf("Seed: %d, Iterations: %d\n", seed, n);
    
    return total != 0 ? 0 : 1;
}

/* Dummy external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ + rand() % 100;
}
