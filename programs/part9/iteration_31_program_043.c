/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

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

/* State machine labels for computed goto */
static void* states[] = {
    &&STATE_0, &&STATE_1, &&STATE_2, &&STATE_3, &&STATE_4,
    &&STATE_5, &&STATE_6, &&STATE_7, &&STATE_8, &&STATE_9
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2, vi4 = seed + 3;
    volatile float vf1 = seed * 0.5f, vf2 = seed * 0.7f, vf3 = seed * 1.1f;
    volatile double vd1 = seed * 0.3, vd2 = seed * 0.9;
    volatile int state = seed % 10;
    volatile int array_index = 0;
    int local_array[32];
    
    /* Initialize local array with volatile access pattern */
    for (int i = 0; i < 32; i++) {
        local_array[i] = (i * seed) & 0xFF;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex data-dependent control flow */
        if (opaque_int(vi1) % 7 == 0) {
            state = (state + 3) % 10;
        } else if (opaque_int(vi2) % 5 == 0) {
            state = (state + 7) % 10;
        }
        
        /* Computed goto state machine - creates complex CFG */
        goto *states[state];
        
        STATE_0:
            /* Chain of dependent integer operations */
            vi1 = vi2 + vi3;
            vi2 = vi1 * vi4;
            vi3 = vi2 >> (vi4 & 3);
            vi4 = vi3 - vi1;
            vi1 = vi4 ^ vi2;
            vi2 = opaque_int(vi1) | vi3;
            vi3 = vi2 * 0x9E3779B9; /* Mixing constant */
            scheduling_barrier();
            state = (vi3 % 13 > 6) ? 1 : 2;
            continue;
            
        STATE_1:
            /* Mixed integer/float operations */
            vf1 = (float)vi1 * 1.5f;
            vf2 = vf1 + (float)vi2 * 0.7f;
            vi1 = (int)(vf2 * 2.0f);
            vf3 = opaque_float(vf2) / (vf1 + 1.0f);
            vi2 = (int)(vf3 * 100.0f);
            vd1 = (double)vi2 * 0.314;
            vd2 = opaque_double(vd1) + (double)vi1 * 0.271;
            scheduling_barrier();
            state = (vi2 % 17 > 8) ? 3 : 4;
            continue;
            
        STATE_2:
            /* Memory access pattern with volatile index */
            array_index = (array_index + vi1) % 32;
            vi3 = local_array[array_index] + vi2;
            array_index = (array_index + vi3) % 32;
            vi4 = local_array[array_index] * vi1;
            array_index = (array_index + vi4) % 32;
            vi1 = local_array[array_index] ^ vi3;
            scheduling_barrier();
            state = (vi4 % 11 > 5) ? 5 : 6;
            continue;
            
        STATE_3:
            /* Long dependency chain with function calls */
            vi1 = opaque_int(vi1 + vi2);
            vi2 = opaque_int(vi2 * vi3);
            vi3 = opaque_int(vi3 - vi4);
            vi4 = opaque_int(vi4 ^ vi1);
            vi1 = opaque_int(vi1 + vi3);
            vi2 = opaque_int(vi2 * vi4);
            scheduling_barrier();
            state = 7;
            continue;
            
        STATE_4:
            /* Floating point intensive */
            vf1 = opaque_float(vf1 * 1.1f);
            vf2 = opaque_float(vf2 + vf3);
            vf3 = opaque_float(vf3 / (vf1 + 0.5f));
            vd1 = opaque_double(vd1 * 0.9);
            vd2 = opaque_double(vd2 + vd1);
            scheduling_barrier();
            state = 8;
            continue;
            
        STATE_5:
            /* Integer arithmetic with shifts */
            vi1 = (vi1 << 3) | (vi1 >> 29);
            vi2 = (vi2 << 5) | (vi2 >> 27);
            vi3 = (vi3 << 7) | (vi3 >> 25);
            vi4 = (vi4 << 11) | (vi4 >> 21);
            vi1 = vi1 ^ vi2 ^ vi3 ^ vi4;
            scheduling_barrier();
            state = 9;
            continue;
            
        STATE_6:
            /* Mixed operations with memory barriers */
            asm volatile ("" : : : "memory");
            vi1 = vi1 + local_array[(vi2 + outer) % 32];
            asm volatile ("" : : : "memory");
            vi2 = vi2 * local_array[(vi3 + outer) % 32];
            asm volatile ("" : : : "memory");
            scheduling_barrier();
            state = 0;
            continue;
            
        STATE_7:
            /* Use rdtsc for x86-specific scheduling requirements */
            {
                unsigned long long tsc;
                #ifdef __x86_64__
                tsc = __builtin_ia32_rdtsc();
                vi1 = (int)(tsc & 0xFFFFFFFF);
                vi2 = (int)(tsc >> 32);
                #else
                /* Fallback for non-x86 */
                vi1 = time(NULL);
                vi2 = clock();
                #endif
                vi3 = vi1 * vi2;
                vi4 = vi2 - vi1;
            }
            scheduling_barrier();
            state = (vi3 % 19 > 9) ? 1 : 2;
            continue;
            
        STATE_8:
            /* Nested loop with break conditions */
            for (int inner = 0; inner < 10; inner++) {
                if (opaque_int(vi1 + inner) % 3 == 0) {
                    vi2 = vi2 + local_array[(vi1 + inner) % 32];
                    if (opaque_int(vi2) % 5 == 0) break;
                } else {
                    vi3 = vi3 - local_array[(vi2 + inner) % 32];
                    if (opaque_int(vi3) % 7 == 0) continue;
                }
                vi4 = vi4 ^ (vi1 + vi2 + vi3);
            }
            scheduling_barrier();
            state = 3;
            continue;
            
        STATE_9:
            /* Final mixing of all values */
            vi1 = vi1 + (int)vf1 + (int)vd1;
            vi2 = vi2 * (int)vf2 + (int)vd2;
            vf3 = vf3 + (float)vi3 + (float)vi4;
            vd2 = vd2 * 0.99 + (double)vi1 * 0.01;
            scheduling_barrier();
            state = (outer % 3 == 0) ? 4 : 5;
            continue;
    }
    
    /* Compute final checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3 + vi4;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    
    /* Add array checksum */
    for (int i = 0; i < 32; i++) {
        checksum += local_array[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase coverage probability */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
        
        /* Occasionally call opaque functions to create additional scheduling boundaries */
        if (i % 17 == 0) {
            opaque_int(i);
            opaque_float(i * 0.5f);
            opaque_double(i * 0.3);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
