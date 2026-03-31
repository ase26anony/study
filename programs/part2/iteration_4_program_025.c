/* 
 * Target: Trigger uncovered delay slot filling logic in GCC's reorg.cc
 * Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Avoid optimization removing our carefully crafted patterns */
#define KEEP(expr) do { asm volatile("" : : "r"(expr)); } while(0)

MIPS_TARGET
static void delay_slot_patterns(int *arr1, int *arr2, int size) {
    /* Use distinct register sets to avoid resource conflicts */
    register int r1 asm("$16") = 0;
    register int r2 asm("$17") = 0;
    register int r3 asm("$18") = 0;
    register int r4 asm("$19") = 0;
    register int r5 asm("$20") = 0;
    
    int i = 0;
    
    /* Create multiple basic blocks with jumps to labels */
    while (i < size) {
        /* Pattern 1: Simple jump to label with eligible follower */
        if (__builtin_expect(arr1[i] > 100, 0)) {
            goto label1;  /* simplejump_p(trial) = true */
        }
        
        /* Some computation to create scheduling context */
        r1 = arr1[i] * 3;
        r2 = arr2[i] + 7;
        
        /* Pattern 2: Another jump pattern */
        if (__builtin_expect(r1 < r2, 1)) {
            goto label2;
        }
        
        /* Continue normal flow */
        arr1[i] = r1 + r2;
        i++;
        continue;
        
    label1:
        /* Candidate instruction for delay slot filling */
        /* Must be: NONJUMP_INSN_P, not SEQUENCE, not JUMP_P */
        /* Must not trap: use safe arithmetic */
        r3 = r4 + r5;  /* Simple add - won't trap */
        
        /* Ensure this doesn't conflict with resources in delay slot */
        arr1[i] = r3;
        i++;
        continue;
        
    label2:
        /* Another candidate with different registers */
        r4 = r2 ^ r1;  /* Bitwise operation - won't trap */
        
        /* Memory barrier to constrain scheduling in other paths */
        asm volatile("" ::: "memory");
        
        arr2[i] = r4;
        i++;
        continue;
    }
}

MIPS_TARGET
static void nested_loop_patterns(int *matrix, int dim) {
    /* Create complex control flow with multiple jump targets */
    register int a asm("$21") = 0;
    register int b asm("$22") = 0;
    register int c asm("$23") = 0;
    register int d asm("$24") = 0;
    
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            /* Pattern 3: Jump to label with simple arithmetic follower */
            if (__builtin_expect(matrix[i * dim + j] == 0, 0)) {
                goto process_zero;
            }
            
            /* Normal processing */
            a = matrix[i * dim + j];
            b = a * 2;
            
            /* Pattern 4: Conditional jump */
            if (__builtin_expect(b > 1000, 0)) {
                goto handle_large;
            }
            
            matrix[i * dim + j] = b;
            continue;
            
        process_zero:
            /* Eligible delay slot candidate */
            c = d + 1;  /* Simple increment - splittable, non-trapping */
            matrix[i * dim + j] = c;
            continue;
            
        handle_large:
            /* Another candidate */
            d = a >> 2;  /* Shift operation - won't trap */
            matrix[i * dim + j] = d;
            
            /* Insert scheduling barrier in alternate path */
            if (i % 2 == 0) {
                __sync_synchronize();
            }
            continue;
        }
    }
}

MIPS_TARGET
static void mixed_operations(int *data, int n) {
    /* Mix integer and control flow to stress scheduler */
    register int x asm("$25") = 0;
    register int y asm("$26") = 0;
    register int z asm("$27") = 0;
    
    int i = 0;
    while (i < n) {
        /* Create multiple jump opportunities */
        switch (data[i] % 4) {
            case 0:
                goto case0_label;
            case 1:
                goto case1_label;
            case 2:
                /* Fall through - no jump */
                break;
            case 3:
                goto case3_label;
        }
        
        /* Default processing */
        x = data[i] * 3;
        i++;
        continue;
        
    case0_label:
        /* Candidate: simple arithmetic after label */
        y = z + x;  /* Uses different registers than delay slot would */
        data[i] = y;
        i++;
        continue;
        
    case1_label:
        /* Another candidate */
        z = x & 0xFF;  /* Bitwise AND - safe */
        data[i] = z;
        i++;
        continue;
        
    case3_label:
        /* More complex to create scheduling pressure */
        x = (y << 1) | 1;
        data[i] = x;
        i++;
        continue;
    }
}

MIPS_TARGET
int main(void) {
    const int SIZE = 1000;
    const int DIM = 32;
    
    /* Initialize data arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(DIM * DIM * sizeof(int));
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !matrix || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varied data to trigger different branches */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 200;
        arr2[i] = rand() % 200;
        data[i] = rand() % 100;
    }
    
    for (int i = 0; i < DIM * DIM; i++) {
        matrix[i] = rand() % 100;
        if (rand() % 10 == 0) matrix[i] = 0;  /* Ensure some zeros */
    }
    
    /* Execute patterns to generate specific RTL sequences */
    delay_slot_patterns(arr1, arr2, SIZE);
    nested_loop_patterns(matrix, DIM);
    mixed_operations(data, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + data[i];
    }
    
    for (int i = 0; i < DIM * DIM; i++) {
        checksum += matrix[i];
    }
    
    /* Use result to ensure no optimization removes our code */
    KEEP(checksum);
    
    printf("Result checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(matrix);
    free(data);
    
    return 0;
}
