/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, returns_twice));
float opaque_float(float x) __attribute__((noinline, returns_twice));
double opaque_double(double x) __attribute__((noinline, returns_twice));

int opaque_int(int x) {
    volatile int sink;
    sink = x;
    /* Memory clobber to force scheduling barrier */
    asm volatile ("" : : : "memory");
    return sink;
}

float opaque_float(float x) {
    volatile float sink;
    sink = x;
    asm volatile ("" : : : "memory");
    return sink;
}

double opaque_double(double x) {
    volatile double sink;
    sink = x;
    asm volatile ("" : : : "memory");
    return sink;
}

/* Helper to create complex data dependencies */
int complex_dependency_chain(int start) __attribute__((noinline));
int complex_dependency_chain(int start) {
    volatile int a = start;
    volatile int b = a + 1;
    volatile int c = b * 2;
    volatile int d = c - a;
    volatile int e = d >> 2;
    volatile int f = e | b;
    volatile int g = f & 0xFF;
    volatile int h = g * 3;
    volatile int i = h % 17;
    volatile int j = i ^ c;
    
    /* Force memory dependencies */
    volatile int arr[10];
    for (int k = 0; k < 10; k++) {
        arr[k] = j + k;
    }
    
    int result = 0;
    for (int k = 0; k < 10; k++) {
        result ^= arr[k];
    }
    
    asm volatile ("" : : : "memory");
    return result;
}

/* State machine with complex control flow */
void scheduling_stress(void) __attribute__((noinline, returns_twice));
void scheduling_stress(void) {
    /* Volatile variables to prevent optimization */
    volatile int state = 0;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int arr_indices[5] = {0, 1, 2, 3, 4};
    volatile int memory[10];
    
    /* Initialize memory array */
    for (int i = 0; i < 10; i++) {
        memory[i] = i * 3;
    }
    
    /* Outer loop - creates scheduling region */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch */
        switch (state) {
            case 0: {
                /* Integer dependency chain */
                int t1 = opaque_int(counter);
                int t2 = opaque_int(t1 * 3 + 1);
                int t3 = opaque_int(t2 >> 2);
                int t4 = opaque_int(t3 | 0x55);
                int t5 = opaque_int(t4 ^ t1);
                counter = t5;
                
                /* Mixed type operations */
                f1 = opaque_float(f1 * 2.0f + (float)t5);
                d1 = opaque_double(d1 / 1.5 + (double)t5);
                
                /* Memory access with volatile index */
                int idx = arr_indices[outer % 5];
                memory[idx] = opaque_int(memory[idx] + t5);
                
                state = (counter % 7 == 0) ? 1 : 3;
                asm volatile ("" : : : "memory");
                break;
            }
            
            case 1: {
                /* Different integer chain */
                int t1 = opaque_int(counter + 100);
                int t2 = opaque_int(t1 & 0xFF);
                int t3 = opaque_int(t2 * 7);
                int t4 = opaque_int(t3 - 50);
                int t5 = opaque_int(t4 % 23);
                counter = t5;
                
                /* Floating point chain */
                f2 = opaque_float(f2 + f1 * 0.5f);
                f3 = opaque_float(f3 - f2 * 0.25f);
                d2 = opaque_double(d2 * d1);
                
                /* Array access pattern */
                for (int i = 0; i < 5; i++) {
                    int idx = arr_indices[i];
                    memory[idx] = opaque_int(memory[idx] ^ counter);
                }
                
                state = (counter % 11 == 0) ? 2 : 0;
                asm volatile ("" : : : "memory");
                break;
            }
            
            case 2: {
                /* Long dependency chain */
                int t1 = opaque_int(counter * 2);
                int t2 = opaque_int(t1 + 17);
                int t3 = opaque_int(t2 << 3);
                int t4 = opaque_int(t3 / 2);
                int t5 = opaque_int(t4 | 0xAA);
                int t6 = opaque_int(t5 ^ 0x55);
                int t7 = opaque_int(t6 + t1);
                int t8 = opaque_int(t7 % 31);
                counter = t8;
                
                /* Mixed operations */
                f1 = opaque_float(f1 + f2 - f3);
                d3 = opaque_double(d3 * 0.9 + d2 * 0.1);
                
                /* Call to create scheduling boundary */
                counter = complex_dependency_chain(counter);
                
                state = 3;
                asm volatile ("" : : : "memory");
                break;
            }
            
            case 3: {
                /* Computed goto state machine */
                static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
                int label_idx = counter % 4;
                
                goto *labels[label_idx];
                
                label0: {
                    int t1 = opaque_int(counter + 1);
                    counter = opaque_int(t1 * 2);
                    f1 = opaque_float(f1 * 1.1f);
                    goto end_labels;
                }
                
                label1: {
                    int t1 = opaque_int(counter - 1);
                    counter = opaque_int(t1 / 2);
                    f2 = opaque_float(f2 * 0.9f);
                    goto end_labels;
                }
                
                label2: {
                    int t1 = opaque_int(counter ^ 0xFF);
                    counter = opaque_int(t1 + 100);
                    f3 = opaque_float(f3 + 0.5f);
                    goto end_labels;
                }
                
                label3: {
                    int t1 = opaque_int(counter & 0x7F);
                    counter = opaque_int(t1 * 3);
                    d1 = opaque_double(d1 + 0.1);
                    goto end_labels;
                }
                
                end_labels:
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                
                /* Inner loop with memory dependencies */
                for (int inner = 0; inner < 10; inner++) {
                    volatile int idx = (counter + inner) % 10;
                    memory[idx] = opaque_int(memory[idx] + inner);
                    
                    /* Create data dependency between iterations */
                    counter = opaque_int(counter + memory[idx] % 5);
                }
                
                state = (counter % 13 == 0) ? 0 : 2;
                asm volatile ("" : : : "memory");
                break;
            }
        }
        
        /* Periodic call to noinline function */
        if (outer % 7 == 0) {
            counter = opaque_int(counter + outer);
            f1 = opaque_float(f1 + (float)outer);
        }
        
        /* Use x86-specific builtin if available */
        #ifdef __x86_64__
        if (outer % 13 == 0) {
            unsigned long long tsc;
            asm volatile ("rdtsc" : "=A" (tsc));
            counter ^= (int)(tsc & 0xFFFFFFFF);
        }
        #endif
    }
    
    /* Final checksum calculation */
    volatile int checksum = counter;
    checksum ^= (int)f1;
    checksum ^= (int)f2;
    checksum ^= (int)f3;
    checksum ^= (int)d1;
    checksum ^= (int)d2;
    checksum ^= (int)d3;
    
    for (int i = 0; i < 10; i++) {
        checksum ^= memory[i];
    }
    
    /* Force result to be used */
    asm volatile ("" : : "r"(checksum) : "memory");
}

int main(void) {
    srand(time(NULL));
    
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Vary input slightly each iteration */
        int seed = rand() % 1000;
        total_checksum ^= seed;
        
        /* Call opaque function to create scheduling boundary */
        total_checksum = opaque_int(total_checksum);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum & 0xFF;
}
