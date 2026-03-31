/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
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

/* Function with mixed operations to create scheduling complexity */
void __attribute__((noinline)) scheduling_barrier(int a, float b, double c) {
    volatile int barrier = a + (int)b + (int)c;
    (void)barrier;
}

/* State machine states */
enum states {
    STATE_ARITH,
    STATE_FLOAT,
    STATE_MEM,
    STATE_MIXED,
    STATE_BRANCH,
    STATE_COUNT
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile float f1 = seed * 0.5f;
    volatile float f2 = seed * 1.5f;
    volatile double d1 = seed * 0.25;
    volatile double d2 = seed * 0.75;
    
    volatile int state = STATE_ARITH;
    volatile int counter = 0;
    volatile int array[16];
    
    /* Initialize array with volatile indices */
    for (volatile int i = 0; i < 16; i++) {
        array[i] = (seed + i) * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (volatile int outer = 0; outer < 50; outer++) {
        
        /* Complex state machine using switch - creates multiple basic blocks */
        switch (state) {
            case STATE_ARITH: {
                /* Long chain of dependent integer operations */
                v1 = opaque_int(v1);
                v2 = v1 * 3 + v2;
                v3 = v2 / 2 + v3;
                v1 = v3 ^ v1;
                v2 = v1 + v2 * 7;
                v3 = v2 - v3 / 3;
                v1 = v3 | v1;
                v2 = v1 * 11 - v2;
                v3 = v2 + v3 * 13;
                
                /* Memory access with volatile index */
                int idx = opaque_int(counter % 16);
                v1 = array[idx] + v1;
                
                /* Scheduling barrier */
                asm volatile("" : : : "memory");
                
                state = (v1 % 5 == 0) ? STATE_FLOAT : STATE_MEM;
                break;
            }
            
            case STATE_FLOAT: {
                /* Mixed float/double operations with dependencies */
                f1 = opaque_float(f1);
                d1 = opaque_double(d1);
                f2 = f1 * 2.3f + f2;
                d2 = d1 * 1.7 + d2;
                f1 = f2 / 1.1f - f1;
                d1 = d2 * 0.9 - d1;
                f2 = f1 + f2 * 3.14f;
                d2 = d1 + d2 * 2.71;
                
                /* Convert and mix with integers */
                v1 = (int)f1 + v1;
                v2 = (int)d1 + v2;
                
                /* Call to create scheduling boundary */
                scheduling_barrier(v1, f1, d1);
                
                state = (f1 > 1000.0f) ? STATE_MIXED : STATE_ARITH;
                break;
            }
            
            case STATE_MEM: {
                /* Memory-intensive operations with dependencies */
                for (volatile int i = 0; i < 8; i++) {
                    int idx1 = opaque_int(i);
                    int idx2 = opaque_int((i + 1) % 8);
                    array[idx1] = array[idx2] * 2 + v1;
                    v1 = array[idx1] + v2;
                    v2 = v1 - v3;
                }
                
                /* Complex addressing */
                int base = opaque_int(counter % 8);
                v3 = array[base] + array[base + 1] + array[base + 2];
                
                asm volatile("" : : : "memory");
                
                state = STATE_BRANCH;
                break;
            }
            
            case STATE_MIXED: {
                /* Mixed-type dependency chain */
                v1 = opaque_int(v1);
                f1 = (float)v1 * 0.5f + f1;
                v2 = (int)f1 * 3 + v2;
                d1 = (double)v2 * 0.25 + d1;
                v3 = (int)d1 + v3;
                f2 = (float)v3 * 1.5f + f2;
                
                /* Memory access pattern */
                int idx = (v1 ^ v2) % 16;
                array[idx] = v3 + array[(idx + 1) % 16];
                
                scheduling_barrier(v2, f2, d2);
                
                state = (counter % 3 == 0) ? STATE_ARITH : STATE_FLOAT;
                break;
            }
            
            case STATE_BRANCH: {
                /* Complex branching pattern */
                volatile int branch_var = v1 + v2 + v3;
                
                if (branch_var % 2 == 0) {
                    v1 = v1 * 2 + 1;
                    f1 = f1 * 1.1f;
                } else if (branch_var % 3 == 0) {
                    v2 = v2 / 2 - 1;
                    d1 = d1 * 0.9;
                } else if (branch_var % 5 == 0) {
                    v3 = v3 ^ 0xAAAA;
                    f2 = f2 / 1.3f;
                } else {
                    v1 = v1 + v2;
                    v2 = v2 + v3;
                    v3 = v3 + v1;
                }
                
                /* Nested conditionals */
                if (f1 > f2) {
                    d1 = d1 + d2;
                } else {
                    d2 = d2 - d1;
                }
                
                asm volatile("" : : : "memory");
                
                state = STATE_MEM;
                break;
            }
        }
        
        /* Inner loop with array accesses */
        for (volatile int inner = 0; inner < 10; inner++) {
            int idx_a = opaque_int((counter + inner) % 16);
            int idx_b = opaque_int((counter + inner + 5) % 16);
            
            /* Create memory dependencies */
            array[idx_a] = array[idx_b] + inner;
            v1 = array[idx_a] - v1;
            v2 = v1 + v2;
            
            /* Small dependency chain */
            v3 = v2 * 3 - v3;
            v1 = v3 / 2 + v1;
        }
        
        counter = opaque_int(counter + 1);
        
        /* Occasionally use computed goto for complex control flow */
        if (counter % 7 == 0) {
            static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
            int label_idx = (v1 + v2) % 4;
            
            goto *labels[label_idx];
            
            L0:
                v1 = v1 * 3;
                goto after_labels;
            L1:
                v2 = v2 * 5;
                goto after_labels;
            L2:
                v3 = v3 * 7;
                goto after_labels;
            L3:
                v1 = v1 ^ v2 ^ v3;
                goto after_labels;
            after_labels:
                /* Continue execution */
                ;
        }
    }
    
    /* Final checksum */
    int result = v1 + v2 + v3 + (int)f1 + (int)f2 + (int)d1 + (int)d2;
    for (int i = 0; i < 16; i++) {
        result = result ^ array[i];
    }
    
    return opaque_int(result);
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total = opaque_int(total + result);
        
        /* Vary control flow occasionally */
        if (i % 23 == 0) {
            /* Additional scheduling complexity */
            volatile int temp = 0;
            for (int j = 0; j < 50; j++) {
                temp = temp * 3 + j;
            }
            total += temp;
        }
    }
    
    printf("Final checksum: %d\n", total);
    return 0;
}
