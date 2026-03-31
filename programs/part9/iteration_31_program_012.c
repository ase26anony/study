/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));
void complex_calculation(int *state, volatile int *vars, volatile float *fvars, 
                         volatile double *dvars) __attribute__((noinline, noipa));

/* Opaque function implementations */
int opaque_int(int x) {
    volatile int dummy = x;
    asm volatile ("" : "+r" (dummy) : : "memory");
    return dummy;
}

float opaque_float(float x) {
    volatile float dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

double opaque_double(double x) {
    volatile double dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

void scheduling_barrier(void) {
    /* Inline assembly with memory clobber to force scheduling boundary */
    asm volatile ("" : : : "memory");
}

/* Complex calculation with mixed operations to challenge scheduler */
void complex_calculation(int *state, volatile int *vars, volatile float *fvars, 
                         volatile double *dvars) {
    int local_state = *state;
    
    /* Long dependency chain with mixed operations */
    vars[0] = vars[1] + vars[2];
    vars[3] = vars[0] * vars[4] - vars[5];
    fvars[0] = fvars[1] * 1.5f + fvars[2];
    dvars[0] = dvars[1] / 2.0 + dvars[2] * 0.75;
    
    /* Cross-type dependencies */
    vars[6] = (int)fvars[0] ^ vars[3];
    fvars[3] = (float)vars[6] * 0.25f;
    
    /* State-dependent operation */
    if (local_state & 1) {
        vars[7] = vars[6] << (vars[3] & 0x7);
    } else {
        vars[7] = vars[6] >> (vars[4] & 0x7);
    }
    
    /* Update state based on complex condition */
    *state = (vars[0] ^ vars[3] ^ vars[6]) & 0xF;
    
    scheduling_barrier();
}

/* Main scheduling stress function */
void scheduling_stress(void) __attribute__((noinline, noipa));
void scheduling_stress(void) {
    volatile int int_vars[16];
    volatile float float_vars[8];
    volatile double double_vars[8];
    volatile int array_indices[4];
    int local_array[32];
    int state = 0;
    int i, j, k;
    
    /* Initialize volatile variables with pseudo-random values */
    for (i = 0; i < 16; i++) {
        int_vars[i] = (i * 37 + 123) & 0xFF;
    }
    for (i = 0; i < 8; i++) {
        float_vars[i] = (float)(i * 19 + 45) / 10.0f;
        double_vars[i] = (double)(i * 23 + 67) / 15.0;
    }
    for (i = 0; i < 4; i++) {
        array_indices[i] = (i * 11 + 7) & 0x1F;
    }
    for (i = 0; i < 32; i++) {
        local_array[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (i = 0; i < 50; i++) {
        /* Update array indices with data-dependent calculation */
        array_indices[0] = (array_indices[0] + int_vars[0]) & 0x1F;
        array_indices[1] = (array_indices[1] + int_vars[1]) & 0x1F;
        array_indices[2] = (array_indices[2] * 3 + 7) & 0x1F;
        array_indices[3] = (array_indices[3] ^ int_vars[2]) & 0x1F;
        
        /* State machine using switch statement - creates multiple basic blocks */
        switch (state & 0x7) {
            case 0:
                /* Chain of dependent integer operations */
                int_vars[8] = int_vars[0] + int_vars[1];
                int_vars[9] = int_vars[8] * int_vars[2] - int_vars[3];
                int_vars[10] = int_vars[9] >> (int_vars[4] & 0x3);
                int_vars[11] = int_vars[10] ^ int_vars[5];
                int_vars[12] = int_vars[11] + int_vars[6] * int_vars[7];
                /* Call to create scheduling boundary */
                int_vars[13] = opaque_int(int_vars[12]);
                scheduling_barrier();
                state = (state + 1) & 0x7;
                break;
                
            case 1:
                /* Mixed integer/float operations */
                float_vars[4] = float_vars[0] * 2.0f + float_vars[1];
                int_vars[14] = (int)(float_vars[4] * 10.0f);
                float_vars[5] = (float)int_vars[14] / 3.0f + float_vars[2];
                double_vars[4] = (double)float_vars[5] * 1.5;
                int_vars[15] = (int)(double_vars[4] * 2.0);
                scheduling_barrier();
                state = (state + 3) & 0x7;
                break;
                
            case 2:
                /* Memory access pattern with volatile indices */
                for (k = 0; k < 4; k++) {
                    int idx = array_indices[k];
                    local_array[idx] = local_array[idx] + int_vars[k];
                    /* Create dependency between array accesses */
                    if (k > 0) {
                        local_array[idx] = local_array[idx] ^ local_array[array_indices[k-1]];
                    }
                }
                scheduling_barrier();
                state = (state + 2) & 0x7;
                break;
                
            case 3:
                /* Long dependency chain with function calls */
                int_vars[0] = opaque_int(int_vars[15] + int_vars[14]);
                float_vars[0] = opaque_float(float_vars[5] * 0.5f);
                double_vars[0] = opaque_double(double_vars[4] / 1.3);
                int_vars[1] = int_vars[0] * 3 - (int)float_vars[0];
                float_vars[1] = (float)int_vars[1] + float_vars[0];
                scheduling_barrier();
                state = (state + 1) & 0x7;
                break;
                
            case 4:
                /* Computed goto state machine simulation */
                {
                    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
                    int label_idx = int_vars[2] & 0x3;
                    
                    goto *labels[label_idx];
                    
                label0:
                    int_vars[2] = int_vars[2] + int_vars[3];
                    goto end_labels;
                label1:
                    int_vars[2] = int_vars[2] * int_vars[4];
                    goto end_labels;
                label2:
                    int_vars[2] = int_vars[2] - int_vars[5];
                    goto end_labels;
                label3:
                    int_vars[2] = int_vars[2] ^ int_vars[6];
                    goto end_labels;
                end_labels:
                    scheduling_barrier();
                }
                state = (state + 2) & 0x7;
                break;
                
            case 5:
                /* Nested loops with break conditions */
                for (j = 0; j < 10; j++) {
                    int_vars[7] = int_vars[7] + j * int_vars[8];
                    if (int_vars[7] > 1000) {
                        /* Data-dependent break */
                        break;
                    }
                    float_vars[6] = float_vars[6] + (float)j * 0.1f;
                }
                scheduling_barrier();
                state = (state + 1) & 0x7;
                break;
                
            case 6:
                /* Use x86-specific builtin if available */
                #ifdef __x86_64__
                {
                    unsigned long long tsc1, tsc2;
                    tsc1 = __builtin_ia32_rdtsc();
                    int_vars[9] = int_vars[9] * 7 + 11;
                    tsc2 = __builtin_ia32_rdtsc();
                    int_vars[10] = (int)(tsc2 - tsc1);
                }
                #else
                /* Alternative for non-x86 */
                int_vars[9] = int_vars[9] * 7 + 11;
                int_vars[10] = int_vars[9] ^ 0x55AA55AA;
                #endif
                scheduling_barrier();
                state = (state + 1) & 0x7;
                break;
                
            case 7:
                /* Call complex calculation function */
                complex_calculation(&state, int_vars, float_vars, double_vars);
                state = (state + 1) & 0x7;
                break;
        }
        
        /* Inner loop with memory dependencies */
        for (j = 0; j < 8; j++) {
            int idx1 = array_indices[j & 0x3];
            int idx2 = array_indices[(j + 1) & 0x3];
            local_array[idx1] = local_array[idx1] + int_vars[j];
            local_array[idx2] = local_array[idx2] ^ local_array[idx1];
            /* Volatile memory write to prevent reordering */
            *(volatile int *)&local_array[idx1] = local_array[idx1];
        }
        
        /* Manual loop unrolling to expand scheduler state */
        /* Unrolled dependency chain */
        int_vars[0] = int_vars[0] + 1;
        int_vars[1] = int_vars[1] * int_vars[0];
        int_vars[2] = int_vars[2] + int_vars[1];
        int_vars[3] = int_vars[3] ^ int_vars[2];
        int_vars[4] = int_vars[4] * 3 + int_vars[3];
        int_vars[5] = int_vars[5] - int_vars[4];
        int_vars[6] = int_vars[6] & int_vars[5];
        int_vars[7] = int_vars[7] | int_vars[6];
        
        /* Call opaque function to force scheduling context save */
        if (i % 7 == 0) {
            int_vars[8] = opaque_int(int_vars[7]);
        }
    }
    
    /* Final scheduling barrier */
    scheduling_barrier();
}

int main(void) {
    int i;
    int checksum = 0;
    
    srand(time(NULL));
    
    /* Repeated calls to stress the scheduler */
    for (i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Simple progress indicator */
        if (i % 10 == 0) {
            printf("Iteration %d\n", i);
        }
    }
    
    printf("Scheduling stress test completed.\n");
    
    return 0;
}
