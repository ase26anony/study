/* Target: MIPS architecture with delay slots */
#ifndef __mips__
#error "This code is designed for MIPS architecture with delay slots. Compile with -march=mips"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Force delay slot filling context with MIPS target */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_kernel(int *a, int *b, int size) {
    int i;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create tight loop with multiple jump contexts */
    for (i = 0; i < size; i++) {
        /* Branch prediction hints to influence scheduling */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            if (a[i] > b[i]) {
                /* Unconditional jump to label L1 */
                goto L1;
            }
            
            /* Alternate path with different resource usage */
            r1 = a[i] + 1;
            r2 = b[i] - 1;
            continue;
            
        L1:
            /* Candidate instruction for delay slot filling */
            /* Simple, non-trapping integer operation */
            s1 = a[i] * 2;  /* This should be eligible for delay slot */
            
            /* Continue with other operations */
            acc += s1;
            continue;
        }
        
        /* Pattern 2: Another jump-label pattern with different registers */
        if (__builtin_expect((i & 3) == 0, 0)) {
            if (a[i] < 0) {
                goto L2;
            }
            
            r3 = a[i] << 1;
            r4 = b[i] >> 1;
            continue;
            
        L2:
            /* Another candidate - bitwise operation, no trapping */
            s2 = b[i] & 0xFF;  /* Eligible for delay slot */
            
            acc += s2;
            continue;
        }
        
        /* Pattern 3: Nested conditional jumps */
        if (a[i] != 0) {
            if (b[i] % 2 == 0) {
                goto L3;
            }
            
            temp1 = a[i] | b[i];
            continue;
            
        L3:
            /* Simple arithmetic - addition is non-trapping */
            s3 = a[i] + b[i];  /* Delay slot candidate */
            
            acc += s3;
            continue;
        }
        
        /* Pattern 4: Loop with multiple exit points using goto */
        if (i % 10 == 0) {
            int j;
            for (j = 0; j < 5; j++) {
                if (a[i] + j > 100) {
                    goto L4;
                }
                temp2 = a[i] * j;
            }
            continue;
            
        L4:
            /* Simple subtraction - safe for delay slot */
            s4 = b[i] - a[i];  /* Eligible candidate */
            
            acc += s4;
            continue;
        }
        
        /* Default path with memory barrier to constrain scheduling */
        __sync_synchronize();
        acc += a[i] + b[i];
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different register pressure */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void complex_control_flow(int *arr, int n) {
    int i, j;
    int x = 0, y = 0, z = 0;
    
    /* Create complex CFG with many basic blocks */
    for (i = 0; i < n; i++) {
        /* Multiple conditional jumps in sequence */
        if (arr[i] > 0) {
            if (arr[i] < 100) {
                goto BLOCK_A;
            }
            x += arr[i];
            goto BLOCK_C;
        }
        
        if (arr[i] == 0) {
            goto BLOCK_B;
        }
        
        y -= arr[i];
        continue;
        
    BLOCK_A:
        /* Simple operation after label - delay slot candidate */
        z = arr[i] + 1;  /* Eligible: non-trapping, simple */
        x += z;
        continue;
        
    BLOCK_B:
        /* Another candidate */
        z = arr[i] * 2;  /* Eligible */
        y += z;
        continue;
        
    BLOCK_C:
        /* Mix integer and bitwise ops */
        z = arr[i] ^ 0x55;  /* Eligible */
        x ^= z;
        continue;
    }
    
    arr[0] = x + y + z;
}

/* Main function that sets up the test */
int main() {
    const int SIZE = 1000;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        array2[i] = (i * 3) % 100;
    }
    
    /* Execute kernels designed to trigger delay slot filling */
    delay_slot_kernel(array1, array2, SIZE);
    complex_control_flow(array1, SIZE / 2);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", array1[0]);
    
    free(array1);
    free(array2);
    
    return 0;
}
