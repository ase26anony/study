/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */
/* This program creates patterns to trigger delay slot filling logic in reorg.cc */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
static void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int acc1 = 0, acc2 = 0;
    int temp1, temp2, temp3;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main computation loop with dense branching */
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with simple jumps to labels */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            goto label1;
        } else if (__builtin_expect(arr1[i] < -100, 0)) {
            /* Jump pattern 2 */
            goto label2;
        } else {
            /* Normal path with arithmetic */
            r1 = arr1[i] + arr2[i];
            r2 = r1 * 2;
            continue;
        }
        
    label1:
        /* Candidate for delay slot filling: simple non-trapping arithmetic */
        /* Uses different registers than those in the jump's context */
        s1 = s2 + s3;  /* Simple add, no trap possible */
        s2 = s1 ^ s4;  /* Bitwise op, safe */
        
        /* Continue with main computation */
        r3 = arr1[i] * 3;
        r4 = r3 - arr2[i];
        continue;
        
    label2:
        /* Another candidate with different register set */
        temp1 = temp2 & temp3;  /* Bitwise AND, safe */
        temp2 = temp1 | 0xFF;   /* OR with constant */
        
        /* More computation */
        r1 = arr1[i] / 2;  /* Division by constant 2 - safe, no trap */
        r2 = r1 + 5;
        continue;
    }
    
    /* Second loop with nested jumps */
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            if (__builtin_expect(arr1[i] == arr2[j], 0)) {
                /* Jump to label with simple arithmetic follower */
                goto match_found;
            }
            
            /* Some computation to create scheduling pressure */
            acc1 += arr1[i] * arr2[j];
            acc2 += arr1[j] - arr2[i];
            
            /* Memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            continue;
            
        match_found:
            /* Eligible instruction for delay slot */
            /* Uses fresh variables to avoid resource conflicts */
            int local_a = acc1;
            int local_b = acc2;
            local_a = local_b + 1;  /* Simple increment */
            
            /* More computation */
            acc1 = local_a * 2;
            acc2 = acc1 >> 1;
        }
    }
    
    /* Complex control flow with multiple labels */
    int k = 0;
    while (k < size) {
        switch (arr1[k] % 4) {
            case 0:
                goto case0_label;
            case 1:
                goto case1_label;
            case 2:
                goto case2_label;
            default:
                k++;
                continue;
        }
        
    case0_label:
        /* Simple arithmetic after label - good delay slot candidate */
        r1 = r2 + r3;  /* Add, no trap */
        r2 = r1 ^ r4;  /* XOR, safe */
        k += 2;
        continue;
        
    case1_label:
        /* Another candidate */
        s1 = s3 - s4;  /* Subtract, safe */
        s3 = s1 & 0x7F; /* AND with constant */
        k += 3;
        continue;
        
    case2_label:
        /* Mix integer operations */
        temp1 = temp3 << 2;  /* Shift left, safe */
        temp3 = temp1 >> 1;  /* Shift right, safe */
        k += 4;
        continue;
    }
    
    /* Final accumulation to prevent dead code elimination */
    arr1[0] = acc1 + acc2 + r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4 + temp1 + temp2 + temp3;
}

/* Helper function to create more scheduling contexts */
#ifdef __GNUC__
__attribute__((target("arch=mips"), noinline))
#endif
static int branch_dense_loop(int *a, int *b, int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Multiple conditional jumps */
        if (__builtin_expect(a[i] > b[i], 1)) {
            goto greater;
        } else if (__builtin_expect(a[i] < b[i], 0)) {
            goto lesser;
        } else {
            goto equal;
        }
        
    greater:
        /* Simple arithmetic after label */
        sum += a[i] - b[i];
        continue;
        
    lesser:
        /* Different simple operation */
        sum += b[i] - a[i];
        continue;
        
    equal:
        /* Another simple operation */
        sum += a[i];
        continue;
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 1000;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 1000 - 500;  /* Range: -500 to 499 */
        array2[i] = (i * 23) % 1000 - 500;  /* Different pattern */
    }
    
    /* Call the function with delay slot patterns */
    delay_slot_pattern(array1, array2, SIZE);
    
    /* Call branch-dense function */
    int result = branch_dense_loop(array1, array2, SIZE);
    
    /* Use results to prevent optimization */
    printf("Result: %d, First element: %d\n", result, array1[0]);
    
    free(array1);
    free(array2);
    
    return 0;
}
