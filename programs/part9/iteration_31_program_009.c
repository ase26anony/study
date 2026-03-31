/* sched_context_test.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or with: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

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
    asm volatile ("" : : : "memory");
}

/* State machine states */
enum states {
    STATE_A, STATE_B, STATE_C, STATE_D, STATE_E,
    STATE_F, STATE_G, STATE_H, STATE_I, STATE_J,
    STATE_K, STATE_L, STATE_M, STATE_N, STATE_O
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile int vi3 = seed + 1;
    volatile int vi4 = seed - 1;
    volatile float vf1 = seed * 0.5f;
    volatile float vf2 = seed * 1.5f;
    volatile double vd1 = seed * 0.25;
    volatile double vd2 = seed * 0.75;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = (seed + i) & 0xF;
    }
    
    volatile int state = STATE_A;
    volatile int counter = 0;
    volatile int array_index = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch - creates many basic blocks */
        switch (state) {
            case STATE_A: {
                /* Chain of dependent integer operations */
                vi1 = vi2 + vi3;
                vi2 = vi1 * vi4;
                vi3 = vi2 >> (vi4 & 3);
                vi4 = vi3 ^ vi1;
                vi1 = vi4 - vi2;
                vi2 = vi1 + vi3;
                vi3 = vi2 * vi4;
                vi4 = vi3 & 0xFF;
                
                /* Mix with float operations */
                vf1 = vf2 * 1.1f;
                vf2 = vf1 + (float)vi1;
                vf1 = vf2 / 2.0f;
                
                /* Call to create scheduling boundary */
                vi1 = opaque_int(vi1);
                scheduling_barrier();
                
                /* Update state based on complex condition */
                state = (vi1 & 1) ? STATE_B : STATE_C;
                break;
            }
            
            case STATE_B: {
                /* Different chain of operations */
                vi2 = vi3 * vi4;
                vi3 = vi2 + vi1;
                vi4 = vi3 - vi2;
                vi1 = vi4 ^ vi3;
                vi2 = vi1 << 2;
                vi3 = vi2 | vi4;
                vi4 = vi3 % 17;
                
                /* Mixed float/double operations */
                vd1 = vd2 * 1.01;
                vd2 = vd1 + (double)vi2;
                vf1 = (float)vd2;
                vf2 = vf1 * 2.0f;
                
                vi2 = opaque_int(vi2);
                scheduling_barrier();
                
                state = (vi2 > 100) ? STATE_D : STATE_E;
                break;
            }
            
            case STATE_C: {
                /* More complex dependency chain */
                vi3 = vi1 + vi2;
                vi4 = vi3 * vi1;
                vi1 = vi4 - vi3;
                vi2 = vi1 ^ vi4;
                vi3 = vi2 + vi1;
                vi4 = vi3 >> 1;
                vi1 = vi4 * vi2;
                
                /* Memory access with volatile index */
                array_index = (vi1 + outer) & 0xF;
                vi2 = arr[array_index] + vi3;
                arr[array_index] = vi2;
                
                vf2 = opaque_float(vf2);
                scheduling_barrier();
                
                state = STATE_F;
                break;
            }
            
            case STATE_D: {
                /* Long chain with mixed operations */
                vd1 = (double)vi1 * 0.5;
                vd2 = vd1 + (double)vi2;
                vf1 = (float)vd2;
                vf2 = vf1 * 3.14f;
                vi1 = (int)vf2;
                vi2 = vi1 * vi3;
                vi3 = vi2 + (int)vd1;
                vi4 = vi3 ^ vi1;
                
                /* Another memory access */
                array_index = (vi4 + counter) & 0xF;
                vi3 = arr[array_index] * 2;
                arr[array_index] = vi3;
                
                vd1 = opaque_double(vd1);
                scheduling_barrier();
                
                state = (counter & 3) ? STATE_G : STATE_H;
                break;
            }
            
            case STATE_E: {
                /* Integer dependency chain */
                vi1 = vi2 + vi3 + vi4;
                vi2 = vi1 * 3;
                vi3 = vi2 / 2;
                vi4 = vi3 - vi1;
                vi1 = vi4 & 0x7F;
                vi2 = vi1 | 0x80;
                vi3 = vi2 ^ vi4;
                vi4 = vi3 << 1;
                
                scheduling_barrier();
                state = STATE_I;
                break;
            }
            
            case STATE_F: {
                /* Float-intensive operations */
                vf1 = vf2 * 2.0f;
                vf2 = vf1 + (float)vi1;
                vf1 = vf2 / 1.5f;
                vf2 = vf1 * vf1;
                vf1 = vf2 - 1.0f;
                
                /* Convert to int and back */
                vi1 = (int)vf1;
                vi2 = vi1 * 5;
                vf2 = (float)vi2;
                
                vi1 = opaque_int(vi1);
                scheduling_barrier();
                
                state = STATE_J;
                break;
            }
            
            case STATE_G: {
                /* Mixed operations with array access */
                vi1 = arr[counter & 0xF];
                vi2 = vi1 + counter;
                vi3 = vi2 * vi4;
                vi4 = vi3 >> 2;
                
                vd1 = (double)vi4 * 0.25;
                vd2 = vd1 + 1.0;
                
                arr[(counter + 1) & 0xF] = vi4;
                
                scheduling_barrier();
                state = STATE_K;
                break;
            }
            
            case STATE_H: {
                /* Another dependency chain */
                vi1 = vi3 ^ vi2;
                vi2 = vi1 + vi4;
                vi3 = vi2 * 3;
                vi4 = vi3 - vi1;
                vi1 = vi4 & vi2;
                vi2 = vi1 | vi3;
                
                vf1 = (float)vi2 * 0.5f;
                vf2 = vf1 + 1.0f;
                
                vf1 = opaque_float(vf1);
                scheduling_barrier();
                
                state = STATE_L;
                break;
            }
            
            case STATE_I: {
                /* Complex integer chain */
                vi3 = vi1 * vi2 + vi4;
                vi4 = vi3 / (vi1 + 1);
                vi1 = vi4 ^ vi2;
                vi2 = vi1 + vi3;
                vi3 = vi2 * vi4;
                vi4 = vi3 - vi1;
                vi1 = vi4 << 1;
                vi2 = vi1 >> 2;
                
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                
                state = STATE_M;
                break;
            }
            
            case STATE_J: {
                /* Double precision chain */
                vd1 = vd2 * 1.5;
                vd2 = vd1 + (double)vi1;
                vd1 = vd2 / 2.0;
                vd2 = vd1 * vd1;
                
                vi1 = (int)vd2;
                vi2 = vi1 + 100;
                
                vd1 = opaque_double(vd1);
                scheduling_barrier();
                
                state = STATE_N;
                break;
            }
            
            /* Additional states to create more basic blocks */
            case STATE_K: {
                vi1 = vi2 + vi3;
                vi2 = vi1 * 2;
                state = STATE_O;
                break;
            }
            
            case STATE_L: {
                vi3 = vi4 - vi1;
                vi4 = vi3 & 0xFF;
                state = STATE_A;
                break;
            }
            
            case STATE_M: {
                vf1 = vf2 * 0.9f;
                vf2 = vf1 + 0.1f;
                state = STATE_B;
                break;
            }
            
            case STATE_N: {
                vd2 = vd1 * 0.99;
                state = STATE_C;
                break;
            }
            
            case STATE_O: {
                vi4 = vi1 ^ vi2;
                state = STATE_D;
                break;
            }
        }
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx = (vi1 + inner) & 0xF;
            volatile int val = arr[idx];
            arr[idx] = val + vi2 + inner;
            
            /* Small dependency chain inside inner loop */
            vi3 = vi4 + val;
            vi4 = vi3 * (inner + 1);
        }
        
        counter++;
        
        /* Occasionally call opaque function */
        if ((counter & 7) == 0) {
            vi1 = opaque_int(vi1);
        }
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3 + vi4;
    checksum += (int)vf1 + (int)vf2;
    checksum += (int)vd1 + (int)vd2;
    
    /* Sum array elements */
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase coverage probability */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((i % 20) == 0) {
            printf("Iteration %d, checksum: %d\n", i, result);
        }
    }
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
