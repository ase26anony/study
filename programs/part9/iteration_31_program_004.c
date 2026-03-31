#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void memory_barrier() __attribute__((noinline, noipa));
void complex_helper(int a, float b, double c, int* d, float* e) __attribute__((noinline, noipa));

/* Implementation of opaque functions */
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

void memory_barrier() {
    asm volatile ("" : : : "memory");
}

void complex_helper(int a, float b, double c, int* d, float* e) {
    volatile int v1 = a;
    volatile float v2 = b;
    volatile double v3 = c;
    
    /* Create data dependencies */
    v1 = v1 * 3 + 7;
    v2 = v2 * 1.5f + 2.0f;
    v3 = v3 / 2.0 + 1.0;
    
    *d = v1 + (int)v2;
    *e = (float)v3 + v2;
    
    /* Scheduling barrier */
    asm volatile ("" : : : "memory");
}

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));
int scheduling_stress(int seed) {
    volatile int state = seed % 8;
    volatile int counter = 0;
    volatile int result = 0;
    volatile float f_result = 0.0f;
    volatile double d_result = 0.0;
    
    /* Local arrays with volatile access patterns */
    volatile int arr_int[16];
    volatile float arr_float[16];
    volatile double arr_double[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_int[i] = (i * seed) % 97;
        arr_float[i] = (float)((i * seed) % 97) / 3.0f;
        arr_double[i] = (double)((i * seed) % 97) / 5.0;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch statement */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                int t1 = arr_int[outer & 0xF];
                float t2 = arr_float[(outer + 1) & 0xF];
                double t3 = arr_double[(outer + 2) & 0xF];
                
                t1 = t1 * 3 + 7;
                t2 = t2 * 1.5f + t1;
                t3 = t3 / 2.0 + (double)t2;
                t1 = t1 ^ (int)t3;
                t2 = t2 + (float)(t1 % 17);
                t3 = t3 * 1.1 - (double)((int)t2 % 13);
                
                result += t1;
                f_result += t2;
                d_result += t3;
                
                /* Call helper to force scheduling boundary */
                int temp_int;
                float temp_float;
                complex_helper(t1, t2, t3, &temp_int, &temp_float);
                
                state = (temp_int + outer) % 8;
                break;
            }
            
            case 1: {
                /* Different dependency pattern */
                volatile int v1 = result;
                volatile float v2 = f_result;
                volatile double v3 = d_result;
                
                for (int i = 0; i < 5; i++) {
                    v1 = v1 * 2 - i;
                    v2 = v2 + (float)v1 / (i + 2.0f);
                    v3 = v3 - (double)v2 * 0.5;
                }
                
                result = opaque_int(v1);
                f_result = opaque_float(v2);
                d_result = opaque_double(v3);
                
                /* Memory barrier as scheduling boundary */
                asm volatile ("" : : : "memory");
                
                state = (result + outer * 3) % 8;
                break;
            }
            
            case 2: {
                /* Array-based computations */
                int idx = (outer * 7) % 16;
                int t = arr_int[idx];
                float f = arr_float[(idx + 3) % 16];
                double d = arr_double[(idx + 7) % 16];
                
                /* Chain of dependent operations */
                t = (t << 3) | (t >> 5);
                f = f * 0.75f + (float)t;
                d = d + (double)f * 2.0;
                t = t ^ (int)d;
                f = f - (float)(t % 19);
                d = d * 0.9 + (double)((int)f % 11);
                
                arr_int[idx] = t;
                arr_float[(idx + 3) % 16] = f;
                arr_double[(idx + 7) % 16] = d;
                
                state = (t + idx) % 8;
                break;
            }
            
            /* Additional cases to create more basic blocks */
            case 3: {
                volatile int a = result * 3;
                volatile int b = a + 17;
                for (int i = 0; i < 4; i++) {
                    a = a ^ b;
                    b = b + i;
                }
                result = a;
                state = (b + outer) % 8;
                break;
            }
            
            case 4: {
                volatile float a = f_result;
                volatile float b = a * 0.5f;
                for (int i = 0; i < 3; i++) {
                    a = a + b;
                    b = b * 1.1f;
                }
                f_result = a;
                state = ((int)a + outer * 5) % 8;
                break;
            }
            
            case 5: {
                volatile double a = d_result;
                volatile double b = a / 3.0;
                a = a * b + 2.0;
                b = b - a * 0.25;
                a = a + b * 1.5;
                d_result = a;
                state = ((int)b + outer * 7) % 8;
                break;
            }
            
            case 6: {
                /* Mixed operations */
                int t = result + (int)f_result;
                float f = f_result + (float)d_result;
                double d = d_result + (double)result;
                
                t = t * 3 - 11;
                f = f * 1.3f - 2.0f;
                d = d * 0.7 + 1.5;
                
                result = t;
                f_result = f;
                d_result = d;
                
                state = (t + (int)f + outer * 11) % 8;
                break;
            }
            
            case 7: {
                /* Complex loop with dependencies */
                int sum = 0;
                float fsum = 0.0f;
                for (int i = 0; i < 8; i++) {
                    sum += arr_int[i] * i;
                    fsum += arr_float[i + 8] * (float)i;
                    /* Dependency between iterations */
                    arr_int[i] = sum % 256;
                    arr_float[i + 8] = fsum / (i + 1.0f);
                }
                result += sum;
                f_result += fsum;
                state = (sum + outer * 13) % 8;
                break;
            }
        }
        
        counter++;
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx = (inner + outer) & 0xF;
            volatile int val = arr_int[idx];
            
            /* Chain of operations */
            val = val * 3 + inner;
            val = val ^ (val >> 3);
            val = val + arr_int[(idx + 1) & 0xF];
            val = val * 2 - outer;
            
            arr_int[idx] = val;
            
            /* Access other arrays */
            arr_float[idx] = arr_float[idx] * 1.01f + (float)val;
            arr_double[idx] = arr_double[idx] + (double)val * 0.01;
        }
        
        /* Periodic call to create scheduling boundary */
        if ((outer % 7) == 0) {
            int temp_int;
            float temp_float;
            complex_helper(result, f_result, d_result, &temp_int, &temp_float);
            result ^= temp_int;
            f_result += temp_float;
        }
    }
    
    /* Final computation mixing all results */
    int final_result = result + (int)f_result + (int)d_result;
    for (int i = 0; i < 16; i++) {
        final_result ^= arr_int[i];
        final_result += (int)arr_float[i];
    }
    
    return opaque_int(final_result);
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Multiple calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total += result;
        
        /* Prevent optimization of loop */
        asm volatile ("" : : "r" (result) : "memory");
    }
    
    printf("Final checksum: %d\n", total);
    return total & 0xFF;
}
