/* This program is specifically designed to trigger the uncovered delay slot
   filling logic in GCC's reorg.cc, lines 2135-2149. It uses goto-label patterns
   with simple arithmetic operations after labels, targeting MIPS architecture
   to exploit delay slot filling opportunities. */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical patterns */
static volatile int guard = 0;

MIPS_TARGET
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = arr1[0], r2 = arr2[0];
    int s1 = 5, s2 = 6, s3 = 7;
    
    for (i = 0; i < size; i++) {
        /* Create multiple jump contexts with different conditions */
        if (__builtin_expect(arr1[i] > arr2[i], 0)) {
            /* Jump pattern 1: simple jump to label with safe arithmetic */
            if (r1 > 0) goto label1;
            else goto label2;
            
        label1:
            /* Candidate for delay slot: simple, non-trapping integer operation
               Uses registers not in conflict with jump context */
            s1 = s2 + s3;  /* Safe addition */
            a = b + c;     /* Another safe operation */
            continue;
            
        label2:
            /* Different simple operation */
            s3 = s1 - s2;
            d = x ^ y;
            continue;
        }
        
        /* Second jump pattern with different register set */
        if (__builtin_expect(arr1[i] < arr2[i], 1)) {
            int t1 = i * 2, t2 = i + 1;
            if (t1 > t2) goto label3;
            
        after_label3:
            /* Continue with other operations */
            r1 = arr1[i] ^ arr2[i];
            continue;
            
        label3:
            /* Another delay slot candidate - simple bitwise operation */
            z = x | y;  /* Non-trapping, simple operation */
            goto after_label3;
        }
        
        /* Third pattern: nested conditional jumps */
        if (arr1[i] == arr2[i]) {
            switch (i % 3) {
                case 0:
                    if (guard) goto label4;
                    break;
                case 1:
                    if (!guard) goto label5;
                    break;
                default:
                    goto label6;
            }
            
        after_labels:
            /* Mix in some memory operations in different paths */
            arr1[i] = arr2[i] + 1;
            continue;
            
        label4:
            /* Simple arithmetic after label */
            a = b * 2;  /* Multiplication by constant is safe */
            goto after_labels;
            
        label5:
            c = d << 1;  /* Shift operation - non-trapping */
            goto after_labels;
            
        label6:
            y = z & 0xFF;  /* Bitwise AND with constant */
            goto after_labels;
        }
        
        /* Memory barrier to constrain scheduling in some paths */
        if (i % 7 == 0) {
            __sync_synchronize();
        }
        
        /* Additional arithmetic to create more scheduling contexts */
        arr1[i] = arr1[i] + arr2[i] - (i % 5);
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = a + b + c + d + x + y + z + r1 + s1 + s2 + s3;
    arr1[0] = result;
}

/* Helper function to create more complex control flow */
MIPS_TARGET
void complex_control_flow(int *arr, int size) {
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Unrolled loop to create more scheduling opportunities */
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx >= size) break;
            
            /* Pattern with forward jump */
            if (arr[idx] % 2 == 0) {
                int temp = arr[idx];
                if (temp > 100) goto forward_label;
                
            after_forward:
                arr[idx] = temp * 3;
                continue;
                
            forward_label:
                /* Simple operation after label - candidate for delay slot */
                temp = temp / 2;  /* Division by constant is safe */
                goto after_forward;
            }
        }
    }
}

int main() {
    const int SIZE = 256;
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 13) % 97;
        arr2[i] = (i * 17) % 101;
    }
    
    /* Execute the patterns multiple times */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_pattern(arr1, arr2, SIZE);
        complex_control_flow(arr1, SIZE);
        
        /* Modify guard to change execution paths */
        guard = iter % 2;
    }
    
    /* Compute and print checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
    }
    printf("Result checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    return 0;
}
