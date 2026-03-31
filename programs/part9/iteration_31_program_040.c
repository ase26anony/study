/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int sink = x;
    asm volatile ("" : "+r" (sink) : : "memory");
    return sink;
}

float opaque_float(float x) {
    volatile float sink = x;
    asm volatile ("" : "+f" (sink) : : "memory");
    return sink;
}

double opaque_double(double x) {
    volatile double sink = x;
    asm volatile ("" : "+f" (sink) : : "memory");
    return sink;
}

/* Memory barrier function */
void scheduling_barrier(void) __attribute__((noinline));
void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Complex state machine with data-dependent transitions */
int scheduling_stress(void) __attribute__((noinline));
int scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    volatile double d1 = 1.25, d2 = 2.25, d3 = 3.25;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex switch with many cases - creates multiple basic blocks */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed operations */
                a = b + c;
                f1 = opaque_float(f2 * f3);
                d = a * e;
                f2 = f1 + 1.0f;
                c = d >> 2;
                d1 = opaque_double(d2 * d3);
                e = c ^ a;
                f3 = f2 / 2.0f;
                d2 = d1 + 1.0;
                b = e & 0xFF;
                d3 = d2 * 0.5;
                
                /* Memory access with volatile index */
                int idx = (a + b) & 0xF;
                arr[idx] = arr[(idx + 1) & 0xF] + c;
                
                scheduling_barrier();
                state = (a + b) % 7;
                break;
            }
            
            case 1: {
                /* Different dependency pattern */
                b = c * d;
                f2 = opaque_float(f3 - f1);
                a = b ^ e;
                f3 = f2 * 3.0f;
                d = a << 3;
                d2 = opaque_double(d3 / d1);
                c = d | b;
                f1 = f3 - 2.0f;
                d3 = d2 - 0.25;
                e = c + a;
                
                /* Another memory access pattern */
                int idx = (c + d) & 0xF;
                arr[(idx + 3) & 0xF] = arr[idx] * 2;
                
                scheduling_barrier();
                state = (c + d + e) % 7;
                break;
            }
            
            case 2: {
                /* More complex floating point chain */
                f1 = opaque_float(f2 + f3);
                d1 = opaque_double(d2 - d3);
                a = (int)(f1 * 10.0f);
                b = (int)(d1 * 20.0);
                c = a * b;
                f2 = f1 * 2.0f;
                d = c >> 4;
                d2 = d1 * 1.5;
                e = d ^ c;
                f3 = f2 / 1.5f;
                d3 = d2 + 0.75;
                
                /* Array update */
                arr[(a + e) & 0xF] += b;
                
                scheduling_barrier();
                state = (a + c + e) % 7;
                break;
            }
            
            case 3: {
                /* Integer-heavy chain */
                a = b * c + d;
                b = (a << 2) | (e >> 1);
                c = b ^ d;
                d = c * 3 - a;
                e = (d & 0xFFFF) | (b & 0xFFFF0000);
                a = e + c;
                b = a * 7;
                c = b / 3;
                d = c ^ e;
                e = d + a + b;
                
                /* Memory operation */
                arr[(e >> 2) & 0xF] = arr[(e >> 4) & 0xF];
                
                scheduling_barrier();
                state = (b + d) % 7;
                break;
            }
            
            case 4: {
                /* Mixed type computations */
                f1 = opaque_float((float)a + f2);
                d1 = opaque_double((double)b + d2);
                c = (int)(f1 * 100.0f);
                d = (int)(d1 * 50.0);
                f2 = opaque_float((float)c / 10.0f);
                d2 = opaque_double((double)d / 5.0);
                e = c * d;
                f3 = f2 * f1;
                d3 = d2 * d1;
                a = e + (int)f3;
                b = a ^ (int)d3;
                
                scheduling_barrier();
                state = (e + (int)f3) % 7;
                break;
            }
            
            case 5: {
                /* Chain with many dependencies */
                a = opaque_int(b + c);
                b = opaque_int(d * e);
                c = opaque_int(a ^ b);
                d = opaque_int(c << 2);
                e = opaque_int(d >> 1);
                f1 = opaque_float(f2 * 3.14f);
                f2 = opaque_float(f3 + 2.71f);
                f3 = opaque_float(f1 / 1.618f);
                d1 = opaque_double(d2 * 1.414);
                d2 = opaque_double(d3 - 1.732);
                d3 = opaque_double(d1 / 2.718);
                
                /* Complex array update */
                for (int i = 0; i < 4; i++) {
                    int idx = (a + i) & 0xF;
                    arr[idx] = arr[(idx + i) & 0xF] + b;
                }
                
                scheduling_barrier();
                state = (a + b + c + d + e) % 7;
                break;
            }
            
            case 6: {
                /* Final state with RDTSC-like pattern (x86-specific) */
                uint64_t tsc1, tsc2;
                asm volatile ("rdtsc" : "=A" (tsc1));
                
                /* Do some work between measurements */
                a = b * c * d * e;
                f1 = f2 * f3 * 1.5f;
                d1 = d2 * d3 * 2.5;
                
                asm volatile ("rdtsc" : "=A" (tsc2));
                
                /* Use the difference in computation */
                b = (int)(tsc2 - tsc1) & 0xFFFF;
                c = a + b;
                
                scheduling_barrier();
                state = (outer + b) % 7;
                break;
            }
        }
        
        /* Inner loop with data-dependent condition */
        int inner_limit = (a + b + c) & 0xF;
        for (int inner = 0; inner < inner_limit; inner++) {
            /* Create memory dependencies */
            int idx1 = (inner + a) & 0xF;
            int idx2 = (inner + b) & 0xF;
            int idx3 = (inner + c) & 0xF;
            
            arr[idx1] = arr[idx2] + arr[idx3];
            arr[idx2] = arr[idx1] - inner;
            arr[idx3] = arr[idx2] * 2;
            
            /* Small dependency chain inside inner loop */
            d = arr[idx1] + arr[idx2];
            e = d * inner;
            f1 = (float)e / 256.0f;
            d1 = (double)d / 128.0;
        }
        
        counter++;
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = a + b + c + d + e;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2 + (int)d3;
    
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of
     * triggering scheduler context save/restore cycles */
    for (int i = 0; i < 100; i++) {
        total_checksum += scheduling_stress();
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
