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
        /* Create multiple basic blocks with jumps to labels */
        if (__builtin_expect((a[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            goto label1;
        } else {
            /* Some arithmetic to create scheduling context */
            r1 = a[i] + b[i];
            r2 = r1 * 2;
            goto label2;
        }
        
    label1:
        /* Candidate instruction for delay slot: simple, non-trapping arithmetic */
        /* Uses different registers than those set before the jump */
        s1 = s2 + s3;  /* Simple add, no trap possible */
        /* Follow with more operations to prevent SEQUENCE formation */
        s4 = s1 ^ 0xFF;
        continue;
        
    label2:
        /* Another candidate instruction */
        s3 = s4 - s1;  /* Simple subtract, no trap */
        s2 = s3 | 0x55;
        
        /* Memory barrier to constrain scheduling */
        __asm__ volatile ("" ::: "memory");
        
        /* Nested conditional to create more jump opportunities */
        if (__builtin_expect(r2 > 100, 0)) {
            goto label3;
        }
        r3 = r2 + r1;
        continue;
        
    label3:
        /* Another delay slot candidate */
        s4 = s1 & s2;  /* Bitwise AND, safe */
        s3 = s4 << 2;
    }
    
    /* Second loop with different pattern */
    for (i = 0; i < n; i += 2) {
        /* Use volatile to prevent certain optimizations */
        dummy = a[i];
        
        if (__builtin_expect(b[i] < 0, 1)) {
            /* Jump to label with arithmetic follower */
            goto label4;
        }
        
        /* Some floating point in alternate path to diversify resources */
        float f1 = (float)a[i];
        float f2 = f1 * 2.0f;
        dummy = (int)f2;
        continue;
        
    label4:
        /* Eligible delay slot candidate */
        temp1 = temp2 + temp3;  /* Uses separate temp variables */
        temp4 = temp1 ^ i;
        
        /* Another jump to create chain */
        if (__builtin_expect(temp4 == 0, 0)) {
            goto label5;
        }
        continue;
        
    label5:
        /* Final candidate */
        temp2 = temp3 * 2;  /* Multiplication, still safe for integers */
        temp3 = temp2 + 1;
    }
    
    /* Force use of results to prevent dead code elimination */
    a[0] = r1 + s1 + temp1;
    b[0] = r2 + s2 + temp2;
}

/* Helper function to create more scheduling contexts */
#ifdef __GNUC__
__attribute__((target("arch=mips"), noinline))
#endif
static int conditional_helper(int x, int y) {
    int result = 0;
    
    /* Multiple labels with goto patterns */
    if (__builtin_expect(x > y, 0)) {
        goto helper_label1;
    }
    
    result = x + y;
    if (__builtin_expect(result < 0, 0)) {
        goto helper_label2;
    }
    
    return result;
    
helper_label1:
    /* Simple arithmetic after label */
    result = y - x;
    result = result & 0xFF;
    return result;
    
helper_label2:
    /* Another simple operation */
    result = x ^ y;
    result = result + 1;
    return result;
}

int main() {
    const int N = 1000;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch variance */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 3;
        array2[i] = (i % 2 == 0) ? i : -i;
    }
    
    /* Call the kernel multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        delay_slot_kernel(array1, array2, N);
        
        /* Also call helper to create more jump patterns */
        for (int i = 0; i < N; i += 10) {
            array1[i] = conditional_helper(array1[i], array2[i]);
        }
    }
    
    /* Compute checksum to ensure computation happens */
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
