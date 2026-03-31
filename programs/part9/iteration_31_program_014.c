/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void __attribute__((noinline)) scheduling_barrier(void) {
    /* Inline assembly with memory clobber to force scheduler to save state */
    asm volatile ("" : : : "memory");
}

/* Helper to create data dependencies */
int __attribute__((noinline)) complex_dependency_chain(int seed) {
    volatile int a = seed;
    volatile int b = a * 3;
    volatile int c = b + 7;
    volatile int d = c ^ 0x55AA55AA;
    volatile int e = d >> 3;
    volatile int f = e * 11;
    volatile int g = f & 0x00FF00FF;
    volatile int h = g - 19;
    volatile int i = h | 0x11111111;
    volatile int j = i << 2;
    
    scheduling_barrier();
    return j;
}

float __attribute__((noinline)) float_dependency_chain(float seed) {
    volatile float a = seed;
    volatile float b = a * 1.5f;
    volatile float c = b + 3.14159f;
    volatile float d = c / 2.0f;
    volatile float e = d * d;
    volatile float f = e - 1.0f;
    volatile float g = f * 0.5f;
    volatile float h = g + 0.25f;
    
    scheduling_barrier();
    return h;
}

/* State machine implementation with computed goto */
int __attribute__((noinline)) state_machine_computation(int start_state, int iterations) {
    static void* states[] = {
        &&state_0, &&state_1, &&state_2, &&state_3, &&state_4,
        &&state_5, &&state_6, &&state_7, &&state_8, &&state_9
    };
    
    volatile int state = start_state;
    volatile int accumulator = 0;
    volatile float f_accum = 0.0f;
    volatile double d_accum = 0.0;
    
    /* Local array with volatile accesses */
    volatile int local_array[20];
    for (int i = 0; i < 20; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < iterations; i++) {
        volatile int array_idx = (state * 7 + i) % 20;
        
        /* Jump to current state */
        goto *states[state % 10];
        
    state_0:
        {
            /* Long dependency chain with mixed operations */
            volatile int t1 = local_array[array_idx];
            volatile int t2 = complex_dependency_chain(t1);
            volatile float ft1 = float_dependency_chain(t2 * 0.01f);
            accumulator += t2 + (int)(ft1 * 100);
            f_accum += ft1;
            d_accum += (double)t2 * 0.001;
            state = (accumulator & 0xF) ^ 1;
        }
        scheduling_barrier();
        continue;
        
    state_1:
        {
            volatile int t1 = accumulator * 3;
            volatile int t2 = t1 ^ 0x12345678;
            volatile int t3 = t2 >> (i & 3);
            volatile int t4 = t3 * 7;
            volatile int t5 = t4 - 19;
            accumulator = opaque_int(t5);
            state = (t5 & 0x7) + 2;
        }
        scheduling_barrier();
        continue;
        
    state_2:
        {
            volatile float f1 = f_accum * 2.0f;
            volatile float f2 = f1 + 3.14f;
            volatile float f3 = f2 / 1.618f;
            volatile float f4 = f3 * f3;
            f_accum = opaque_float(f4);
            volatile int t1 = (int)f4 * 11;
            accumulator += t1;
            state = (t1 & 0x3) + 3;
        }
        scheduling_barrier();
        continue;
        
    state_3:
        {
            volatile double d1 = d_accum * 1.5;
            volatile double d2 = d1 + 2.71828;
            volatile double d3 = d2 / 3.14159;
            volatile double d4 = d3 * d_accum;
            d_accum = opaque_double(d4);
            accumulator += (int)(d4 * 1000);
            state = ((int)d4 & 0x5) + 4;
        }
        scheduling_barrier();
        continue;
        
    state_4:
        {
            /* Memory-intensive operations */
            volatile int temp[5];
            temp[0] = local_array[(array_idx + 0) % 20];
            temp[1] = local_array[(array_idx + 1) % 20];
            temp[2] = local_array[(array_idx + 2) % 20];
            temp[3] = local_array[(array_idx + 3) % 20];
            temp[4] = local_array[(array_idx + 4) % 20];
            
            volatile int sum = 0;
            for (int j = 0; j < 5; j++) {
                sum += temp[j] * (j + 1);
            }
            accumulator += sum;
            state = (sum & 0x7) + 5;
        }
        scheduling_barrier();
        continue;
        
    state_5:
        {
            volatile int t1 = accumulator * 13;
            volatile int t2 = t1 ^ accumulator;
            volatile int t3 = t2 << 1;
            volatile int t4 = t3 | 0x01;
            volatile int t5 = t4 - 27;
            accumulator = t5;
            state = (t5 & 0x3) + 6;
        }
        scheduling_barrier();
        continue;
        
    state_6:
        {
            volatile float f1 = f_accum + 1.0f;
            volatile float f2 = f1 * 0.333f;
            volatile float f3 = f2 - 0.5f;
            volatile float f4 = f3 * f_accum;
            f_accum = f4;
            accumulator += (int)(f4 * 50);
            state = ((int)f4 & 0x7) + 7;
        }
        scheduling_barrier();
        continue;
        
    state_7:
        {
            volatile double d1 = d_accum + 0.1;
            volatile double d2 = d1 * 2.0;
            volatile double d3 = d2 / 1.1;
            volatile double d4 = d3 - d_accum;
            d_accum = d4;
            accumulator += (int)(d4 * 200);
            state = ((int)d4 & 0x3) + 8;
        }
        scheduling_barrier();
        continue;
        
    state_8:
        {
            /* Mixed integer/float operations */
            volatile int t1 = accumulator & 0xFF;
            volatile float f1 = (float)t1 * 0.01f;
            volatile int t2 = (int)(f1 * 1000);
            volatile double d1 = (double)t2 * 0.001;
            volatile int t3 = (int)(d1 * 10000);
            accumulator = t3;
            state = (t3 & 0x7) + 9;
        }
        scheduling_barrier();
        continue;
        
    state_9:
        {
            /* Final complex chain */
            volatile int t1 = local_array[array_idx] * 17;
            volatile int t2 = t1 + accumulator;
            volatile int t3 = t2 ^ 0xAA55AA55;
            volatile int t4 = t3 >> 2;
            volatile int t5 = t4 * 3;
            volatile int t6 = t5 - 42;
            accumulator = t6;
            state = (t6 & 0xF);
        }
        scheduling_barrier();
        continue;
    }
    
    /* Combine all accumulators into final result */
    volatile int result = accumulator + (int)f_accum + (int)d_accum;
    return opaque_int(result);
}

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int checksum = seed;
    
    /* Outer loop with varying trip counts */
    volatile int outer_iterations = 50;
    
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Switch statement with many cases - creates multiple basic blocks */
        volatile int switch_var = (checksum + outer) % 12;
        
        switch (switch_var) {
            case 0: {
                volatile int a = checksum * 3;
                volatile int b = a + 7;
                volatile int c = b ^ 0x1234;
                volatile int d = c >> 1;
                checksum = opaque_int(d);
                break;
            }
            case 1: {
                volatile float a = checksum * 0.1f;
                volatile float b = a + 3.14f;
                volatile float c = b * b;
                checksum += (int)(c * 10);
                break;
            }
            case 2: {
                volatile int a = checksum & 0xFF;
                volatile int b = a * 11;
                volatile int c = b | 0x80808080;
                volatile int d = c - 19;
                checksum = opaque_int(d);
                break;
            }
            case 3: {
                volatile double a = checksum * 0.01;
                volatile double b = a + 2.71828;
                volatile double c = b / 1.414;
                checksum += (int)(c * 100);
                break;
            }
            case 4: {
                volatile int a = checksum ^ checksum;
                volatile int b = a + 1;
                volatile int c = b * 3;
                volatile int d = c << 2;
                checksum = opaque_int(d);
                break;
            }
            case 5: {
                volatile float a = checksum * 0.5f;
                volatile float b = a - 1.0f;
                volatile float c = b * 0.333f;
                checksum += (int)(c * 30);
                break;
            }
            case 6: {
                volatile int a = checksum | 0x55555555;
                volatile int b = a * 7;
                volatile int c = b >> 3;
                volatile int d = c - 11;
                checksum = opaque_int(d);
                break;
            }
            case 7: {
                volatile double a = checksum * 0.001;
                volatile double b = a + 1.618;
                volatile double c = b * 0.5;
                checksum += (int)(c * 200);
                break;
            }
            case 8: {
                volatile int a = checksum & 0x0F0F0F0F;
                volatile int b = a * 13;
                volatile int c = b ^ 0xF0F0F0F0;
                volatile int d = c + 23;
                checksum = opaque_int(d);
                break;
            }
            case 9: {
                volatile float a = checksum * 0.2f;
                volatile float b = a + 0.5f;
                volatile float c = b / 1.2f;
                checksum += (int)(c * 40);
                break;
            }
            case 10: {
                volatile int a = checksum * 17;
                volatile int b = a | 0x33333333;
                volatile int c = b >> 1;
                volatile int d = c - 29;
                checksum = opaque_int(d);
                break;
            }
            case 11: {
                volatile double a = checksum * 0.0001;
                volatile double b = a + 0.7071;
                volatile double c = b * 2.0;
                checksum += (int)(c * 50);
                break;
            }
        }
        
        /* Inner loop with array accesses */
        volatile int array[10];
        volatile int inner_iterations = 10 + (checksum & 0xF);
        
        for (volatile int inner = 0; inner < inner_iterations; inner++) {
            volatile int idx = (checksum + inner) % 10;
            array[idx] = checksum + inner;
            
            /* Create dependency chain within inner loop */
            volatile int temp = array[idx];
            volatile int dep1 = temp * 3;
            volatile int dep2 = dep1 + 7;
            volatile int dep3 = dep2 ^ 0xAA;
            volatile int dep4 = dep3 >> 1;
            array[(idx + 1) % 10] = dep4;
            
            checksum += dep4;
        }
        
        /* Call state machine periodically */
        if ((outer % 7) == 0) {
            volatile int state_result = state_machine_computation(checksum & 0xF, 5);
            checksum ^= state_result;
        }
        
        scheduling_barrier();
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    volatile int final_result = 0;
    
    printf("Starting scheduler context test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        volatile int seed = rand() % 1000;
        volatile int result = scheduling_stress(seed);
        final_result ^= result;
        
        /* Use inline assembly with rdtsc on x86 to create backend scheduling complexity */
        #ifdef __x86_64__
        unsigned long long tsc1, tsc2;
        asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
        final_result += (int)(tsc1 ^ tsc2);
        #endif
        
        if ((i % 10) == 0) {
            printf("Iteration %d, intermediate result: %d\n", i, final_result);
        }
    }
    
    printf("Final checksum: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
