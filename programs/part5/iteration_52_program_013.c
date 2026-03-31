/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE_ASM 1
#else
#define AGGRESSIVE_ASM 0
#endif

/* Complex arithmetic with volatile variables to prevent optimization */
static int complex_volatile_chain(volatile int a, volatile int b, 
                                  volatile int c, volatile int d,
                                  volatile int e, volatile int f) {
    /* Long dependency chain with temporaries */
    int t1 = a * b + c;
    int t2 = d / (e | 1);  /* Avoid division by zero */
    int t3 = f % (b | 1);  /* Avoid modulo by zero */
    int t4 = t1 - t2;
    int t5 = t4 * t3;
    int t6 = t5 + a;
    int t7 = t6 - b;
    int t8 = t7 * c;
    int t9 = t8 / (d | 1);
    int t10 = t9 % (e | 1);
    
    /* Multi-use temporary value */
    int base = t10 * t5;
    
    /* Use base in multiple contexts */
    int result = 0;
    if (base > 1000) {
        result = base + t1 + t2;
    } else if (base > 500) {
        result = base - t3 - t4;
    } else {
        result = base * t5 / (t6 | 1);
    }
    
    /* Another use of base */
    result += base % 256;
    
    return result;
}

/* Function with inline assembly clobbering registers */
static int asm_clobber_stress(int x, int y, int z) {
    int result = x;
    
    #if AGGRESSIVE_ASM
    /* Clobber multiple hard registers to increase pressure */
    asm volatile (
        "nop\n\t"
        : 
        : "r"(x), "r"(y), "r"(z)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    #endif
    
    /* Complex computation after clobber */
    result = result * y + z;
    result = result / (y | 1) * x;
    result = result - z + y;
    result = result % (x | 1) * z;
    
    return result;
}

/* Address computation with multiple offsets */
static int address_computation_stress(int *array, int index, int n) {
    if (!array || n <= 0) return 0;
    
    /* Compute base address (candidate for rematerialization) */
    int *base = &array[index % n];
    
    /* Use with multiple offsets */
    int sum = 0;
    sum += base[0];    /* offset 0 */
    sum += base[1];    /* offset 1 */
    sum += base[-1];   /* offset -1 */
    sum += base[2];    /* offset 2 */
    sum += base[-2];   /* offset -2 */
    
    /* Another computation with same base */
    int diff = base[0] - base[1] + base[-1];
    
    return sum + diff;
}

/* Main stress function with loop-carried dependencies */
int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed + 1;
    volatile int v3 = seed + 2;
    volatile int v4 = seed + 3;
    volatile int v5 = seed + 4;
    volatile int v6 = seed + 5;
    
    int result = 0;
    int *array = malloc(n * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array with opaque values */
    for (int i = 0; i < n; i++) {
        array[i] = rand() % 1000;  /* Opaque function call */
    }
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int loop_bound = n;
    for (volatile int i = 0; i < loop_bound && i < 10; i++) {
        /* Mix different stress patterns */
        int temp = complex_volatile_chain(v1 + i, v2, v3, v4, v5, v6);
        
        #if AGGRESSIVE_ASM
        temp = asm_clobber_stress(temp, v1, v2);
        #endif
        
        temp += address_computation_stress(array, i, n);
        
        /* Use result in conditional to create control flow */
        if (temp % 2 == 0) {
            result += temp * 2;
        } else {
            result += temp / 2;
        }
        
        /* Modify volatiles to create dependencies */
        v1 += i % 3;
        v2 += temp % 5;
        v3 = v1 * v2;
    }
    
    /* Additional complex expression with temporaries */
    int final_temp = v1 * v2 + v3 / (v4 | 1) - v5 % (v6 | 1);
    final_temp = final_temp * v1 - v2;
    final_temp = final_temp / (v3 | 1) + v4;
    
    result += final_temp;
    
    free(array);
    return result;
}

/* Secondary stress function with different pattern */
int stress_computation2(int seed, int n) {
    int result = seed;
    
    /* Switch with multi-use temporaries */
    switch (seed % 4) {
        case 0: {
            int base = seed * 3 + 7;
            result = base * 2;
            result += base % 10;  /* Second use */
            break;
        }
        case 1: {
            int base = seed / 2 + 5;
            result = base - 3;
            result *= base % 8;   /* Second use */
            break;
        }
        case 2: {
            int base = seed + 100;
            result = base / 4;
            result += base * 2;   /* Second use */
            break;
        }
        default: {
            int base = seed - 50;
            result = base % 20;
            result = base - result; /* Second use */
            break;
        }
    }
    
    /* Chain of computations */
    for (int i = 0; i < n && i < 5; i++) {
        int t1 = result * i;
        int t2 = t1 + seed;
        int t3 = t2 / (i + 1);
        int t4 = t3 % 256;
        result = t4 * 2 - t1 + t2 - t3;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    int total_result = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < iterations; i++) {
        int seed = rand() % 1000;
        
        /* Alternate between different stress functions */
        if (i % 2 == 0) {
            total_result += stress_computation(seed, 20);
        } else {
            total_result += stress_computation2(seed, 15);
        }
        
        /* Additional call site with different parameters */
        total_result += stress_computation(seed + 1, 10);
    }
    
    printf("Result: %d\n", total_result);
    
    /* Verify result isn't optimized away */
    if (total_result == 0) {
        printf("Warning: Result was zero\n");
    }
    
    return 0;
}
