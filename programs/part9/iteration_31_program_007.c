/* sched_context_test.c
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
    /* Inline assembly with memory clobber to force scheduler to save state */
    asm volatile ("" : : : "memory");
}

/* State machine labels for computed goto */
static void* state_labels[] = {
    &&STATE_0, &&STATE_1, &&STATE_2, &&STATE_3, &&STATE_4,
    &&STATE_5, &&STATE_6, &&STATE_7, &&STATE_8, &&STATE_9
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 173;
    volatile int v4 = seed - 89;
    volatile float f1 = seed * 0.5f;
    volatile float f2 = seed * 1.7f;
    volatile double d1 = seed * 0.123;
    volatile double d2 = seed * 0.789;
    
    /* Local array with volatile accesses */
    int array[32];
    for (int i = 0; i < 32; i++) {
        array[i] = i * i;
    }
    
    volatile int state = seed % 10;
    volatile int outer_counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (outer_counter = 0; outer_counter < 50; outer_counter++) {
        /* Complex control flow using computed goto */
        goto *state_labels[state];
        
        STATE_0:
            /* Chain of dependent integer operations */
            v1 = v2 + v3;
            v2 = v1 * v4;
            v3 = v2 >> (v4 & 7);
            v4 = v3 ^ v1;
            v1 = v4 - v2;
            v2 = opaque_int(v1 + v3);
            v3 = v2 * 137;
            v4 = v3 / (opaque_int(v1) + 1);
            scheduling_barrier();
            state = (v1 + v2 + v3 + v4) % 10;
            continue;
            
        STATE_1:
            /* Mixed integer/float operations */
            f1 = v1 * 0.25f + f2;
            v1 = (int)f1 * v2;
            f2 = opaque_float(f1 * 3.14f);
            d1 = f2 * 2.71828 + d2;
            v2 = (int)d1 ^ v3;
            d2 = opaque_double(d1 * 0.987);
            v3 = v4 + (int)(f1 * 100);
            f1 = opaque_float(f2 + d2);
            scheduling_barrier();
            state = ((v1 ^ v2) + (int)f1) % 10;
            continue;
            
        STATE_2:
            /* Memory-intensive operations with volatile array access */
            volatile int idx = (v1 + v2) & 31;
            v1 = array[idx] + v3;
            idx = (v2 + v3) & 31;
            v2 = array[idx] * v4;
            idx = (v3 + v4) & 31;
            v3 = array[idx] ^ v1;
            idx = (v4 + v1) & 31;
            v4 = array[idx] | v2;
            /* Update array */
            array[(v1 + v4) & 31] = v2 + v3;
            scheduling_barrier();
            state = (v1 + array[(v2 + v3) & 31]) % 10;
            continue;
            
        STATE_3:
            /* Long dependency chain */
            v1 = opaque_int(v1 * 3 + 1);
            v2 = v1 + opaque_int(v4);
            v3 = v2 * opaque_int(v1);
            v4 = v3 - opaque_int(v2);
            v1 = v4 ^ opaque_int(v3);
            v2 = v1 | opaque_int(v4);
            v3 = v2 & opaque_int(v1);
            v4 = v3 * opaque_int(v2);
            scheduling_barrier();
            state = (opaque_int(v1) + opaque_int(v4)) % 10;
            continue;
            
        STATE_4:
            /* Floating point intensive */
            f1 = opaque_float(f1 * 1.1f);
            f2 = f1 + opaque_float(d1);
            d1 = opaque_double(d2 * 0.9);
            d2 = d1 - opaque_double(f2);
            f1 = opaque_float(f2 * d2);
            f2 = opaque_float(f1 / d1);
            v1 = (int)(f1 * 1000);
            v2 = (int)(f2 * 1000);
            scheduling_barrier();
            state = (v1 + v2 + (int)d1) % 10;
            continue;
            
        STATE_5:
        STATE_6:
        STATE_7:
        STATE_8:
        STATE_9:
            /* Similar patterns for other states */
            v1 = v1 * 1103515245 + 12345;
            v2 = v2 * 1103515245 + 12345;
            v3 = v3 * 1103515245 + 12345;
            v4 = v4 * 1103515245 + 12345;
            f1 = f1 * 1.2345f + 0.6789f;
            f2 = f2 * 1.2345f - 0.6789f;
            d1 = d1 * 1.23456789 + 0.98765432;
            d2 = d2 * 1.23456789 - 0.98765432;
            
            /* Call noinline function to force scheduling boundary */
            v1 = opaque_int(v1 ^ v2);
            v2 = opaque_int(v3 + v4);
            f1 = opaque_float(f1 * f2);
            d1 = opaque_double(d1 / (d2 + 1.0));
            
            scheduling_barrier();
            state = (v1 + v2 + v3 + v4 + (int)f1 + (int)d1) % 10;
            continue;
    }
    
    /* Inner loop with array dependencies */
    volatile int inner_sum = 0;
    for (int i = 0; i < 100; i++) {
        volatile int idx1 = (v1 + i) & 31;
        volatile int idx2 = (v2 + i * 3) & 31;
        volatile int idx3 = (v3 + i * 5) & 31;
        
        array[idx1] = array[idx2] + array[idx3];
        inner_sum += array[idx1];
        
        /* Inline assembly barrier every 7 iterations */
        if (i % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Switch statement creating multiple basic blocks */
    volatile int switch_var = inner_sum % 15;
    switch (switch_var) {
        case 0: v1 = v2 * v3; v4 = opaque_int(v1); break;
        case 1: v1 = v2 + v3; v4 = opaque_int(v1); break;
        case 2: v1 = v2 - v3; v4 = opaque_int(v1); break;
        case 3: v1 = v2 ^ v3; v4 = opaque_int(v1); break;
        case 4: v1 = v2 | v3; v4 = opaque_int(v1); break;
        case 5: v1 = v2 & v3; v4 = opaque_int(v1); break;
        case 6: f1 = f2 * 2.0f; d1 = opaque_double(f1); break;
        case 7: f1 = f2 / 2.0f; d1 = opaque_double(f1); break;
        case 8: f1 = f2 + 2.0f; d1 = opaque_double(f1); break;
        case 9: f1 = f2 - 2.0f; d1 = opaque_double(f1); break;
        case 10: d1 = d2 * 3.14; f1 = opaque_float(d1); break;
        case 11: d1 = d2 / 3.14; f1 = opaque_float(d1); break;
        case 12: d1 = d2 + 3.14; f1 = opaque_float(d1); break;
        case 13: d1 = d2 - 3.14; f1 = opaque_float(d1); break;
        default: v1 = opaque_int(v4); v4 = opaque_int(v1); break;
    }
    
    /* Final checksum */
    int result = v1 + v2 + v3 + v4 + (int)f1 + (int)f2 + (int)d1 + (int)d2 + inner_sum;
    return opaque_int(result);
}

int main(void) {
    srand(time(NULL));
    int total = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand();
        total += scheduling_stress(seed);
        
        /* Occasionally add a scheduling barrier */
        if (i % 23 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total);
    return 0;
}
