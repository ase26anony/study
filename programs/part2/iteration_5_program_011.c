/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -funroll-loops=2 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force register pressure by using many variables in a small scope */
#define FORCE_REGISTER_PRESSURE \
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15; \
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10; \
    volatile int mem_barrier __attribute__((unused));

/* Different computation paths to create scheduling complexity */
__attribute__((noinline))
static int path_a_computation(int base, int idx) {
    FORCE_REGISTER_PRESSURE;
    
    /* Initialize with non-trivial values to prevent optimization */
    r0 = base + idx * 3;
    r1 = base ^ idx;
    r2 = r0 * 7;
    r3 = r1 - r2;
    r4 = r3 & 0xFF;
    r5 = r4 | 0x100;
    r6 = r5 << 3;
    r7 = r6 >> 1;
    r8 = r7 + r0;
    r9 = r8 - r1;
    r10 = r9 * 2;
    r11 = r10 / 3;
    r12 = r11 ^ r2;
    r13 = r12 + r3;
    r14 = r13 & r4;
    r15 = r14 | r5;
    
    /* Floating point ops to use different functional units */
    f0 = r0 * 0.5f;
    f1 = f0 + 1.0f;
    f2 = f1 * f0;
    f3 = f2 - f1;
    f4 = f3 / 2.0f;
    f5 = f4 + f0;
    f6 = f5 * f1;
    f7 = f6 - f2;
    f8 = f7 + f3;
    f9 = f8 * f4;
    f10 = f9 / f5;
    
    /* Memory barrier to create serialization point */
    asm volatile("" ::: "memory");
    
    /* Complex result mixing int and float */
    return r15 + (int)(f10 * 100.0f);
}

__attribute__((noinline))
static int path_b_computation(int base, int idx) {
    FORCE_REGISTER_PRESSURE;
    
    /* Different computation pattern for path B */
    r0 = base * idx;
    r1 = r0 ^ 0xABCD;
    r2 = r1 + 17;
    r3 = r2 * 3;
    r4 = r3 / 5;
    r5 = r4 | 0x3F;
    r6 = r5 << 2;
    r7 = r6 >> 1;
    r8 = r7 ^ r0;
    r9 = r8 + r1;
    r10 = r9 - r2;
    r11 = r10 * r3;
    r12 = r11 & r4;
    r13 = r12 | r5;
    r14 = r13 ^ r6;
    r15 = r14 + r7;
    
    /* Different floating point pattern */
    f0 = r0 * 0.3f;
    f1 = f0 - 0.5f;
    f2 = f1 * 2.0f;
    f3 = f2 + f0;
    f4 = f3 / 1.5f;
    f5 = f4 - f1;
    f6 = f5 * f2;
    f7 = f6 + f3;
    f8 = f7 - f4;
    f9 = f8 * f5;
    f10 = f9 / f6;
    
    /* Memory barrier at different position */
    asm volatile("" ::: "memory");
    
    return r15 - (int)(f10 * 50.0f);
}

/* Complex control flow with switch and goto to create CFG complexity */
__attribute__((noinline))
static int complex_control_flow(int *arr1, int *arr2, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Data-dependent branch with __builtin_expect */
        if (__builtin_expect((arr1[i] & 0x7) > 4, 0)) {
            /* Path with nested loop */
            for (j = 0; j < 3; j++) {
                int temp = arr1[i] + j;
                result += path_a_computation(temp, i);
            }
        } else {
            /* Different path with switch */
            switch (arr1[i] & 0x3) {
                case 0:
                    result += path_b_computation(arr1[i], i);
                    /* Fall through to create merge point */
                case 1:
                    result += arr2[i] * 2;
                    goto common_label;
                case 2:
                    result += arr1[i] ^ arr2[i];
                    break;
                case 3:
                    result += arr1[i] | arr2[i];
                    goto common_label;
                default:
                    break;
            }
            continue;
            
        common_label:
            /* Common code reached from multiple paths */
            result += 1;
            asm volatile("" ::: "memory");
        }
        
        /* Additional computation to increase basic block size */
        result += (arr1[i] * arr2[i]) & 0xFF;
    }
    
    return result;
}

/* Main computational kernel */
__attribute__((noinline))
static int scheduler_kernel(int iterations) {
    const int ARRAY_SIZE = 256;
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    int i, final_result = 0;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 123456789;
    for (i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Main loop with high scheduling pressure */
    for (int iter = 0; iter < iterations; iter++) {
        /* Modify arrays slightly each iteration */
        for (i = 0; i < ARRAY_SIZE; i++) {
            array1[i] = (array1[i] * 13 + 7) & 0xFFF;
        }
        
        /* Complex computation with high register pressure */
        final_result += complex_control_flow(array1, array2, ARRAY_SIZE);
        
        /* Prevent loop invariant code motion */
        asm volatile("" ::: "memory");
    }
    
    return final_result;
}

int main(void) {
    int result;
    
    printf("Starting scheduler stress test...\n");
    
    /* Run kernel multiple times to increase chance of scheduler state saving */
    result = scheduler_kernel(1000);
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    /* Use result to prevent dead code elimination */
    return result == 0 ? 0 : 1;
}
