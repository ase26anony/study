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
    volatile int barrier = 0; /* Prevent optimizations */
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create multiple jump-to-label patterns */
    for (i = 0; i < n; i++) {
        /* Branch prediction hints to influence scheduling */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Pattern 1: Simple jump to label with eligible follower */
            goto label1;
            
            /* This should never execute due to goto */
            barrier = 1;
            
        label1:
            /* Eligible instruction: simple, non-trapping arithmetic */
            /* Uses distinct registers (r1-r4) not used in delay slot */
            r1 = a[i] + 1;      /* Candidate for delay slot */
            r2 = r1 * 2;
            a[i] = r2;
        }
        
        /* Alternate path with different register set */
        if (__builtin_expect((i & 2) == 0, 0)) {
            /* Pattern 2: Another jump pattern */
            goto label2;
            
            barrier = 2;
            
        label2:
            /* Another eligible instruction */
            s1 = b[i] - 1;      /* Candidate for delay slot */
            s2 = s1 / 2;        /* Safe division by constant 2 */
            b[i] = s2;
        }
        
        /* Complex control flow to stress scheduler */
        switch (i % 4) {
            case 0:
                /* Pattern 3: Jump in switch context */
                if (a[i] > b[i]) {
                    goto label3;
                    
                    barrier = 3;
                    
                label3:
                    /* Simple bitwise operation - safe and splittable */
                    temp1 = a[i] & 0xFF;  /* Candidate for delay slot */
                    a[i] = temp1 | 0x100;
                }
                break;
                
            case 1:
                /* Pattern 4: Nested conditional jumps */
                for (int j = 0; j < 2; j++) {
                    if (j == 0) {
                        goto label4;
                        
                        barrier = 4;
                        
                    label4:
                        /* Simple arithmetic with local variables */
                        temp2 = r3 + s3;  /* Candidate for delay slot */
                        r3 = temp2;
                    } else {
                        /* Memory barrier to constrain scheduling */
                        asm volatile("" ::: "memory");
                    }
                }
                break;
                
            case 2:
                /* Pattern 5: Jump with multiple eligible followers */
                if (i % 3 == 0) {
                    goto label5;
                    
                    barrier = 5;
                    
                label5:
                    /* Sequence of simple operations */
                    temp3 = i * 3;        /* First candidate */
                    temp4 = temp3 + 5;    /* Second candidate */
                    a[i] = temp4;
                    
                    /* Force another basic block */
                    if (temp4 > 100) {
                        asm volatile("" ::: "memory");
                    }
                }
                break;
                
            case 3:
                /* Pattern 6: Jump in loop with mixed operations */
                int k = 0;
                while (k < 2) {
                    if (k == 1) {
                        goto label6;
                        
                        barrier = 6;
                        
                    label6:
                        /* Safe shift operation */
                        temp1 = b[i] << 2;  /* Candidate for delay slot */
                        b[i] = temp1 >> 1;
                    }
                    k++;
                }
                break;
        }
        
        /* Mix in floating point to diversify resource usage */
        if (i % 8 == 0) {
            float ftemp = (float)a[i];
            ftemp = ftemp * 1.5f;
            a[i] = (int)ftemp;
        }
    }
    
    /* Final computation using all temporaries to prevent elimination */
    int sum = r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4 + temp1 + temp2 + temp3 + temp4;
    a[0] += sum;
}

/* Helper function to create more jump contexts */
#ifdef __GNUC__
__attribute__((target("arch=mips"), noinline))
#endif
static void jump_pattern_helper(int *x, int *y) {
    /* Create isolated jump patterns */
    if (*x > *y) {
        goto helper_label;
        
        volatile int dummy = 0;
        
    helper_label:
        /* Very simple operation - ideal delay slot candidate */
        *x = *y + 1;  /* Candidate for delay slot */
    }
    
    /* Another pattern */
    if (*x < 0) {
        goto helper_label2;
        
        dummy = 1;
        
    helper_label2:
        *y = *x - 1;  /* Candidate for delay slot */
    }
}

int main() {
    const int N = 256;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create predictable branches */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
    }
    
    /* Call the kernel multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(array1, array2, N);
        
        /* Call helper to add more jump contexts */
        for (int i = 0; i < N; i += 16) {
            jump_pattern_helper(&array1[i], &array2[i]);
        }
    }
    
    /* Compute and print result to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < N; i++) {
        total += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", total);
    
    free(array1);
    free(array2);
    
    return 0;
}
