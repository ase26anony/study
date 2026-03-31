/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing logic in haifa-sched.cc
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
static void* states[] = {
    &&state_0, &&state_1, &&state_2, &&state_3, &&state_4,
    &&state_5, &&state_6, &&state_7, &&state_8, &&state_9
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2;
    volatile int vi4 = seed + 3, vi5 = seed + 4, vi6 = seed + 5;
    volatile float vf1 = seed * 0.1f, vf2 = seed * 0.2f, vf3 = seed * 0.3f;
    volatile double vd1 = seed * 0.01, vd2 = seed * 0.02;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow using computed goto state machine */
        int state = (vi1 + outer) % 10;
        
        goto *states[state];
        
        /* State 0: Integer dependency chain */
        state_0:
            vi1 = vi2 + vi3;
            vi2 = vi1 * vi4;
            vi3 = vi2 >> (vi5 & 3);
            vi4 = vi3 - vi6;
            vi5 = vi4 ^ vi1;
            vi6 = vi5 | vi2;
            /* Call to force scheduling boundary */
            vi1 = opaque_int(vi1);
            scheduling_barrier();
            state = (vi6 + arr[vi1 & 15]) % 10;
            continue;
        
        /* State 1: Mixed integer/float operations */
        state_1:
            vi1 = vi2 * vi3 + vi4;
            vf1 = (float)vi1 * 0.5f;
            vi2 = (int)(vf1 * 2.0f) + vi5;
            vf2 = vf1 + (float)vi2 * 0.25f;
            vi3 = (int)(vf2 * 4.0f) ^ vi6;
            vf3 = vf2 - (float)vi3 * 0.125f;
            vi4 = (int)vf3 | vi1;
            scheduling_barrier();
            state = (vi4 + (int)vf3) % 10;
            continue;
        
        /* State 2: Double precision chain */
        state_2:
            vd1 = (double)vi1 * 0.01 + (double)vi2 * 0.02;
            vd2 = vd1 * 1.5 - (double)vi3 * 0.03;
            vi1 = (int)(vd1 * 100.0) + (int)(vd2 * 50.0);
            vd1 = vd2 * 2.0 + (double)vi4 * 0.04;
            vi2 = (int)vd1 ^ (int)(vd2 * 200.0);
            vd2 = opaque_double(vd1);
            scheduling_barrier();
            state = (vi1 + vi2 + (int)vd2) % 10;
            continue;
        
        /* State 3: Memory-intensive operations */
        state_3:
            for (int i = 0; i < 8; i++) {
                volatile int idx = (vi1 + i) & 15;
                arr[idx] = arr[idx] + vi2;
                vi2 = arr[idx] * vi3;
                arr[(idx + 1) & 15] = arr[(idx + 1) & 15] ^ vi2;
                vi3 = arr[(idx + 1) & 15] + vi4;
            }
            vi1 = vi2 + vi3;
            scheduling_barrier();
            state = (arr[vi1 & 15] + vi3) % 10;
            continue;
        
        /* State 4: Long dependency chain */
        state_4:
            vi1 = vi2 + vi3;
            vi2 = vi1 * vi4 - vi5;
            vi3 = vi2 >> 1 | vi6;
            vi4 = vi3 ^ vi1 + vi2;
            vi5 = vi4 * 3 - vi3;
            vi6 = vi5 & 0xFF | vi4;
            vi1 = vi6 + vi5 * 2;
            vi2 = vi1 - vi3 / 2;
            vi3 = vi2 | vi4 & vi5;
            vi4 = opaque_int(vi3);
            scheduling_barrier();
            state = (vi1 + vi2 + vi3 + vi4) % 10;
            continue;
        
        /* State 5: Floating point intensive */
        state_5:
            vf1 = (float)vi1 * 0.1f + (float)vi2 * 0.2f;
            vf2 = vf1 * 2.0f - (float)vi3 * 0.3f;
            vf3 = vf2 / 1.5f + (float)vi4 * 0.4f;
            vi1 = (int)(vf1 * 10.0f) + (int)(vf2 * 5.0f);
            vi2 = (int)(vf3 * 8.0f) ^ vi1;
            vf1 = opaque_float(vf3);
            scheduling_barrier();
            state = (vi1 + vi2 + (int)vf1) % 10;
            continue;
        
        /* State 6: Mixed operations with array */
        state_6:
            vi1 = arr[vi2 & 15] + arr[vi3 & 15];
            vf1 = (float)vi1 * 0.25f;
            vi2 = (int)(vf1 * 4.0f) + arr[vi4 & 15];
            vd1 = (double)vi2 * 0.01;
            vi3 = (int)(vd1 * 100.0) ^ vi5;
            arr[vi3 & 15] = vi3 + vi6;
            scheduling_barrier();
            state = (vi1 + vi2 + vi3 + arr[vi3 & 15]) % 10;
            continue;
        
        /* State 7: Bit manipulation chain */
        state_7:
            vi1 = (vi2 << 3) | (vi3 >> 2);
            vi2 = (vi1 ^ vi4) & 0xFFFF;
            vi3 = (vi2 << 1) + (vi5 >> 1);
            vi4 = (vi3 ^ vi6) | 0xFF;
            vi5 = (vi4 << 2) & (vi1 >> 2);
            vi6 = (vi5 ^ vi2) + vi3;
            vi1 = opaque_int(vi6);
            scheduling_barrier();
            state = (vi1 ^ vi2 ^ vi3 ^ vi4 ^ vi5 ^ vi6) % 10;
            continue;
        
        /* State 8: Complex arithmetic */
        state_8:
            vi1 = vi2 * vi3 + vi4 / (vi5 ? vi5 : 1);
            vi2 = vi1 - vi3 * vi6;
            vi3 = vi2 + vi4 ^ vi5;
            vi4 = vi3 * 2 - vi6;
            vi5 = vi4 / (vi1 ? (vi1 & 7) + 1 : 1);
            vi6 = vi5 + vi2 * vi3;
            vf1 = (float)vi6 * 0.1f;
            scheduling_barrier();
            state = (vi1 + vi3 + vi5 + (int)vf1) % 10;
            continue;
        
        /* State 9: All types combined */
        state_9:
            vi1 = vi2 + vi3 * vi4;
            vf1 = (float)vi1 * 0.33f;
            vd1 = (double)vf1 * 1.5;
            vi2 = (int)vd1 + vi5;
            vf2 = (float)vi2 * 0.66f - vf1;
            vi3 = (int)vf2 ^ vi6;
            vd2 = vd1 + (double)vi3 * 0.01;
            vi4 = (int)vd2 | vi1;
            arr[vi4 & 15] = opaque_int(vi4);
            scheduling_barrier();
            state = (vi1 + vi2 + vi3 + vi4 + arr[vi4 & 15]) % 10;
            continue;
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3 + vi4 + vi5 + vi6;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduling stress test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((i + 1) % 20 == 0) {
            printf("Completed iteration %d, current checksum: %d\n", i + 1, total_checksum);
        }
    }
    
    printf("Final total checksum: %d\n", total_checksum);
    return 0;
}
