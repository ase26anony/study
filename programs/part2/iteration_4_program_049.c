/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int n) {
    int i, temp1, temp2, temp3, temp4;
    volatile int dummy = 0; /* Prevent optimizations */
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    for (i = 0; i < n; i++) {
        /* Create multiple jump contexts with different patterns */
        if (__builtin_expect((a[i] & 1) != 0, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            goto label1;
back1:
            continue;
            
label1:
            /* Candidate for delay slot: Simple, non-trapping arithmetic */
            /* Uses different registers than those set before the jump */
            r1 = s1 + s2;  /* Simple add, no trap possible */
            s1 = r1 ^ 0x55; /* Bitwise op, safe */
            goto back1;
        }
        
        if (__builtin_expect((a[i] & 2) != 0, 1)) {
            /* Pattern 2: Another jump pattern */
            goto label2;
back2:
            b[i] = temp1;
            continue;
            
label2:
            /* Another candidate - different register set */
            r2 = s3 - s4;  /* Simple subtract, safe */
            temp1 = r2 * 2; /* Multiplication by constant, safe */
            goto back2;
        }
        
        if (__builtin_expect((a[i] & 4) != 0, 0)) {
            /* Pattern 3: Nested conditional jumps */
            if (b[i] > 0) {
                goto label3;
back3:
                /* Insert memory barrier to constrain scheduling */
                asm volatile("" ::: "memory");
                continue;
                
label3:
                /* Candidate using only local temporaries */
                temp3 = temp4 | 0xFF; /* Bitwise OR, safe */
                temp4 = temp3 & 0x7F; /* Bitwise AND, safe */
                goto back3;
            }
        }
        
        /* Default path with mixed operations */
        temp2 = a[i] * b[i];
        r3 = temp2 + i;
        s2 = r3 ^ s4;
        
        /* Force occasional jumps to create scheduling pressure */
        if (__builtin_expect((i % 7) == 0, 0)) {
            goto label4;
back4:
            dummy = r4; /* Use volatile to prevent elimination */
            continue;
            
label4:
            /* Simple arithmetic candidate */
            r4 = s2 + 1;  /* Increment, safe */
            s3 = r4 << 2; /* Shift, safe */
            goto back4;
        }
    }
    
    /* Final computation mixing integer and bitwise ops */
    for (i = 0; i < n; i += 2) {
        if (__builtin_expect(a[i] != b[i], 1)) {
            goto label5;
back5:
            a[i] = s4;
            continue;
            
label5:
            /* Final pattern candidate */
            s4 = r1 + r2;  /* Add previously computed values */
            r1 = s4 ^ r3;  /* Mix with another register */
            goto back5;
        }
    }
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void stress_scheduler(int *arr, int size) {
    int i, j, acc = 0;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    /* Complex loop structure to create many basic blocks */
    for (i = 0; i < size; i++) {
        /* Multiple conditional jumps in sequence */
        if (arr[i] < 0) {
            goto neg_label;
neg_back:
            t1 = t2 + 1;
            continue;
            
neg_label:
            /* Simple arithmetic after label */
            t2 = t3 * 2;  /* Safe multiplication by constant */
            t3 = t2 | 1;  /* Bitwise OR with constant */
            goto neg_back;
        }
        
        if (arr[i] > 100) {
            goto large_label;
large_back:
            t4 = acc ^ t1;
            continue;
            
large_label:
            /* Another candidate */
            acc = acc + arr[i];  /* Simple accumulation */
            goto large_back;
        }
        
        /* Default case with its own jump */
        if ((arr[i] & 3) == 0) {
            goto zero_label;
zero_back:
            arr[i] = t4;
            continue;
            
zero_label:
            t4 = t1 - t2;  /* Simple subtraction */
            goto zero_back;
        }
    }
    
    /* Cross-jump pattern */
    for (j = 0; j < size; j += 3) {
        if (__builtin_expect((j & 1) == 0, 0)) {
            goto cross_label;
cross_back:
            arr[j] = acc;
            continue;
            
cross_label:
            acc = acc + t3;  /* Simple add */
            t3 = acc >> 1;   /* Safe shift */
            goto cross_back;
        }
    }
}

int main() {
    const int N = 1024;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17) & 0xFF;
        array2[i] = (i * 13) & 0xFF;
    }
    
    /* Call the kernel functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(array1, array2, N);
        stress_scheduler(array1, N);
        
        /* Mix in some array operations to create data dependencies */
        for (int i = 1; i < N; i++) {
            array1[i] += array2[i-1];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
