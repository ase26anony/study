/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */
/* This program creates patterns to trigger delay slot filling logic */
/* focusing on the uncovered lines 2135-2149 in reorg.cc */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical variables */
static volatile int keep_alive = 0;

MIPS_TARGET
void delay_slot_test(int *arr1, int *arr2, int size) {
    int i, j;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3;
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int reg_set1 = 10, reg_set2 = 20, reg_set3 = 30;
    int temp1, temp2, temp3;
    
    /* Create complex control flow with many jumps */
    for (i = 0; i < size; i++) {
        /* First jump pattern - simple unconditional jump to label */
        if (arr1[i] > 100) {
            /* This goto creates a simplejump to label L1 */
            goto L1;
        }
        
        /* Some computation to create scheduling context */
        temp1 = reg_set1 + reg_set2;
        reg_set1 = temp1 ^ arr1[i];
        
        /* Another jump pattern */
        if (arr2[i] < 50) {
            goto L2;
        }
        
        /* Continue with other operations */
        reg_set2 = reg_set2 * 2 + 1;
        continue;
        
    L1:
        /* Instruction immediately after label - must be eligible for delay slot */
        /* Simple, non-trapping integer operation using distinct registers */
        temp2 = reg_set3 + 5;  /* Candidate for delay slot filling */
        reg_set3 = temp2;
        
        /* Follow with more operations */
        arr1[i] = reg_set3;
        if (i % 2 == 0) {
            goto L3;
        }
        continue;
        
    L2:
        /* Another candidate instruction after label */
        temp3 = reg_set1 - reg_set2;  /* Another delay slot candidate */
        reg_set1 = temp3;
        
        arr2[i] = reg_set1;
        if (i % 3 == 0) {
            goto L4;
        }
        continue;
        
    L3:
        /* More operations to create scheduling pressure */
        a = b + c;
        b = c + d;
        c = d + a;
        d = a + b;
        continue;
        
    L4:
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect(arr1[i] > arr2[i], 0)) {
            x = y + z;
        } else {
            y = z + x;
        }
        z = x + y;
    }
    
    /* Nested loop to increase scheduling complexity */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Jump to label with simple arithmetic after it */
            if ((i + j) % 5 == 0) {
                goto L5;
            }
            
            /* Mix integer operations */
            r1 = r2 ^ r3;
            r2 = r3 & r1;
            r3 = r1 | r2;
            
            /* Memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            continue;
            
        L5:
            /* Perfect delay slot candidate: simple add with distinct registers */
            int sum = r1 + 10;  /* Non-trapping, splittable operation */
            r1 = sum;
            
            /* More operations to prevent SEQUENCE formation */
            arr1[i] += j;
            arr2[j] += i;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    keep_alive = a + b + c + d + x + y + z + r1 + r2 + r3;
}

/* Secondary function with different patterns */
MIPS_TARGET
void jump_pattern_stress(int *data, int n) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (i = 0; i < n; i++) {
        /* Pattern 1: Jump to label with simple increment after */
        if (data[i] % 7 == 0) {
            goto PATTERN_A;
        }
        
        /* Pattern 2: Another jump with different register usage */
        if (data[i] % 11 == 0) {
            goto PATTERN_B;
        }
        
        acc1 += data[i];
        continue;
        
    PATTERN_A:
        /* Candidate instruction: simple bitwise operation */
        acc2 = acc2 ^ 0x55AA;  /* Non-trapping, uses distinct accumulator */
        data[i] = acc2;
        continue;
        
    PATTERN_B:
        /* Another candidate: subtraction */
        acc3 = acc3 - 1;  /* Safe, non-trapping operation */
        data[i] = acc3;
        
        /* Conditional jump to create more opportunities */
        if (acc3 > 1000) {
            goto PATTERN_C;
        }
        continue;
        
    PATTERN_C:
        /* Simple shift operation */
        acc1 = acc1 << 1;
    }
    
    /* Use results to prevent optimization */
    keep_alive += acc1 + acc2 + acc3;
}

/* Main function to drive the test */
int main() {
    const int SIZE = 1000;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates many branch conditions */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 233;
        array2[i] = (i * 13) % 197;
    }
    
    /* Call the delay slot test functions */
    delay_slot_test(array1, array2, SIZE);
    
    /* Create another dataset for stress testing */
    int *data = malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 19) % 251;
    }
    
    jump_pattern_stress(data, SIZE);
    
    /* Print results to ensure no dead code elimination */
    printf("Result check: %d\n", keep_alive);
    printf("Sample values: %d, %d, %d\n", array1[0], array2[0], data[0]);
    
    free(array1);
    free(array2);
    free(data);
    
    return 0;
}
