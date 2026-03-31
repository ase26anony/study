/* sched_context_test.c - Program to trigger scheduler context saving/freeing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int __attribute__((noinline)) opaque_int(int x) { 
    asm volatile("" : "+r"(x) : : "memory");
    return x; 
}

float __attribute__((noinline)) opaque_float(float x) { 
    asm volatile("" : "+t"(x) : : "memory");
    return x; 
}

double __attribute__((noinline)) opaque_double(double x) { 
    asm volatile("" : "+t"(x) : : "memory");
    return x; 
}

void __attribute__((noinline)) scheduling_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Complex helper with mixed operations to force backend scheduling data */
int __attribute__((noinline)) complex_helper(volatile int a, volatile float b, 
                                            volatile double c, volatile int d) {
    int r1 = a + (int)b;
    double r2 = c * (double)d;
    float r3 = b * (float)r2;
    int r4 = r1 ^ (int)r3;
    
    /* Use x86-specific builtin if available */
    #ifdef __x86_64__
    unsigned long long tsc = __builtin_ia32_rdtsc();
    r4 ^= (int)(tsc & 0xFFFFFFFF);
    #endif
    
    asm volatile("" : "+r"(r4) : : "memory");
    return r4;
}

/* State machine states */
enum states {
    STATE_ARITH,
    STATE_FLOAT,
    STATE_MEM,
    STATE_MIXED,
    STATE_BARRIER,
    STATE_COUNT
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed ^ 0x12345678;
    volatile int v2 = seed * 1103515245 + 12345;
    volatile float f1 = (float)seed * 1.2345f;
    volatile float f2 = (float)(seed ^ 0x87654321) * 0.9876f;
    volatile double d1 = (double)seed * 3.1415926535;
    volatile double d2 = (double)(seed % 1000) * 2.7182818284;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = (seed + i) * 7;
    }
    
    volatile int state = STATE_ARITH;
    volatile int counter = 0;
    volatile int checksum = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch - creates many basic blocks */
        switch (state) {
            case STATE_ARITH: {
                /* Long chain of integer dependencies */
                int t1 = v1 + v2;
                int t2 = t1 * (v1 ^ v2);
                int t3 = t2 >> (v1 & 0xF);
                int t4 = t3 - (v2 % 37);
                int t5 = (t4 ^ t3) & (t2 | t1);
                int t6 = t5 * 0x9E3779B9;
                int t7 = (t6 << 4) | (t6 >> 28);
                int t8 = t7 + opaque_int(t5);
                v1 = t8;
                v2 = opaque_int(t3 + t4);
                checksum ^= t8;
                
                /* Memory dependency chain */
                volatile int idx = (t8 & 0xF);
                arr[idx] = arr[(idx + 1) & 0xF] + t8;
                arr[(idx + 2) & 0xF] = arr[idx] * 3;
                
                state = STATE_FLOAT;
                break;
            }
            
            case STATE_FLOAT: {
                /* Mixed float/int dependency chain */
                float ft1 = f1 * f2;
                float ft2 = ft1 + (float)v1;
                double dt1 = d1 * d2;
                double dt2 = dt1 / (double)(v2 + 1);
                int it1 = (int)ft2 * (int)dt2;
                float ft3 = opaque_float(ft2 * 2.0f);
                double dt3 = opaque_double(dt2 * 1.5);
                int it2 = it1 ^ (int)(ft3 * 100.0f);
                int it3 = it2 + (int)(dt3 * 10.0);
                
                f1 = ft3;
                f2 = (float)(it3 & 0xFF) / 256.0f;
                d1 = dt3;
                d2 = (double)(it3 % 100) / 100.0;
                checksum += it3;
                
                /* Call to force scheduling boundary */
                int helper_result = complex_helper(v1, f1, d1, it3);
                v2 ^= helper_result;
                
                state = STATE_MEM;
                break;
            }
            
            case STATE_MEM: {
                /* Memory-intensive operations with volatile accesses */
                for (int i = 0; i < 8; i++) {
                    volatile int idx1 = (v1 + i) & 0xF;
                    volatile int idx2 = (v2 + i * 3) & 0xF;
                    volatile int idx3 = (checksum + i * 5) & 0xF;
                    
                    arr[idx1] = arr[idx2] + arr[idx3];
                    arr[idx2] = arr[idx1] * (i + 1);
                    arr[idx3] = arr[idx1] ^ arr[idx2];
                    
                    /* Inline assembly barrier every few iterations */
                    if (i & 1) {
                        asm volatile("" : : : "memory");
                    }
                }
                
                /* Compute checksum from array */
                int arr_sum = 0;
                for (int i = 0; i < 16; i++) {
                    arr_sum ^= arr[i];
                }
                checksum += arr_sum;
                
                state = STATE_MIXED;
                break;
            }
            
            case STATE_MIXED: {
                /* Complex mixed operations */
                int mix1 = v1 * 0x5A827999;
                float mix2 = f1 * 3.14159f;
                double mix3 = d1 * 2.71828;
                
                for (int i = 0; i < 4; i++) {
                    mix1 = (mix1 << 3) | (mix1 >> 29);
                    mix2 = mix2 * 1.5f + (float)i;
                    mix3 = mix3 / (1.0 + i * 0.1);
                    
                    int temp = (int)mix2 ^ (int)mix3;
                    mix1 = mix1 ^ (temp * (i + 1));
                    
                    /* Partial unrolling creates multiple instructions */
                    arr[i * 4] = mix1;
                    arr[i * 4 + 1] = (int)mix2;
                    arr[i * 4 + 2] = (int)mix3;
                    arr[i * 4 + 3] = temp;
                }
                
                v1 = mix1;
                f1 = mix2;
                d1 = mix3;
                
                state = STATE_BARRIER;
                break;
            }
            
            case STATE_BARRIER: {
                /* Force scheduling barrier with inline asm */
                int barrier_val = v1 + v2 + (int)f1 + (int)f2 + (int)d1 + (int)d2;
                
                asm volatile(
                    "movl %0, %%eax\n\t"
                    "rorl $7, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "+r"(barrier_val)
                    :
                    : "%eax", "memory"
                );
                
                checksum ^= barrier_val;
                
                /* Update state based on complex condition */
                state = (barrier_val & 0x3) + 
                       ((v1 & 0x1) << 2) + 
                       ((counter & 0x1) << 3);
                state = state % STATE_COUNT;
                
                counter++;
                break;
            }
            
            default:
                state = STATE_ARITH;
                break;
        }
        
        /* Additional scheduling boundary every 10 iterations */
        if ((outer % 10) == 9) {
            scheduling_barrier();
        }
    }
    
    /* Final computation to use all variables */
    int final_result = checksum ^ v1 ^ v2 ^ (int)f1 ^ (int)f2 ^ (int)d1 ^ (int)d2;
    for (int i = 0; i < 16; i++) {
        final_result += arr[i];
    }
    
    return final_result;
}

int main(void) {
    srand(time(NULL));
    int total_result = 0;
    
    /* Multiple calls to stress scheduler context management */
    for (int i = 0; i < 100; i++) {
        int seed = rand() ^ (i * 0x9E3779B9);
        int result = scheduling_stress(seed);
        total_result ^= result;
        
        /* Print progress occasionally */
        if ((i % 25) == 0) {
            printf("Iteration %d, result: %d\n", i, result);
        }
    }
    
    printf("Final checksum: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
