/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-sched */
/* Alternative: -O3 -march=mips -funroll-loops -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not default */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *arr1, int *arr2, int size) {
    int i, j;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int temp1, temp2, temp3;
    
    /* Use distinct registers/variables to avoid resource conflicts */
    register int r1 asm("$16");
    register int r2 asm("$17");
    register int r3 asm("$18");
    register int r4 asm("$19");
    
    /* Initialize working registers */
    r1 = arr1[0];
    r2 = arr2[0];
    r3 = 0;
    r4 = 0;
    
    /* Main loop with multiple jump-to-label patterns */
    for (i = 0; i < size; i++) {
        /* Pattern 1: Simple conditional with goto to label */
        if (__builtin_expect((arr1[i] & 1) != 0, 0)) {
            /* Jump to label L1 - should be simplejump_p */
            goto L1;
        }
        
        /* Some arithmetic to create scheduling context */
        temp1 = arr1[i] * 3;
        temp2 = arr2[i] * 5;
        
        /* Continue normal flow */
        acc1 += temp1 + temp2;
        continue;
        
    L1:
        /* Candidate for delay slot filling: simple non-trapping arithmetic */
        /* This must not reference resources in &set or &needed */
        r3 = r1 + r2;  /* Simple add - no trap possible */
        acc2 += r3;
        
        /* Another jump pattern */
        if (__builtin_expect((arr2[i] & 2) != 0, 1)) {
            goto L2;
        }
        
        temp3 = arr1[i] - arr2[i];
        acc3 += temp3;
        continue;
        
    L2:
        /* Another candidate instruction */
        r4 = r1 ^ r2;  /* Bitwise XOR - safe, non-trapping */
        acc2 ^= r4;
        
        /* Nested loop to increase scheduling complexity */
        for (j = 0; j < 3; j++) {
            /* Pattern 3: Jump within nested loop */
            if (__builtin_expect((i + j) % 5 == 0, 0)) {
                goto L3;
            }
            
            /* Some memory operations to stress scheduler */
            arr1[i] += j;
            arr2[i] -= j;
            continue;
            
        L3:
            /* Candidate: simple arithmetic with different registers */
            temp1 = r3 << 2;  /* Shift operation - safe */
            acc1 += temp1;
            
            /* Insert memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            /* Another goto pattern */
            if (__builtin_expect(j % 2 == 0, 1)) {
                goto L4;
            }
            
            arr1[i] *= 2;
            continue;
            
        L4:
            /* Final candidate: arithmetic with constants */
            r2 = r4 + 42;  /* Constant addition - definitely safe */
            acc3 += r2;
        }
    }
    
    /* Use results to prevent dead code elimination */
    arr1[0] = acc1 + acc2 + acc3;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_region(int *data, int n) {
    int i = 0;
    int a = 0, b = 0, c = 0, d = 0;
    
    /* Create many basic blocks with label jumps */
    while (i < n) {
        /* Pattern A */
        if (data[i] > 100) goto LABEL_A;
        a += data[i];
        i++;
        continue;
        
    LABEL_A:
        /* Candidate: simple subtraction */
        b = data[i] - 10;  /* Safe if data[i] is not INT_MIN */
        i++;
        
        /* Pattern B */
        if (data[i] < 50) goto LABEL_B;
        c += data[i] * 2;
        i++;
        continue;
        
    LABEL_B:
        /* Candidate: bitwise operation */
        d = data[i] & 0xFF;  /* Masking - always safe */
        i++;
        
        /* Pattern C - unrolled for more opportunities */
        if (i + 1 < n) {
            if (data[i] == data[i + 1]) goto LABEL_C;
            a += data[i];
            b += data[i + 1];
            i += 2;
            continue;
            
        LABEL_C:
            /* Candidate: addition with constant */
            c = data[i] + 7;  /* Safe addition */
            i += 2;
        }
    }
    
    /* Prevent elimination */
    data[0] = a + b + c + d;
}

/* Mix integer and float to diversify resource usage */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void mixed_operations(float *farr, int *iarr, int len) {
    int i;
    float fsum = 0.0f;
    int isum = 0;
    
    for (i = 0; i < len; i++) {
        /* Integer branch pattern */
        if (__builtin_expect(iarr[i] % 3 == 0, 0)) {
            goto FLOAT_LABEL;
        }
        
        /* Integer arithmetic */
        isum += iarr[i] * 2;
        continue;
        
    FLOAT_LABEL:
        /* Candidate: simple float addition (non-trapping) */
        fsum += farr[i];  /* Assuming farr contains normal numbers */
        
        /* Another integer branch */
        if (__builtin_expect(iarr[i] > 1000, 1)) {
            goto INT_LABEL;
        }
        
        fsum *= 1.01f;
        continue;
        
    INT_LABEL:
        /* Candidate: integer bit operation */
        isum |= (iarr[i] & 0xFFFF);  /* Safe masking */
    }
    
    /* Use results */
    iarr[0] = isum + (int)fsum;
}

int main() {
    const int SIZE = 1000;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    float *farray = malloc(SIZE * sizeof(float));
    
    if (!array1 || !array2 || !farray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch diversity */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) % 100;
        array2[i] = (i * 53) % 100;
        farray[i] = (float)i * 0.1f;
    }
    
    /* Execute kernels designed to trigger delay slot filling */
    delay_slot_kernel(array1, array2, SIZE);
    branch_dense_region(array1, SIZE / 2);
    mixed_operations(farray, array2, SIZE);
    
    /* Print result to ensure no dead code elimination */
    printf("Result: %d %d %f\n", array1[0], array2[0], farray[0]);
    
    free(array1);
    free(array2);
    free(farray);
    
    return 0;
}
