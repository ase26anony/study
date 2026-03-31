/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* External function to create opaque values */
extern int rand(void);

/* Complex arithmetic with long dependency chain */
static int complex_chain(int a, int b, int c, int d, int e, int f, int g) {
    /* Multi-step computation creating many temporaries */
    int t1 = a * b + c;
    int t2 = d / (e + 1);
    int t3 = f % (g + 2);
    int t4 = t1 - t2;
    int t5 = t4 * t3;
    int t6 = t5 + (a << 2);
    int t7 = t6 - (b >> 1);
    
    /* Use inline assembly to clobber registers */
    asm volatile (
        "# Clobber multiple registers\n"
        : 
        : "r"(a), "r"(b), "r"(c)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    return t7 * 2;
}

/* Function with multi-use temporary values in different control flow paths */
static int multi_use_temp(int x, int y, int selector) {
    /* Compute value once, use in multiple places */
    int base = x * y + vol_a;
    
    switch (selector & 3) {
        case 0:
            return base + vol_b;
        case 1:
            return base - vol_c;
        case 2:
            return base * 2 + vol_d;
        case 3:
            return (base >> 1) - vol_a;
        default:
            return base;
    }
}

/* Address computation with multiple offsets */
static int address_computation(int *array, int index, int n) {
    /* Base address computation that might be rematerialized */
    int *base = &array[index];
    
    /* Use with different offsets */
    int sum = base[0] + base[1] + base[2] + base[3];
    sum += base[-1] + base[-2];
    sum += base[n] + base[n * 2];
    
    return sum;
}

/* Main stress function with loop-carried dependencies */
int stress_computation(int seed, int n) {
    int result = seed;
    volatile int vol_counter = n; /* Prevent loop optimizations */
    
    /* Loop with volatile bound to prevent unrolling */
    for (volatile int i = 0; i < vol_counter; i = i + 1) {
        /* Complex floating-point chain */
        float f1 = vol_f1 * vol_f2 + vol_f3 / (vol_f1 + 1.0f);
        float f2 = f1 * 2.5f - vol_f2 / 1.5f;
        float f3 = f2 + vol_f3 * 3.0f;
        
        /* Integer chain with function calls */
        int r1 = rand() % 100;
        int r2 = rand() % 100;
        int r3 = rand() % 100;
        
        /* Multi-use temporary pattern */
        int temp = complex_chain(r1, r2, r3, vol_a, vol_b, vol_c, vol_d);
        
        /* Use temp in different ways */
        if (i & 1) {
            result += temp * 2;
        } else {
            result -= temp / 2;
        }
        
        /* More arithmetic to increase register pressure */
        result += (r1 * r2) / (r3 + 1);
        result -= (r1 % (r2 + 1)) * 3;
        
        /* Address computation pattern */
        static int local_array[100];
        result += address_computation(local_array, i % 50, n % 10);
        
        /* Another inline asm to clobber registers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# More register clobbering\n"
            : 
            : "r"(result), "r"(i)
            : "r6", "r7", "r8", "r9", "r10", "memory"
        );
        #endif
    }
    
    /* Final multi-use of values computed earlier */
    int final_temp = result * vol_a + vol_b;
    result = multi_use_temp(final_temp, vol_c, n);
    
    return result;
}

/* Secondary stress function with different pattern */
int stress_computation2(int seed, int n) {
    int result = 0;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < n; i++) {
        volatile int inner_bound = (i % 5) + 1;
        
        for (volatile int j = 0; j < inner_bound; j++) {
            /* Complex expression chain */
            int a = rand() % 256;
            int b = rand() % 256;
            int c = rand() % 256;
            
            /* Long dependency chain */
            int val = a * b + c;
            val = val / (a + 1) * (b + 2);
            val = val - (c << 1) + (a >> 1);
            val = val % 1000 + (b % 50);
            
            /* Multi-use in conditionals */
            if (val > 500) {
                result += val * 2;
            } else if (val > 200) {
                result += val / 2;
            } else {
                result += val + 100;
            }
        }
        
        /* Function call barrier */
        result += rand() % 100;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
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
    
    int total_result = 0;
    
    /* Call stress functions multiple times */
    for (int i = 0; i < iterations; i++) {
        total_result += stress_computation(rand() % 100, i % 20 + 1);
        
        if (i % 3 == 0) {
            total_result += stress_computation2(rand() % 100, i % 10 + 1);
        }
        
        /* Modify volatiles to change patterns */
        vol_a = (vol_a + 1) % 10;
        vol_b = (vol_b + 3) % 10;
        vol_f1 = vol_f1 + 0.1f;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total_result % 1000000);
    
    return 0;
}
