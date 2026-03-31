/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test haifa_sched_test.c */
/* Or with: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test haifa_sched_test.c */

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
    /* Inline assembly with memory clobber to force scheduler to save/restore state */
    asm volatile ("" : : : "memory");
}

/* State machine labels for computed goto */
static void* states[] = {
    &&STATE_0, &&STATE_1, &&STATE_2, &&STATE_3, &&STATE_4,
    &&STATE_5, &&STATE_6, &&STATE_7, &&STATE_8, &&STATE_9
};

#define NUM_STATES 10

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int vi1 = seed, vi2 = seed + 1, vi3 = seed + 2;
    volatile float vf1 = seed * 0.5f, vf2 = seed * 0.7f;
    volatile double vd1 = seed * 0.3, vd2 = seed * 0.9;
    volatile int state = seed % NUM_STATES;
    volatile int array_index = 0;
    
    /* Local array with volatile accesses */
    int local_array[20];
    for (int i = 0; i < 20; i++) {
        local_array[i] = i * seed;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow using computed goto */
        goto *states[state];
        
        STATE_0:
            /* Chain of dependent integer operations */
            vi1 = opaque_int(vi1 + vi2);
            vi2 = opaque_int(vi2 * vi3);
            vi3 = opaque_int(vi3 - vi1);
            vi1 = opaque_int(vi1 ^ vi2);
            vi2 = opaque_int(vi2 | vi3);
            vi3 = opaque_int(vi3 & vi1);
            vi1 = opaque_int(vi1 << 2);
            vi2 = opaque_int(vi2 >> 1);
            
            /* Mixed type operations */
            vf1 = opaque_float(vf1 + vf2);
            vd1 = opaque_double(vd1 * vd2);
            vf2 = opaque_float(vf2 * (float)vi1);
            
            /* Memory access with volatile index */
            array_index = (array_index + 1) % 20;
            vi3 = local_array[array_index] + vi2;
            
            scheduling_barrier();
            state = (vi1 + vi2) % NUM_STATES;
            continue;
            
        STATE_1:
            /* Different chain of operations */
            vf1 = opaque_float(vf1 * 1.1f);
            vf2 = opaque_float(vf2 / 2.0f);
            vi1 = opaque_int((int)vf1 + vi1);
            vi2 = opaque_int(vi2 * 3);
            vi3 = opaque_int(vi3 - (int)vf2);
            
            vd1 = opaque_double(vd1 + vd2);
            vd2 = opaque_double(vd2 * 0.5);
            
            /* Memory dependency chain */
            array_index = (array_index * 2) % 20;
            local_array[array_index] = vi1;
            array_index = (array_index + 5) % 20;
            vi2 = local_array[array_index];
            
            scheduling_barrier();
            state = (vi3 + (int)vd1) % NUM_STATES;
            continue;
            
        STATE_2:
            /* Integer arithmetic chain */
            vi1 = opaque_int(vi1 + 12345);
            vi2 = opaque_int(vi2 * 6789);
            vi3 = opaque_int(vi3 / 4567);
            vi1 = opaque_int(vi1 % 2345);
            vi2 = opaque_int(vi2 ^ 0xABCD);
            vi3 = opaque_int(vi3 | 0x1234);
            
            /* Floating point chain */
            vf1 = opaque_float(vf1 * 3.14159f);
            vf2 = opaque_float(vf2 + 2.71828f);
            
            scheduling_barrier();
            state = (vi1 + vi2 + vi3) % NUM_STATES;
            continue;
            
        STATE_3:
            /* Mixed operations with memory */
            for (int i = 0; i < 5; i++) {
                vi1 = opaque_int(vi1 + local_array[i]);
                vi2 = opaque_int(vi2 * (i + 1));
            }
            
            vd1 = opaque_double(vd1 + (double)vi1);
            vd2 = opaque_double(vd2 * (double)vi2);
            
            scheduling_barrier();
            state = ((int)vd1 + (int)vd2) % NUM_STATES;
            continue;
            
        STATE_4:
            /* Complex integer chain */
            vi1 = opaque_int(vi1 * vi2 + vi3);
            vi2 = opaque_int(vi2 * vi3 - vi1);
            vi3 = opaque_int(vi3 * vi1 ^ vi2);
            vi1 = opaque_int(vi1 << (vi2 & 3));
            vi2 = opaque_int(vi2 >> (vi3 & 3));
            
            /* Use __builtin_ia32_rdtsc() on x86 for backend-specific scheduling */
            #ifdef __x86_64__
            {
                unsigned long long tsc = __builtin_ia32_rdtsc();
                vi3 = opaque_int(vi3 + (int)(tsc & 0xFFFFFFFF));
            }
            #endif
            
            scheduling_barrier();
            state = (vi1 * 1103515245 + 12345) % NUM_STATES;
            continue;
            
        /* Additional states to create more basic blocks */
        STATE_5:
            vi1 = opaque_int(vi1 * 11);
            vi2 = opaque_int(vi2 / 7);
            vf1 = opaque_float(vf1 * 1.5f);
            state = (vi1 + outer) % NUM_STATES;
            continue;
            
        STATE_6:
            vi3 = opaque_int(vi3 * 13);
            vf2 = opaque_float(vf2 / 1.3f);
            vd1 = opaque_double(vd1 + 0.1);
            state = (vi3 + (int)vf2) % NUM_STATES;
            continue;
            
        STATE_7:
            for (int i = 0; i < 3; i++) {
                vi1 = opaque_int(vi1 + i);
                local_array[i] = vi1;
            }
            state = (vi1 + state) % NUM_STATES;
            continue;
            
        STATE_8:
            vd2 = opaque_double(vd2 * 0.99);
            vi2 = opaque_int(vi2 + (int)vd2);
            state = (vi2 % NUM_STATES);
            continue;
            
        STATE_9:
            vi1 = opaque_int(vi1 ^ vi2 ^ vi3);
            vi2 = opaque_int(vi2 | vi1);
            vi3 = opaque_int(vi3 & vi2);
            state = (vi1 + vi2 + vi3 + outer) % NUM_STATES;
            continue;
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = vi1 + vi2 + vi3 + (int)vf1 + (int)vf2 + (int)vd1 + (int)vd2;
    
    /* Final memory barrier */
    scheduling_barrier();
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of hitting
       the scheduler context save/restore logic */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
