/* sched_context_test.c
 * Program designed to trigger scheduler context saving/freeing logic
 * in haifa-sched.cc lines 4681-4691
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noclone));
float opaque_float(float x) __attribute__((noinline, noclone));
double opaque_double(double x) __attribute__((noinline, noclone));
void scheduling_barrier(void) __attribute__((noinline, noclone));
void complex_state_machine(int iterations) __attribute__((noinline, noclone));

/* Global volatile variables to create memory dependencies */
volatile int g_counter = 0;
volatile float g_accumulator = 0.0f;
volatile double g_double_accum = 0.0;

/* Opaque function implementations */
int opaque_int(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x ^ 0x55AA55AA;
}

float opaque_float(float x) {
    /* Create a floating-point dependency chain */
    volatile float y = x * 1.5f;
    asm volatile ("" : "+t" (x));
    return y + x * 0.5f;
}

double opaque_double(double x) {
    /* Complex FP operation to challenge scheduler */
    volatile double y = x * 2.718281828459045;
    asm volatile ("" : "+f" (x));
    return y - x * 1.414213562373095;
}

void scheduling_barrier(void) {
    /* Memory clobber acts as scheduling barrier */
    asm volatile ("" : : : "memory");
}

/* State machine states */
enum states {
    STATE_ARITHMETIC,
    STATE_FLOAT_OPS,
    STATE_MEMORY_ACCESS,
    STATE_MIXED_TYPES,
    STATE_CONTROL_FLOW,
    STATE_BARRIER,
    STATE_RESET,
    NUM_STATES
};

/* State machine implementation with complex scheduling requirements */
void complex_state_machine(int iterations) {
    volatile int state = STATE_ARITHMETIC;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    volatile int array[32];
    volatile int idx = 0;
    
    /* Initialize array with volatile indices */
    for (int i = 0; i < 32; i++) {
        array[i] = i * 3;
    }
    
    /* Main state machine loop */
    for (int iter = 0; iter < iterations; iter++) {
        /* Switch creates multiple basic blocks for scheduler */
        switch (state) {
            case STATE_ARITHMETIC: {
                /* Long chain of dependent integer operations */
                i1 = i2 + i3;
                i2 = i1 * i4;
                i3 = i2 >> i5;
                i4 = i3 ^ i1;
                i5 = i4 & 0xFF;
                i1 = i5 + i2;
                i2 = i1 - i3;
                i3 = i2 * i4;
                i4 = i3 / (i5 | 1);
                i5 = i4 % 17;
                
                /* Call opaque function to create scheduling boundary */
                i1 = opaque_int(i1);
                i2 = opaque_int(i2);
                
                state = STATE_FLOAT_OPS;
                break;
            }
            
            case STATE_FLOAT_OPS: {
                /* Mixed floating-point operations with dependencies */
                f1 = f2 * f3;
                f2 = f1 + f3;
                f3 = f2 - f1;
                f1 = f3 * 1.618034f;
                f2 = f1 / 2.0f;
                f3 = opaque_float(f2);
                
                /* Double precision operations */
                d1 = d2 * d3;
                d2 = d1 + 1.0;
                d3 = opaque_double(d2);
                d1 = d3 - d2;
                
                state = STATE_MEMORY_ACCESS;
                break;
            }
            
            case STATE_MEMORY_ACCESS: {
                /* Memory access pattern with volatile indices */
                volatile int temp_sum = 0;
                for (int j = 0; j < 8; j++) {
                    idx = (idx + 1) & 31;
                    temp_sum += array[idx];
                    array[idx] = temp_sum;
                }
                
                /* Create memory dependencies */
                i1 = array[0] + array[1];
                i2 = array[2] * array[3];
                array[4] = i1 ^ i2;
                
                state = STATE_MIXED_TYPES;
                break;
            }
            
            case STATE_MIXED_TYPES: {
                /* Mixed type operations challenging scheduler */
                f1 = (float)i1 * 0.5f;
                i2 = (int)(f2 * 2.0f);
                d1 = (double)i3 + f3;
                i4 = (int)(d2 * 3.0);
                f2 = (float)(d3 - 1.0);
                
                /* Complex expression with multiple dependencies */
                i5 = ((i1 + i2) * (i3 - i4)) / ((i5 | 1) + 1);
                f3 = (f1 * f2) / (f3 + 1.0f);
                d3 = (d1 + d2) * (d3 - 0.5);
                
                state = STATE_CONTROL_FLOW;
                break;
            }
            
            case STATE_CONTROL_FLOW: {
                /* Unpredictable control flow */
                if ((counter & 3) == 0) {
                    i1 = i2 + i3;
                    f1 = f2 * 3.14f;
                } else if ((counter & 3) == 1) {
                    i1 = i3 - i2;
                    f1 = f3 / 2.0f;
                } else if ((counter & 3) == 2) {
                    i1 = i2 * i3;
                    f1 = f2 + f3;
                } else {
                    i1 = i3 ^ i2;
                    f1 = f3 - f2;
                }
                
                /* Nested conditional */
                if (i1 > 1000) {
                    d1 = d2 * 1.5;
                } else if (i1 < -1000) {
                    d1 = d3 / 1.5;
                } else {
                    d1 = (d2 + d3) * 0.5;
                }
                
                state = STATE_BARRIER;
                break;
            }
            
            case STATE_BARRIER: {
                /* Scheduling barrier with inline asm */
                asm volatile (
                    "movl %0, %%eax\n\t"
                    "addl $1, %%eax\n\t"
                    "movl %%eax, %0"
                    : "+m" (counter)
                    :
                    : "%eax", "memory"
                );
                
                /* Another memory clobber barrier */
                scheduling_barrier();
                
                state = STATE_RESET;
                break;
            }
            
            case STATE_RESET: {
                /* Reset some values, keep others */
                i1 = (i1 + 1) & 0xFF;
                i2 = (i2 * 2) & 0xFFF;
                f1 = f1 * 0.99f;
                f2 = f2 + 0.01f;
                d1 = d1 * 0.999;
                
                counter++;
                if (counter > 10) {
                    counter = 0;
                    state = STATE_ARITHMETIC;
                } else {
                    state = STATE_FLOAT_OPS;
                }
                break;
            }
        }
        
        /* Update global volatile variables periodically */
        if ((iter & 7) == 0) {
            g_counter += i1;
            g_accumulator += f1;
            g_double_accum += d1;
        }
    }
}

/* Main test function with multiple scheduling regions */
int scheduling_stress(int outer_iterations) __attribute__((noinline, noclone));

int scheduling_stress(int outer_iterations) {
    volatile int result = 0;
    volatile float f_result = 0.0f;
    volatile double d_result = 0.0;
    
    /* Create multiple scheduling contexts */
    for (int i = 0; i < outer_iterations; i++) {
        /* Each call creates a new scheduling context */
        complex_state_machine(50);
        
        /* Mix in some direct operations between calls */
        volatile int temp = g_counter;
        for (int j = 0; j < 10; j++) {
            temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
            result ^= temp;
        }
        
        /* Floating point chain */
        f_result = g_accumulator;
        for (int j = 0; j < 5; j++) {
            f_result = f_result * 1.1f - 0.1f;
            f_result = opaque_float(f_result);
        }
        
        /* Double precision chain */
        d_result = g_double_accum;
        for (int j = 0; j < 5; j++) {
            d_result = d_result * 1.01 - 0.01;
            d_result = opaque_double(d_result);
        }
        
        /* Memory access pattern */
        volatile int local_array[16];
        for (int j = 0; j < 16; j++) {
            local_array[j] = (j + i) & 0xF;
        }
        
        /* Compute checksum from array */
        volatile int array_sum = 0;
        for (int j = 0; j < 16; j++) {
            array_sum += local_array[j] * (j + 1);
        }
        
        result += array_sum;
        f_result += (float)array_sum * 0.01f;
        d_result += (double)array_sum * 0.001;
        
        /* Scheduling barrier every few iterations */
        if ((i & 3) == 0) {
            scheduling_barrier();
        }
    }
    
    /* Final mixing of results */
    result += (int)f_result;
    result += (int)d_result;
    result = opaque_int(result);
    
    return result;
}

int main(void) {
    /* Initialize random seed for unpredictable control flow */
    srand(time(NULL));
    
    printf("Starting scheduler context test...\n");
    
    /* Run multiple times to increase chance of hitting uncovered code */
    int final_result = 0;
    for (int run = 0; run < 100; run++) {
        /* Vary iteration count slightly to prevent pattern recognition */
        int iterations = 30 + (rand() % 20);
        
        /* Each call may trigger scheduler context save/restore */
        int run_result = scheduling_stress(iterations);
        
        final_result ^= run_result;
        
        /* Update globals to maintain dependencies */
        g_counter += run_result;
        g_accumulator += (float)run_result * 0.01f;
        g_double_accum += (double)run_result * 0.001;
        
        /* Progress indicator */
        if ((run % 10) == 0) {
            printf("Completed run %d\n", run);
        }
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed.\n");
    
    return final_result != 0 ? 0 : 1;
}
