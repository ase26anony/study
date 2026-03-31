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

/* Helper to force memory dependencies */
void memory_barrier() __attribute__((noinline));
void memory_barrier() {
    asm volatile("" : : : "memory");
}

/* Main scheduling stress function */
unsigned long long scheduling_stress(int seed) __attribute__((noinline));
unsigned long long scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2, vi4 = seed + 3;
    volatile float vf1 = seed * 0.1f, vf2 = seed * 0.2f, vf3 = seed * 0.3f;
    volatile double vd1 = seed * 0.01, vd2 = seed * 0.02;
    volatile int state = seed % 10;
    volatile int counter = 0;
    volatile int arr_indices[8] = {0,1,2,3,4,5,6,7};
    volatile int array[8] = {0};
    
    unsigned long long checksum = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch with many cases */
        switch (state) {
            case 0:
                /* Chain of dependent integer operations */
                vi1 = vi2 + vi3;
                vi4 = vi1 * vi2;
                vi2 = vi4 >> (vi3 & 7);
                vi3 = vi2 - vi1;
                vi1 = vi3 ^ vi4;
                vi4 = vi1 | vi2;
                vi2 = vi4 + vi3;
                vi3 = vi2 * 3;
                vi1 = opaque_int(vi3);
                memory_barrier();
                state = (vi1 + outer) % 10;
                break;
                
            case 1:
                /* Mixed integer/float dependencies */
                vf1 = vf2 + vf3;
                vi1 = (int)vf1 * vi2;
                vf2 = vi1 * 0.5f;
                vi3 = (int)(vf2 * vf3);
                vf3 = opaque_float(vf2 + vf1);
                vi4 = vi3 << (vi1 & 3);
                memory_barrier();
                state = (vi4 + outer * 2) % 10;
                break;
                
            case 2:
                /* Double precision chain */
                vd1 = vd2 * 1.1;
                vd2 = vd1 + seed * 0.001;
                vi1 = (int)(vd1 * 100);
                vd1 = opaque_double(vd2 - vd1);
                vi2 = vi1 ^ vi3;
                memory_barrier();
                state = (vi1 + vi2 + outer) % 10;
                break;
                
            case 3:
                /* Longer integer dependency chain */
                vi1 = vi2 + vi3;
                vi4 = vi1 - vi2;
                vi2 = vi4 * vi3;
                vi3 = vi2 / (vi1 ? vi1 : 1);
                vi1 = vi3 << 2;
                vi4 = vi1 | vi2;
                vi2 = vi4 ^ vi3;
                vi3 = vi2 + vi1;
                vi1 = vi3 * 2;
                vi4 = opaque_int(vi1);
                memory_barrier();
                state = (vi4 + outer * 3) % 10;
                break;
                
            case 4:
                /* Memory access pattern with volatile indices */
                for (int i = 0; i < 8; i++) {
                    int idx = arr_indices[i];
                    array[idx] = array[(idx + 1) & 7] + vi1;
                    vi1 = array[idx] - vi2;
                    arr_indices[i] = (arr_indices[i] + vi1) & 7;
                }
                memory_barrier();
                state = (array[0] + outer) % 10;
                break;
                
            case 5:
                /* Mixed operations with calls */
                vf1 = opaque_float(vf2 + vf3);
                vi1 = opaque_int((int)vf1 + vi2);
                vd1 = opaque_double(vd2 * 2.0);
                vi3 = vi1 * (int)vd1;
                vf2 = vf1 * 3.0f;
                memory_barrier();
                state = (vi3 + outer * 4) % 10;
                break;
                
            case 6:
                /* Complex integer chain */
                vi1 = (vi2 * vi3) + vi4;
                vi2 = (vi1 >> 3) ^ vi3;
                vi3 = vi2 * vi4;
                vi4 = vi3 - vi1;
                vi1 = vi4 | vi2;
                vi2 = vi1 & vi3;
                vi3 = vi2 + vi4;
                vi4 = opaque_int(vi3 * 2);
                memory_barrier();
                state = (vi4 + outer * 5) % 10;
                break;
                
            case 7:
                /* Float/double mixing */
                vf1 = vf2 * 1.5f;
                vd1 = vf1 * 2.0;
                vf3 = (float)vd1 + vf2;
                vi1 = (int)(vf3 * 10);
                vd2 = opaque_double(vd1 + vd2);
                memory_barrier();
                state = (vi1 + outer * 6) % 10;
                break;
                
            case 8:
                /* Memory intensive */
                for (int i = 0; i < 4; i++) {
                    int idx1 = arr_indices[i];
                    int idx2 = arr_indices[i + 4];
                    array[idx1] = array[idx2] + vi1;
                    array[idx2] = array[idx1] - vi2;
                    vi1 = array[idx1] ^ array[idx2];
                    vi2 = vi1 + i;
                }
                memory_barrier();
                state = (array[3] + outer * 7) % 10;
                break;
                
            case 9:
                /* All types combined */
                vi1 = vi2 + vi3;
                vf1 = (float)vi1 * 0.25f;
                vd1 = (double)vf1 * 2.0;
                vi4 = (int)vd1 * vi2;
                vf2 = opaque_float(vf3 + vf1);
                vi3 = opaque_int(vi4 ^ vi1);
                vd2 = opaque_double(vd1 - vd2);
                memory_barrier();
                state = (vi3 + vi4 + outer * 8) % 10;
                break;
        }
        
        /* Inner loop with data-dependent array accesses */
        for (int inner = 0; inner < 10; inner++) {
            int idx = (vi1 + inner) & 7;
            array[idx] = array[(idx + 1) & 7] + vi2;
            vi2 = array[idx] - vi3;
            vi3 = vi2 ^ vi4;
            
            /* Small inline asm with memory clobber */
            asm volatile("" : : "r"(array[idx]), "r"(vi2) : "memory");
        }
        
        counter++;
        if (counter > 100) counter = 0;
    }
    
    /* Compute checksum from all volatile variables */
    checksum = (unsigned long long)vi1 + vi2 + vi3 + vi4;
    checksum += (unsigned long long)(vf1 * 1000) + (unsigned long long)(vf2 * 1000);
    checksum += (unsigned long long)(vd1 * 10000) + (unsigned long long)(vd2 * 10000);
    for (int i = 0; i < 8; i++) {
        checksum += array[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    unsigned long long total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of hitting the target code */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    return 0;
}
