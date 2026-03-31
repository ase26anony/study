/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1;
volatile int vol_b = 2;
volatile int vol_c = 3;
volatile int vol_d = 4;
volatile int vol_e = 5;
volatile int vol_f = 6;
volatile int vol_g = 7;

/* Stress function with complex arithmetic and register pressure */
#ifdef __OPTIMIZE__
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Complex arithmetic expression creating many temporaries */
    int base = vol_a * vol_b + vol_c / vol_d - vol_e % vol_f;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int temp1 = base * seed + vol_g;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        : 
        : "r" (temp1), "r" (base), "r" (seed)
        : "r0", "r1", "r2", "memory"
    );
    
    /* Loop with volatile variables to prevent optimization */
    for (i = 0; i < n; i += vol_a) {
        /* Complex expression with multiple temporaries */
        int expr1 = vol_b * vol_c + vol_d / (vol_e + 1);
        int expr2 = vol_f % (vol_g + i) * expr1;
        int expr3 = expr2 - base + temp1;
        
        /* Address computation with multiple offsets */
        int array[100];
        int *ptr = &array[i % 50];
        
        /* Use base address with different offsets */
        ptr[0] = expr3 + base;
        ptr[5] = expr3 - base;
        ptr[10] = expr3 * base;
        ptr[15] = expr3 / (base + 1);
        
        /* Switch with multi-use temporaries in different arms */
        switch (i % 4) {
            case 0:
                result += ptr[0] + temp1;
                break;
            case 1:
                result += ptr[5] - temp1;
                break;
            case 2:
                result += ptr[10] * temp1;
                break;
            case 3:
                result += ptr[15] / (temp1 + 1);
                break;
        }
        
        /* More complex arithmetic chains */
        for (j = 0; j < 3; j++) {
            int chain1 = vol_a * vol_b + vol_c;
            int chain2 = chain1 * vol_d - vol_e;
            int chain3 = chain2 / (vol_f + j) + vol_g;
            int chain4 = chain3 % (base + 1) * temp1;
            
            result += chain4;
            
            /* Opaque function call creating unanalyzable values */
            int opaque = get_external_value();
            result += opaque % 256;
        }
        
        /* Prevent loop unrolling with volatile increment */
        vol_a = (vol_a + 1) % 5 + 1;
    }
    
    /* Final complex expression */
    result = result * base - temp1 + (vol_a * vol_b * vol_c * vol_d) / 
             (vol_e + vol_f + vol_g + 1);
    
    return result;
}
#else
/* Simple version for non-optimized builds */
int stress_computation(int seed, int n) {
    return seed + n;
}
#endif

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    int result = seed;
    volatile int vol_counter = 0;
    
    /* Long dependency chain */
    int a = vol_a * 3;
    int b = a + vol_b;
    int c = b * vol_c;
    int d = c / vol_d;
    int e = d % vol_e;
    int f = e - vol_f;
    int g = f + vol_g;
    
    /* Use all computed values in spatially separated contexts */
    if (n > 10) {
        result += a * b;
    } else {
        result += c - d;
    }
    
    if (n % 2 == 0) {
        result += e * f;
    } else {
        result += g / 2;
    }
    
    /* Loop with loop-carried dependency through volatile */
    for (int i = 0; i < n; i++) {
        vol_counter = vol_counter + 1;
        result += (a * i + b * vol_counter) / (c + 1);
        
        /* More inline assembly for register pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5"
        );
    }
    
    return result;
}

/* Main test harness */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int total_result = 0;
    
    /* Use command line or default iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Seed random for variability */
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    vol_a = rand() % 100 + 1;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100 + 1;
    vol_d = rand() % 100 + 1;
    vol_e = rand() % 100 + 1;
    vol_f = rand() % 100 + 1;
    vol_g = rand() % 100 + 1;
    
    /* Call stress functions multiple times */
    for (int i = 0; i < iterations; i++) {
        int seed = rand() % 1000;
        int n = (i % 20) + 5;  /* Varying loop bounds */
        
        /* Alternate between different stress functions */
        if (i % 2 == 0) {
            total_result += stress_computation(seed, n);
        } else {
            total_result += stress_computation2(seed, n);
        }
        
        /* Modify volatiles to change computation patterns */
        vol_a = (vol_a * 13 + 17) % 100 + 1;
        vol_b = (vol_b * 7 + 23) % 100 + 1;
    }
    
    /* Output result to prevent elimination */
    printf("Total result: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}

/* Dummy external function implementation */
int get_external_value(void) {
    static int counter = 0;
    return counter++ + rand() % 100;
}
