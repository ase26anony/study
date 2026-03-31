/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int size) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int reg_set1 = 0, reg_set2 = 0, reg_set3 = 0, reg_set4 = 0;
    
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect(a[i] > 0, 1)) {
            /* Jump pattern 1: Simple unconditional jump to label */
            if (b[i] & 1) {
                goto label_arithmetic_1;
            } else {
                goto label_arithmetic_2;
            }
            
            /* This should never be reached due to goto */
            __asm__ volatile("" ::: "memory");
        }
        
        /* Continue with other operations */
        a[i] = b[i] * 2;
        continue;
        
    /* Label placed immediately before simple arithmetic operations */
    label_arithmetic_1:
        /* Simple, non-trapping integer operation - candidate for delay slot */
        reg_set1 = reg_set1 + a[i];  /* This is the 'next_trial' candidate */
        reg_set2 = reg_set2 ^ b[i];  /* Additional operation to create scheduling context */
        acc += reg_set1;
        
        /* Memory barrier to constrain scheduling */
        __asm__ volatile("" ::: "memory");
        continue;
        
    label_arithmetic_2:
        /* Another candidate instruction after label */
        reg_set3 = reg_set3 - b[i];  /* Simple subtraction - non-trapping */
        reg_set4 = reg_set4 | a[i];  /* Bitwise operation */
        acc += reg_set3;
        
        /* Create another jump context */
        if (__builtin_expect(acc & 1, 0)) {
            goto label_arithmetic_3;
        }
        continue;
        
    label_arithmetic_3:
        /* More simple arithmetic for delay slot candidates */
        temp1 = reg_set1 * 3;  /* Multiplication by constant - safe */
        temp2 = reg_set2 / 2;  /* Division by constant 2 - safe, no trap */
        acc += temp1 + temp2;
    }
    
    /* Nested loop to increase scheduling complexity */
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            /* Conditional jumps with goto patterns */
            if (__builtin_expect(a[i] > a[j], 0)) {
                if (b[i] > b[j]) {
                    goto label_swap_check;
                } else {
                    goto label_no_swap;
                }
            }
            
            /* Simple operations that don't trap */
            temp3 = a[i] + b[j];
            temp4 = a[j] + b[i];
            continue;
            
        label_swap_check:
            /* Candidate for delay slot filling */
            temp1 = a[i] - a[j];  /* Simple subtraction */
            temp2 = b[i] - b[j];  /* Another simple operation */
            
            if (temp1 > temp2) {
                /* Another unconditional jump */
                goto label_do_swap;
            }
            continue;
            
        label_do_swap:
            /* More simple arithmetic */
            int swap_temp = a[i];
            a[i] = a[j];
            a[j] = swap_temp;
            continue;
            
        label_no_swap:
            /* Simple bitwise operation - good delay slot candidate */
            a[i] = a[i] & 0xFFFF;
            a[j] = a[j] & 0xFFFF;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    a[0] = acc;
}

/* Additional function to create more scheduling contexts */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void complex_branch_pattern(int *arr, int n) {
    int i = 0;
    
    /* Unrolled loop with goto labels */
    while (i < n) {
        /* Pattern 1: Jump to label with simple follower */
        if (arr[i] == 0) {
            goto handle_zero;
        }
        
        /* Pattern 2: Another jump pattern */
        if (arr[i] < 0) {
            goto handle_negative;
        }
        
        /* Default path */
        arr[i] = arr[i] * 2;
        i++;
        continue;
        
    handle_zero:
        /* Simple arithmetic after label - delay slot candidate */
        arr[i] = arr[i] + 1;  /* This is 'next_trial' */
        i++;
        continue;
        
    handle_negative:
        /* Another simple operation */
        arr[i] = -arr[i];  /* Negation - safe operation */
        i++;
        
        /* Create chain of jumps */
        if (i < n && arr[i] > 100) {
            goto handle_large;
        }
        continue;
        
    handle_large:
        /* More simple arithmetic */
        arr[i] = arr[i] % 100;  /* Modulo with constant - safe */
        i++;
    }
}

int main() {
    const int SIZE = 100;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pattern to create various branch conditions */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 53;
        array2[i] = (i * 23) % 47;
    }
    
    /* Call the kernel function with goto patterns */
    delay_slot_kernel(array1, array2, SIZE);
    
    /* Call additional function for more coverage */
    complex_branch_pattern(array1, SIZE);
    
    /* Compute and print result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i];
    }
    
    printf("Result: %d\n", sum);
    
    return 0;
}
