/* sched_context_test.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
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
    asm volatile ("" : : : "memory");
}

/* Complex helper function with mixed operations */
int __attribute__((noinline)) complex_helper(volatile int a, volatile float b, volatile double c) {
    int r1 = a + (int)b;
    double r2 = c * 1.5;
    float r3 = b * 2.0f;
    int r4 = r1 ^ (int)r2;
    float r5 = r3 + (float)r2;
    
    /* Memory access pattern */
    volatile int arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = (i * a) + (int)(b * i);
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i] * (i + 1);
    }
    
    return sum + r4 + (int)r5;
}

/* State machine implementation */
typedef enum {
    STATE_A,
    STATE_B, 
    STATE_C,
    STATE_D,
    STATE_E,
    STATE_F,
    STATE_G,
    STATE_H,
    STATE_I,
    STATE_J,
    NUM_STATES
} state_t;

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed * 3;
    volatile float f1 = seed * 1.5f;
    volatile float f2 = seed * 2.5f;
    volatile double d1 = seed * 0.75;
    volatile double d2 = seed * 1.25;
    
    volatile int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = (i * seed) & 0xFF;
    }
    
    state_t state = STATE_A;
    int outer_iterations = 50;
    int checksum = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < outer_iterations; outer++) {
        /* Switch-based state machine - creates multiple basic blocks */
        switch (state) {
            case STATE_A: {
                /* Chain of dependent integer operations */
                v1 = v2 + v3;
                v2 = v1 * 3;
                v3 = (v2 >> 4) ^ v1;
                v1 = v3 - v2;
                v2 = opaque_int(v1 + v3);
                v3 = v2 * 7 + 1;
                checksum += v1 + v2 + v3;
                
                /* Mixed type operations */
                f1 = (float)v1 * 1.414f;
                d1 = (double)v2 * 3.14159;
                f2 = opaque_float(f1 + (float)d1);
                
                scheduling_barrier();
                state = (v1 & 0x1) ? STATE_B : STATE_C;
                break;
            }
            
            case STATE_B: {
                /* Different operation chain */
                v3 = v1 ^ v2;
                v1 = v3 * 11;
                v2 = (v1 << 3) | (v3 & 0xF);
                v3 = opaque_int(v2 - v1);
                v1 = v3 / 3;
                v2 = v1 * v3 + 5;
                checksum += v2 - v1;
                
                f1 = opaque_float(f2 * 2.0f);
                d2 = d1 * 0.9;
                f2 = (float)d2 + f1;
                
                scheduling_barrier();
                state = (v2 % 3 == 0) ? STATE_D : STATE_A;
                break;
            }
            
            case STATE_C: {
                /* More complex chain */
                v2 = v3 + (v1 * 2);
                v3 = opaque_int(v2 >> 1);
                v1 = v3 * v2;
                v2 = (v1 & 0xFFFF) + 12345;
                v3 = v2 ^ v1;
                v1 = v3 * 3 + 7;
                checksum += v3;
                
                d1 = opaque_double(d2 + 1.0);
                f1 = f2 * 3.14f;
                d2 = (double)f1 * d1;
                
                scheduling_barrier();
                state = STATE_E;
                break;
            }
            
            case STATE_D: {
                /* Use array with volatile index */
                volatile int idx = (v1 + v2) & 0xF;
                v3 = array[idx] + v1;
                array[idx] = v3;
                v1 = v3 * 2;
                v2 = opaque_int(v1 + array[(idx + 1) & 0xF]);
                checksum += array[idx];
                
                f2 = opaque_float(f1 * 0.5f);
                d1 = d2 / 2.0;
                
                scheduling_barrier();
                state = (idx > 8) ? STATE_F : STATE_G;
                break;
            }
            
            case STATE_E: {
                /* Long dependency chain */
                v1 = v2 + 1;
                v2 = v3 * v1;
                v3 = v2 - v1;
                v1 = v3 & 0x7FFF;
                v2 = opaque_int(v1 | 0x8000);
                v3 = v2 ^ 0xABCD;
                v1 = v3 + 111;
                v2 = v1 * 13;
                v3 = v2 % 17;
                checksum += v1 + v2 + v3;
                
                scheduling_barrier();
                state = STATE_H;
                break;
            }
            
            case STATE_F: {
                /* Call helper to force scheduling boundary */
                int helper_result = complex_helper(v1, f1, d1);
                v2 = helper_result + v3;
                v3 = opaque_int(v2 * 2);
                checksum += helper_result;
                
                scheduling_barrier();
                state = STATE_I;
                break;
            }
            
            case STATE_G: {
                /* Mixed operations with memory */
                for (int i = 0; i < 4; i++) {
                    volatile int mem_idx = (v1 + i) & 0xF;
                    array[mem_idx] = array[mem_idx] + v2 + i;
                    v3 = array[mem_idx] * (i + 1);
                    checksum += v3;
                }
                
                scheduling_barrier();
                state = STATE_J;
                break;
            }
            
            case STATE_H: {
                /* Another operation chain */
                v1 = (v2 << 1) | (v3 & 1);
                v2 = opaque_int(v1 ^ v3);
                v3 = v2 + v1 * 3;
                v1 = v3 / 5;
                checksum += v1;
                
                f1 = opaque_float(f2 + 1.0f);
                d2 = opaque_double(d1 * 1.1);
                
                scheduling_barrier();
                state = STATE_A;
                break;
            }
            
            case STATE_I: {
                /* Use builtin for machine-specific instruction */
                unsigned long long tsc;
                asm volatile ("rdtsc" : "=A" (tsc));
                v1 = (int)(tsc & 0xFFFFFFFF);
                v2 = opaque_int(v1 + v3);
                checksum += (int)(tsc >> 32);
                
                scheduling_barrier();
                state = STATE_C;
                break;
            }
            
            case STATE_J: {
                /* Final state with complex condition */
                v3 = v1 * v2 + v3;
                v1 = opaque_int(v3 >> 2);
                v2 = v1 * 7 + 3;
                
                /* Complex condition for state transition */
                int cond = (v1 * v2 + v3) & 0x7;
                switch (cond) {
                    case 0: state = STATE_A; break;
                    case 1: state = STATE_B; break;
                    case 2: state = STATE_C; break;
                    case 3: state = STATE_D; break;
                    case 4: state = STATE_E; break;
                    case 5: state = STATE_F; break;
                    case 6: state = STATE_G; break;
                    default: state = STATE_H; break;
                }
                
                checksum += cond;
                scheduling_barrier();
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 8; inner++) {
            volatile int idx1 = (v1 + inner) & 0xF;
            volatile int idx2 = (v2 + inner * 2) & 0xF;
            
            int temp = array[idx1];
            array[idx1] = array[idx2] + v3;
            array[idx2] = temp - v1;
            checksum += array[idx1] + array[idx2];
            
            /* Small dependency chain within inner loop */
            v3 = v3 + array[idx1] * inner;
            v1 = opaque_int(v1 ^ array[idx2]);
        }
        
        /* Update volatile variables for next iteration */
        f1 = f1 * 1.01f;
        f2 = opaque_float(f2 - 0.5f);
        d1 = d1 + 0.25;
        d2 = opaque_double(d2 * 0.99);
    }
    
    /* Final computation */
    checksum += v1 + v2 + v3;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    for (int i = 0; i < 16; i++) {
        checksum += array[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduling stress test...\n");
    
    /* Multiple calls to stress scheduler context management */
    for (int iteration = 0; iteration < 100; iteration++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((iteration + 1) % 20 == 0) {
            printf("Completed %d iterations, current checksum: %d\n", 
                   iteration + 1, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
