/* haifa-sched-trigger.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer haifa-sched-trigger.c -o haifa-sched-trigger
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noclone));
float opaque_float(float x) __attribute__((noinline, noclone));
double opaque_double(double x) __attribute__((noinline, noclone));
void scheduling_barrier(void) __attribute__((noinline, noclone));
int complex_condition(int a, int b, int c) __attribute__((noinline, noclone));

/* State machine states */
enum states {
    STATE_A, STATE_B, STATE_C, STATE_D, STATE_E,
    STATE_F, STATE_G, STATE_H, STATE_I, STATE_J,
    STATE_K, STATE_L, STATE_M, STATE_N, STATE_O
};

int opaque_int(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x;
}

float opaque_float(float x) {
    asm volatile ("" : "+f" (x));
    return x;
}

double opaque_double(double x) {
    asm volatile ("" : "+f" (x));
    return x;
}

void scheduling_barrier(void) {
    /* Memory clobber acts as scheduling barrier */
    asm volatile ("" : : : "memory");
}

int complex_condition(int a, int b, int c) {
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    
    /* Complex data-dependent computation */
    int r1 = (v1 * v2) ^ (v2 + v3);
    int r2 = (v3 << 4) | (v1 >> 2);
    int r3 = (r1 & 0xFF) + (r2 & 0xFF);
    
    /* Force scheduling boundary */
    scheduling_barrier();
    
    return (r3 > 128) ? 1 : 0;
}

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline, noclone));

int scheduling_stress(int seed) {
    /* Volatile variables to prevent optimization and create dependencies */
    volatile int v_int1 = seed;
    volatile int v_int2 = seed * 2;
    volatile int v_int3 = seed + 1;
    volatile float v_float1 = seed * 0.5f;
    volatile float v_float2 = seed * 0.25f;
    volatile double v_double1 = seed * 0.125;
    
    /* Local array with volatile access */
    int local_array[16];
    volatile int array_index = 0;
    
    /* Initialize array with complex pattern */
    for (int i = 0; i < 16; i++) {
        local_array[i] = (i * seed) ^ (seed >> (i & 3));
    }
    
    /* State machine control */
    int state = STATE_A;
    int outer_iterations = 50;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < outer_iterations; outer++) {
        /* Update volatile variables to create dependencies across iterations */
        v_int1 = opaque_int(v_int1 + outer);
        v_int2 = opaque_int(v_int2 ^ outer);
        
        /* Complex switch-based state machine */
        switch (state) {
            case STATE_A: {
                /* Chain of dependent integer operations */
                int t1 = v_int1 + v_int2;
                int t2 = t1 * v_int3;
                int t3 = t2 >> (v_int1 & 7);
                int t4 = t3 ^ (v_int2 << 3);
                int t5 = t4 + (v_int3 * 2);
                v_int1 = t5;
                
                /* Mixed-type operations */
                float f1 = v_float1 * 2.0f;
                float f2 = f1 + v_float2;
                v_float1 = opaque_float(f2);
                
                /* Memory access with volatile index */
                array_index = (array_index + 1) & 15;
                v_int3 = local_array[array_index] + v_int1;
                
                /* Scheduling barrier */
                asm volatile ("" : : : "memory");
                
                /* Update state based on complex condition */
                if (complex_condition(v_int1, v_int2, v_int3)) {
                    state = STATE_B;
                } else {
                    state = STATE_C;
                }
                break;
            }
            
            case STATE_B: {
                /* Different chain of operations */
                int t1 = v_int2 - v_int1;
                int t2 = t1 * 3;
                int t3 = t2 & 0xFF;
                int t4 = t3 | (v_int3 << 8);
                int t5 = t4 ^ 0xABCD;
                v_int2 = t5;
                
                /* Floating point chain */
                double d1 = v_double1 * 1.5;
                double d2 = d1 + (v_int1 * 0.01);
                v_double1 = opaque_double(d2);
                
                /* Memory access */
                array_index = (array_index * 3 + 1) & 15;
                local_array[array_index] = v_int2;
                
                asm volatile ("" : : : "memory");
                
                if (complex_condition(v_int2, v_int3, v_int1)) {
                    state = STATE_C;
                } else {
                    state = STATE_D;
                }
                break;
            }
            
            case STATE_C: {
                /* More complex operations */
                int t1 = v_int3 * v_int1;
                int t2 = t1 / (v_int2 + 1);
                int t3 = t2 << 2;
                int t4 = t3 ^ v_int1;
                int t5 = t4 + (v_int2 >> 1);
                v_int3 = t5;
                
                /* Mixed integer/float */
                float f1 = v_float2 + v_int1;
                float f2 = f1 * 0.75f;
                v_float2 = opaque_float(f2);
                
                array_index = (array_index + 5) & 15;
                v_int1 = local_array[array_index] ^ v_int3;
                
                asm volatile ("" : : : "memory");
                
                if (complex_condition(v_int3, v_int1, v_int2)) {
                    state = STATE_D;
                } else {
                    state = STATE_E;
                }
                break;
            }
            
            case STATE_D: {
                int t1 = v_int1 ^ v_int2;
                int t2 = t1 * v_int3;
                int t3 = t2 + 12345;
                int t4 = t3 & 0xFFFF;
                int t5 = t4 | (v_int1 << 16);
                v_int1 = t5;
                
                double d1 = v_double1 - v_int2;
                double d2 = d1 * 0.9;
                v_double1 = opaque_double(d2);
                
                array_index = (array_index + 7) & 15;
                local_array[array_index] = v_int1 + v_int2;
                
                asm volatile ("" : : : "memory");
                
                if (complex_condition(v_int1, v_int2, v_int3)) {
                    state = STATE_E;
                } else {
                    state = STATE_A;
                }
                break;
            }
            
            case STATE_E: {
                int t1 = v_int2 + v_int3;
                int t2 = t1 * 7;
                int t3 = t2 ^ 0x1234;
                int t4 = t3 >> 4;
                int t5 = t4 + v_int1;
                v_int2 = t5;
                
                float f1 = v_float1 * v_float2;
                float f2 = f1 + 1.0f;
                v_float1 = opaque_float(f2);
                
                array_index = (array_index + 11) & 15;
                v_int3 = local_array[array_index] | v_int2;
                
                asm volatile ("" : : : "memory");
                
                if (complex_condition(v_int2, v_int3, v_int1)) {
                    state = STATE_A;
                } else {
                    state = STATE_B;
                }
                break;
            }
            
            /* Additional states to create more basic blocks */
            case STATE_F: case STATE_G: case STATE_H:
            case STATE_I: case STATE_J: case STATE_K:
            case STATE_L: case STATE_M: case STATE_N:
            case STATE_O: {
                /* Simplified operations for remaining states */
                v_int1 = v_int1 ^ v_int2 ^ v_int3;
                v_int2 = v_int2 + outer;
                v_int3 = v_int3 * 2;
                
                asm volatile ("" : : : "memory");
                
                state = STATE_A;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 8; inner++) {
            volatile int idx = (v_int1 + inner) & 15;
            int val = local_array[idx];
            
            /* Dependent operations */
            val = val + v_int2;
            val = val * (inner + 1);
            val = val ^ v_int3;
            
            local_array[idx] = val;
            
            /* Small scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Call noinline function to force scheduling boundary */
        v_int1 = opaque_int(v_int1);
        v_float1 = opaque_float(v_float1);
        v_double1 = opaque_double(v_double1);
    }
    
    /* Compute final checksum */
    int checksum = v_int1 + v_int2 + v_int3;
    checksum += (int)v_float1 + (int)v_float2;
    checksum += (int)v_double1;
    
    /* Sum array elements */
    for (int i = 0; i < 16; i++) {
        checksum += local_array[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Run scheduling stress multiple times */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((i % 25) == 0) {
            printf("Iteration %d: checksum = %d\n", i, result);
        }
    }
    
    printf("Final total checksum: %d\n", total_checksum);
    return 0;
}
