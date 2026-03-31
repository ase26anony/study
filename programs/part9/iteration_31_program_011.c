/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline));
float opaque_float(float x) __attribute__((noinline));
double opaque_double(double x) __attribute__((noinline));

int opaque_int(int x) {
    volatile int dummy = x;
    return dummy;
}

float opaque_float(float x) {
    volatile float dummy = x;
    return dummy;
}

double opaque_double(double x) {
    volatile double dummy = x;
    return dummy;
}

/* Helper to create memory dependencies */
void memory_barrier() __attribute__((noinline));
void memory_barrier() {
    asm volatile ("" : : : "memory");
}

/* State machine labels for computed goto */
static void* states[] = {
    &&state_0, &&state_1, &&state_2, &&state_3, &&state_4,
    &&state_5, &&state_6, &&state_7, &&state_8, &&state_9
};

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));
int scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2;
    volatile float vf1 = seed * 0.5f, vf2 = seed * 0.7f;
    volatile double vd1 = seed * 0.3, vd2 = seed * 0.9;
    volatile int state = seed % 10;
    volatile int counter = 0;
    volatile int array[16];
    volatile int idx;
    
    /* Initialize array with volatile indices */
    for (idx = 0; idx < 16; idx++) {
        array[idx] = (seed + idx) % 100;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (counter = 0; counter < 50; counter++) {
        /* Complex control flow using computed goto */
        goto *states[state];
        
        /* State 0: Integer dependency chain */
        state_0:
            vi1 = vi2 + vi3;
            vi2 = vi1 * vi3;
            vi3 = vi2 - vi1;
            vi1 = vi3 >> (vi2 & 7);
            vi2 = vi1 | vi3;
            vi3 = vi2 ^ vi1;
            vi1 = opaque_int(vi3);
            /* Memory access with volatile index */
            idx = (vi1 + counter) & 15;
            vi2 = array[idx];
            array[idx] = vi3;
            asm volatile ("" : : : "memory");
            state = (vi1 + vi2 + vi3) % 10;
            continue;
        
        /* State 1: Mixed integer/float dependencies */
        state_1:
            vf1 = vf2 * 1.5f;
            vi1 = (int)vf1 + vi2;
            vf2 = (float)vi1 * 0.7f;
            vi2 = vi3 + (int)vf2;
            vf1 = opaque_float(vf2);
            vi3 = opaque_int(vi2);
            idx = (vi1 + vi3) & 15;
            array[idx] = (int)vf1;
            asm volatile ("" : : : "memory");
            state = (vi1 + vi2 + (int)vf1) % 10;
            continue;
        
        /* State 2: Double precision chain */
        state_2:
            vd1 = vd2 * 2.5;
            vd2 = vd1 / 1.7;
            vi1 = (int)(vd1 + vd2);
            vd1 = opaque_double(vd2);
            vd2 = vd1 * 0.9;
            idx = (vi1 + counter) & 15;
            vd1 += array[idx];
            asm volatile ("" : : : "memory");
            state = ((int)vd1 + (int)vd2) % 10;
            continue;
        
        /* State 3: Long integer chain */
        state_3:
            vi1 = vi2 * vi3;
            vi2 = vi1 + vi3;
            vi3 = vi2 - vi1;
            vi1 = vi3 * 3;
            vi2 = vi1 / 2;
            vi3 = vi2 % 17;
            vi1 = vi3 << 2;
            vi2 = vi1 | 0xFF;
            vi3 = vi2 & 0x7F;
            vi1 = opaque_int(vi3);
            asm volatile ("" : : : "memory");
            state = (vi1 + vi2 + vi3) % 10;
            continue;
        
        /* State 4: Memory-intensive operations */
        state_4:
            for (idx = 0; idx < 8; idx++) {
                array[idx] = array[idx + 8] + vi1;
                array[idx + 8] = array[idx] - vi2;
                vi1 = (vi1 + array[idx]) & 31;
                vi2 = (vi2 + array[idx + 8]) & 31;
            }
            vi3 = opaque_int(vi1 + vi2);
            asm volatile ("" : : : "memory");
            state = (vi3 + counter) % 10;
            continue;
        
        /* States 5-9: Similar patterns with variations */
        state_5:
            vi1 = vi2 * vi3 + counter;
            vi2 = vi1 ^ vi3;
            vi3 = vi2 - vi1;
            vf1 = (float)vi1 * 0.3f;
            vf2 = vf1 + (float)vi2;
            vi1 = opaque_int((int)vf2 + vi3);
            asm volatile ("" : : : "memory");
            state = (vi1 + state) % 10;
            continue;
        
        state_6:
            vd1 = (double)vi1 * 1.1;
            vd2 = vd1 + (double)vi2;
            vi3 = (int)(vd1 + vd2);
            vi1 = vi3 * 2;
            vi2 = opaque_int(vi1);
            asm volatile ("" : : : "memory");
            state = (vi2 + vi3) % 10;
            continue;
        
        state_7:
            vi1 = (vi2 << 3) | (vi3 & 7);
            vi2 = vi1 >> 2;
            vi3 = vi2 * vi1;
            vf1 = (float)vi3 * 0.25f;
            vi1 = (int)vf1 + vi2;
            asm volatile ("" : : : "memory");
            state = (vi1 + vi3) % 10;
            continue;
        
        state_8:
            for (idx = 0; idx < 4; idx++) {
                vi1 = array[idx * 2] + array[idx * 2 + 1];
                array[idx] = vi1;
                vi2 = opaque_int(vi1 + vi2);
            }
            vi3 = vi1 + vi2;
            asm volatile ("" : : : "memory");
            state = (vi3 + state) % 10;
            continue;
        
        state_9:
            vf1 = vf2 * 2.0f;
            vf2 = vf1 / 1.8f;
            vi1 = (int)vf1;
            vi2 = (int)vf2;
            vi3 = vi1 * vi2;
            vd1 = (double)vi3 * 0.5;
            vi1 = (int)vd1;
            asm volatile ("" : : : "memory");
            state = (vi1 + vi2 + vi3) % 10;
            continue;
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3;
    checksum += (int)vf1 + (int)vf2;
    checksum += (int)vd1 + (int)vd2;
    for (idx = 0; idx < 16; idx++) {
        checksum += array[idx];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress scheduler context saving/restoring */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
