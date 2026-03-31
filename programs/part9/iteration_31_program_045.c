#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));
void scheduling_barrier(void) __attribute__((noinline, noipa));

/* Helper to create memory dependencies */
void memory_access_pattern(volatile int* arr, int idx) __attribute__((noinline, noipa));

/* State machine states */
enum states {
    STATE_ARITH_INT,
    STATE_ARITH_FLOAT,
    STATE_MEMORY_OPS,
    STATE_MIXED_OPS,
    STATE_CONTROL_FLOW,
    STATE_BARRIER,
    STATE_COMPLEX_DEPS,
    STATE_FINALIZE,
    NUM_STATES
};

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

void scheduling_barrier(void) {
    /* Memory clobber forces scheduler to save/restore state */
    asm volatile ("" ::: "memory");
}

void memory_access_pattern(volatile int* arr, int idx) {
    /* Complex memory access pattern to create dependencies */
    arr[idx % 16] = arr[(idx + 1) % 16] + arr[(idx + 3) % 16];
    arr[(idx + 2) % 16] = arr[idx % 16] * arr[(idx + 5) % 16];
    arr[(idx + 4) % 16] = arr[(idx + 2) % 16] >> (arr[idx % 16] & 0x7);
    scheduling_barrier();
}

/* Main scheduling stress function */
unsigned long long scheduling_stress(int iteration) __attribute__((noinline, noipa));

unsigned long long scheduling_stress(int iteration) {
    volatile int vi1 = iteration * 3;
    volatile int vi2 = iteration * 5 + 1;
    volatile int vi3 = iteration * 7 + 2;
    volatile int vi4 = iteration * 11 + 3;
    volatile float vf1 = iteration * 1.5f;
    volatile float vf2 = iteration * 2.7f;
    volatile double vd1 = iteration * 3.14159;
    volatile double vd2 = iteration * 2.71828;
    
    volatile int state = iteration % NUM_STATES;
    volatile int counter = 0;
    volatile int mem_array[16];
    
    /* Initialize memory array with volatile values */
    for (int i = 0; i < 16; i++) {
        mem_array[i] = (iteration * i) ^ 0xABCDEF;
    }
    
    unsigned long long checksum = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < 50; outer++) {
        /* State machine using switch - creates multiple basic blocks */
        switch (state) {
            case STATE_ARITH_INT: {
                /* Long chain of dependent integer operations */
                vi1 = vi2 + vi3;
                vi4 = vi1 * vi2;
                vi2 = vi4 >> (vi3 & 0xF);
                vi3 = vi2 ^ vi1;
                vi1 = vi3 + vi4;
                vi2 = vi1 * 3;
                vi3 = vi2 - vi4;
                vi4 = vi3 / (vi1 + 1);
                vi1 = opaque_int(vi4);
                
                /* Call to create scheduling boundary */
                vi2 = opaque_int(vi1 + vi3);
                state = STATE_ARITH_FLOAT;
                break;
            }
            
            case STATE_ARITH_FLOAT: {
                /* Mixed floating point operations */
                vf1 = vf2 * 1.5f;
                vf2 = vf1 + vf2;
                vf1 = opaque_float(vf2 * 3.14f);
                vf2 = vf1 / (vf2 + 1.0f);
                
                /* Mix with integer operations */
                vi1 = (int)(vf1 * 100.0f);
                vi2 = vi1 + (int)vf2;
                vf1 = (float)vi2 * 0.01f;
                
                state = STATE_MEMORY_OPS;
                break;
            }
            
            case STATE_MEMORY_OPS: {
                /* Complex memory access pattern */
                for (int i = 0; i < 8; i++) {
                    int idx = (vi1 + i) & 0xF;
                    mem_array[idx] = mem_array[(idx + 1) & 0xF] + 
                                    mem_array[(idx + 3) & 0xF];
                    mem_array[(idx + 2) & 0xF] = mem_array[idx] * 
                                                mem_array[(idx + 5) & 0xF];
                    
                    /* Call to function with memory clobber */
                    memory_access_pattern(mem_array, idx + vi2);
                }
                
                state = STATE_MIXED_OPS;
                break;
            }
            
            case STATE_MIXED_OPS: {
                /* Mixed integer and floating point with dependencies */
                vd1 = vd2 * 2.0;
                vi1 = (int)(vd1 * 100.0);
                vd2 = vd1 + (double)vi1;
                vi2 = vi1 * 3 + (int)vd2;
                vf1 = (float)vi2 * 0.5f;
                vf2 = vf1 + (float)vi1;
                vi3 = (int)(vf2 * 10.0f);
                vd1 = opaque_double(vd2 + (double)vi3);
                
                state = STATE_CONTROL_FLOW;
                break;
            }
            
            case STATE_CONTROL_FLOW: {
                /* Complex control flow within the case */
                volatile int temp = vi1;
                for (int j = 0; j < 5; j++) {
                    if (temp & (1 << j)) {
                        vi2 += mem_array[j];
                    } else {
                        vi2 -= mem_array[j + 8];
                    }
                    
                    /* Nested condition */
                    if (j % 2 == 0) {
                        vf1 += 1.0f;
                        if (vi2 > 1000) {
                            vi3 >>= 1;
                        }
                    }
                }
                
                state = STATE_BARRIER;
                break;
            }
            
            case STATE_BARRIER: {
                /* Multiple scheduling barriers */
                scheduling_barrier();
                vi1 = opaque_int(vi2);
                scheduling_barrier();
                vf1 = opaque_float(vf2);
                scheduling_barrier();
                vd1 = opaque_double(vd2);
                scheduling_barrier();
                
                state = STATE_COMPLEX_DEPS;
                break;
            }
            
            case STATE_COMPLEX_DEPS: {
                /* Very long dependency chain */
                int chain1 = vi1 + vi2;
                int chain2 = chain1 * vi3;
                int chain3 = chain2 >> (vi4 & 0x7);
                int chain4 = chain3 ^ chain1;
                int chain5 = chain4 + chain2;
                int chain6 = chain5 * 7;
                int chain7 = chain6 - chain3;
                int chain8 = chain7 / (chain4 + 1);
                int chain9 = chain8 ^ chain5;
                int chain10 = chain9 + chain6;
                
                vi1 = chain10;
                vi2 = opaque_int(chain10 + chain7);
                vi3 = opaque_int(vi2 * chain8);
                
                state = STATE_FINALIZE;
                break;
            }
            
            case STATE_FINALIZE: {
                /* Final computations and state transition */
                checksum += (unsigned long long)vi1;
                checksum += (unsigned long long)vi2;
                checksum += (unsigned long long)vi3;
                checksum += (unsigned long long)vi4;
                checksum += (unsigned long long)vf1;
                checksum += (unsigned long long)vf2;
                checksum += (unsigned long long)vd1;
                checksum += (unsigned long long)vd2;
                
                /* Update state based on complex condition */
                state = (vi1 + vi2 + vi3 + vi4) % NUM_STATES;
                break;
            }
        }
        
        /* Inline assembly with memory clobber at the end of each iteration */
        asm volatile ("" : : "r"(vi1), "r"(vi2), "r"(vi3), "r"(vi4) : "memory");
        
        /* Update volatile counter with data-dependent computation */
        counter = (counter + vi1 + vi2 + vi3) & 0xFF;
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 10; inner++) {
            int idx = (counter + inner) & 0xF;
            mem_array[idx] = mem_array[(idx + 1) & 0xF] + 
                            mem_array[(idx + 3) & 0xF] + 
                            inner;
            
            /* Use builtin for architecture-specific scheduling */
            if (inner % 3 == 0) {
                unsigned long long tsc;
                asm volatile ("rdtsc" : "=A" (tsc));
                checksum ^= tsc;
            }
        }
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    unsigned long long total_checksum = 0;
    
    /* Call scheduling_stress multiple times to increase coverage probability */
    for (int i = 0; i < 100; i++) {
        total_checksum ^= scheduling_stress(i + rand());
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    return 0;
}
