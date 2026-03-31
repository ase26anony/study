/* haifa-sched-trigger.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer haifa-sched-trigger.c -o haifa-sched-trigger
 * Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops
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
    /* Empty function that forces scheduler to save/restore context */
    asm volatile("" : : : "memory");
}

/* State machine with complex control flow */
typedef enum {
    STATE_A, STATE_B, STATE_C, STATE_D, STATE_E,
    STATE_F, STATE_G, STATE_H, STATE_I, STATE_J,
    STATE_K, STATE_L, STATE_M, STATE_N, STATE_O
} state_t;

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed * 3;
    volatile int v4 = seed * 4;
    volatile float f1 = seed * 1.1f;
    volatile float f2 = seed * 2.2f;
    volatile double d1 = seed * 1.11;
    volatile double d2 = seed * 2.22;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Volatile index for unpredictable array access */
    volatile int idx = 0;
    
    /* State variable with complex transitions */
    state_t state = STATE_A;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex switch statement with many basic blocks */
        switch (state) {
            case STATE_A: {
                /* Long dependency chain with mixed types */
                v1 = v2 + v3;
                f1 = f2 * 3.14f;
                v2 = v1 * v4;
                d1 = d2 / 2.0;
                v3 = (int)(f1) + v2;
                f2 = (float)v3 * 1.618f;
                v4 = v1 ^ v2 ^ v3;
                d2 = d1 * d1 + d2;
                
                /* Memory access with volatile index */
                idx = (idx + 1) & 0xF;
                v1 += arr[idx];
                
                /* Call to noinline function - scheduling boundary */
                v1 = opaque_int(v1);
                
                /* Inline assembly barrier */
                asm volatile("" : : : "memory");
                
                /* Complex state transition */
                state = (v1 & 1) ? STATE_B : STATE_C;
                break;
            }
            
            case STATE_B: {
                /* Different dependency pattern */
                v2 = v3 * v4;
                f1 = f1 + f2;
                v3 = v2 >> 2;
                d1 = d2 - d1;
                v4 = v1 | v2 | v3;
                f2 = f1 * f2 / 2.0f;
                v1 = v4 - v3;
                d2 = d1 * 3.14159;
                
                idx = (idx * 3 + 7) & 0xF;
                v2 += arr[idx];
                
                f1 = opaque_float(f1);
                asm volatile("" : : : "memory");
                
                state = (v2 % 3 == 0) ? STATE_C : STATE_D;
                break;
            }
            
            case STATE_C: {
                v3 = v1 + v2 + v4;
                f2 = f1 - f2;
                v1 = v3 * 7;
                d1 = d2 / d1;
                v2 = v1 & 0x55555555;
                f1 = (float)v2 * 0.5f;
                v4 = v3 ^ 0xAAAAAAAA;
                d2 = d1 + d2;
                
                idx = (idx + 5) & 0xF;
                v3 += arr[idx];
                
                d1 = opaque_double(d1);
                asm volatile("" : : : "memory");
                
                state = (v3 > 1000) ? STATE_D : STATE_E;
                break;
            }
            
            case STATE_D: {
                v4 = v1 * v2 * v3;
                f1 = f2 * f1;
                v1 = v4 / 3;
                d2 = d1 * 2.71828;
                v2 = v1 << 3;
                f2 = f1 + 42.0f;
                v3 = v2 | v4;
                d1 = d2 - 1.0;
                
                idx = (idx * 2 + 1) & 0xF;
                v4 += arr[idx];
                
                scheduling_barrier();
                asm volatile("" : : : "memory");
                
                state = (v4 % 5 == 0) ? STATE_E : STATE_F;
                break;
            }
            
            case STATE_E: {
                v1 = (v2 + v3) * v4;
                f2 = f1 / f2;
                v2 = v1 % 17;
                d1 = d2 * 0.5;
                v3 = ~v1;
                f1 = (float)v3 * 0.25f;
                v4 = v2 ^ v3;
                d2 = d1 + 3.14159;
                
                idx = (idx + 11) & 0xF;
                v1 += arr[idx];
                
                v2 = opaque_int(v2);
                asm volatile("" : : : "memory");
                
                state = STATE_F;
                break;
            }
            
            case STATE_F: case STATE_G: case STATE_H: case STATE_I: case STATE_J:
            case STATE_K: case STATE_L: case STATE_M: case STATE_N: case STATE_O: {
                /* Additional states with similar patterns */
                int base_state = state - STATE_F;
                v1 = v2 + base_state;
                v2 = v3 * (base_state + 1);
                v3 = v4 >> (base_state & 3);
                v4 = v1 ^ v2 ^ v3;
                f1 = f2 + (float)base_state;
                f2 = f1 * 1.1f;
                d1 = d2 / (base_state + 2.0);
                d2 = d1 * 1.618;
                
                idx = (idx + base_state * 3) & 0xF;
                arr[idx] = v1 + v2 + v3 + v4;
                
                if (base_state % 2 == 0) {
                    f1 = opaque_float(f1);
                } else {
                    d1 = opaque_double(d1);
                }
                
                asm volatile("" : : : "memory");
                
                /* Complex state transition network */
                int next = (v1 + v2 + v3 + v4) % 15;
                state = (state_t)next;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int temp = arr[inner & 0xF];
            v1 = v1 + temp;
            v2 = v2 - temp;
            v3 = v3 * (temp & 0xFF);
            v4 = v4 ^ temp;
            
            /* Mix in floating point operations */
            f1 = f1 + (float)temp;
            f2 = f2 * (1.0f + (float)(temp % 10) / 10.0f);
            d1 = d1 + (double)temp;
            d2 = d2 / (1.0 + (double)(temp % 5) / 5.0);
            
            /* Occasional scheduling barrier */
            if ((inner & 3) == 0) {
                scheduling_barrier();
            }
        }
        
        /* Update volatile variables to prevent dead code elimination */
        v1 = opaque_int(v1);
        v2 = opaque_int(v2);
        v3 = opaque_int(v3);
        v4 = opaque_int(v4);
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = v1 + v2 + v3 + v4;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    /* Final array checksum */
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((i % 25) == 0) {
            printf("Iteration %d: checksum = %d\n", i, result);
        }
    }
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
