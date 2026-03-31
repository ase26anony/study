/* 
 * This program is designed to trigger the uncovered delay slot filling logic
 * in GCC's reorg.cc (lines 2135-2149) by creating specific RTL patterns.
 * It targets MIPS architecture with delay slots and uses goto-label patterns
 * with eligible follower instructions that pass resource dependency checks.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of delay slot candidates */
static volatile int g_counter = 0;

MIPS_TARGET
void delay_slot_test(int *arr1, int *arr2, int size) {
    int i, temp1, temp2, temp3, temp4;
    int res1 = 0, res2 = 0, res3 = 0, res4 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create complex control flow with many branches */
    for (i = 0; i < size; i++) {
        /* Branch prediction hints to influence scheduling */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            goto label_arithmetic_1;
            
back_from_1:
            /* Mix in memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            /* Another branch pattern */
            if (__builtin_expect(arr2[i] < 50, 1)) {
                goto label_arithmetic_2;
                
back_from_2:
                /* Continue with other operations */
                r3 = r3 ^ arr1[i];
            }
        } else {
            /* Alternate path with different resource usage */
            s1 = s1 + arr2[i];
            if (arr1[i] % 2 == 0) {
                goto label_arithmetic_3;
                
back_from_3:
                s2 = s2 | arr1[i];
            }
        }
        
        /* Nested loop to increase scheduling complexity */
        for (temp1 = 0; temp1 < 2; temp1++) {
            if (temp1 == (i & 1)) {
                goto label_arithmetic_4;
                
back_from_4:
                /* Floating point in alternate path diversifies resources */
                float ftemp = (float)arr1[i] * 0.5f;
                res4 += (int)ftemp;
            }
        }
        
        /* Force register pressure */
        r1 = r1 + r2;
        r2 = r2 ^ r3;
        r3 = r3 - r4;
        r4 = r4 | r1;
    }
    
    /* Avoid dead code elimination */
    g_counter = res1 + res2 + res3 + res4 + r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4;
    return;
    
/* TARGET LABELS WITH ELIGIBLE FOLLOWERS */
/* Each label must be followed by a simple, non-trapping, non-jump instruction
   that doesn't conflict with resources used in delay slots */

label_arithmetic_1:
    /* Simple arithmetic - safe, non-trapping, splittable */
    res1 = res1 + arr1[i];  /* Eligible for delay slot */
    goto back_from_1;

label_arithmetic_2:
    /* Bitwise operation - no resource conflict with jump's context */
    res2 = res2 ^ arr2[i];  /* Uses different array than pattern 1 */
    goto back_from_2;

label_arithmetic_3:
    /* Subtraction - safe integer operation */
    res3 = res3 - arr1[i];  /* Different result register than others */
    goto back_from_3;

label_arithmetic_4:
    /* Logical operation - no memory access, no trap possible */
    s3 = s3 & arr2[i];      /* Uses 's' register set, not 'r' set */
    goto back_from_4;
}

/* Secondary test function with different patterns */
MIPS_TARGET
void secondary_patterns(int *data, int n) {
    int a = 0, b = 0, c = 0, d = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Multiple consecutive jumps to create scheduling pressure */
        if (data[i] > 0) {
            if (data[i] & 1) {
                goto secondary_label_1;
                
secondary_back_1:
                a = a << 1;
            } else {
                goto secondary_label_2;
                
secondary_back_2:
                b = b >> 1;
            }
        }
        
        /* More complex condition to force different paths */
        switch (data[i] % 4) {
            case 0:
                goto secondary_label_3;
                
secondary_back_3:
                c = c + i;
                break;
            case 1:
                d = d - i;
                break;
            case 2:
                goto secondary_label_4;
                
secondary_back_4:
                a = a ^ b;
                break;
        }
    }
    
    g_counter += a + b + c + d;
    return;

/* More eligible followers for delay slots */
secondary_label_1:
    /* Simple increment - very safe for delay slot */
    a = a + 1;
    goto secondary_back_1;

secondary_label_2:
    /* Decrement - also safe */
    b = b - 1;
    goto secondary_back_2;

secondary_label_3:
    /* Bitwise operation */
    c = c | 0xFF;
    goto secondary_back_3;

secondary_label_4:
    /* Arithmetic with constant */
    d = d * 2;
    goto secondary_back_4;
}

/* Main driver that creates the right conditions */
int main() {
    const int SIZE = 1000;
    int *array1, *array2;
    int i;
    
    /* Allocate and initialize arrays with varying values */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with values that create diverse branch behaviors */
    for (i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 233;
        array2[i] = (i * 23) % 197;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (i = 0; i < 10; i++) {
        delay_slot_test(array1, array2, SIZE);
        secondary_patterns(array1, SIZE);
        
        /* Modify data slightly each iteration */
        array1[i % SIZE] = i;
        array2[i % SIZE] = SIZE - i;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", g_counter);
    
    free(array1);
    free(array2);
    
    return 0;
}
