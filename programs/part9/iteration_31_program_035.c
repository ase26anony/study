#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

/* Memory barrier function */
void memory_barrier(void) __attribute__((noinline, noipa));

/* State machine helper */
int next_state(int current, int condition) __attribute__((noinline, noipa));

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline, noipa));

/* Implementation of opaque functions */
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

void memory_barrier(void) {
    asm volatile ("" : : : "memory");
}

int next_state(int current, int condition) {
    volatile int a = condition;
    volatile int b = current;
    
    /* Complex state transition with data dependencies */
    int t1 = a * 1103515245 + 12345;
    int t2 = (t1 >> 16) & 32767;
    int t3 = b * 1664525 + 1013904223;
    int t4 = t2 ^ t3;
    int t5 = (t4 * 1103515245) + 12345;
    int t6 = (t5 >> 16) & 32767;
    
    return opaque_int(t6 % 10);
}

int scheduling_stress(int seed) {
    volatile int state = seed % 10;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    
    /* Local array with volatile accesses */
    volatile int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 1103515245 + 12345;
    }
    
    /* Outer loop - fixed iterations */
    for (int outer = 0; outer < 50; outer++) {
        /* Update volatile variables to prevent optimization */
        i1 = opaque_int(i1 + outer);
        i2 = opaque_int(i2 * 1103515245 + 12345);
        
        /* Complex switch-based state machine */
        switch (state) {
            case 0: {
                /* Long chain of dependent integer operations */
                int t1 = i1 * i2 + i3;
                int t2 = t1 >> (i4 & 3);
                int t3 = t2 * 1103515245;
                int t4 = t3 + i5;
                int t5 = (t4 ^ t1) & 0x7FFFFFFF;
                int t6 = t5 * 1664525 + 1013904223;
                i1 = opaque_int(t6);
                
                /* Mixed floating point operations */
                f1 = f1 * 1.1f + f2;
                f2 = f2 / 1.2f - f3;
                f3 = opaque_float(f3 + f1 * f2);
                
                /* Memory access with volatile index */
                int idx = (i1 + outer) % 20;
                i3 = arr[idx] + i2;
                
                memory_barrier();
                break;
            }
            
            case 1: {
                /* Different chain of operations */
                double t1 = d1 * d2 + d3;
                double t2 = t1 / 1.23456789;
                double t3 = t2 * 0.987654321;
                d1 = opaque_double(t3);
                
                int t4 = i2 * i3 - i4;
                int t5 = (t4 << 3) | (t4 >> 29);
                int t6 = t5 ^ i1;
                i2 = opaque_int(t6);
                
                /* Array access pattern */
                for (int j = 0; j < 5; j++) {
                    int idx = (i2 + j) % 20;
                    arr[idx] = arr[idx] * 1103515245 + 12345;
                }
                
                memory_barrier();
                break;
            }
            
            case 2: {
                /* More complex mixed operations */
                f1 = opaque_float(f1 * 2.0f - f2 / 1.5f);
                int t1 = (int)(f1 * 1000.0f);
                i3 = opaque_int(i3 * t1 + i4);
                
                d2 = d1 * 0.5 + d3 * 0.5;
                d3 = opaque_double(d2 * 1.1 - d1 * 0.1);
                
                /* Chain of integer operations */
                int t2 = i1 + i2;
                int t3 = t2 * i3;
                int t4 = t3 >> (i5 & 7);
                int t5 = t4 ^ i4;
                i4 = opaque_int(t5);
                
                memory_barrier();
                break;
            }
            
            case 3: {
                /* Use inline assembly as scheduling barrier */
                asm volatile (
                    "movl %0, %%eax\n\t"
                    "imull %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "movl %%eax, %0"
                    : "+r" (i5)
                    : "r" (i1), "r" (i2)
                    : "%eax", "memory"
                );
                
                f3 = opaque_float(f3 * 3.14159265f);
                d1 = d2 * 2.718281828 + d3;
                
                memory_barrier();
                break;
            }
            
            case 4: {
                /* Long dependency chain */
                int t1 = i1 * 1103515245;
                int t2 = t1 + 12345;
                int t3 = (t2 >> 16) & 32767;
                int t4 = t3 * i2;
                int t5 = t4 ^ i3;
                int t6 = t5 * 1664525;
                int t7 = t6 + 1013904223;
                int t8 = (t7 >> 16) & 32767;
                i1 = opaque_int(t8);
                
                f2 = opaque_float(f2 + f1 * 0.1f);
                d3 = opaque_double(d3 * 0.99);
                
                memory_barrier();
                break;
            }
            
            /* Additional cases to create more basic blocks */
            case 5: {
                int t1 = i4 * i5 + i1;
                i2 = opaque_int(t1 ^ i3);
                f1 = opaque_float(f1 / 1.41421356f);
                break;
            }
            
            case 6: {
                d2 = opaque_double(d1 + d3);
                int t1 = i2 * 3 + i4;
                i3 = opaque_int(t1);
                break;
            }
            
            case 7: {
                f3 = opaque_float(f2 * f1 - f3);
                int t1 = i5 << 2;
                i4 = opaque_int(t1 | i2);
                break;
            }
            
            case 8: {
                int t1 = i1 * i3;
                int t2 = t1 / (i2 + 1);
                i5 = opaque_int(t2 + i4);
                d1 = opaque_double(d2 * 0.5);
                break;
            }
            
            case 9: {
                f2 = opaque_float(f3 * 2.0f + f1);
                int t1 = i4 ^ i5;
                i1 = opaque_int(t1 * i2);
                break;
            }
        }
        
        /* Update state with complex condition */
        int cond = (i1 ^ i2) + (i3 * i4) - i5;
        cond += (int)(f1 * 100.0f) + (int)(d1 * 100.0);
        state = next_state(state, cond);
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx1 = (i1 + inner) % 20;
            volatile int idx2 = (i2 + inner * 2) % 20;
            
            /* Create memory dependencies */
            int temp = arr[idx1];
            arr[idx1] = arr[idx2] * 1103515245 + 12345;
            arr[idx2] = temp ^ arr[idx1];
            
            /* Small computation to use the result */
            i3 = opaque_int(i3 + arr[idx1] % 100);
        }
        
        counter = opaque_int(counter + 1);
    }
    
    /* Compute final checksum */
    int checksum = i1 + i2 + i3 + i4 + i5;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2 + (int)d3;
    
    /* Use array values in checksum */
    for (int i = 0; i < 20; i++) {
        checksum ^= arr[i];
    }
    
    return opaque_int(checksum);
}

int main(void) {
    srand(time(NULL));
    int total = 0;
    
    /* Call scheduling_stress multiple times */
    for (int i = 0; i < 100; i++) {
        int seed = rand();
        int result = scheduling_stress(seed);
        total = opaque_int(total + result);
        
        /* Occasionally add a scheduling barrier */
        if (i % 23 == 0) {
            memory_barrier();
        }
    }
    
    printf("Final checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
