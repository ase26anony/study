/* haifa-sched-trigger.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer haifa-sched-trigger.c -o haifa-sched-trigger
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int __attribute__((noinline)) opaque_int(int x) {
    volatile int dummy = x;
    return dummy;
}

float __attribute__((noinline)) opaque_float(float x) {
    volatile float dummy = x;
    return dummy;
}

double __attribute__((noinline)) opaque_double(double x) {
    volatile double dummy = x;
    return dummy;
}

/* Barrier function with memory clobber */
void __attribute__((noinline)) scheduling_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Complex dependency chain with mixed types */
int __attribute__((noinline)) complex_dependency_chain(int seed) {
    volatile int a = seed;
    volatile int b = a * 3;
    volatile int c = b + 7;
    volatile float d = c * 1.5f;
    volatile float e = d / 2.0f;
    volatile double f = e * 3.14159;
    volatile double g = f + 2.71828;
    volatile int h = (int)g;
    
    /* Create a longer chain */
    for (int i = 0; i < 3; i++) {
        h = opaque_int(h * 2 - i);
        d = opaque_float(d + 0.5f);
        f = opaque_double(f * 0.9);
    }
    
    return h;
}

/* State machine with complex control flow */
int __attribute__((noinline)) state_machine_operations(int state, int input) {
    volatile int result = input;
    
    switch (state) {
        case 0: {
            /* Long dependency chain */
            volatile int a = input;
            volatile int b = a * 2 + 1;
            volatile int c = b ^ 0x55AA55AA;
            volatile int d = c >> 3;
            volatile int e = d * 7;
            volatile int f = e - 13;
            volatile int g = f & 0x00FF00FF;
            result = g;
            
            /* Memory operations */
            volatile int arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = result + i;
            }
            
            /* Use array values */
            for (int i = 0; i < 8; i += 2) {
                result += arr[i] - arr[i+1];
            }
            break;
        }
        
        case 1: {
            /* Floating point intensive */
            volatile float x = (float)input;
            volatile float y = x * 1.234f;
            volatile float z = y / 0.789f;
            volatile double d1 = (double)z * 3.14159;
            volatile double d2 = d1 + 2.71828;
            result = (int)(d2 * 100.0);
            
            /* Mix with integer ops */
            for (int i = 0; i < 5; i++) {
                result = result * 3 - i;
                x = x + 0.1f;
            }
            break;
        }
        
        case 2: {
            /* Bit manipulation chain */
            volatile uint32_t u = (uint32_t)input;
            u = (u << 13) | (u >> 19);  /* Rotate left */
            u ^= 0xDEADBEEF;
            u += u * 3;
            u = ~u;
            result = (int)u;
            
            /* Partial unrolling */
            result = result + (result >> 1);
            result = result + (result >> 2);
            result = result + (result >> 4);
            result = result + (result >> 8);
            result = result + (result >> 16);
            break;
        }
        
        case 3: {
            /* Mixed operations with memory barriers */
            volatile int tmp = input;
            
            /* Chain 1 */
            tmp = tmp * 3 + 7;
            scheduling_barrier();
            
            /* Chain 2 */
            volatile float f1 = (float)tmp;
            f1 = f1 * 0.5f + 1.0f;
            scheduling_barrier();
            
            /* Chain 3 */
            tmp = (int)f1;
            tmp = (tmp << 3) | (tmp >> 29);
            scheduling_barrier();
            
            result = tmp;
            break;
        }
        
        case 4: {
            /* Complex loop with data-dependent exit */
            volatile int counter = input & 0xF;
            volatile int acc = 1;
            
            while (counter > 0) {
                acc = acc * 2 + (counter & 1);
                counter = opaque_int(counter - 1);
                
                /* Memory access pattern */
                volatile int local_arr[4];
                for (int i = 0; i < 4; i++) {
                    local_arr[i] = acc + i;
                }
                
                /* Use the array */
                for (int i = 0; i < 4; i++) {
                    acc += local_arr[i];
                }
            }
            result = acc;
            break;
        }
        
        default: {
            /* Default case with computed goto simulation */
            static const void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
            int idx = state % 4;
            
            goto *labels[idx];
            
            L0:
                result = input * 11;
                goto end;
            L1:
                result = input + 0x1234;
                goto end;
            L2:
                result = input ^ 0xABCD;
                goto end;
            L3:
                result = input - 999;
                goto end;
            end:
            break;
        }
    }
    
    scheduling_barrier();
    return result;
}

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int state = seed % 6;
    volatile int accumulator = 0;
    volatile int counter = 0;
    
    /* Outer loop - creates scheduling region boundaries */
    for (int outer = 0; outer < 50; outer++) {
        /* Update state based on complex condition */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        state = state % 6;
        
        /* Perform state-dependent operations */
        int state_result = state_machine_operations(state, accumulator + outer);
        
        /* Complex dependency chain */
        accumulator += complex_dependency_chain(state_result);
        
        /* Inner loop with memory dependencies */
        volatile int inner_arr[16];
        volatile int indices[16];
        
        /* Initialize with volatile values */
        for (int i = 0; i < 16; i++) {
            indices[i] = (i * 7 + outer) & 0xF;
        }
        
        /* Memory access pattern that's hard to schedule */
        for (int i = 0; i < 16; i++) {
            int idx = indices[i];
            inner_arr[idx] = accumulator + i + idx;
            
            /* Dependent operation */
            accumulator += opaque_int(inner_arr[idx] * 3 - i);
        }
        
        /* Occasionally call opaque functions to force scheduling boundaries */
        if ((outer & 3) == 0) {
            accumulator = opaque_int(accumulator);
        }
        
        counter++;
    }
    
    /* Final mixing */
    volatile int final_result = accumulator;
    
    /* Use rdtsc on x86 to potentially trigger backend-specific scheduling */
    #ifdef __x86_64__
    {
        unsigned long long tsc1, tsc2;
        asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
        final_result ^= (int)(tsc1 ^ tsc2);
    }
    #endif
    
    /* One more scheduling barrier */
    scheduling_barrier();
    
    return final_result;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduling stress test...\n");
    
    /* Run multiple iterations to increase chance of triggering the code */
    for (int iteration = 0; iteration < 100; iteration++) {
        int seed = rand();
        int result = scheduling_stress(seed);
        
        total_checksum += result;
        
        /* Print progress occasionally */
        if ((iteration % 20) == 0) {
            printf("Iteration %d: result = %d, total = %d\n", 
                   iteration, result, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
