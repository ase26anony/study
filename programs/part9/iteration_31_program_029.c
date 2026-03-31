/* sched_context_test.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

/* Memory barrier via inline assembly */
static inline void scheduling_barrier(void) {
    __asm__ volatile ("" : : : "memory");
}

/* State machine helper with complex control flow */
void state_machine_transition(int *state, volatile int *counter) __attribute__((noinline));

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));

/* Implementation of opaque functions */
int opaque_int(int x) {
    scheduling_barrier();
    return x ^ 0x55AA55AA;
}

float opaque_float(float x) {
    scheduling_barrier();
    return x * 1.0001f;
}

double opaque_double(double x) {
    scheduling_barrier();
    return x * 1.0000001;
}

/* Complex state machine that creates multiple basic blocks */
void state_machine_transition(int *state, volatile int *counter) {
    int local_state = *state;
    volatile int local_counter = *counter;
    
    switch (local_state) {
        case 0: {
            /* Chain of dependent integer operations */
            int a = local_counter + 1;
            int b = a * 3;
            int c = b >> 2;
            int d = c ^ 0x12345678;
            int e = d * 7;
            int f = e - 11;
            int g = f & 0xFF;
            *state = g % 10;
            break;
        }
        case 1: {
            /* Mixed integer/float operations */
            float x = (float)local_counter * 1.5f;
            float y = x * x;
            float z = y / 3.14159f;
            int i = (int)z;
            *state = (i & 7) + 1;
            break;
        }
        case 2: {
            /* Long dependency chain with calls */
            int val = local_counter;
            val = opaque_int(val);
            val = val * 3 + 1;
            val = opaque_int(val);
            val = (val << 4) | (val >> 28);
            val = opaque_int(val);
            *state = val % 10;
            break;
        }
        case 3: {
            /* Double precision chain */
            double d1 = (double)local_counter;
            double d2 = d1 * 2.718281828;
            double d3 = opaque_double(d2);
            double d4 = d3 / 1.414213562;
            int i = (int)d4;
            *state = (i * 7) % 10;
            break;
        }
        case 4: {
            /* Complex integer chain */
            int x = local_counter;
            for (int i = 0; i < 5; i++) {
                x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
                x = opaque_int(x);
            }
            *state = x % 10;
            break;
        }
        case 5: {
            /* Mixed operations with barrier */
            int a = local_counter;
            float f = (float)a;
            f = opaque_float(f);
            a = (int)f;
            scheduling_barrier();
            a = a * 13 + 7;
            *state = a % 10;
            break;
        }
        case 6: {
            /* Another long chain */
            int x = local_counter;
            x = x ^ (x >> 16);
            x = x * 0x85EBCA6B;
            x = x ^ (x >> 13);
            x = x * 0xC2B2AE35;
            x = x ^ (x >> 16);
            *state = (x < 0) ? 7 : 3;
            break;
        }
        case 7: {
            /* Chain with memory operations */
            volatile int arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = local_counter + i;
            }
            int sum = 0;
            for (int i = 0; i < 8; i++) {
                sum += arr[i];
            }
            *state = sum % 10;
            break;
        }
        case 8: {
            /* Complex floating point chain */
            float f = (float)local_counter;
            f = f * 1.2345f;
            f = opaque_float(f);
            f = f + 5.6789f;
            f = opaque_float(f);
            f = f / 2.3456f;
            *state = ((int)f) % 10;
            break;
        }
        case 9: {
            /* Final state with many operations */
            int x = local_counter;
            x = x * 3;
            x = x + 1;
            x = x >> 1;
            x = x * 5;
            x = x - 2;
            x = x & 0xFFF;
            x = opaque_int(x);
            x = x * 9;
            x = x + 3;
            *state = x % 10;
            break;
        }
        default:
            *state = 0;
            break;
    }
    
    scheduling_barrier();
}

/* Main scheduling stress function */
int scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 100;
    volatile float f1 = (float)seed / 3.0f;
    volatile float f2 = (float)seed * 1.5f;
    volatile double d1 = (double)seed * 2.5;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    int state = seed % 10;
    int checksum = 0;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < 50; outer++) {
        /* Update volatile variables */
        v1 = v1 * 3 + 1;
        v2 = v2 ^ v1;
        v3 = v3 + outer;
        f1 = opaque_float(f1);
        f2 = f2 * 1.1f;
        d1 = opaque_double(d1);
        
        /* Call to create scheduling boundary */
        state_machine_transition(&state, &v1);
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 8; inner++) {
            volatile int idx = (v1 + inner) & 0xF;
            arr[idx] = arr[idx] * 3 + 1;
            scheduling_barrier();
        }
        
        /* Complex switch-based state machine */
        switch (state) {
            case 0:
                v1 = v1 + v2;
                v2 = v2 * 2;
                break;
            case 1:
                v1 = v1 ^ v3;
                v3 = v3 >> 1;
                break;
            case 2:
                f1 = f1 + f2;
                v1 = (int)f1;
                break;
            case 3:
                d1 = d1 * 1.01;
                v2 = (int)d1;
                break;
            case 4:
                v3 = v3 + arr[v1 & 0xF];
                break;
            case 5:
                v1 = opaque_int(v1);
                v2 = opaque_int(v2);
                break;
            case 6:
                f1 = opaque_float(f1);
                f2 = opaque_float(f2);
                break;
            case 7:
                d1 = opaque_double(d1);
                break;
            case 8:
                for (int i = 0; i < 4; i++) {
                    arr[i] = arr[i] + arr[i+4];
                }
                break;
            case 9:
                v1 = (v1 << 3) | (v1 >> 29);
                v2 = (v2 << 5) | (v2 >> 27);
                break;
        }
        
        scheduling_barrier();
        
        /* Update checksum */
        checksum += v1 + v2 + v3 + (int)f1 + (int)f2 + (int)d1;
        for (int i = 0; i < 8; i++) {
            checksum += arr[i];
        }
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
