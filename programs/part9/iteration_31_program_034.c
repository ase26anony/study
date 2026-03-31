#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

/* Helper to force memory dependencies */
void memory_barrier() __attribute__((noinline, noipa));

/* State machine helper */
int next_state(int current, int condition) __attribute__((noinline, noipa));

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline, noipa));

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

int next_state(int current, int condition) {
    volatile int a = condition;
    volatile int b = current;
    
    /* Complex state transition with data dependencies */
    int c = a + b;
    int d = c * 3;
    int e = d >> 2;
    int f = e ^ a;
    int g = f * 7;
    int h = g % 13;
    
    asm volatile ("" : : "r"(h) : "memory");
    return h % 8;
}

int scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 777;
    volatile float f1 = seed * 1.5f;
    volatile float f2 = seed * 2.7f;
    volatile double d1 = seed * 3.14159;
    volatile double d2 = seed * 2.71828;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = (seed + i) * 3;
    }
    
    int state = seed % 8;
    int checksum = 0;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < 50; outer++) {
        v1 = opaque_int(v1);
        v2 = opaque_int(v2);
        
        /* Complex switch-based state machine */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                v1 = v1 + v2;
                f1 = f1 + f2;
                v3 = (int)f1 + v1;
                d1 = d1 * 1.1;
                v2 = (int)d1 ^ v3;
                f2 = f2 * 1.3f;
                v1 = v1 * 3;
                d2 = d2 / 1.7;
                v3 = v3 + (int)d2;
                checksum += v1 + v2 + v3;
                
                /* Memory access with volatile index */
                int idx = (v1 + outer) & 15;
                arr[idx] = arr[idx] + v2;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                v2 = v2 - v1;
                f2 = f2 - f1;
                v3 = (int)f2 * v2;
                d2 = d2 + 2.5;
                v1 = (int)d2 | v3;
                f1 = f1 / 1.7f;
                v2 = v2 << 2;
                d1 = d1 - 0.5;
                v3 = v3 ^ (int)d1;
                checksum += v1 * 2 - v2;
                
                int idx = (v2 + outer) & 15;
                arr[idx] = arr[idx] - v1;
                break;
            }
            case 2: {
                /* More complex arithmetic */
                v3 = v3 ^ v1;
                f1 = f1 * f2;
                v2 = (int)(f1 * 2.0f) + v3;
                d1 = d1 / d2;
                v1 = (int)(d1 * 100) & v2;
                f2 = f2 + 1.0f;
                v3 = v3 >> 1;
                d2 = d2 * 1.5;
                v2 = v2 + (int)d2;
                checksum += v3 * 3 + v1;
                
                int idx = (v3 + outer) & 15;
                arr[idx] = arr[idx] * 2;
                break;
            }
            case 3: {
                /* Integer-heavy chain */
                v1 = v1 * v2;
                v2 = v2 + v3;
                v3 = v3 ^ v1;
                v1 = v1 << (v2 & 3);
                v2 = v2 >> (v3 & 3);
                v3 = v3 * 7;
                v1 = v1 + 11;
                v2 = v2 * 13;
                v3 = v3 % 17;
                checksum += v1 + v2 + v3;
                
                int idx = (v1 + outer) & 15;
                arr[idx] = arr[idx] ^ v2;
                break;
            }
            case 4: {
                /* Float/double heavy chain */
                f1 = f1 * 1.1f;
                f2 = f2 + 2.2f;
                d1 = d1 / 1.3;
                d2 = d2 * 1.4;
                f1 = f1 + f2;
                d1 = d1 - d2;
                v1 = (int)(f1 * 10);
                v2 = (int)(d1 * 20);
                checksum += v1 - v2;
                
                int idx = (v1 + v2) & 15;
                arr[idx] = arr[idx] + idx;
                break;
            }
            case 5: {
                /* Mixed operations with barriers */
                v1 = v1 + 1;
                memory_barrier();
                f1 = opaque_float(f1 + 0.5f);
                v2 = v2 * 2;
                memory_barrier();
                d1 = opaque_double(d1 * 1.1);
                v3 = v3 ^ 0x55;
                checksum += v1 + v2 + v3;
                
                int idx = (checksum + outer) & 15;
                arr[idx] = arr[idx] | 1;
                break;
            }
            case 6: {
                /* Complex bit manipulations */
                v1 = (v1 << 3) | (v1 >> 29);
                v2 = (v2 ^ 0xAAAAAAAA) + v1;
                v3 = (v3 * 1103515245 + 12345) & 0x7FFFFFFF;
                v1 = v1 ^ v2 ^ v3;
                v2 = (v2 * 3) % 65537;
                v3 = v3 + (v1 << 1);
                checksum ^= v1 ^ v2 ^ v3;
                
                int idx = v3 & 15;
                arr[idx] = arr[idx] + checksum;
                break;
            }
            case 7: {
                /* All types combined */
                v1 = v1 + (int)f1;
                f2 = f2 + (float)v2;
                d1 = d1 + (double)v3;
                v2 = v2 * (int)(f2 * 2.0f);
                f1 = f1 * (float)(d1 / 2.0);
                d2 = d2 + (double)(v1 % 100);
                v3 = v3 + (int)d2;
                checksum += v1 + v2 + v3 + (int)f1 + (int)d1;
                
                for (int i = 0; i < 4; i++) {
                    int idx = (v1 + i) & 15;
                    arr[idx] = arr[idx] + i;
                }
                break;
            }
        }
        
        /* Inline assembly as scheduling barrier */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            : 
            : "r" (v1), "r" (v2)
            : "eax", "memory"
        );
        
        /* Update state with complex condition */
        int cond = (v1 ^ v2 ^ v3 ^ checksum ^ outer) & 0xFF;
        state = next_state(state, cond);
        
        /* Call to noinline function creates scheduling boundary */
        f1 = opaque_float(f1);
        d1 = opaque_double(d1);
    }
    
    /* Final computation with all variables */
    checksum += v1 + v2 + v3;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    /* Use array values */
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Repeated calls to stress scheduler context saving/restoring */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 10000;
        int result = scheduling_stress(seed);
        total += result;
        
        /* Prevent optimization of loop */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    printf("Final checksum: %d\n", total);
    return total & 0xFF;
}
