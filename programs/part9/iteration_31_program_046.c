#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
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

/* Helper to create scheduling barriers */
void scheduling_barrier(void) __attribute__((noinline));
void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Complex state machine with many basic blocks */
void scheduling_stress(void) __attribute__((noinline, optimize("no-unroll-loops")));
void scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer = opaque_int(outer + 1)) {
        /* Complex switch with many cases - each creates a basic block */
        switch (state % 12) {
            case 0: {
                /* Long dependency chain with mixed types */
                a = b + c;
                f1 = (float)a * f2;
                d = (int)f1 ^ e;
                d1 = (double)d * d2;
                b = (int)d1 + arr[a & 0xF];
                c = b * a - d;
                scheduling_barrier();
                state = (a + b) % 12;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                c = a * b + d;
                f2 = f1 * (float)c;
                e = (int)f2 | arr[c & 0xF];
                d2 = d1 / (double)(e + 1);
                a = (int)d2 * b;
                f3 = f2 + f1;
                scheduling_barrier();
                state = (c ^ e) % 12;
                break;
            }
            case 2: {
                /* Integer-heavy chain */
                b = c << 2;
                d = b * a + e;
                a = d ^ (arr[b & 0xF] + 1);
                c = a * b - d;
                e = c % (a + 1);
                b = e * 3 + arr[d & 0xF];
                scheduling_barrier();
                state = (b + d) % 12;
                break;
            }
            case 3: {
                /* Float-heavy chain */
                f1 = f2 * 1.5f;
                f3 = f1 + f2;
                f2 = f3 * 0.75f;
                d1 = (double)f1 * d2;
                d3 = d1 + d2;
                a = (int)(f1 + f2 + f3);
                scheduling_barrier();
                state = (a + (int)f1) % 12;
                break;
            }
            case 4: {
                /* Mixed operations with memory */
                a = arr[b & 0xF] + c;
                d = a * b - arr[c & 0xF];
                f1 = (float)d / 2.0f;
                e = (int)f1 ^ arr[d & 0xF];
                c = e + a * b;
                scheduling_barrier();
                state = (d + e) % 12;
                break;
            }
            case 5: {
                /* Complex integer chain */
                b = (a * 3) / 2;
                c = b ^ 0xABCD;
                d = c + arr[a & 0xF] * 2;
                e = d % 17;
                a = e * b + c;
                b = a ^ d;
                scheduling_barrier();
                state = (a * b) % 12;
                break;
            }
            case 6: {
                /* Double precision chain */
                d1 = d2 * 1.618;
                d3 = d1 + d2;
                d2 = d3 / 2.0;
                a = (int)(d1 * 100.0);
                b = a + arr[(int)d2 & 0xF];
                scheduling_barrier();
                state = (b + (int)d1) % 12;
                break;
            }
            case 7: {
                /* Bit manipulation chain */
                c = a ^ b;
                d = c << (b & 3);
                e = d | arr[c & 0xF];
                a = e ^ (d >> 2);
                b = a * c + d;
                scheduling_barrier();
                state = (c ^ e) % 12;
                break;
            }
            case 8: {
                /* Another mixed chain */
                f1 = (float)a / 3.0f;
                f2 = f1 * (float)b;
                c = (int)f2 + arr[(int)f1 & 0xF];
                d1 = (double)c * 0.5;
                e = (int)d1 ^ b;
                scheduling_barrier();
                state = (c + e) % 12;
                break;
            }
            case 9: {
                /* Complex with modulo */
                a = (b * c) % 31;
                d = (a + e) * 2;
                f1 = (float)d / 4.0f;
                b = (int)f1 + arr[d & 0xF];
                c = b ^ a;
                scheduling_barrier();
                state = (a + d) % 12;
                break;
            }
            case 10: {
                /* Multi-step chain */
                e = a + b + c + d;
                f2 = (float)e * 0.25f;
                d1 = (double)f2 * 3.14159;
                a = (int)d1;
                b = a * c - e;
                c = b + arr[e & 0xF];
                scheduling_barrier();
                state = (e + b) % 12;
                break;
            }
            case 11: {
                /* Final complex chain */
                b = c * d - a;
                f3 = (float)b / 2.0f;
                d2 = (double)f3 * d1;
                e = (int)d2 | arr[b & 0xF];
                a = e ^ (b * 3);
                scheduling_barrier();
                state = (b + e) % 12;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 8; inner = opaque_int(inner + 1)) {
            volatile int idx = (a + inner) & 0xF;
            int val = arr[idx];
            arr[idx] = val * 2 + b;
            b = b ^ val;
            c = c + arr[(idx + 1) & 0xF];
            
            /* Small dependency chain in inner loop */
            d = d * 3 - c;
            e = e ^ (d + inner);
            f1 = f1 + (float)(val % 5);
        }
        
        /* Call to noinline function creates scheduling boundary */
        a = opaque_int(a);
        f1 = opaque_float(f1);
        d1 = opaque_double(d1);
        
        /* Update counter with complex condition */
        counter = opaque_int(counter + 1);
        if ((counter & 3) == 0) {
            state = (state + 1) % 12;
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    asm volatile ("" 
        : "+r" (a), "+r" (b), "+r" (c), "+r" (d), "+r" (e), 
          "+f" (f1), "+f" (f2), "+f" (f3),
          "+f" (d1), "+f" (d2), "+f" (d3)
        : 
        : "memory");
}

int main(void) {
    srand(time(NULL));
    
    /* Call stress function multiple times */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Vary control flow slightly each iteration */
        if ((i % 17) == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Scheduling stress test completed.\n");
    return 0;
}
