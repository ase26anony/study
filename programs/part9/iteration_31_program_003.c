#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int temp = x;
    asm volatile ("" : "+r" (temp) : : "memory");
    return temp;
}

float opaque_float(float x) {
    volatile float temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

double opaque_double(double x) {
    volatile double temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Complex state machine with scheduling boundaries */
static int scheduling_stress(void) __attribute__((noinline, noipa));

static int scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer = opaque_int(outer + 1)) {
        /* Complex switch statement with distinct operation chains */
        switch (state) {
            case 0: {
                /* Integer dependency chain */
                a = b + c;
                d = a * e;
                e = d >> 2;
                b = e ^ a;
                c = b * d;
                a = c - e;
                
                /* Mixed type operations */
                f1 = (float)a + f2;
                f3 = f1 * f2;
                d1 = (double)f3 + d2;
                d3 = d1 * d2;
                
                /* Memory access with volatile index */
                int idx = (a + b) & 15;
                arr[idx] = arr[(idx + 1) & 15] + c;
                
                scheduling_barrier();
                state = (a & 3) + 1;
                break;
            }
            
            case 1: {
                /* Different integer chain */
                b = c * d;
                a = b >> e;
                c = a ^ b;
                d = c + e;
                e = d * a;
                b = e - c;
                
                /* Floating point chain */
                f2 = f3 * f1;
                f1 = f2 + 3.14f;
                f3 = opaque_float(f1 - f2);
                
                /* Double precision chain */
                d2 = d3 / d1;
                d1 = opaque_double(d2 * 3.14159);
                
                /* Array access pattern */
                for (int i = 0; i < 4; i++) {
                    int idx = (i + a) & 15;
                    arr[idx] = arr[(idx + 4) & 15] + b;
                }
                
                scheduling_barrier();
                state = (b & 2) ? 2 : 3;
                break;
            }
            
            case 2: {
                /* Long dependency chain */
                c = a + b;
                for (int i = 0; i < 5; i++) {
                    c = opaque_int(c * d + e);
                    d = opaque_int(d ^ c);
                    e = opaque_int(e + a);
                }
                
                /* Mixed operations */
                f3 = (float)c / 2.0f;
                f1 = f2 + f3;
                d3 = (double)f1 * d2;
                
                /* Memory barrier */
                asm volatile ("mfence" : : : "memory");
                
                state = (c & 1) ? 0 : 3;
                break;
            }
            
            case 3: {
                /* Complex chain with function calls */
                a = opaque_int(b * c);
                b = opaque_int(c + d);
                c = opaque_int(d ^ e);
                
                /* Floating point intensive */
                f1 = opaque_float(f2 * f3);
                f2 = opaque_float(f1 + 1.618f);
                f3 = opaque_float(f2 / f1);
                
                /* Double precision */
                d1 = opaque_double(d2 + d3);
                d2 = opaque_double(d1 * 1.414);
                
                /* Array computation */
                int sum = 0;
                for (int i = 0; i < 8; i++) {
                    sum += arr[(i + a) & 15];
                }
                arr[a & 15] = sum;
                
                scheduling_barrier();
                state = (sum & 1) ? 1 : 2;
                break;
            }
            
            default:
                state = 0;
                break;
        }
        
        counter = opaque_int(counter + 1);
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner = opaque_int(inner + 1)) {
            int idx1 = (inner + a) & 15;
            int idx2 = (inner + b) & 15;
            int idx3 = (inner + c) & 15;
            
            arr[idx1] = arr[idx2] + arr[idx3];
            arr[idx2] = arr[idx1] - inner;
            arr[idx3] = arr[idx2] * arr[idx1];
            
            /* Small inline asm with memory clobber */
            asm volatile ("# scheduler barrier" : : : "memory");
        }
    }
    
    /* Compute checksum from all variables */
    int checksum = a + b + c + d + e;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2 + (int)d3;
    
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return opaque_int(checksum);
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        total_checksum ^= scheduling_stress();
        
        /* Vary conditions slightly each iteration */
        if (i % 7 == 0) {
            asm volatile ("# periodic barrier" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
