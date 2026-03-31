#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));

/* Memory barrier via inline assembly */
#define MEMORY_BARRIER() __asm__ volatile("" ::: "memory")

/* State machine states */
enum states {
    STATE_A, STATE_B, STATE_C, STATE_D, STATE_E,
    STATE_F, STATE_G, STATE_H, STATE_I, STATE_J,
    STATE_K, STATE_L, STATE_M, STATE_N, STATE_O
};

int opaque_int(int x) {
    /* Complex enough to not be inlined */
    volatile int v = x;
    for (int i = 0; i < 3; i++) {
        v = (v * 1103515245 + 12345) & 0x7fffffff;
    }
    return v;
}

float opaque_float(float x) {
    /* Force floating point operations */
    volatile float v = x;
    v = v * 1.5f;
    v = v / 2.0f;
    v = v + 1.0f;
    return v;
}

double opaque_double(double x) {
    /* Force double precision operations */
    volatile double v = x;
    v = v * 1.7;
    v = v / 2.3;
    v = v + 1.1;
    return v;
}

void scheduling_barrier(void) {
    /* Function call that acts as scheduling boundary */
    MEMORY_BARRIER();
}

/* Main scheduling stress function */
unsigned long long scheduling_stress(int seed) __attribute__((noinline));
unsigned long long scheduling_stress(int seed) {
    /* Volatile variables to prevent optimization */
    volatile int vi1 = seed;
    volatile int vi2 = seed + 1;
    volatile int vi3 = seed + 2;
    volatile int vi4 = seed + 3;
    volatile float vf1 = seed * 1.0f;
    volatile float vf2 = seed * 2.0f;
    volatile double vd1 = seed * 1.0;
    volatile double vd2 = seed * 2.0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * seed;
    }
    
    /* State machine implementation with switch */
    volatile int state = STATE_A;
    volatile int counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine with many cases */
        switch (state) {
            case STATE_A: {
                /* Chain of dependent integer operations */
                vi1 = vi2 + vi3;
                vi4 = vi1 * vi2;
                vi2 = vi4 >> 3;
                vi3 = vi2 | vi1;
                vi1 = vi3 ^ vi4;
                vi4 = vi1 - vi2;
                vi2 = vi4 * 7;
                vi3 = vi2 / 3;
                
                /* Mix with floating point */
                vf1 = (float)vi1 + vf2;
                vf2 = vf1 * 1.5f;
                
                /* Call to create scheduling boundary */
                vi1 = opaque_int(vi1);
                MEMORY_BARRIER();
                
                /* Update state based on complex condition */
                state = (vi1 & 1) ? STATE_B : STATE_C;
                break;
            }
            
            case STATE_B: {
                /* Different chain of operations */
                vi2 = vi3 * vi4;
                vi1 = vi2 + vi3;
                vi4 = vi1 - vi2;
                vi3 = vi4 << 2;
                vi2 = vi3 | vi1;
                vi1 = vi2 ^ vi4;
                vi4 = vi1 * 13;
                vi3 = vi4 % 17;
                
                /* Floating point chain */
                vf2 = vf1 * 2.0f;
                vf1 = vf2 / 1.7f;
                vf2 = opaque_float(vf1);
                
                /* Memory operations with volatile array */
                volatile int idx = vi1 & 0xF;
                vi2 = arr[idx];
                arr[(idx + 1) & 0xF] = vi3;
                
                MEMORY_BARRIER();
                state = (vi2 > 100) ? STATE_D : STATE_E;
                break;
            }
            
            case STATE_C: {
                /* Double precision operations */
                vd1 = (double)vi1 + vd2;
                vd2 = vd1 * 1.7;
                vd1 = opaque_double(vd2);
                
                /* Integer operations mixed with double */
                vi3 = (int)vd1 + vi2;
                vi4 = vi3 * vi1;
                vi2 = vi4 >> 1;
                vi1 = vi2 + vi3;
                
                /* Array access pattern */
                for (int i = 0; i < 4; i++) {
                    volatile int idx = (vi1 + i) & 0xF;
                    arr[idx] = arr[idx] + vi2;
                }
                
                scheduling_barrier();
                state = STATE_F;
                break;
            }
            
            case STATE_D: {
                /* More complex mixed operations */
                vi1 = vi2 * vi3 + vi4;
                vi2 = vi1 / (vi3 + 1);
                vf1 = (float)vi1 * vf2;
                vf2 = opaque_float(vf1);
                vi3 = (int)vf2 + vi4;
                vi4 = vi3 << (vi1 & 3);
                
                MEMORY_BARRIER();
                state = STATE_G;
                break;
            }
            
            case STATE_E: {
                /* Chain with modulo operations */
                vi1 = (vi2 * 1103515245 + 12345) & 0x7fffffff;
                vi2 = vi1 % 65537;
                vi3 = vi2 * vi4;
                vi4 = vi3 ^ vi1;
                
                /* Floating point chain */
                vf1 = vf2 + 1.0f;
                vf2 = vf1 * 3.14f;
                vf1 = opaque_float(vf2);
                
                scheduling_barrier();
                state = STATE_H;
                break;
            }
            
            /* Additional states to create more basic blocks */
            case STATE_F: {
                vi1 = vi1 + vi2 + vi3 + vi4;
                state = STATE_I;
                break;
            }
            
            case STATE_G: {
                vi2 = vi2 * vi3 - vi4;
                state = STATE_J;
                break;
            }
            
            case STATE_H: {
                vi3 = vi3 | vi1 & vi2;
                state = STATE_K;
                break;
            }
            
            case STATE_I: {
                vi4 = vi4 ^ vi1;
                state = STATE_L;
                break;
            }
            
            case STATE_J: {
                vf1 = vf1 + vf2;
                state = STATE_M;
                break;
            }
            
            case STATE_K: {
                vf2 = vf2 * 2.0f;
                state = STATE_N;
                break;
            }
            
            case STATE_L: {
                vd1 = vd1 + vd2;
                state = STATE_O;
                break;
            }
            
            case STATE_M: {
                vd2 = vd2 * 1.5;
                state = STATE_A;
                break;
            }
            
            case STATE_N: {
                vi1 = vi1 + 1;
                state = STATE_B;
                break;
            }
            
            case STATE_O: {
                vi2 = vi2 - 1;
                state = STATE_C;
                break;
            }
        }
        
        /* Inner loop with array accesses and volatile indices */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx1 = (vi1 + inner) & 0xF;
            volatile int idx2 = (vi2 + inner * 2) & 0xF;
            
            /* Create memory dependencies */
            arr[idx1] = arr[idx1] + vi3;
            arr[idx2] = arr[idx2] + vi4;
            
            /* Dependent operations */
            vi3 = vi3 + arr[idx1];
            vi4 = vi4 ^ arr[idx2];
        }
        
        /* Periodic call to opaque function */
        if (outer % 7 == 0) {
            vi1 = opaque_int(vi1 + vi2);
        }
        
        counter++;
    }
    
    /* Compute checksum from all volatile variables */
    unsigned long long checksum = 0;
    checksum += (unsigned long long)vi1;
    checksum += (unsigned long long)vi2;
    checksum += (unsigned long long)vi3;
    checksum += (unsigned long long)vi4;
    checksum += (unsigned long long)(vf1 * 1000);
    checksum += (unsigned long long)(vf2 * 1000);
    checksum += (unsigned long long)(vd1 * 1000);
    checksum += (unsigned long long)(vd2 * 1000);
    
    /* Include array in checksum */
    for (int i = 0; i < 16; i++) {
        checksum += (unsigned long long)arr[i];
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    unsigned long long total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase chance of
       hitting the scheduler context save/restore logic */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum += scheduling_stress(seed);
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    return 0;
}
