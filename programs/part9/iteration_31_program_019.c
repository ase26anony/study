/* sched_context_test.c
 * Program designed to trigger scheduler context saving/freeing logic
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

/* State machine helper with scheduling barrier */
void state_machine_helper(int state, volatile int* vars) __attribute__((noinline));

/* Memory access pattern creator */
int memory_stress(volatile int* arr, int size) __attribute__((noinline));

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));

/* Opaque function implementations */
int opaque_int(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x;
}

float opaque_float(float x) {
    asm volatile ("" : "+f" (x));
    return x;
}

double opaque_double(double x) {
    asm volatile ("" : "+f" (x));
    return x;
}

/* State machine helper - creates scheduling boundaries */
void state_machine_helper(int state, volatile int* vars) {
    /* Complex dependency chain */
    int t1 = vars[0] + state;
    int t2 = t1 * vars[1];
    int t3 = t2 >> (vars[2] & 0x7);
    int t4 = t3 - vars[3];
    int t5 = t4 ^ vars[4];
    
    /* Mixed type operations */
    float f1 = (float)t5 * 1.5f;
    double d1 = (double)f1 * 2.5;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Store results back */
    vars[0] = opaque_int(t5);
    vars[1] = (int)f1;
    vars[2] = (int)d1;
}

/* Memory stress function */
int memory_stress(volatile int* arr, int size) {
    int sum = 0;
    volatile int idx = 0;
    
    /* Complex memory access pattern */
    for (int i = 0; i < size * 2; i++) {
        idx = (idx * 13 + 17) % size;
        arr[idx] = arr[idx] * 3 + i;
        
        /* Dependency chain with memory */
        int temp = arr[(idx + 1) % size];
        temp = opaque_int(temp);
        arr[idx] = arr[idx] + temp;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        sum += arr[idx];
    }
    
    return sum;
}

/* Main scheduling stress function */
int scheduling_stress(int seed) {
    volatile int vars[10];
    volatile float fvars[5];
    volatile double dvars[3];
    
    /* Initialize with seed-dependent values */
    for (int i = 0; i < 10; i++) {
        vars[i] = seed + i * 17;
    }
    for (int i = 0; i < 5; i++) {
        fvars[i] = (float)(seed + i) * 0.7f;
    }
    for (int i = 0; i < 3; i++) {
        dvars[i] = (double)(seed + i) * 1.3;
    }
    
    /* Local array for memory stress */
    volatile int mem_array[20];
    for (int i = 0; i < 20; i++) {
        mem_array[i] = i * 3;
    }
    
    /* State machine implementation */
    int state = seed % 8;
    volatile int counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Switch-based state machine with complex control flow */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                int a = vars[0] + vars[1];
                int b = a * vars[2];
                float c = (float)b * fvars[0];
                double d = (double)c + dvars[0];
                int e = (int)d >> (vars[3] & 0x3);
                vars[0] = opaque_int(e);
                
                /* Call helper to create scheduling boundary */
                state_machine_helper(state, (volatile int*)vars);
                
                /* Memory access */
                mem_array[outer % 20] = e;
                
                /* Update state */
                state = (e & 0x7);
                break;
            }
            
            case 1: {
                /* Different dependency pattern */
                float a = fvars[1] * 2.0f;
                double b = (double)a + dvars[1];
                int c = vars[4] + (int)b;
                int d = c * vars[5];
                int e = d - vars[6];
                fvars[1] = opaque_float((float)e * 0.5f);
                
                state_machine_helper(state, (volatile int*)vars);
                
                /* Complex condition */
                if ((e & 0xF) > 8) {
                    state = 2;
                } else {
                    state = 3;
                }
                break;
            }
            
            case 2: {
                /* Integer-heavy chain */
                int t1 = vars[7] ^ vars[8];
                int t2 = t1 + vars[9];
                int t3 = t2 * 3;
                int t4 = t3 >> 1;
                int t5 = t4 - vars[0];
                int t6 = t5 * 7;
                int t7 = t6 + vars[1];
                int t8 = t7 ^ 0x55AA;
                vars[2] = opaque_int(t8);
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = (t8 & 0x1) ? 4 : 5;
                break;
            }
            
            case 3: {
                /* Floating point intensive */
                double d1 = dvars[0] * 1.7;
                double d2 = d1 + dvars[1];
                float f1 = (float)d2 * 0.3f;
                float f2 = f1 + fvars[2];
                int i1 = (int)f2 + vars[3];
                dvars[0] = opaque_double(d2);
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = 6;
                break;
            }
            
            case 4: {
                /* Mixed operations with memory */
                int idx = outer % 10;
                int val = mem_array[idx] + vars[idx];
                float fval = (float)val * fvars[idx % 5];
                vars[idx] = (int)fval;
                
                /* Additional dependency chain */
                for (int i = 0; i < 3; i++) {
                    vars[i] = vars[i] + i;
                }
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = 7;
                break;
            }
            
            case 5: {
                /* Complex integer chain */
                int chain = vars[0];
                for (int i = 1; i < 8; i++) {
                    chain = chain * 3 + vars[i];
                    chain = (chain << 3) | (chain >> 29); /* rotate */
                }
                vars[9] = opaque_int(chain);
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = 0;
                break;
            }
            
            case 6: {
                /* Use x86-specific builtin if available */
                #ifdef __x86_64__
                unsigned long long tsc = __builtin_ia32_rdtsc();
                vars[5] = opaque_int((int)(tsc & 0xFFFFFFFF));
                #else
                vars[5] = opaque_int(vars[5] * 13 + 17);
                #endif
                
                /* Memory stress */
                int mem_sum = memory_stress(mem_array, 20);
                vars[6] = opaque_int(mem_sum);
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = 1;
                break;
            }
            
            case 7: {
                /* Final state with complex dependencies */
                double d = dvars[2];
                for (int i = 0; i < 4; i++) {
                    d = d * 1.1 + (double)vars[i];
                }
                dvars[2] = opaque_double(d);
                
                /* Update all volatile variables */
                for (int i = 0; i < 5; i++) {
                    fvars[i] = fvars[i] * 0.9f + (float)i;
                }
                
                state_machine_helper(state, (volatile int*)vars);
                
                state = (outer & 0x1) ? 0 : 2;
                break;
            }
        }
        
        /* Inline assembly barrier - forces scheduler to save/restore state */
        asm volatile ("" : : : "memory");
        
        /* Update counter with complex dependency */
        counter = counter + (vars[0] & 0x3) - (vars[1] & 0x1) + (state * 2);
        
        /* Occasionally call opaque functions to create scheduling boundaries */
        if ((outer % 7) == 0) {
            vars[3] = opaque_int(vars[3] + vars[4]);
            fvars[2] = opaque_float(fvars[2] * 1.1f);
        }
    }
    
    /* Inner loop with array accesses */
    int inner_sum = 0;
    volatile int inner_idx = 0;
    for (int i = 0; i < 100; i++) {
        inner_idx = (inner_idx * 7 + 11) % 20;
        mem_array[inner_idx] = mem_array[inner_idx] + vars[i % 10];
        
        /* Dependency between array accesses */
        int next_idx = (inner_idx + 1) % 20;
        mem_array[next_idx] = mem_array[next_idx] - (mem_array[inner_idx] & 0xF);
        
        inner_sum += mem_array[inner_idx];
        
        /* Scheduling barrier every 8 iterations */
        if ((i % 8) == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum = checksum * 31 + vars[i];
    }
    for (int i = 0; i < 20; i++) {
        checksum = checksum + mem_array[i];
    }
    checksum = opaque_int(checksum + inner_sum + counter);
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int rep = 0; rep < 100; rep++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum = opaque_int(total_checksum + result);
        
        /* Progress indicator */
        if ((rep % 20) == 0) {
            printf("Iteration %d, checksum so far: %d\n", rep, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return total_checksum & 0xFF;
}
