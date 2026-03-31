/* Target-specific compilation: gcc -O2 -march=mips -fdump-rtl-reorg -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not compiling with -march=mips */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_kernel(int *a, int *b, int size) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Create complex control flow with many jumps to labels */
    for (i = 0; i < size; i++) {
        /* Force branch prediction variance */
        if (__builtin_expect((i & 3) == 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            if (a[i] > 100) {
                /* Use volatile to prevent certain optimizations */
                asm volatile("" ::: "memory");
                goto label1;
            }
            
            /* Jump pattern 2 */
            if (b[i] < 50) {
                goto label2;
            }
            
            /* Jump pattern 3 */
            if ((a[i] + b[i]) > 150) {
                goto label3;
            }
            
            continue;
        }
        
        /* Alternate path with different resource usage */
        if (__builtin_expect((i & 1) == 0, 1)) {
            /* Mix integer operations to stress scheduler */
            r1 = a[i] * 3;
            r2 = b[i] / 2;  /* Safe division by constant */
            r3 = r1 ^ r2;
            acc += r3;
            
            /* Memory barrier to constrain scheduling */
            __sync_synchronize();
            
            /* Another jump pattern */
            if (r3 > 1000) {
                goto label4;
            }
        }
        
        /* Continue normal loop */
        s1 = a[i] + b[i];
        s2 = s1 * 2;
        acc += s2;
        continue;
        
    /* Target labels with simple, non-trapping arithmetic followers */
    label1:
        /* Simple arithmetic that doesn't trap and uses different registers */
        temp1 = r4 + s4;  /* Use previously unused registers */
        acc += temp1;
        /* Ensure this is not optimized away */
        asm volatile("" : "+r"(acc));
        continue;
        
    label2:
        /* Another simple operation - bitwise AND */
        temp2 = r3 & s3;
        acc ^= temp2;
        continue;
        
    label3:
        /* Subtraction - safe, non-trapping */
        temp3 = s2 - s1;
        acc += temp3;
        /* Insert scheduling barrier */
        asm volatile("" ::: "memory");
        continue;
        
    label4:
        /* Simple addition with constants */
        temp4 = 42 + 17;
        acc -= temp4;
        /* Force register usage */
        asm volatile("" : "+r"(acc));
        continue;
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void nested_loop_pattern(int *arr, int n) {
    int i, j, k;
    int x = 0, y = 0, z = 0;
    
    for (i = 0; i < n; i++) {
        /* Create inner loops with jumps */
        for (j = 0; j < 8; j++) {
            /* Varying trip counts create scheduling pressure */
            if (__builtin_expect((j & 1) == 0, 1)) {
                /* Jump to label with simple follower */
                if (arr[i] > arr[j]) {
                    goto inner_label1;
                }
                
                x = arr[i] + arr[j];
                y = x * 2;
                continue;
                
            inner_label1:
                /* Eligible delay slot candidate */
                z = y - x;  /* Simple subtraction */
                arr[i] += z;
                continue;
            } else {
                /* Different resource pattern */
                for (k = 0; k < 4; k++) {
                    if (__builtin_expect(k == 2, 0)) {
                        if (arr[k] != 0) {
                            goto inner_label2;
                        }
                    }
                    arr[k] = arr[k] + 1;
                    continue;
                    
                inner_label2:
                    /* Another simple operation */
                    arr[k] = arr[k] & 0xFF;
                    continue;
                }
            }
        }
        
        /* Mix with floating point in separate blocks */
        {
            float f1 = (float)arr[i];
            float f2 = f1 * 1.5f;
            /* Cast back to avoid FP in delay slot candidates */
            arr[i] = (int)f2;
        }
    }
}

/* Main driver */
int main() {
    const int SIZE = 256;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37) % 256;
        array2[i] = (i * 73) % 256;
    }
    
    /* Call kernels multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        compute_kernel(array1, array2, SIZE);
        nested_loop_pattern(array1, SIZE / 4);
    }
    
    /* Compute and print result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array1[i] + array2[i];
    }
    printf("Result: %d\n", sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
