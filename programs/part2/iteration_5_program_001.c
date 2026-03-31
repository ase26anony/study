/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -mips32 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex resource allocation */
#define FORCE_SCHEDULER_BARRIER() asm volatile("" ::: "memory")

/* Function pointer to create control flow complexity */
typedef int (*compute_func_t)(int, int, float*);

/* Different computation paths to create scheduling pressure */
static int compute_path_a(int base, int idx, float* fp_vars) {
    volatile int v1 = base + idx * 3;
    int v2 = v1 ^ 0x55AA55AA;
    int v3 = v2 * 7;
    int v4 = v3 - (idx << 2);
    int v5 = v4 & 0x0F0F0F0F;
    int v6 = v5 | (idx * 0x101);
    int v7 = v6 ^ v1;
    int v8 = v7 * 13;
    
    /* Mix in floating point operations */
    float f1 = fp_vars[0] * 1.5f + (float)v8;
    float f2 = fp_vars[1] / 1.3f - f1;
    float f3 = fp_vars[2] + f2 * 2.0f;
    float f4 = fp_vars[3] - f3 / 1.7f;
    
    fp_vars[0] = f1;
    fp_vars[1] = f2;
    fp_vars[2] = f3;
    fp_vars[3] = f4;
    
    return v8 + (int)f4;
}

static int compute_path_b(int base, int idx, float* fp_vars) {
    volatile int v1 = base - idx * 5;
    int v2 = v1 & 0xAA55AA55;
    int v3 = v2 / 3;
    int v4 = v3 + (idx << 3);
    int v5 = v4 ^ 0xF0F0F0F0;
    int v6 = v5 | (idx * 0x202);
    int v7 = v6 & v1;
    int v8 = v7 * 17;
    
    /* Different floating point pattern */
    float f1 = fp_vars[4] / 2.0f + (float)v8;
    float f2 = fp_vars[5] * 1.8f - f1;
    float f3 = fp_vars[6] - f2 / 1.2f;
    float f4 = fp_vars[7] + f3 * 3.0f;
    
    fp_vars[4] = f1;
    fp_vars[5] = f2;
    fp_vars[6] = f3;
    fp_vars[7] = f4;
    
    return v8 - (int)f4;
}

/* Complex control flow with goto to create basic block merging */
static void process_block(int* arr1, int* arr2, int idx, int* int_results, float* fp_results) {
    int local1, local2, local3, local4, local5, local6;
    int local7, local8, local9, local10, local11, local12;
    float flocal1, flocal2, flocal3, flocal4, flocal5, flocal6;
    float flocal7, flocal8, flocal9, flocal10, flocal11, flocal12;
    
    /* Initialize many variables to create register pressure */
    local1 = arr1[idx];
    local2 = arr2[idx];
    local3 = local1 ^ local2;
    local4 = local1 & local2;
    local5 = local1 | local2;
    local6 = local1 - local2;
    
    flocal1 = (float)local1 * 0.5f;
    flocal2 = (float)local2 * 1.5f;
    flocal3 = flocal1 + flocal2;
    flocal4 = flocal1 - flocal2;
    
    /* Data-dependent branch - hard to predict */
    if (__builtin_expect((local1 & 0x7F) > (local2 & 0x3F), 0)) {
        compute_func_t func = compute_path_a;
        local7 = func(local1, idx, &flocal1);
        local8 = local7 * 3;
        local9 = local8 ^ 0x12345678;
        
        flocal5 = flocal3 * 2.0f;
        flocal6 = flocal4 / 1.5f;
        flocal7 = flocal5 + flocal6;
        
        /* Force serialization point */
        FORCE_SCHEDULER_BARRIER();
        
        goto merge_point;
    } else {
        compute_func_t func = compute_path_b;
        local10 = func(local2, idx, &flocal3);
        local11 = local10 / 5;
        local12 = local11 & 0x87654321;
        
        flocal8 = flocal3 / 3.0f;
        flocal9 = flocal4 * 1.7f;
        flocal10 = flocal8 - flocal9;
        
        /* Different serialization point */
        FORCE_SCHEDULER_BARRIER();
        
        goto merge_point;
    }
    
merge_point:
    /* More operations after merge - creates scheduling complexity */
    local7 = local3 * local4 + local5;
    local8 = local6 ^ local7;
    local9 = local8 * 19;
    local10 = local9 - idx;
    local11 = local10 & 0x0FF00FF;
    local12 = local11 | 0xA5A5A5A5;
    
    flocal11 = flocal7 + flocal10;
    flocal12 = flocal11 * 0.9f;
    
    /* Store results to prevent optimization */
    int_results[0] = local7;
    int_results[1] = local8;
    int_results[2] = local9;
    int_results[3] = local10;
    int_results[4] = local11;
    int_results[5] = local12;
    
    fp_results[0] = flocal7;
    fp_results[1] = flocal8;
    fp_results[2] = flocal9;
    fp_results[3] = flocal10;
    fp_results[4] = flocal11;
    fp_results[5] = flocal12;
}

int main() {
    const int ARRAY_SIZE = 256;
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFFFFFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    int int_results[6];
    float fp_results[6];
    
    /* Main computation loop with switch for control flow complexity */
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Switch creates multiple basic blocks */
            switch (iter & 0x3) {
                case 0:
                    process_block(array1, array2, i, int_results, fp_results);
                    array1[i] ^= int_results[0];
                    break;
                case 1:
                    process_block(array2, array1, i, int_results, fp_results);
                    array2[i] ^= int_results[1];
                    break;
                case 2:
                    process_block(array1, array2, (i + 1) % ARRAY_SIZE, 
                                 int_results, fp_results);
                    array1[i] += int_results[2];
                    break;
                case 3:
                    process_block(array2, array1, (i + 1) % ARRAY_SIZE,
                                 int_results, fp_results);
                    array2[i] += int_results[3];
                    break;
            }
            
            /* Another barrier inside the loop */
            if ((i & 0x1F) == 0) {
                FORCE_SCHEDULER_BARRIER();
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
        fchecksum += (float)array1[i] * 0.001f;
        fchecksum -= (float)array2[i] * 0.001f;
    }
    
    checksum += (int)fchecksum;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
