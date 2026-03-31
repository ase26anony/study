/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int dummy = x;
    asm volatile ("" : "+r" (dummy) : : "memory");
    return dummy;
}

float opaque_float(float x) {
    volatile float dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

double opaque_double(double x) {
    volatile double dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

/* Complex helper to force scheduler context save */
void scheduling_barrier(volatile int* a, volatile float* b, volatile double* c) 
    __attribute__((noinline, noipa));

void scheduling_barrier(volatile int* a, volatile float* b, volatile double* c) {
    /* Force memory dependencies */
    *a = *a ^ 0x55555555;
    *b = *b * 1.0001f;
    *c = *c + 0.000000001;
    
    /* Inline asm with memory clobber - forces scheduler to save context */
    asm volatile (
        "# Scheduling Barrier\n\t"
        : 
        : 
        : "memory"
    );
}

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline, noipa));

int scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed * 3;
    volatile int v4 = seed * 4;
    volatile float f1 = seed * 1.0f;
    volatile float f2 = seed * 2.0f;
    volatile double d1 = seed * 1.0;
    volatile double d2 = seed * 2.0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Complex control flow with switch */
    volatile int state = seed % 8;
    volatile int counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* State machine using switch - creates many basic blocks */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                v1 = v2 + v3;
                f1 = f2 * (float)v1;
                d1 = d2 + (double)f1;
                v4 = (int)d1 ^ v4;
                f2 = (float)v4 * 1.41421356f;
                d2 = d1 * 0.70710678;
                v2 = v3 * v4;
                v3 = opaque_int(v2 + v1);
                
                /* Memory access with volatile index */
                arr[v1 & 0xF] = v3;
                v1 = arr[v2 & 0xF];
                
                scheduling_barrier(&v1, &f1, &d1);
                asm volatile ("# State 0 barrier" : : : "memory");
                state = (v1 + outer) % 8;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                f1 = f2 + 3.14159f;
                v1 = (int)f1 * v2;
                d1 = (double)v1 / 2.71828;
                v3 = v4 ^ (int)d1;
                f2 = opaque_float((float)v3 * 0.57721f);
                d2 = opaque_double(d1 + 1.61803);
                v2 = v1 * v3;
                v4 = opaque_int(v2 - v1);
                
                arr[v3 & 0xF] = v4;
                v2 = arr[v1 & 0xF];
                
                scheduling_barrier(&v2, &f2, &d2);
                asm volatile ("# State 1 barrier" : : : "memory");
                state = (v2 + outer * 3) % 8;
                break;
            }
            case 2: {
                /* Integer-heavy chain */
                v1 = v1 * 1103515245 + 12345;
                v2 = v2 ^ (v1 >> 16);
                v3 = v3 + v1 * v2;
                v4 = v4 * 1664525 + 1013904223;
                v1 = v1 ^ v3 ^ v4;
                v2 = v2 * 134775813 + 1;
                v3 = opaque_int(v3 + v2);
                v4 = opaque_int(v4 * v1);
                
                f1 = (float)v1 * 0.01f;
                d1 = (double)v2 * 0.001;
                
                arr[v4 & 0xF] = v1;
                v3 = arr[v2 & 0xF];
                
                scheduling_barrier(&v3, &f1, &d1);
                asm volatile ("# State 2 barrier" : : : "memory");
                state = (v3 ^ outer) % 8;
                break;
            }
            case 3: {
                /* Floating-point intensive */
                f1 = f1 * 1.0001f + 0.0001f;
                f2 = f2 * 0.9999f - 0.0001f;
                d1 = d1 * 1.000000001 + 0.000000001;
                d2 = d2 * 0.999999999 - 0.000000001;
                
                v1 = (int)(f1 * 1000.0f);
                v2 = (int)(d1 * 1000.0);
                v3 = opaque_int(v1 + v2);
                v4 = opaque_int((int)(f2 * 1000.0f) ^ (int)(d2 * 1000.0));
                
                f1 = opaque_float(f1 + f2);
                d1 = opaque_double(d1 - d2);
                
                arr[v3 & 0xF] = v4;
                v1 = arr[v2 & 0xF];
                
                scheduling_barrier(&v1, &f1, &d1);
                asm volatile ("# State 3 barrier" : : : "memory");
                state = ((int)f1 + outer) % 8;
                break;
            }
            case 4: {
                /* Mixed operations with memory */
                for (int i = 0; i < 8; i++) {
                    volatile int idx = (v1 + i) & 0xF;
                    arr[idx] = arr[idx] * 6364136223846793005ULL + 1442695040888963407ULL;
                    v1 = v1 ^ arr[idx];
                }
                
                f1 = (float)v1 * 0.001f;
                d1 = (double)v1 * 0.000001;
                v2 = opaque_int((int)(f1 * 1000.0f));
                v3 = opaque_int((int)(d1 * 1000000.0));
                
                scheduling_barrier(&v2, &f1, &d1);
                asm volatile ("# State 4 barrier" : : : "memory");
                state = (v2 + v3 + outer) % 8;
                break;
            }
            case 5: {
                /* Complex integer chain */
                v1 = v1 * 3 + 1;
                while ((v1 & 1) == 0) {
                    v1 = v1 >> 1;
                    v2 = v2 + 1;
                }
                v3 = v3 * 5 + 1;
                v4 = v4 ^ v1 ^ v2 ^ v3;
                
                /* Use x86-specific builtin if available */
                #ifdef __x86_64__
                unsigned long long tsc1, tsc2;
                asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
                v1 = v1 ^ (int)(tsc1 & 0xFFFFFFFF);
                #endif
                
                f1 = opaque_float((float)v4 * 0.333333f);
                d1 = opaque_double((double)v3 * 0.666666);
                
                scheduling_barrier(&v4, &f1, &d1);
                asm volatile ("# State 5 barrier" : : : "memory");
                state = (v4 * outer) % 8;
                break;
            }
            case 6: {
                /* Nested loops with break conditions */
                for (int i = 0; i < 5; i++) {
                    v1 = v1 + v2;
                    v2 = v2 - v3;
                    if (v1 > 1000000) {
                        v1 = v1 % 1000;
                        break;
                    }
                    v3 = v3 * v4;
                    if (v3 < 0) {
                        v3 = -v3;
                        continue;
                    }
                    v4 = v4 ^ v1;
                }
                
                f1 = (float)v1 * 0.5f;
                f2 = (float)v2 * 1.5f;
                d1 = (double)v3 * 0.25;
                d2 = (double)v4 * 0.75;
                
                scheduling_barrier(&v1, &f1, &d1);
                asm volatile ("# State 6 barrier" : : : "memory");
                state = (v1 + v2 + outer * 7) % 8;
                break;
            }
            case 7: {
                /* Computed goto state machine */
                static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
                volatile int label_idx = v1 & 3;
                
                goto *labels[label_idx];
                
                L0:
                    v1 = v1 * 2;
                    v2 = v2 + 1;
                    goto end_labels;
                L1:
                    v1 = v1 / 2;
                    v2 = v2 - 1;
                    goto end_labels;
                L2:
                    v1 = v1 ^ 0xAAAA;
                    v2 = v2 ^ 0x5555;
                    goto end_labels;
                L3:
                    v1 = v1 + v2;
                    v2 = v1 - v2;
                    goto end_labels;
                end_labels:
                
                /* More dependencies */
                v3 = opaque_int(v1 * v2);
                v4 = opaque_int(v1 + v2 + v3);
                f1 = opaque_float((float)v3 * 0.12345f);
                d1 = opaque_double((double)v4 * 0.54321);
                
                scheduling_barrier(&v3, &f1, &d1);
                asm volatile ("# State 7 barrier" : : : "memory");
                state = (v3 + outer * 11) % 8;
                break;
            }
        }
        
        counter++;
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx1 = (v1 + inner) & 0xF;
            volatile int idx2 = (v2 + inner * 2) & 0xF;
            volatile int idx3 = (v3 + inner * 3) & 0xF;
            
            arr[idx1] = arr[idx1] + arr[idx2];
            arr[idx2] = arr[idx2] ^ arr[idx3];
            arr[idx3] = arr[idx3] * arr[idx1];
            
            v1 = v1 + arr[idx1];
            v2 = v2 ^ arr[idx2];
            v3 = v3 * (arr[idx3] | 1); /* Avoid multiply by 0 */
        }
        
        /* Occasionally call opaque functions */
        if ((outer & 3) == 0) {
            v1 = opaque_int(v1);
            f1 = opaque_float(f1);
            d1 = opaque_double(d1);
        }
    }
    
    /* Final checksum */
    int checksum = v1 ^ v2 ^ v3 ^ v4;
    checksum ^= (int)f1 ^ (int)f2;
    checksum ^= (int)d1 ^ (int)d2;
    
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of
     * triggering scheduler context save/restore cycles */
    for (int rep = 0; rep < 100; rep++) {
        int seed = rand();
        int result = scheduling_stress(seed);
        total_checksum ^= result;
        
        /* Print progress occasionally */
        if ((rep % 25) == 0) {
            printf("Iteration %d, checksum so far: %d\n", rep, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum & 0xFF;
}
