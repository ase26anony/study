/* Test program to trigger virtual register creation stress in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

#ifdef __OPTIMIZE__
/* Inline assembly to clobber registers and increase pressure */
#define CLOBBER_REGS() \
    __asm__ volatile ( \
        "# Clobber multiple registers\n" \
        "mov r0, #0\n" \
        "mov r1, #0\n" \
        "mov r2, #0\n" \
        "mov r3, #0\n" \
        "mov r4, #0\n" \
        "mov r5, #0\n" \
        : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory" \
    )
#else
#define CLOBBER_REGS() ((void)0)
#endif

/* Stress function with complex arithmetic and control flow */
int stress_computation(int seed, int n) {
    int result = 0;
    volatile int vol_local = seed;
    
    /* Complex arithmetic expression creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Use volatile variables in complex expressions */
        int temp1 = vol_a * vol_b + vol_c / (vol_d + 1);
        int temp2 = vol_b % (vol_c + 1) - vol_a * vol_d;
        
        /* Multi-use temporary value */
        int base = temp1 * temp2 + vol_local;
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 10; j++) {
            array[j] = j * j;
        }
        
        /* Use base with different offsets in control flow */
        if (i % 3 == 0) {
            result += base + array[0] - array[1];
        } else if (i % 3 == 1) {
            result += base + array[2] * array[3];
        } else {
            result += base - array[4] / (array[5] + 1);
        }
        
        /* More complex floating point arithmetic */
        float f_temp = vol_f1 * vol_f2 + vol_f3 / (vol_f1 + 0.1f);
        f_temp = f_temp - vol_f2 * vol_f3 + vol_f1 / vol_f2;
        
        /* Convert to int and use */
        result += (int)f_temp;
        
        /* Call opaque function */
        int opaque = get_external_value();
        result += opaque % 100;
        
        /* Clobber registers to increase pressure */
        CLOBBER_REGS();
        
        /* Update volatile to prevent loop optimization */
        vol_local += (result % 7);
    }
    
    return result;
}

/* Another stress function with different patterns */
int stress_computation2(int seed, int n) {
    int result = seed;
    volatile int vol_counter = 0;
    
    /* Loop with volatile bound */
    for (volatile int i = 0; i < n; i++) {
        /* Very long dependency chain */
        int a = result + vol_a;
        int b = a * vol_b - vol_c;
        int c = b % (vol_d + 1) + a;
        int d = c * vol_a / (vol_b + 1);
        int e = d - vol_c * vol_d;
        int f = e % 256 + b;
        int g = f * 3 - d;
        int h = g + e / 2;
        
        /* Switch with multi-use values */
        switch (i % 4) {
            case 0:
                result += h + a;
                break;
            case 1:
                result += h * b - c;
                break;
            case 2:
                result += h / (d + 1) + e;
                break;
            case 3:
                result += h % (f + 1) * g;
                break;
        }
        
        /* More register pressure */
        float fa = result * 1.1f;
        float fb = fa / 2.3f;
        float fc = fb * 3.7f;
        float fd = fc - 1.2f;
        
        result += (int)(fa + fb + fc + fd);
        
        vol_counter++;
    }
    
    return result;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return counter++ + rand() % 1000;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    srand(seed);
    
    /* Initialize volatile variables with random values */
    vol_a = rand() % 100 + 1;
    vol_b = rand() % 100 + 1;
    vol_c = rand() % 100 + 1;
    vol_d = rand() % 100 + 1;
    
    vol_f1 = (rand() % 100) / 10.0f + 0.1f;
    vol_f2 = (rand() % 100) / 10.0f + 0.1f;
    vol_f3 = (rand() % 100) / 10.0f + 0.1f;
    
    /* Call stress functions multiple times */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        int r1 = stress_computation(seed + i, iterations);
        int r2 = stress_computation2(seed + i * 7, iterations / 2 + 10);
        
        total += r1 + r2;
        
        /* Modify volatiles between calls */
        vol_a += i;
        vol_b += i * 2;
    }
    
    /* Output result to prevent elimination */
    printf("Result: %d (seed: %d, iterations: %d)\n", total, seed, iterations);
    
    return total != 0 ? 0 : 1;
}
