/* haifa-sched-trigger.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler-test haifa-sched-trigger.c
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int __attribute__((noinline)) opaque_int(int x) {
    volatile int dummy = x;
    return dummy;
}

float __attribute__((noinline)) opaque_float(float x) {
    volatile float dummy = x;
    return dummy;
}

double __attribute__((noinline)) opaque_double(double x) {
    volatile double dummy = x;
    return dummy;
}

void __attribute__((noinline)) scheduling_barrier(void) {
    /* Inline assembly with memory clobber to force scheduling boundary */
    asm volatile("" : : : "memory");
}

/* State machine states */
enum states {
    STATE_ARITH_INT,
    STATE_ARITH_FLOAT,
    STATE_MEM_OPS,
    STATE_MIXED,
    STATE_CONTROL_FLOW,
    STATE_BARRIER,
    STATE_NEXT
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int vi1 = seed;
    volatile int vi2 = seed + 1;
    volatile int vi3 = seed + 2;
    volatile int vi4 = seed + 3;
    volatile int vi5 = seed + 4;
    
    volatile float vf1 = seed * 0.1f;
    volatile float vf2 = seed * 0.2f;
    volatile float vf3 = seed * 0.3f;
    
    volatile double vd1 = seed * 0.01;
    volatile double vd2 = seed * 0.02;
    
    volatile int state = STATE_ARITH_INT;
    volatile int loop_counter = 0;
    
    /* Local array with volatile accesses */
    int local_array[16];
    volatile int array_idx = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        local_array[i] = seed + i;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine with switch statement */
        switch (state) {
            case STATE_ARITH_INT: {
                /* Long chain of dependent integer operations */
                vi1 = opaque_int(vi1) + vi2;
                vi2 = vi1 * vi3 - vi4;
                vi3 = (vi2 >> 3) | (vi1 << 2);
                vi4 = vi3 ^ vi5;
                vi5 = (vi4 + vi1) * 7;
                
                /* More dependencies */
                vi1 = vi5 % 31;
                vi2 = vi1 * vi2 + vi3;
                vi3 = vi2 - vi4 * 2;
                vi4 = vi3 / (vi5 + 1);
                vi5 = vi4 | (vi1 & vi2);
                
                /* Call to noinline function creates scheduling boundary */
                vi1 = opaque_int(vi1 + vi2 + vi3 + vi4 + vi5);
                
                state = STATE_ARITH_FLOAT;
                break;
            }
            
            case STATE_ARITH_FLOAT: {
                /* Mixed float/double operations with dependencies */
                vf1 = opaque_float(vf1) + vf2 * 1.5f;
                vf2 = vf1 - vf3 / 2.0f;
                vf3 = vf2 * vf1 + 3.14f;
                
                vd1 = opaque_double(vd1) + vd2 * 1.7;
                vd2 = vd1 - vd2 / 3.0;
                
                /* Cross-type dependencies */
                vi1 = (int)(vf1 + vf2 + vf3);
                vf1 = (float)(vi1 + vi2) * 0.5f;
                
                state = STATE_MEM_OPS;
                break;
            }
            
            case STATE_MEM_OPS: {
                /* Memory operations with volatile indices */
                array_idx = (array_idx + 1) & 0xF;
                volatile int idx = array_idx;
                
                /* Chain of dependent memory operations */
                local_array[idx] = local_array[(idx + 1) & 0xF] + vi1;
                vi1 = local_array[idx] - vi2;
                local_array[(idx + 2) & 0xF] = vi1 * vi3;
                vi2 = local_array[(idx + 3) & 0xF] >> 2;
                local_array[(idx + 4) & 0xF] = vi2 | vi4;
                
                /* More complex memory pattern */
                for (int i = 0; i < 8; i++) {
                    volatile int j = (idx + i) & 0xF;
                    local_array[j] = local_array[j] + i * vi1;
                    vi3 = local_array[j] - vi5;
                }
                
                state = STATE_MIXED;
                break;
            }
            
            case STATE_MIXED: {
                /* Mixed operations creating complex dependencies */
                vi1 = vi1 + (int)vf1;
                vf1 = vf1 + (float)vi2;
                vd1 = vd1 + (double)vi3;
                
                /* Long dependency chain */
                vi2 = vi1 * 3 - vi4;
                vf2 = vf1 / 2.0f + (float)vi2;
                vi3 = vi2 + (int)(vf2 * 10.0f);
                vd2 = vd1 * 1.1 + (double)vi3;
                vi4 = vi3 ^ (int)(vd2 * 100.0);
                vf3 = (float)vi4 * 0.25f + vf2;
                vi5 = vi4 + (int)vf3;
                
                /* Call to noinline function */
                vi1 = opaque_int(vi5);
                
                state = STATE_CONTROL_FLOW;
                break;
            }
            
            case STATE_CONTROL_FLOW: {
                /* Complex control flow within the state */
                if (vi1 & 1) {
                    vi2 = vi2 * 2 + 1;
                    if (vi2 > 1000) {
                        vi3 = vi3 / 2;
                    } else {
                        vi3 = vi3 * 3;
                    }
                } else {
                    vi2 = vi2 / 2;
                    if (vi2 < 100) {
                        vi3 = vi3 + vi1;
                    } else {
                        vi3 = vi3 - vi2;
                    }
                }
                
                /* Nested conditional */
                for (int i = 0; i < 4; i++) {
                    if ((vi3 >> i) & 1) {
                        vi4 = vi4 + i * vi1;
                    } else {
                        vi4 = vi4 - i * vi2;
                    }
                }
                
                state = STATE_BARRIER;
                break;
            }
            
            case STATE_BARRIER: {
                /* Scheduling barrier with inline assembly */
                scheduling_barrier();
                
                /* Additional operations after barrier */
                vi1 = vi1 ^ vi2 ^ vi3;
                vf1 = vf1 + vf2 - vf3;
                
                state = STATE_NEXT;
                break;
            }
            
            case STATE_NEXT: {
                /* Update state based on complex condition */
                int cond = (vi1 + vi2 + vi3 + vi4 + vi5) & 0x7;
                state = cond % 6;  /* Cycle through 0-5 */
                
                /* Update loop counter with data-dependent condition */
                loop_counter = opaque_int(loop_counter + 1);
                if (loop_counter > 20) {
                    loop_counter = 0;
                    vi5 = vi5 ^ 0xAAAA;
                }
                
                /* Another scheduling barrier */
                asm volatile("" : : : "memory");
                break;
            }
        }
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 8; inner++) {
            volatile int idx = (vi1 + inner) & 0xF;
            local_array[idx] = local_array[idx] + vi2;
            vi3 = local_array[idx] - vi4;
            local_array[(idx + 1) & 0xF] = vi3 * vi5;
            
            /* Small dependency chain */
            vi2 = vi2 + (local_array[idx] >> 1);
            vi4 = vi4 ^ local_array[(idx + 2) & 0xF];
        }
        
        /* Periodic call to noinline function */
        if ((outer & 7) == 0) {
            vi1 = opaque_int(vi1 + vi2);
            vf1 = opaque_float(vf1 + vf2);
        }
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3 + vi4 + vi5;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 16; i++) {
        checksum += local_array[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((i % 25) == 0) {
            printf("Iteration %d, checksum: %d\n", i, result);
        }
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
