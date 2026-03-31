/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline));
float opaque_float(float x) __attribute__((noinline));
double opaque_double(double x) __attribute__((noinline));

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

/* Complex helper function that forces scheduler to save context */
void scheduling_barrier(volatile int* a, volatile float* b, volatile double* c) __attribute__((noinline));

void scheduling_barrier(volatile int* a, volatile float* b, volatile double* c) {
    /* Use inline assembly with memory clobber to force scheduling boundary */
    asm volatile (
        "mfence\n\t"
        :
        :
        : "memory"
    );
    
    /* Complex dependency chain that scheduler must handle */
    int temp1 = *a + 1;
    float temp2 = *b * 1.1f;
    double temp3 = *c / 1.01;
    
    /* More operations with dependencies */
    temp1 = temp1 * 3 - temp1 / 2;
    temp2 = temp2 + (float)temp1 * 0.5f;
    temp3 = temp3 - (double)temp2 * 0.25;
    
    /* Store results back */
    *a = opaque_int(temp1);
    *b = opaque_float(temp2);
    *c = opaque_double(temp3);
}

/* State machine function with complex control flow */
int scheduling_stress(void) __attribute__((noinline));

int scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int result = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Update state based on complex condition */
        state = (state + outer * 7) % 10;
        
        /* Switch statement with many cases - creates multiple basic blocks */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                int t1 = counter * 2 + 1;
                float t2 = f1 * (float)t1;
                double t3 = d1 + (double)t2;
                t1 = t1 ^ (int)t3;
                t2 = t2 / f2 + 1.5f;
                t3 = t3 * d2 - 0.5;
                t1 = (t1 << 3) | (t1 >> 5);
                result += t1 + (int)t2 + (int)t3;
                
                /* Memory access with volatile index */
                arr[counter % 16] = result;
                f1 = (float)arr[(counter + 1) % 16] * 0.25f;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                int t1 = result & 0xFF;
                float t2 = f2 * 2.0f - f3;
                double t3 = d3 / d1;
                t1 = t1 * 3 + (int)(t2 * 10.0f);
                t2 = t2 + (float)t1 * 0.1f;
                t3 = t3 - (double)t2 * 0.01;
                t1 = (t1 * 7) % 256;
                result ^= t1;
                
                /* Complex array access pattern */
                arr[(t1 + counter) % 16] = result;
                d2 = (double)arr[counter % 16] * 0.33;
                break;
            }
            case 2: {
                /* Integer-only dependency chain */
                int t1 = counter;
                int t2 = result;
                t1 = t1 * 3 + 1;
                t2 = t2 ^ (t1 << 1);
                t1 = t1 * 5 - 3;
                t2 = t2 + (t1 >> 2);
                t1 = t1 * 7 % 1024;
                t2 = t2 | (t1 & 0xFF);
                result = t2;
                
                f3 = (float)result * 0.125f;
                break;
            }
            case 3: {
                /* Floating-point intensive */
                float t1 = f1;
                double t2 = d1;
                t1 = t1 * 1.5f + f2;
                t2 = t2 / 1.3 + d3;
                t1 = t1 - f3 * 0.5f;
                t2 = t2 * 2.0 - 1.0;
                f1 = opaque_float(t1);
                d1 = opaque_double(t2);
                result += (int)(t1 * 100.0f) + (int)(t2 * 50.0);
                break;
            }
            case 4: {
                /* Mixed operations with memory barriers */
                int t1 = arr[counter % 16];
                float t2 = (float)t1 * f1;
                t1 = t1 + (int)(t2 * 2.0f);
                t2 = t2 / f2 + 1.0f;
                
                /* Scheduling barrier */
                volatile int* ptr1 = &t1;
                volatile float* ptr2 = &t2;
                volatile double* ptr3 = &d3;
                scheduling_barrier(ptr1, ptr2, ptr3);
                
                result = t1 + (int)t2;
                break;
            }
            case 5: {
                /* Use builtin for potential backend scheduling requirements */
                #ifdef __x86_64__
                unsigned long long tsc1 = __builtin_ia32_rdtsc();
                result ^= (int)(tsc1 & 0xFFFFFFFF);
                #endif
                
                int t1 = result * 11;
                float t2 = (float)t1 * 0.01f;
                t1 = t1 + (int)(f3 * 100.0f);
                t2 = t2 + f1 - f2;
                result = t1 ^ (int)(t2 * 10.0f);
                break;
            }
            case 6: {
                /* Complex chain with many dependencies */
                int t1 = counter * 3;
                int t2 = result * 5;
                float t3 = f1 + f2;
                double t4 = d1 * d2;
                
                t1 = t1 + t2;
                t2 = t2 ^ t1;
                t3 = t3 * (float)t1;
                t4 = t4 / (double)t2;
                
                t1 = (t1 << 2) | (t2 >> 3);
                t3 = t3 + (float)t1;
                t4 = t4 - (double)t3;
                
                result = t1 + (int)t3 + (int)t4;
                f3 = (float)result * 0.333f;
                break;
            }
            case 7: {
                /* Array processing loop - creates scheduling region */
                int sum = 0;
                for (int i = 0; i < 8; i++) {
                    sum += arr[(counter + i) % 16];
                    arr[(counter + i) % 16] = sum;
                }
                result += sum;
                
                /* Force scheduling boundary */
                asm volatile ("" : : : "memory");
                break;
            }
            case 8: {
                /* Computed goto state machine simulation */
                static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
                int label_idx = counter % 4;
                
                goto *labels[label_idx];
                
                L0:
                    result = result * 2 + 1;
                    f1 = f1 * 1.1f;
                    goto end_label;
                L1:
                    result = result / 2 - 3;
                    f2 = f2 + 0.5f;
                    goto end_label;
                L2:
                    result = result ^ 0xAA;
                    f3 = f3 - 0.25f;
                    goto end_label;
                L3:
                    result = result | 0x55;
                    d1 = d1 * 1.01;
                    goto end_label;
                end_label:
                break;
            }
            case 9: {
                /* Final complex state */
                int t1 = result;
                float t2 = f1 + f2 + f3;
                double t3 = d1 + d2 + d3;
                
                for (int i = 0; i < 5; i++) {
                    t1 = t1 * 3 - i;
                    t2 = t2 * 1.1f - (float)i * 0.1f;
                    t3 = t3 / 1.05 + (double)i * 0.01;
                }
                
                result = t1 + (int)t2 + (int)t3;
                
                /* Memory barrier */
                asm volatile ("mfence\n\t" : : : "memory");
                break;
            }
        }
        
        /* Update volatile variables for next iteration */
        counter = opaque_int(counter + 1);
        f1 = opaque_float(f1 * 1.01f);
        d1 = opaque_double(d1 * 0.99);
        
        /* Periodically call scheduling barrier */
        if (outer % 7 == 0) {
            scheduling_barrier(&result, &f1, &d1);
        }
    }
    
    /* Final computation mixing all values */
    int final_result = result;
    final_result += (int)(f1 * 100.0f);
    final_result += (int)(f2 * 50.0f);
    final_result += (int)(f3 * 25.0f);
    final_result += (int)(d1 * 10.0);
    final_result += (int)(d2 * 5.0);
    final_result += (int)(d3 * 2.0);
    
    /* Array checksum */
    for (int i = 0; i < 16; i++) {
        final_result ^= arr[i];
    }
    
    return final_result;
}

int main(void) {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase coverage chance */
    for (int i = 0; i < 100; i++) {
        int result = scheduling_stress();
        total_checksum ^= result;
        
        /* Vary initial conditions slightly */
        if (i % 3 == 0) {
            /* Force different optimization decisions */
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
