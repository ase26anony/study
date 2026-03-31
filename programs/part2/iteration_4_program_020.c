/* reorg_coverage.c - Target delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target for delay slots */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_patterns(int *arr1, int *arr2, int n) {
    int i, j;
    int a = 0, b = 0, c = 0, d = 0;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Pattern 1: Simple jump to label with eligible follower */
    for (i = 0; i < n; i++) {
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump to label L1 - should be simplejump_p */
            goto L1;
        } else {
            /* Different operations to create resource pressure */
            a = arr1[i] + arr2[i];
            b = a ^ 0x55;
            barrier = b; /* Memory barrier */
        }
        
        /* Continue normal loop */
        arr2[i] = arr1[i] * 2;
        continue;
        
    L1:
        /* Candidate for delay slot filling - must be non-trapping */
        /* Uses distinct registers from jump context */
        t1 = t2 + t3;  /* Simple add, no trap possible */
        /* Followed by more operations to prevent SEQUENCE formation */
        t4 = t1 | 0xFF;
        arr1[i] = t4;
    }
    
    /* Pattern 2: Nested loops with multiple jump-label patterns */
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            /* Create resource sets that don't overlap with candidates */
            int r1 = arr1[i];
            int r2 = arr2[j];
            int sum = r1 + r2;
            
            /* Conditional jump to label */
            if (__builtin_expect(sum > 200, 1)) {
                /* Force simple jump to label */
                goto L2;
            }
            
            /* Alternative path with different resource usage */
            c = r1 - r2;
            d = c & 0x7F;
            asm volatile("" ::: "memory"); /* Scheduling barrier */
            continue;
            
        L2:
            /* Another delay slot candidate */
            /* Uses completely different variables than preceding context */
            int x = t1, y = t2;  /* t1,t2 already set, no new dependencies */
            int z = x ^ y;       /* Simple bitwise, non-trapping */
            arr2[j] = z;
            
            /* Prevent fall-through to next label */
            if (z == 0) goto L3;
        }
    }
    
    /* Pattern 3: Switch-like structure with goto labels */
    int mode = arr1[0] % 4;
    
    if (mode == 0) {
        goto L4;
    } else if (mode == 1) {
        /* Complex path to stress scheduler */
        for (int k = 0; k < 3; k++) {
            arr1[k] += arr2[k];
        }
        goto L5;
    }
    
    /* Default path */
    a = b + c;
    goto L6;
    
L4:
    /* Candidate after label - safe integer arithmetic */
    t3 = t4 * 2;    /* Multiplication by 2 is safe (shift) */
    /* Ensure not a SEQUENCE by keeping it simple */
    arr1[1] = t3;
    goto L7;
    
L5:
    /* Another candidate - bitwise operations are safe */
    t2 = t1 ^ 0xAA;
    arr2[1] = t2;
    goto L7;
    
L6:
    /* Different operation type */
    t1 = t2 - t3;
    arr1[2] = t1;
    /* Fall through */
    
L7:
    /* Final mixing */
    t4 = t1 + t2 + t3;
    arr1[3] = t4;
}

/* Pattern 4: Function with computed goto (simulated) */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void computed_goto_pattern(int *arr, int n) {
    static void *labels[] = { &&L10, &&L11, &&L12, &&L13 };
    int i = 0;
    
    while (i < n) {
        int idx = arr[i] % 4;
        
        /* Jump to computed label */
        goto *labels[idx];
        
    L10:
        /* Candidate instruction - uses local temps */
        { int tmp = i * 2; arr[i] = tmp; }
        i++;
        continue;
        
    L11:
        { int tmp = i + 5; arr[i] = tmp; }
        i++;
        continue;
        
    L12:
        { int tmp = i ^ 0x33; arr[i] = tmp; }
        i++;
        continue;
        
    L13:
        { int tmp = i & 0x7F; arr[i] = tmp; }
        i++;
        continue;
    }
}

/* Main computational kernel */
int main() {
    const int N = 100;
    int *array1 = malloc(N * sizeof(int));
    int *array2 = malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch diversity */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17) % 233;
        array2[i] = (i * 29) % 197;
    }
    
    /* Call functions with delay slot patterns */
    delay_slot_patterns(array1, array2, N);
    
    /* Also trigger computed goto pattern */
    computed_goto_pattern(array1, N);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    /* Verify some values */
    printf("Sample values: %d, %d, %d\n", array1[0], array1[N/2], array1[N-1]);
    
    free(array1);
    free(array2);
    
    return 0;
}
