#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));

/* Helper to force memory dependencies */
volatile int global_seed;

int opaque_int(int x) {
    asm volatile ("" : "+r" (x) : : "memory");
    return x ^ global_seed;
}

float opaque_float(float x) {
    volatile float temp = x;
    asm volatile ("" : "+f" (x) : : "memory");
    return temp + (global_seed & 0xFF) * 0.001f;
}

double opaque_double(double x) {
    volatile double temp = x;
    asm volatile ("" : "+f" (x) : : "memory");
    return temp + (global_seed & 0x7FF) * 0.000001;
}

void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Main scheduling stress function */
unsigned long long scheduling_stress(int iterations) __attribute__((noinline, noipa));

unsigned long long scheduling_stress(int iterations) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile float f1 = 1.234f, f2 = 5.678f, f3 = 9.012f;
    volatile double d1 = 1.234567, d2 = 8.901234, d3 = 5.678901;
    volatile int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    volatile int arr[16];
    volatile int idx1, idx2;
    
    /* Initialize array with volatile indices */
    for (int i = 0; i < 16; i++) {
        arr[i] = (i * 37) & 0xFF;
    }
    
    unsigned long long checksum = 0;
    
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex state machine with switch */
        switch (state) {
            case 0: {
                /* Chain of dependent integer operations */
                i1 = opaque_int(i1 + i2);
                i2 = opaque_int(i2 * i3 - i1);
                i3 = opaque_int((i3 << 3) | (i1 & 0xF));
                i4 = opaque_int(i4 ^ i3 ^ i2);
                i5 = opaque_int(i5 + (i4 >> 2));
                
                /* Mixed type dependencies */
                f1 = opaque_float(f1 + (float)i1 * 0.1f);
                d1 = opaque_double(d1 + (double)i2 * 0.01);
                
                /* Memory access with volatile indices */
                idx1 = (i1 + i2) & 0xF;
                idx2 = (i3 + i4) & 0xF;
                arr[idx1] = opaque_int(arr[idx1] + arr[idx2]);
                
                scheduling_barrier();
                state = (i5 & 0x1) ? 1 : 7;
                break;
            }
            
            case 1: {
                /* Floating point intensive chain */
                f2 = opaque_float(f2 * f1 + 1.414f);
                f3 = opaque_float(f3 / f2 - 0.707f);
                f1 = opaque_float(f1 + f2 * f3);
                
                /* Convert to integer chain */
                i1 = opaque_int((int)(f1 * 100.0f) ^ i1);
                i2 = opaque_int((int)(f2 * 100.0f) | i2);
                
                /* Double precision chain */
                d2 = opaque_double(d2 * d1 + 3.14159);
                d3 = opaque_double(d3 / d2 - 1.61803);
                d1 = opaque_double(d1 + d2 - d3);
                
                /* Array manipulation */
                idx1 = (i1 ^ i2) & 0xF;
                arr[idx1] = opaque_int(arr[idx1] + (int)(d1 * 10.0));
                
                scheduling_barrier();
                state = (arr[idx1] & 0x2) ? 2 : 0;
                break;
            }
            
            case 2: {
                /* Long integer dependency chain */
                i3 = opaque_int(i3 + i4 * i5);
                i4 = opaque_int(i4 - (i3 >> 1));
                i5 = opaque_int(i5 ^ (i4 << 2));
                i1 = opaque_int(i1 * i3 + i5);
                i2 = opaque_int(i2 / (i1 & 0xFF) + 1);
                i3 = opaque_int(i3 | i2);
                i4 = opaque_int(i4 & ~i3);
                i5 = opaque_int(i5 + i4 * 3);
                
                /* Memory barrier via inline asm */
                asm volatile ("" : : : "memory");
                
                /* Complex array update */
                for (int j = 0; j < 4; j++) {
                    idx1 = (i1 + j) & 0xF;
                    idx2 = (i2 - j) & 0xF;
                    arr[idx1] = opaque_int(arr[idx1] ^ arr[idx2]);
                }
                
                scheduling_barrier();
                state = 3;
                break;
            }
            
            case 3: {
                /* Mixed operations with memory clobber */
                f1 = opaque_float(f1 * 2.0f - f3);
                i1 = opaque_int((int)f1 + i1);
                
                asm volatile ("# scheduler barrier" : : : "memory");
                
                d2 = opaque_double(d2 + (double)i1 * 0.001);
                i2 = opaque_int(i2 ^ (int)(d2 * 1000.0));
                
                /* Multiple array accesses */
                volatile int sum = 0;
                for (int j = 0; j < 8; j++) {
                    idx1 = (i2 + j * 3) & 0xF;
                    sum = opaque_int(sum + arr[idx1]);
                }
                i3 = opaque_int(i3 + sum);
                
                scheduling_barrier();
                state = (i3 & 0x4) ? 4 : 1;
                break;
            }
            
            case 4: {
                /* Use CPU timestamp counter if available */
                #ifdef __x86_64__
                unsigned long long tsc1, tsc2;
                asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
                i4 = opaque_int(i4 ^ (int)(tsc1 & 0xFFFFFFFF));
                asm volatile ("" : : : "memory");
                asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
                i5 = opaque_int(i5 ^ (int)(tsc1 & 0xFFFFFFFF));
                #endif
                
                /* Complex floating chain */
                f2 = opaque_float(f2 + (float)i4 * 0.01f);
                f3 = opaque_float(f3 * f2 / 3.14159f);
                f1 = opaque_float(f1 - f3 + f2);
                
                scheduling_barrier();
                state = 5;
                break;
            }
            
            case 5: {
                /* Nested loop with data-dependent exit */
                volatile int inner_cnt = (i1 & 0x7) + 1;
                for (int k = 0; k < inner_cnt; k++) {
                    i1 = opaque_int(i1 + k);
                    idx1 = (i1 + k) & 0xF;
                    idx2 = (i2 - k) & 0xF;
                    arr[idx1] = opaque_int(arr[idx1] + arr[idx2] + k);
                    
                    /* Small scheduling barrier in loop */
                    if (k & 1) {
                        asm volatile ("" : : : "memory");
                    }
                }
                
                scheduling_barrier();
                state = 6;
                break;
            }
            
            case 6: {
                /* More mixed operations */
                d3 = opaque_double(d3 * 1.5 - d1);
                i2 = opaque_int(i2 + (int)(d3 * 100.0));
                f3 = opaque_float(f3 + (float)i2 * 0.001f);
                i3 = opaque_int(i3 ^ (int)(f3 * 1000.0f));
                
                /* Multiple memory accesses */
                volatile int tmp = arr[(i3 + 1) & 0xF];
                tmp = opaque_int(tmp + arr[(i3 + 2) & 0xF]);
                tmp = opaque_int(tmp + arr[(i3 + 3) & 0xF]);
                i4 = opaque_int(i4 + tmp);
                
                scheduling_barrier();
                state = (i4 & 0x8) ? 7 : 2;
                break;
            }
            
            case 7: {
                /* Final complex chain */
                i5 = opaque_int(i5 * 7 + 13);
                f1 = opaque_float(f1 + (float)i5 * 0.001f);
                d1 = opaque_double(d1 + (double)((int)f1) * 0.0001);
                i1 = opaque_int(i1 ^ (int)(d1 * 10000.0));
                
                /* Update all array elements */
                for (int j = 0; j < 16; j += 2) {
                    arr[j] = opaque_int(arr[j] + i1);
                    arr[j + 1] = opaque_int(arr[j + 1] + i2);
                }
                
                scheduling_barrier();
                state = 0;
                break;
            }
        }
        
        counter = opaque_int(counter + 1);
        
        /* Update checksum with all volatile values */
        checksum ^= (unsigned long long)i1;
        checksum = (checksum << 13) | (checksum >> 51);
        checksum ^= (unsigned long long)i2;
        checksum = (checksum << 17) | (checksum >> 47);
        checksum ^= (unsigned long long)((int)(f1 * 1000.0f));
        checksum = (checksum << 5) | (checksum >> 59);
        checksum ^= (unsigned long long)((int)(d1 * 1000000.0));
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    global_seed = rand();
    
    unsigned long long total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of
       scheduler context saving/restoring */
    for (int i = 0; i < 100; i++) {
        total_checksum ^= scheduling_stress(50);
        global_seed = opaque_int(global_seed + i);
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    return 0;
}
