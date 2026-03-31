/* haifa-sched-trigger.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer haifa-sched-trigger.c -o haifa-sched-trigger
 * Or with: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noclone));
float opaque_float(float x) __attribute__((noinline, noclone));
double opaque_double(double x) __attribute__((noinline, noclone));
void scheduling_barrier(void) __attribute__((noinline, noclone));
void memory_clobber(void);

/* State machine states */
enum states {
    STATE_ARITH_INT,
    STATE_ARITH_FLOAT,
    STATE_MEM_OPS,
    STATE_MIXED,
    STATE_CONTROL_FLOW,
    STATE_BARRIER,
    NUM_STATES
};

/* Opaque functions implementation */
int opaque_int(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x ^ 0x55AA55AA;
}

float opaque_float(float x) {
    /* Force register spill/reload */
    volatile float y = x;
    asm volatile ("" : "+f" (y));
    return y * 1.0001f;
}

double opaque_double(double x) {
    /* Complex enough to require scheduling */
    volatile double y = x;
    asm volatile ("" : "+f" (y));
    return y / 1.00000001;
}

void scheduling_barrier(void) {
    /* Function call acts as scheduling boundary */
    volatile int dummy = 0;
    asm volatile ("" : "+r" (dummy));
}

void memory_clobber(void) {
    /* Inline asm with memory clobber forces scheduler to save state */
    asm volatile ("" : : : "memory");
}

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline, noclone));

int scheduling_stress(int seed) {
    /* Use volatile variables to prevent optimization */
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed * 3;
    volatile float f1 = seed * 1.5f;
    volatile float f2 = seed * 2.5f;
    volatile double d1 = seed * 1.25;
    volatile double d2 = seed * 3.75;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Volatile index for unpredictable array access */
    volatile int idx = 0;
    
    /* State machine control */
    volatile int state = STATE_ARITH_INT;
    volatile int iterations = 0;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow based on runtime values */
        switch (state) {
            case STATE_ARITH_INT: {
                /* Long chain of dependent integer operations */
                v1 = opaque_int(v1);
                v2 = v1 + opaque_int(v3);
                v3 = v2 * opaque_int(v1) - opaque_int(v2);
                v1 = v3 >> (opaque_int(v2) & 0x7);
                v2 = (v1 ^ v3) | opaque_int(v1);
                v3 = opaque_int(v2) * 3 + opaque_int(v1) / 7;
                v1 = opaque_int(v3) - opaque_int(v2) + opaque_int(v1);
                
                /* Memory access with volatile index */
                idx = (opaque_int(v1) + outer) & 0xF;
                v2 = arr[idx] + opaque_int(v3);
                arr[(idx + 1) & 0xF] = opaque_int(v2);
                
                state = STATE_ARITH_FLOAT;
                break;
            }
            
            case STATE_ARITH_FLOAT: {
                /* Dependent floating-point operations */
                f1 = opaque_float(f1);
                f2 = opaque_float(f2) * opaque_float(f1);
                f1 = opaque_float(f2) / opaque_float(f1 + 1.0f);
                f2 = opaque_float(f1) * 3.14159f - opaque_float(f2);
                f1 = opaque_float(f2) + opaque_float(f1) * 2.71828f;
                
                /* Mix with integer operations */
                v1 = (int)(f1 * 1000.0f) ^ opaque_int(v3);
                v2 = opaque_int(v1) + (int)(f2 * 100.0f);
                
                state = STATE_MEM_OPS;
                break;
            }
            
            case STATE_MEM_OPS: {
                /* Memory-intensive operations */
                for (int i = 0; i < 8; i++) {
                    volatile int j = (opaque_int(v1) + i) & 0xF;
                    arr[j] = opaque_int(arr[j]) + opaque_int(v2);
                    v3 = opaque_int(arr[(j + 1) & 0xF]) ^ opaque_int(v3);
                }
                
                /* Double precision operations */
                d1 = opaque_double(d1);
                d2 = opaque_double(d2) * opaque_double(d1);
                d1 = opaque_double(d2) / (opaque_double(d1) + 1.0);
                
                state = STATE_MIXED;
                break;
            }
            
            case STATE_MIXED: {
                /* Mixed integer/float/double operations with dependencies */
                v1 = opaque_int(v1) * 3 + (int)(opaque_float(f1) * 10.0f);
                f2 = opaque_float(f2) + (float)(opaque_int(v2) / 100.0);
                d1 = opaque_double(d1) + (double)(opaque_int(v3));
                v2 = opaque_int(v2) ^ (int)(opaque_double(d2) * 1000.0);
                f1 = opaque_float(f1) * (float)(opaque_double(d1));
                
                /* Complex chain */
                v3 = opaque_int(v1) + opaque_int(v2) - (int)(opaque_float(f2));
                d2 = opaque_double(d2) / (double)(opaque_int(v3) + 1);
                f1 = opaque_float(f1) + (float)(opaque_double(d2));
                v1 = opaque_int(v3) * (int)(opaque_float(f1));
                
                state = STATE_CONTROL_FLOW;
                break;
            }
            
            case STATE_CONTROL_FLOW: {
                /* Unpredictable control flow */
                int r = opaque_int(v1 + v2 + v3 + (int)f1 + (int)d1);
                
                if (r & 0x1) {
                    v1 = opaque_int(v1) * 2;
                } else {
                    v1 = opaque_int(v1) / 3;
                }
                
                if (r & 0x2) {
                    f2 = opaque_float(f2) + 1.0f;
                } else {
                    f2 = opaque_float(f2) - 1.0f;
                }
                
                for (int i = 0; i < 3; i++) {
                    if ((r >> i) & 0x1) {
                        v2 = opaque_int(v2) + opaque_int(i);
                    } else {
                        v2 = opaque_int(v2) - opaque_int(i);
                    }
                }
                
                state = STATE_BARRIER;
                break;
            }
            
            case STATE_BARRIER: {
                /* Force scheduling boundary */
                scheduling_barrier();
                
                /* Use inline asm with memory clobber */
                memory_clobber();
                
                /* Update state based on complex condition */
                int cond = opaque_int(v1) ^ opaque_int(v2) ^ opaque_int(v3) ^ 
                          (int)(opaque_float(f1) * 100.0f) ^ 
                          (int)(opaque_double(d1) * 1000.0);
                
                state = (cond + outer) % NUM_STATES;
                break;
            }
        }
        
        iterations++;
        
        /* Occasionally call opaque functions to create scheduling boundaries */
        if ((outer % 7) == 0) {
            v1 = opaque_int(v1);
            f1 = opaque_float(f1);
            d1 = opaque_double(d1);
        }
        
        /* Use x86-specific builtin if available */
        #ifdef __x86_64__
        if ((outer % 13) == 0) {
            unsigned long long tsc;
            asm volatile ("rdtsc" : "=A" (tsc));
            v3 ^= (int)(tsc & 0xFFFFFFFF);
        }
        #endif
    }
    
    /* Compute checksum from all variables */
    int checksum = v1 ^ v2 ^ v3 ^ (int)f1 ^ (int)f2 ^ (int)d1 ^ (int)d2;
    
    /* Include array elements in checksum */
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i] + i;
    }
    
    return checksum;
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Repeatedly call scheduling_stress to trigger scheduler context operations */
    for (int i = 0; i < 100; i++) {
        int seed = rand() ^ i;
        int result = scheduling_stress(seed);
        total_checksum ^= result;
        
        /* Print progress occasionally */
        if ((i % 20) == 0) {
            printf("Iteration %d, checksum so far: %08x\n", i, total_checksum);
        }
    }
    
    printf("Final checksum: %08x\n", total_checksum);
    return 0;
}
