/* Test program to stress early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with many temporaries */
static int complex_arithmetic(int seed) {
    /* Use seed to create non-constant starting values */
    int a = seed + vol_a;
    int b = a * vol_b;
    int c = b / (vol_c + 1);
    int d = c - vol_d;
    
    /* Long dependency chain with many virtual registers */
    int t1 = a * b + c / d;
    int t2 = t1 % (b + 1);
    int t3 = t2 * c - d;
    int t4 = t3 / (a + 1) % (b + 2);
    int t5 = t4 * t3 - t2 * t1;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = t5 * seed + vol_a;
    
    /* Control flow that separates uses of 'base' */
    int result = 0;
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            result += base * 2;  /* First use */
        } else if (i == 1) {
            result += base / 3;  /* Second use - spatially separated */
        } else {
            result += base % 5;  /* Third use */
        }
    }
    
    /* More complex arithmetic creating register pressure */
    float f1 = vol_f1 * seed;
    float f2 = vol_f2 / (seed + 1);
    float f3 = f1 * f2 - vol_f3;
    float f4 = f3 / f1 + f2 * vol_f1;
    
    /* Inline assembly to clobber hard registers */
    /* This reduces available physical registers */
#ifdef __OPTIMIZE__
    asm volatile (
        "# Clobber multiple registers to increase pressure\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
#endif
    
    /* Address computation with multiple offsets */
    /* This can trigger register recreation patterns */
    int array[100];
    int *ptr = &array[seed % 50];
    
    /* Multiple uses of computed address with different offsets */
    ptr[0] = result;
    ptr[10] = base;      /* Use base again */
    ptr[20] = t5;
    ptr[30] = (int)f4;
    
    return result + ptr[0] + ptr[10] + ptr[20] + ptr[30];
}

/* Main stress function with loop-carried dependencies */
int stress_computation(int seed, int iterations) {
    volatile int vol_counter = seed;  /* Volatile loop counter */
    int total = 0;
    
    /* Loop with volatile counter prevents optimizations */
    for (int i = 0; i < iterations; i++) {
        vol_counter++;
        
        /* Opaque function call creates unanalyzable values */
        int opaque = get_external_value() + vol_counter;
        
        /* Complex expression with many temporaries */
        int a = opaque * vol_a + i;
        int b = vol_b / (a + 1) * vol_c;
        int c = b % (vol_d + opaque);
        int d = c * a - b;
        
        /* Another multi-use temporary */
        int computed = (a * b + c * d) % 1000;
        
        /* Use computed value in multiple separated contexts */
        if (i % 2 == 0) {
            total += computed * 2;
        } else {
            total -= computed / 3;
        }
        
        /* Nested complex arithmetic */
        float f_temp = vol_f1 * i + vol_f2 / (computed + 1);
        f_temp = f_temp * vol_f3 - vol_f1;
        
        /* More register pressure */
        int t1 = computed * i;
        int t2 = t1 % (i + 100);
        int t3 = t2 * computed - i;
        int t4 = t3 / (computed + 1) + t2;
        
        total += t4 + (int)f_temp;
        
        /* Additional inline assembly clobber */
#ifdef __OPTIMIZE__
        asm volatile (
            "# More register clobbering\n\t"
            : 
            : 
            : "memory", "cc", 
              "r0", "r1", "r2", "r3", "r4"
        );
#endif
    }
    
    return total;
}

/* Function with switch statement to separate uses */
static int multi_use_pattern(int mode, int x, int y) {
    /* Compute value once */
    int base_value = x * y + vol_a * vol_b - vol_c;
    
    /* Use in different switch arms - forces RA decisions */
    int result = 0;
    switch (mode % 4) {
        case 0:
            result = base_value * 2;  /* First use location */
            /* Complex arithmetic in case */
            result += (x % y) * (y % x) + vol_d;
            break;
        case 1:
            result = base_value / 3;  /* Second use - separated */
            /* Different computation pattern */
            result -= (x + y) * vol_a - vol_b;
            break;
        case 2:
            result = base_value % 7;  /* Third use */
            /* More arithmetic */
            result *= (x - y) + vol_c * vol_d;
            break;
        default:
            result = base_value + 100; /* Fourth use */
            /* Volatile access chain */
            result += vol_a - vol_b + vol_c - vol_d;
            break;
    }
    
    return result;
}

/* Main test harness */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int iterations = argc > 2 ? atoi(argv[2]) : 100;
    
    srand(seed);
    
    int total_result = 0;
    
    /* Call stress functions multiple times */
    for (int round = 0; round < 5; round++) {
        /* Vary inputs to create different patterns */
        int input = rand() % 1000;
        
        /* Mix different computation patterns */
        total_result += complex_arithmetic(input + round);
        total_result += stress_computation(input, iterations / 10);
        total_result += multi_use_pattern(round, input, input + 1);
        
        /* Additional volatile chain */
        int chain = vol_a;
        chain = chain * vol_b + vol_c;
        chain = chain / (vol_d + 1) - vol_a;
        chain = chain % (vol_b * 2) + vol_c;
        total_result += chain;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}

/* Simulate external function */
int get_external_value(void) {
    static int counter = 0;
    return rand() ^ (counter++);
}
