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

void __attribute__((noinline)) scheduling_barrier(volatile int *ptr) {
    /* Force memory barrier and scheduling boundary */
    asm volatile ("" : : "r"(ptr) : "memory");
}

/* Complex helper with mixed operations to force backend scheduling data */
int __attribute__((noinline)) complex_helper(int a, int b, float c, double d) {
    volatile int v1 = a * b;
    volatile float v2 = c * 1.5f;
    volatile double v3 = d / 2.0;
    
    /* Mixed type chain */
    int r1 = (int)(v2 * 100.0f);
    float r2 = (float)(v3 * 2.0);
    double r3 = (double)(v1 + r1);
    
    /* Memory access pattern */
    volatile int arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = (i % 2) ? v1 : r1;
    }
    
    /* Inline asm with memory clobber */
    asm volatile ("# Complex helper barrier" : : : "memory");
    
    return (int)(r3 + r2 + arr[3]);
}

/* State machine with complex control flow */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int state = seed % 8;
    volatile int counter = 0;
    volatile float f1 = 1.234f * seed;
    volatile double d1 = 5.678 * seed;
    volatile int arr_indices[4] = {0, 2, 4, 6};
    
    /* Local array for memory dependencies */
    volatile int mem_array[16];
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * seed;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex switch with many cases - splits CFG */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                volatile int a = counter * 3;
                volatile float b = f1 * 2.0f;
                volatile double c = d1 / 1.5;
                
                int t1 = a + (int)b;
                float t2 = b * (float)c;
                double t3 = c + (double)t1;
                int t4 = t1 * (int)t3;
                float t5 = t2 / 2.0f;
                double t6 = t3 * 1.1;
                int t7 = t4 ^ (int)t6;
                
                counter = opaque_int(t7);
                f1 = opaque_float(t5);
                d1 = opaque_double(t6);
                
                /* Memory access with volatile index */
                int idx = arr_indices[outer % 4];
                mem_array[idx] = t7;
                
                asm volatile ("# Case 0 barrier" : : : "memory");
                state = (counter % 7) + 1;
                break;
            }
            
            case 1: {
                /* Different operation mix */
                volatile int x = counter + 17;
                volatile float y = f1 - 0.5f;
                
                for (int i = 0; i < 5; i++) {
                    x = x * 3 - i;
                    y = y + (float)x * 0.1f;
                }
                
                /* Call helper to force scheduling boundary */
                counter = complex_helper(x, counter, y, d1);
                
                /* Array access pattern */
                for (int i = 0; i < 4; i++) {
                    mem_array[i * 2] += counter;
                }
                
                asm volatile ("# Case 1 barrier" : : : "memory");
                state = (x % 6) + 2;
                break;
            }
            
            case 2: {
                /* Integer-heavy chain */
                volatile int v = counter;
                v = v * 1103515245 + 12345;
                v = (v >> 16) & 32767;
                v = v * v % 1000;
                v = v ^ (v << 13);
                v = v ^ (v >> 17);
                v = v ^ (v << 5);
                
                /* Memory dependency */
                int idx = v % 16;
                v += mem_array[idx];
                mem_array[(idx + 1) % 16] = v;
                
                counter = opaque_int(v);
                
                asm volatile ("# Case 2 barrier" : : : "memory");
                state = 3 + (v % 4);
                break;
            }
            
            case 3: {
                /* Floating point intensive */
                volatile float f = f1;
                f = f * 1.618034f;
                f = f + (float)counter * 0.01f;
                f = f / 3.14159f;
                f = f * f;
                f = f - (float)((int)f);
                
                /* Mixed operation */
                volatile double d = d1 + (double)f;
                d = d * 0.999;
                d = d + sin((double)counter * 0.01);
                
                f1 = opaque_float(f);
                d1 = opaque_double(d);
                
                asm volatile ("# Case 3 barrier" : : : "memory");
                state = 4;
                break;
            }
            
            case 4: {
                /* Nested loops with break conditions */
                volatile int acc = 0;
                for (int i = 0; i < 10; i++) {
                    if (i == counter % 10) {
                        acc += complex_helper(i, counter, f1, d1);
                        break;
                    }
                    acc += i * i;
                }
                
                /* Memory update */
                for (int i = 0; i < 8; i += 2) {
                    mem_array[i] = acc + i;
                }
                
                counter = acc;
                asm volatile ("# Case 4 barrier" : : : "memory");
                state = 5 + (acc % 3);
                break;
            }
            
            case 5: {
                /* Computed goto state machine within the case */
                static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
                int label_idx = counter % 4;
                
                goto *labels[label_idx];
                
                L0: {
                    volatile int x = counter * 2;
                    x = x + mem_array[0];
                    counter = x;
                    goto end_case5;
                }
                L1: {
                    volatile int x = counter * 3;
                    x = x ^ mem_array[1];
                    counter = x;
                    goto end_case5;
                }
                L2: {
                    volatile int x = counter * 5;
                    x = x | mem_array[2];
                    counter = x;
                    goto end_case5;
                }
                L3: {
                    volatile int x = counter * 7;
                    x = x & mem_array[3];
                    counter = x;
                    goto end_case5;
                }
                
                end_case5:
                asm volatile ("# Case 5 barrier" : : : "memory");
                state = 6;
                break;
            }
            
            case 6: {
                /* Long unrolled dependency chain */
                volatile int a = counter;
                volatile int b = a + 1;
                volatile int c = b * 2;
                volatile int d = c - a;
                volatile int e = d ^ b;
                volatile int f = e * 3;
                volatile int g = f >> 2;
                volatile int h = g + c;
                volatile int i = h % 100;
                volatile int j = i * i;
                volatile int k = j + mem_array[5];
                volatile int l = k & 0xFF;
                
                /* Force backend scheduling with mixed operations */
                volatile float fx = (float)l * 0.1f;
                volatile double dx = (double)fx * 2.0;
                
                counter = l;
                f1 += fx;
                d1 += dx;
                
                asm volatile ("# Case 6 barrier" : : : "memory");
                state = 7;
                break;
            }
            
            case 7: {
                /* Array processing with data-dependent indices */
                volatile int sum = 0;
                for (int i = 0; i < 8; i++) {
                    int idx = (i * counter + outer) % 16;
                    sum += mem_array[idx];
                    mem_array[idx] = sum;
                }
                
                /* Complex condition for state transition */
                if (sum % 2 == 0) {
                    state = 0;
                    counter = sum;
                } else if (sum % 3 == 0) {
                    state = 1;
                    counter = sum * 2;
                } else {
                    state = 2;
                    counter = sum / 2;
                }
                
                /* Final scheduling barrier */
                scheduling_barrier(&counter);
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx1 = (inner * 3) % 16;
            volatile int idx2 = (inner * 7) % 16;
            
            int temp = mem_array[idx1];
            mem_array[idx1] = mem_array[idx2] + counter;
            mem_array[idx2] = temp - inner;
            
            /* Small dependency chain */
            counter = counter + (inner % 3);
        }
    }
    
    /* Compute checksum from all volatile state */
    int checksum = counter;
    checksum += (int)f1;
    checksum += (int)d1;
    
    for (int i = 0; i < 16; i++) {
        checksum ^= mem_array[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduler context stress test...\n");
    
    /* Repeated calls to force multiple scheduler context saves/restores */
    for (int iter = 0; iter < 100; iter++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum ^= result;
        
        /* Print progress occasionally */
        if (iter % 20 == 0) {
            printf("Iteration %d, checksum so far: %d\n", iter, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum & 0xFF;
}
