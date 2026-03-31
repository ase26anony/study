/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -o reorg_test reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling natively */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int size) {
    int i, j;
    int temp1, temp2, temp3;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Main computational loop with dense branching */
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with goto patterns */
        if (__builtin_expect((a[i] & 1) == 0, 1)) {
            /* Jump to label L1 - simple unconditional jump */
            goto L1;
        } else {
            /* Alternate path with different operations */
            r1 = a[i] + b[i];
            r2 = r1 * 2;
            goto L2;
        }
        
    L1:
        /* Candidate instruction for delay slot filling */
        /* Simple, non-trapping integer operation */
        s1 = s2 + s3;  /* Uses different register set than r1-r4 */
        
        /* Continue with other operations */
        temp1 = a[i] * 3;
        acc += temp1;
        continue;
        
    L2:
        /* Another label with eligible follower */
        s3 = s4 ^ 0xFF;  /* Bitwise operation - safe, non-trapping */
        
        temp2 = b[i] - a[i];
        acc += temp2;
        
        /* Nested loop to increase scheduling complexity */
        for (j = 0; j < 4; j++) {
            /* More goto patterns inside nested loop */
            if (__builtin_expect((j & 1) == 0, 0)) {
                goto L3;
            }
            
            /* Memory barrier to constrain scheduling */
            asm volatile("" ::: "memory");
            
            r3 = r4 << 2;
            continue;
            
        L3:
            /* Another candidate for delay slot */
            s2 = s1 | 0x0F;  /* Simple bitwise OR */
            
            r4 = j * 5;
            acc += r4;
        }
    }
    
    /* Additional branching patterns */
    for (i = 0; i < size; i += 2) {
        /* Complex conditional with goto to label */
        if (a[i] > b[i]) {
            /* Force jump to label with simple follower */
            if (__builtin_expect(acc > 1000, 0)) {
                goto L4;
            }
            
            /* Mix integer and bit operations */
            temp3 = a[i] & b[i];
            acc += temp3;
        } else {
            /* Another path with goto */
            goto L5;
        }
        
        /* Insert memory barrier in some paths */
        if (i % 3 == 0) {
            __sync_synchronize();
        }
        
        continue;
        
    L4:
        /* Candidate: simple arithmetic with distinct registers */
        s4 = s3 - s2;  /* Subtraction - non-trapping */
        
        acc += a[i] * b[i];
        continue;
        
    L5:
        /* Candidate: simple assignment */
        r2 = r1 + 1;  /* Increment - safe operation */
        
        acc += b[i] - a[i];
    }
    
    /* Prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void branch_dense_function(int *arr, int n) {
    int i;
    int x = 0, y = 0, z = 0;
    
    /* Very tight loop with many branches */
    for (i = 0; i < n; i++) {
        /* Pattern 1: jump to label with simple arithmetic follower */
        if (arr[i] < 0) {
            goto LABEL_A;
        }
        
        /* Pattern 2: different operation set */
        x = y * 2;
        goto LABEL_B;
        
    LABEL_A:
        /* Perfect delay slot candidate:
           - Simple operation (addition)
           - Uses different registers than surrounding code
           - Non-trapping
           - Not a SEQUENCE */
        z = x + y;
        
        arr[i] = z;
        continue;
        
    LABEL_B:
        /* Another candidate */
        y = z ^ i;  /* Bitwise XOR - safe */
        
        arr[i] = y;
        
        /* Additional conditional to create more basic blocks */
        if (i % 5 == 0) {
            goto LABEL_C;
        }
        
        x++;
        continue;
        
    LABEL_C:
        /* Simple assignment candidate */
        x = y & 0x7F;  /* Bitwise AND with constant */
        
        arr[i] = x * 2;
    }
}

int main() {
    const int SIZE = 100;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
    }
    
    /* Call the kernel function */
    delay_slot_kernel(array1, array2, SIZE);
    
    /* Call secondary function */
    branch_dense_function(array1, SIZE / 2);
    
    /* Compute and print result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i];
    }
    
    printf("Result: %d\n", sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
