/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, returns_twice));
float opaque_float(float x) __attribute__((noinline, returns_twice));
double opaque_double(double x) __attribute__((noinline, returns_twice));
void scheduling_barrier(void) __attribute__((noinline));

/* Volatile array for memory dependencies */
volatile int volatile_array[32];

/* Opaque function implementations */
int opaque_int(int x) {
    volatile int temp = x;
    /* Inline asm to prevent optimization */
    asm volatile ("" : "+r" (temp) : : "memory");
    return temp;
}

float opaque_float(float x) {
    volatile float temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

double opaque_double(double x) {
    volatile double temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

void scheduling_barrier(void) {
    /* Force scheduler to save/restore context */
    asm volatile ("" : : : "memory");
}

/* Complex state machine with scheduling challenges */
int scheduling_stress(void) __attribute__((noinline, returns_twice));

int scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int result = 0;
    
    /* Initialize volatile array with random-ish values */
    for (int i = 0; i < 32; i++) {
        volatile_array[i] = (i * 37) & 0xFF;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer = opaque_int(outer + 1)) {
        /* Complex state machine using switch with many cases */
        switch (state) {
            case 0: {
                /* Long chain of dependent integer operations */
                a = b + c;
                d = a * e;
                e = d >> (c & 3);
                b = e - a;
                c = b * d;
                a = c ^ e;
                d = a + b + c;
                e = opaque_int(d * 3);
                result += e;
                
                /* Mixed type operations */
                f1 = (float)a + f2;
                f3 = f1 * f2;
                d1 = (double)f3 + d2;
                d3 = opaque_double(d1 * d2);
                
                /* Memory dependency with volatile access */
                int idx = (a + b) & 31;
                volatile_array[idx] = a + volatile_array[(idx + 1) & 31];
                
                state = (counter & 1) ? 1 : 7;
                break;
            }
            case 1: {
                /* Different chain of operations */
                b = c * d;
                a = b - e;
                c = a ^ d;
                e = c + b;
                d = e >> 2;
                b = opaque_int(d * a);
                result += b;
                
                /* Floating point chain */
                f2 = f3 / f1;
                f1 = opaque_float(f2 * 3.14f);
                d2 = d3 - d1;
                d3 = d2 * 2.0;
                
                /* Array access pattern */
                for (int i = 0; i < 8; i++) {
                    int idx = (i * 5 + counter) & 31;
                    volatile_array[idx] += volatile_array[(idx + 7) & 31];
                }
                
                state = (counter & 2) ? 2 : 6;
                break;
            }
            case 2: {
                /* More complex dependencies */
                c = d + e;
                a = b * c;
                e = a - d;
                b = e ^ c;
                d = b << (a & 3);
                c = opaque_int(d + e);
                result += c;
                
                /* Mixed operations */
                f3 = f1 + f2;
                f1 = f3 * 2.0f;
                d1 = d2 + d3;
                d2 = opaque_double(d1 / 2.0);
                
                state = 3;
                break;
            }
            case 3: {
                /* Use inline assembly as scheduling barrier */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "=r" (a)
                    : "r" (b), "r" (c)
                    : "%eax", "memory"
                );
                
                d = a * b + c;
                e = opaque_int(d - a);
                result += e;
                
                state = 4;
                break;
            }
            case 4: {
                /* Chain with function calls creating boundaries */
                a = opaque_int(b + c);
                b = opaque_int(c + d);
                c = opaque_int(d + e);
                d = opaque_int(e + a);
                e = opaque_int(a + b);
                
                f1 = opaque_float(f2 + f3);
                f2 = opaque_float(f3 + f1);
                
                state = 5;
                break;
            }
            case 5: {
                /* Memory intensive operations */
                for (int i = 0; i < 16; i += 2) {
                    volatile_array[i] = volatile_array[i + 1] + 
                                      volatile_array[(i + 3) & 31];
                }
                
                b = a * c + d * e;
                result += opaque_int(b);
                
                state = 6;
                break;
            }
            case 6: {
                /* Complex integer chain */
                a = ((b << 3) | (c >> 2)) ^ d;
                b = (a * 0x9E3779B9) ^ e;
                c = opaque_int(b + d);
                d = (c * 11) & 0xFFFF;
                e = d - a + b;
                
                state = 7;
                break;
            }
            case 7: {
                /* Final state with all operations */
                a = b + c + d + e;
                f1 = f2 * f3 + (float)a;
                d1 = d2 + d3 + (double)f1;
                
                /* Another scheduling barrier */
                scheduling_barrier();
                
                result += (int)f1 + (int)d1;
                
                /* Update state based on complex condition */
                int cond = (a ^ b) | (c & d);
                state = (cond & 7) ^ (counter & 7);
                break;
            }
            default: {
                /* Handle unexpected states */
                state = 0;
                break;
            }
        }
        
        /* Update counter with complex dependency */
        counter = opaque_int(counter + state + (result & 0xFF));
        
        /* Inner loop with array accesses */
        for (int inner = 0; inner < 10; inner++) {
            int idx1 = (inner * 3 + counter) & 31;
            int idx2 = (inner * 7 + a) & 31;
            volatile_array[idx1] = opaque_int(volatile_array[idx1] + 
                                             volatile_array[idx2] + inner);
        }
        
        /* Periodic function call to force scheduling boundary */
        if ((outer & 3) == 0) {
            a = opaque_int(b + c);
            f1 = opaque_float(f2 + f3);
        }
    }
    
    /* Compute final checksum */
    int checksum = result;
    for (int i = 0; i < 32; i++) {
        checksum ^= volatile_array[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total = 0;
    
    /* Call scheduling_stress multiple times to increase coverage chance */
    for (int i = 0; i < 100; i++) {
        total += scheduling_stress();
        
        /* Add some variation between calls */
        volatile int temp = rand() & 0xFF;
        asm volatile ("" : : "r" (temp) : "memory");
    }
    
    printf("Final checksum: %d\n", total);
    return 0;
}
