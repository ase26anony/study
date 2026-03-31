#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int dummy = x;
    asm volatile ("" : "+r" (dummy) : : "memory");
    return dummy;
}

float opaque_float(float x) {
    volatile float dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

double opaque_double(double x) {
    volatile double dummy = x;
    asm volatile ("" : "+f" (dummy) : : "memory");
    return dummy;
}

/* Helper to force scheduler state save/restore */
void scheduling_barrier(void) __attribute__((noinline, noipa));
void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Complex state machine with data-dependent control flow */
void scheduling_stress(void) __attribute__((noinline, noipa));
void scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int array[32];
    volatile int idx = 0;
    
    /* Initialize array with volatile accesses */
    for (int i = 0; i < 32; i++) {
        array[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch with many cases */
        switch (state) {
            case 0: {
                /* Long chain of integer dependencies */
                a = b + c;
                d = a * e;
                e = d >> 2;
                b = e ^ a;
                c = b * d;
                a = c - e;
                d = a + b + c;
                e = opaque_int(d * 3);
                
                /* Mix with float operations */
                f1 = f2 + f3;
                f2 = opaque_float(f1 * 2.0f);
                f3 = f2 / f1;
                
                /* Memory access with volatile index */
                idx = (idx + a) & 31;
                array[idx] = array[idx] + b;
                
                scheduling_barrier();
                state = (a + b) & 7;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                c = a | b;
                d = c ^ e;
                a = d * 3;
                b = a - c;
                e = b >> 1;
                c = e + d;
                a = opaque_int(c * b);
                
                /* Double precision chain */
                d1 = d2 * d3;
                d2 = opaque_double(d1 + 1.0);
                d3 = d2 / d1;
                
                idx = (idx + c) & 31;
                array[idx] = array[idx] - d;
                
                scheduling_barrier();
                state = (c + d + e) & 7;
                break;
            }
            case 2: {
                /* Mixed integer/float dependencies */
                a = b * c + d;
                f1 = (float)a / 2.0f;
                b = (int)(f1 * 3.0f);
                c = b ^ a;
                d = opaque_int(c + a);
                f2 = opaque_float(f1 + f3);
                e = (int)f2 * d;
                
                idx = (idx + e) & 31;
                array[idx] = array[idx] * 2;
                
                scheduling_barrier();
                state = (a + e) & 7;
                break;
            }
            case 3: {
                /* Complex chain with memory dependencies */
                a = array[(idx + 1) & 31];
                b = array[(idx + 2) & 31];
                c = a + b;
                d = array[(idx + 3) & 31];
                e = c * d;
                a = opaque_int(e);
                b = a ^ d;
                
                f3 = f1 * f2;
                f1 = opaque_float(f3 + 1.0f);
                
                scheduling_barrier();
                state = (b + c) & 7;
                break;
            }
            case 4: {
                /* More mixed operations */
                d1 = (double)a + (double)b;
                d2 = d1 * d3;
                a = (int)d2;
                c = b << 2;
                d = opaque_int(a | c);
                e = d ^ b;
                f2 = (float)e / 4.0f;
                
                idx = (idx + d) & 31;
                volatile int temp = array[idx];
                array[(idx + 4) & 31] = temp + e;
                
                scheduling_barrier();
                state = (d + e) & 7;
                break;
            }
            case 5: {
                /* Chain with many dependencies */
                a = b + c;
                b = c + d;
                c = d + e;
                d = e + a;
                e = a + b;
                a = b + c;
                b = opaque_int(c + d);
                c = d + e;
                d = opaque_int(e + a);
                
                f1 = f2 + f3;
                f2 = f3 + f1;
                f3 = opaque_float(f1 + f2);
                
                scheduling_barrier();
                state = (a + b + c) & 7;
                break;
            }
            case 6: {
                /* Use inline assembly as scheduling barrier */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %2, %%eax\n\t"
                    "addl %3, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "=r" (a)
                    : "r" (b), "r" (c), "r" (d)
                    : "%eax", "memory"
                );
                
                b = a >> 3;
                c = opaque_int(b * 7);
                d = c ^ a;
                e = d + b;
                
                scheduling_barrier();
                state = (c + 5) & 7;
                break;
            }
            case 7: {
                /* Final state with all types */
                a = ((b & c) | d) ^ e;
                f1 = (float)a * 0.5f;
                d1 = (double)b * 2.0;
                c = (int)(f1 * 4.0f);
                d = (int)d1;
                e = opaque_int(a + c + d);
                
                /* Multiple array accesses */
                for (int i = 0; i < 8; i++) {
                    idx = (idx + i) & 31;
                    array[idx] = array[idx] + i;
                }
                
                scheduling_barrier();
                state = (outer + e) & 7;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int i1 = (a + inner) & 31;
            volatile int i2 = (b + inner * 2) & 31;
            volatile int i3 = (c + inner * 3) & 31;
            
            array[i1] = array[i1] + array[i2];
            array[i2] = array[i2] ^ array[i3];
            array[i3] = opaque_int(array[i3] * inner);
            
            /* Small unrolled dependency chain */
            a = a + b;
            b = b + c;
            c = c + d;
            d = d + e;
            e = e + a;
        }
        
        counter++;
        if (counter > 100) {
            counter = 0;
            scheduling_barrier();
        }
    }
    
    /* Use results to prevent dead code elimination */
    volatile int result = a + b + c + d + e + (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3;
    for (int i = 0; i < 32; i++) {
        result += array[i];
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r" (result) : "memory");
}

int main(void) {
    srand(time(NULL));
    
    /* Call stress function multiple times */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Vary control flow slightly each iteration */
        if (i % 17 == 0) {
            scheduling_barrier();
        }
    }
    
    printf("Scheduling stress test completed.\n");
    return 0;
}
