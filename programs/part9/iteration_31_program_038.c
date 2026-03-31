/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noclone));
float opaque_float(float x) __attribute__((noinline, noclone));
double opaque_double(double x) __attribute__((noinline, noclone));
void scheduling_barrier(void) __attribute__((noinline, noclone));
void memory_access_pattern(int *arr, volatile int idx) __attribute__((noinline, noclone));

/* Helper to create complex data dependencies */
int complex_dependency_chain(volatile int a, volatile int b, volatile int c) 
    __attribute__((noinline, noclone));

/* State machine helper */
int update_state_machine(volatile int state, volatile int counter) 
    __attribute__((noinline, noclone));

/* Implementation of opaque functions */
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
    /* Inline assembly with memory clobber to force scheduling boundary */
    asm volatile ("" : : : "memory");
}

void memory_access_pattern(int *arr, volatile int idx) {
    /* Complex memory access pattern to create dependencies */
    volatile int temp = arr[idx % 16];
    arr[(idx + 1) % 16] = temp * 2 + 1;
    arr[(idx + 3) % 16] = arr[(idx + 2) % 16] + temp;
    scheduling_barrier();
}

int complex_dependency_chain(volatile int a, volatile int b, volatile int c) {
    /* Long chain of dependent integer operations */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 >> (b & 0x7);
    int t4 = t3 - a;
    int t5 = t4 * t1;
    int t6 = t5 & 0xFFFF;
    int t7 = t6 | (c << 16);
    int t8 = t7 ^ t4;
    
    /* Mix with floating point operations */
    float f1 = (float)t8 * 1.5f;
    float f2 = f1 / (float)(b + 1);
    int t9 = (int)f2 + t8;
    
    /* Another dependency chain */
    double d1 = (double)t9 * 3.14159;
    double d2 = d1 / (double)(a + 1);
    int t10 = (int)d2 * t9;
    
    scheduling_barrier();
    return opaque_int(t10);
}

int update_state_machine(volatile int state, volatile int counter) {
    /* Complex state transition logic */
    int new_state = state;
    
    switch (state % 8) {
        case 0:
            new_state = (counter * 3 + 1) % 11;
            break;
        case 1:
            new_state = (counter * 5 + 7) % 11;
            break;
        case 2:
            new_state = (counter * 11 + 13) % 11;
            break;
        case 3:
            new_state = (counter * 17 + 19) % 11;
            break;
        case 4:
            new_state = (counter * 23 + 29) % 11;
            break;
        case 5:
            new_state = (counter * 31 + 37) % 11;
            break;
        case 6:
            new_state = (counter * 41 + 43) % 11;
            break;
        case 7:
            new_state = (counter * 47 + 53) % 11;
            break;
        default:
            new_state = 0;
    }
    
    scheduling_barrier();
    return new_state;
}

/* Main scheduling stress function */
int scheduling_stress(void) __attribute__((noinline));
int scheduling_stress(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float fv1 = 1.0f, fv2 = 2.0f, fv3 = 3.0f;
    volatile double dv1 = 1.0, dv2 = 2.0;
    
    /* Local array with volatile accesses */
    int local_array[16];
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * i + 1;
    }
    
    volatile int state = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (outer_counter = 0; outer_counter < 50; outer_counter++) {
        volatile int inner_counter;
        
        /* Complex switch-based state machine */
        switch (state % 10) {
            case 0:
                v1 = complex_dependency_chain(v1, v2, v3);
                fv1 = opaque_float(fv1 * 1.1f + fv2);
                dv1 = opaque_double(dv1 * 1.01 + dv2);
                memory_access_pattern(local_array, v1);
                break;
                
            case 1:
                v2 = v1 * 3 + v3;
                v3 = v2 / 2 + v4;
                v4 = v3 | v5;
                v5 = v4 ^ v1;
                fv2 = fv1 + fv3;
                fv3 = opaque_float(fv2 * 0.9f);
                break;
                
            case 2:
                v1 = v5 + v2;
                v2 = v1 - v3;
                v3 = v2 * v4;
                v4 = v3 >> 2;
                v5 = v4 & 0xFF;
                dv2 = dv1 * 2.5;
                dv1 = opaque_double(dv2 / 1.7);
                break;
                
            case 3:
                for (inner_counter = 0; inner_counter < 8; inner_counter++) {
                    v1 = v1 + local_array[inner_counter % 16];
                    v2 = v2 * 2 - v1;
                    memory_access_pattern(local_array, v2);
                }
                break;
                
            case 4:
                v3 = (v1 << 3) | (v2 << 1);
                v4 = v3 ^ v5;
                v5 = v4 + v1;
                fv1 = opaque_float((float)v3 * 0.5f);
                fv3 = fv2 + fv1;
                break;
                
            case 5:
                /* Use rdtsc on x86 to create backend-specific scheduling */
                #ifdef __x86_64__
                {
                    unsigned long long tsc1, tsc2;
                    asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
                    v1 = (int)(tsc1 ^ tsc2);
                    asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
                    v2 = (int)(tsc1 ^ tsc2);
                }
                #endif
                v3 = complex_dependency_chain(v1, v2, v3);
                break;
                
            case 6:
                v4 = v1 + v2 + v3 + v4 + v5;
                v5 = v4 * 7;
                fv2 = opaque_float((float)v4 * 0.3f);
                dv2 = opaque_double((double)v5 * 0.7);
                break;
                
            case 7:
                for (inner_counter = 0; inner_counter < 5; inner_counter++) {
                    v1 = v1 * 3 + inner_counter;
                    v2 = v2 / 2 + v1;
                    local_array[inner_counter] = v1 + v2;
                }
                break;
                
            case 8:
                v3 = v1 | v2;
                v4 = v3 & v5;
                v5 = v4 ^ v3;
                fv3 = opaque_float(fv1 + fv2 + fv3);
                break;
                
            case 9:
                v1 = v5 - v4;
                v2 = v1 * v3;
                v3 = v2 >> 1;
                dv1 = opaque_double(dv1 + dv2);
                break;
        }
        
        /* Scheduling barrier at the end of each state */
        asm volatile ("" : : : "memory");
        
        /* Update state with complex condition */
        state = update_state_machine(state, outer_counter);
        
        /* Additional memory dependencies */
        volatile int idx = (v1 + v2 + v3) % 16;
        local_array[idx] = v4 + v5;
        local_array[(idx + 5) % 16] = local_array[idx] * 3;
    }
    
    /* Compute final checksum */
    int checksum = v1 + v2 + v3 + v4 + v5;
    checksum += (int)fv1 + (int)fv2 + (int)fv3;
    checksum += (int)dv1 + (int)dv2;
    
    for (int i = 0; i < 16; i++) {
        checksum += local_array[i];
    }
    
    scheduling_barrier();
    return opaque_int(checksum);
}

int main(void) {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase coverage probability */
    for (int i = 0; i < 100; i++) {
        int result = scheduling_stress();
        total_checksum += result;
        
        /* Vary initial conditions slightly */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
