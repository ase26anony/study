/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int sink = x;
    return sink;
}

float opaque_float(float x) {
    volatile float sink = x;
    return sink;
}

double opaque_double(double x) {
    volatile double sink = x;
    return sink;
}

void scheduling_barrier(void) {
    /* Empty but prevents merging of scheduling regions */
}

/* State machine labels for computed goto */
static void* states[] = {
    &&state_0, &&state_1, &&state_2, &&state_3, &&state_4,
    &&state_5, &&state_6, &&state_7, &&state_8, &&state_9
};

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));
int scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2, vi4 = seed + 3;
    volatile float vf1 = seed * 0.5f, vf2 = seed * 0.7f, vf3 = seed * 1.1f;
    volatile double vd1 = seed * 0.3, vd2 = seed * 0.9;
    volatile int state = seed % 10;
    volatile int arr[16];
    volatile int idx;
    int i, j;
    
    /* Initialize array with volatile writes */
    for (i = 0; i < 16; i++) {
        arr[i] = (seed + i) ^ (i * 37);
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (i = 0; i < 50; i++) {
        idx = opaque_int(i) & 0xF;
        
        /* Complex inner loop with memory dependencies */
        for (j = 0; j < 8; j++) {
            volatile int temp = arr[idx] + arr[(idx + 1) & 0xF];
            arr[idx] = temp ^ (vi1 * j);
            idx = (idx + 3) & 0xF;
            
            /* Mix in some floating point operations */
            vf1 = vf1 * 1.1f + vf2 * 0.9f;
            vf2 = opaque_float(vf2 - vf3 * 0.5f);
        }
        
        /* State machine using computed goto - creates complex CFG */
        goto *states[state];
        
        state_0:
            /* Long dependency chain with mixed types */
            vi1 = vi1 + vi2 * 3;
            vi2 = vi2 - vi1 / 2;
            vi3 = vi3 ^ (vi1 * vi2);
            vf1 = vf1 + (float)vi1 * 0.25f;
            vd1 = vd1 * 1.01 + (double)vf1;
            vi4 = (int)(vd1 * 10.0) + vi3;
            
            /* Inline assembly with memory clobber */
            asm volatile("" : : : "memory");
            
            state = (vi1 + vi2 + vi3 + vi4) % 10;
            scheduling_barrier();
            continue;
            
        state_1:
            vi2 = vi2 * 2 + vi1;
            vi1 = vi1 / 3 + vi2;
            vf2 = vf2 * 2.0f - vf1;
            vd2 = vd2 + (double)vf2 * 0.5;
            vi3 = vi3 ^ (int)vd2;
            
            asm volatile("" : : : "memory");
            
            state = (vi2 * 3 + vi3) % 10;
            scheduling_barrier();
            continue;
            
        state_2:
            vi3 = vi3 + opaque_int(vi1) * 7;
            vi4 = vi4 - vi3 / 4;
            vf3 = opaque_float(vf3 * 1.5f + vf1);
            vd1 = vd1 * 0.99 - (double)vf3;
            vi1 = vi1 ^ (int)(vd1 * 100.0);
            
            asm volatile("" : : : "memory");
            
            state = (vi3 + vi4 * 2) % 10;
            scheduling_barrier();
            continue;
            
        state_3:
            vi4 = vi4 * 5 + vi2;
            vi2 = vi2 - vi4 / 6;
            vf1 = opaque_float(vf1 * 0.8f + vf2);
            vd2 = vd2 * 1.02 + (double)vf1;
            vi3 = vi3 ^ (int)vd2;
            
            asm volatile("" : : : "memory");
            
            state = (vi4 * 7 + vi1) % 10;
            scheduling_barrier();
            continue;
            
        state_4:
            vi1 = vi1 ^ vi2 ^ vi3;
            vi2 = vi2 + vi1 * 11;
            vf2 = vf2 * 1.3f - vf3;
            vd1 = opaque_double(vd1 * 0.95);
            vi4 = vi4 + (int)(vd1 * 20.0);
            
            asm volatile("" : : : "memory");
            
            state = (vi1 + vi2 * 3) % 10;
            scheduling_barrier();
            continue;
            
        state_5:
            vi2 = vi2 * 13 - vi4;
            vi3 = vi3 + vi2 / 7;
            vf3 = vf3 * 0.7f + vf1;
            vd2 = vd2 - (double)vf3 * 0.3;
            vi1 = vi1 ^ (int)vd2;
            
            asm volatile("" : : : "memory");
            
            state = (vi2 + vi3 * 5) % 10;
            scheduling_barrier();
            continue;
            
        state_6:
            vi3 = vi3 + opaque_int(vi4) * 17;
            vi1 = vi1 - vi3 / 8;
            vf1 = vf1 * 1.2f - vf2;
            vd1 = vd1 * 1.05 + (double)vf1;
            vi2 = vi2 ^ (int)(vd1 * 30.0);
            
            asm volatile("" : : : "memory");
            
            state = (vi3 * 11 + vi4) % 10;
            scheduling_barrier();
            continue;
            
        state_7:
            vi4 = vi4 * 19 + vi1;
            vi2 = vi2 - vi4 / 9;
            vf2 = opaque_float(vf2 * 0.6f + vf3);
            vd2 = vd2 * 0.98 - (double)vf2;
            vi3 = vi3 ^ (int)vd2;
            
            asm volatile("" : : : "memory");
            
            state = (vi4 + vi1 * 13) % 10;
            scheduling_barrier();
            continue;
            
        state_8:
            vi1 = vi1 ^ vi3 ^ vi4;
            vi2 = vi2 + vi1 * 23;
            vf3 = vf3 * 1.4f - vf1;
            vd1 = opaque_double(vd1 * 0.92);
            vi3 = vi3 + (int)(vd1 * 40.0);
            
            asm volatile("" : : : "memory");
            
            state = (vi1 * 17 + vi2) % 10;
            scheduling_barrier();
            continue;
            
        state_9:
            vi2 = vi2 * 29 - vi3;
            vi4 = vi4 + vi2 / 10;
            vf1 = vf1 * 0.5f + vf2;
            vd2 = vd2 - (double)vf1 * 0.4;
            vi1 = vi1 ^ (int)vd2;
            
            asm volatile("" : : : "memory");
            
            state = (vi2 + vi4 * 19) % 10;
            scheduling_barrier();
            continue;
    }
    
    /* Final checksum computation */
    int checksum = vi1 + vi2 + vi3 + vi4;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    
    for (i = 0; i < 16; i++) {
        checksum ^= arr[i];
    }
    
    return checksum;
}

int main(void) {
    int i, total_checksum = 0;
    
    srand(time(NULL));
    
    /* Repeated calls to stress the scheduler */
    for (i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum ^= scheduling_stress(seed);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
