/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse -o test test.c */
/* Also try: gcc -O3 -funroll-loops -fno-gcse -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* External function to create opaque values */
extern int rand(void);

/* Inline assembly to clobber registers and increase pressure */
#define CLOBBER_REGS() \
    __asm__ volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7")

/* Stress function with complex arithmetic and control flow */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = seed * vol_a + vol_b / (vol_c + 1);
    
    /* Complex floating-point chain creating many temporaries */
    float f_temp = (vol_f1 * vol_f2) + (vol_f3 / vol_f1) - 
                   (vol_f2 * vol_f3) + (vol_f1 / vol_f2);
    
    /* Address computation with multiple offsets */
    int array[100];
    int *base_ptr = &array[seed % 50];
    
    /* Loop with volatile bounds to prevent optimization */
    for (i = 0; i < vol_a + n; i++) {
        /* Complex integer arithmetic chain */
        int temp1 = base * i + vol_c;
        int temp2 = temp1 / (vol_b + 1) * vol_d;
        int temp3 = temp2 % (vol_a + 2) + temp1;
        
        /* Use base with different offsets - may trigger rematerialization */
        int val1 = *(base_ptr + i);
        int val2 = *(base_ptr + i * 2);
        int val3 = *(base_ptr + i * 3);
        
        /* More complex arithmetic mixing all values */
        int expr = (temp1 * val1 + temp2 / (val2 + 1) - temp3 % (val3 + 2));
        
        /* Nested loop to increase basic blocks */
        for (j = 0; j < vol_b; j++) {
            /* Different expression using same temporaries */
            int nested_expr = expr * j + base / (j + 1);
            
            /* Switch to create multiple basic blocks */
            switch (j % 4) {
                case 0:
                    result += nested_expr + val1;
                    CLOBBER_REGS();  /* Force register spilling */
                    break;
                case 1:
                    result += nested_expr * 2 - val2;
                    break;
                case 2:
                    result += nested_expr / 3 + val3;
                    CLOBBER_REGS();
                    break;
                case 3:
                    result += nested_expr % 4 - base;
                    /* More complex FP chain in some cases */
                    f_temp = f_temp * 1.1f - (float)j * 0.5f;
                    break;
            }
            
            /* Additional arithmetic that uses f_temp */
            result += (int)(f_temp * 100.0f);
        }
        
        /* Call rand() to create opaque values */
        if (i % 3 == 0) {
            int rand_val = rand() % 100;
            result += rand_val * base;
            
            /* Another complex expression using rand result */
            int complex_expr = (rand_val * temp1 + temp2 * 2 - temp3 / 3) % 97;
            result += complex_expr;
        }
    }
    
    /* Final complex expression using all accumulated values */
    result = (result * base + (int)(f_temp * 1000.0f)) % 1000000;
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    int result = seed;
    
    /* Very long dependency chain */
    int chain = seed;
    chain = chain * 3 + vol_a;
    chain = chain / 2 - vol_b;
    chain = chain * 5 + vol_c;
    chain = chain % 17 - vol_d;
    chain = chain * 7 + vol_a * 2;
    chain = chain / 3 - vol_b * 2;
    chain = chain * 11 + vol_c * 3;
    
    /* Use chain value in multiple separated contexts */
    if (n > 10) {
        result += chain * 2;
        CLOBBER_REGS();
    } else {
        result -= chain / 2;
    }
    
    if (n > 20) {
        result += chain % 13;
        CLOBBER_REGS();
    } else {
        result -= chain % 7;
    }
    
    if (n > 30) {
        result += chain * 3 / 4;
    } else {
        result -= chain * 2 / 3;
        CLOBBER_REGS();
    }
    
    /* More arithmetic with volatile */
    for (int i = 0; i < vol_c; i++) {
        result += (chain * i + vol_d * (i % 3)) % 256;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, total = 0;
    
    /* Initialize with command line or random values */
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    
    /* Call stress functions multiple times from different contexts */
    for (i = 0; i < iterations; i++) {
        int seed = rand() % 1000;
        
        /* Alternate between different call patterns */
        if (i % 3 == 0) {
            total += stress_computation(seed, i);
        } else if (i % 3 == 1) {
            total += stress_computation2(seed, i);
        } else {
            /* Mix both */
            total += stress_computation(seed, i) + 
                     stress_computation2(seed + 1, i);
        }
        
        /* Modify volatile variables occasionally */
        if (i % 7 == 0) {
            vol_a = (vol_a + 1) % 10;
            vol_b = (vol_b + 2) % 10;
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
