/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int n) {
    int i, j;
    int temp1, temp2, temp3;
    volatile int barrier = 0; /* Use for memory barriers */
    
    /* Initialize distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main computational loop with dense branching */
    for (i = 0; i < n; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            if (a[i] > b[i]) {
                /* Use goto to create simplejump to label */
                goto label_arithmetic_1;
            } else {
                /* Alternate path with memory barrier */
                __asm__ volatile("" ::: "memory");
                goto label_arithmetic_2;
            }
            
        label_arithmetic_1:
            /* Candidate for delay slot: simple, non-trapping integer operation */
            /* Uses distinct register set (r1-r4) not used in jump path */
            r1 = r2 + r3;  /* Simple add - will pass try_split and eligible_for_delay */
            /* Continue with computation */
            a[i] = r1 * 2;
            continue;
            
        label_arithmetic_2:
            /* Another candidate with different registers */
            s1 = s2 ^ s3;  /* Bitwise XOR - also non-trapping */
            b[i] = s1 - 1;
            continue;
        }
        
        /* Pattern 2: Nested loop with more complex control flow */
        for (j = 0; j < (i & 3); j++) {
            /* Create another jump-to-label pattern */
            if (__builtin_expect((j & 1) == 0, 0)) {
                /* Force cold path prediction */
                goto label_safe_op;
            }
            
            /* Hot path with different operations */
            temp1 = a[i] * b[j];
            temp2 = temp1 + j;
            a[i] = temp2;
            
            /* Insert memory barrier occasionally */
            if ((j & 3) == 0) {
                barrier = 1;
            }
            continue;
            
        label_safe_op:
            /* Another delay slot candidate: simple subtraction */
            /* Uses temp3 which hasn't been touched in this basic block */
            temp3 = r4 - s4;  /* Safe, non-trapping operation */
            b[j] = temp3 & 0xFF;  /* Mask operation */
        }
        
        /* Pattern 3: Switch-like structure with goto labels */
        switch (i % 4) {
            case 0:
                goto case_label_0;
            case 1:
                goto case_label_1;
            case 2:
                /* Use memory barrier to affect scheduling */
                __asm__ volatile("" ::: "memory");
                goto case_label_2;
            default:
                goto case_label_3;
        }
        
        /* These labels must be reached only by goto (creating simplejumps) */
        case_label_0:
            r2 = r3 << 1;  /* Simple shift - eligible for delay slot */
            a[i] += r2;
            break;
            
        case_label_1:
            s2 = s3 | 0x1;  /* Bitwise OR with constant */
            b[i] ^= s2;
            break;
            
        case_label_2:
            /* Mix integer operations */
            temp1 = (r1 + s1) * 2;
            a[i] = temp1;
            break;
            
        case_label_3:
            /* Simple increment */
            r3 = r4 + 1;
            b[i] = r3;
            break;
        }
        
        /* Occasionally use floating point to diversify resource usage */
        if ((i & 7) == 0) {
            float ftemp = (float)a[i] * 0.5f;
            /* Convert back to int without trapping */
            a[i] = (int)ftemp;
        }
    }
    
    /* Final computation with one more jump pattern */
    if (n > 0) {
        /* Create final opportunity for delay slot filling */
        if (a[0] != b[0]) {
            goto final_label;
        }
        
        /* This should not be reached often */
        __asm__ volatile("" ::: "memory");
        return;
        
    final_label:
        /* Final delay slot candidate */
        r4 = s4 * 2;  /* Simple multiplication by 2 (shift in disguise) */
        a[0] = r4;
    }
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void secondary_kernel(int *arr, int size) {
    int i = 0;
    
    /* Unrolled loop with explicit labels */
    while (i < size) {
        /* Create multiple jump-to-label opportunities */
        if (arr[i] > 1000) {
            goto process_large;
        }
        
        /* Small value processing */
        arr[i] = arr[i] + 1;
        i++;
        continue;
        
    process_large:
        /* Candidate instruction for delay slot */
        int tmp = arr[i] >> 2;  /* Safe shift operation */
        arr[i] = tmp - 5;
        i++;
    }
}

int main() {
    const int SIZE = 1024;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch divergence */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = (i & 1) ? i * 2 : i * 5;
    }
    
    /* Call the kernel multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(array1, array2, SIZE);
        secondary_kernel(array1, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
