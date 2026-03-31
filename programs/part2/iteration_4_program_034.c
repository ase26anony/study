/* 
 * Target: Trigger uncovered delay slot filling logic in GCC's reorg.cc
 * Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS architecture if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Avoid optimization removing our careful patterns */
#define KEEP(expr) do { asm volatile("" : : "r"(expr)); } while(0)

MIPS_TARGET
void delay_slot_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int a = 0, b = 0, c = 0, d = 0;
    int x = 1, y = 2, z = 3;
    int tmp1, tmp2, tmp3;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Pattern 1: Simple jump to label with eligible follower */
    for (i = 0; i < size; i++) {
        /* Create scheduling pressure with multiple branches */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            /* Jump to label L1 */
            goto L1;
        }
        
        /* Some computation to create resource dependencies */
        r1 = arr1[i] + arr2[i];
        r2 = r1 * 2;
        
        /* Another conditional jump */
        if (__builtin_expect(r2 < 0, 0)) {
            goto L2;
        }
        
        /* Continue normal execution */
        arr1[i] = r2;
        continue;
        
    L1:
        /* CRITICAL: This instruction must be eligible for delay slot */
        /* Simple, non-trapping integer operation */
        s1 = s2 + s3;  /* Uses different register set than jump context */
        /* Followed by more operations */
        s4 = s1 * 2;
        arr1[i] = s4;
        continue;
        
    L2:
        /* Another eligible candidate */
        s2 = s3 ^ s4;  /* Bitwise operation - non-trapping */
        arr2[i] = s2;
    }
    
    /* Pattern 2: Nested loops with label jumps */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Create complex control flow */
            tmp1 = arr1[i] + j;
            
            /* Force branch prediction variance */
            if (__builtin_expect((tmp1 & 1) == 0, 1)) {
                goto L3;
            }
            
            /* Alternate path */
            tmp2 = arr2[i] - j;
            KEEP(tmp2);
            continue;
            
        L3:
            /* Eligible instruction after label */
            tmp3 = a + b;  /* Uses different vars than surrounding context */
            arr1[i] += tmp3;
        }
    }
    
    /* Pattern 3: Switch-like pattern with goto labels */
    for (i = 0; i < size; i++) {
        int selector = arr1[i] % 4;
        
        /* Multiple jump targets in close proximity */
        switch (selector) {
            case 0: goto TARGET_A;
            case 1: goto TARGET_B;
            case 2: goto TARGET_C;
            default: goto TARGET_D;
        }
        
    TARGET_A:
        /* Simple arithmetic - delay slot candidate */
        c = d << 1;  /* Shift operation - non-trapping */
        arr1[i] = c;
        continue;
        
    TARGET_B:
        d = c | 0xFF;  /* Bitwise OR with constant */
        arr2[i] = d;
        continue;
        
    TARGET_C:
        a = b + 1;  /* Increment - safe */
        arr1[i] = a;
        continue;
        
    TARGET_D:
        b = a - 1;  /* Decrement - safe */
        arr2[i] = b;
    }
    
    /* Pattern 4: While loop with internal label jumps */
    i = 0;
    while (i < size) {
        /* Memory barrier to constrain scheduling */
        asm volatile("" ::: "memory");
        
        if (__builtin_expect(arr1[i] == arr2[i], 0)) {
            goto MATCH_LABEL;
        }
        
        /* Normal path computation */
        x = y * z;
        arr1[i] = x;
        i++;
        continue;
        
    MATCH_LABEL:
        /* Candidate for delay slot filling */
        y = z + x;  /* Simple add with distinct vars */
        arr2[i] = y;
        i++;
    }
    
    /* Mix integer and float to diversify resource usage */
    {
        float f1 = 1.0, f2 = 2.0;
        for (i = 0; i < 8; i++) {
            /* Integer branch in floating context */
            if (__builtin_expect(i % 2 == 0, 0)) {
                goto FLOAT_LABEL;
            }
            
            f1 = f2 * 3.14f;
            KEEP(f1);
            continue;
            
        FLOAT_LABEL:
            /* Integer operation in float context - good candidate */
            r3 = r4 & 0x7F;  /* Bitwise AND - safe */
            KEEP(r3);
        }
    }
}

/* Helper to prevent dead code elimination */
MIPS_TARGET
int compute_hash(int *arr, int size) {
    int hash = 0;
    int i;
    
    /* Tight loop with many branches */
    for (i = 0; i < size; i++) {
        /* Multiple conditional jumps */
        if (__builtin_expect(arr[i] > 0, 1)) {
            goto POSITIVE;
        }
        
        hash = hash * 31 - arr[i];
        continue;
        
    POSITIVE:
        /* Simple operation after label */
        hash = hash * 31 + arr[i];
    }
    
    return hash;
}

MIPS_TARGET
int main() {
    const int SIZE = 1024;
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int i, result;
    
    if (!arr1 || !arr2) {
        return 1;
    }
    
    /* Initialize with pattern that creates branch variance */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (i * 17) % 100;
        arr2[i] = (i * 23) % 100;
    }
    
    /* Execute the delay slot patterns */
    delay_slot_pattern(arr1, arr2, SIZE);
    
    /* Compute result to prevent elimination */
    result = compute_hash(arr1, SIZE) + compute_hash(arr2, SIZE);
    
    /* Print to ensure side effects */
    printf("Result: %d\n", result);
    
    /* Sample output to verify */
    printf("Sample values: %d %d %d\n", arr1[0], arr1[SIZE/2], arr1[SIZE-1]);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
