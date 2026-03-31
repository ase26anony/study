/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

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

/* Helper to create data dependencies across function calls */
int dependency_chain_helper(int a, int b, float c, double d) __attribute__((noinline));
int dependency_chain_helper(int a, int b, float c, double d) {
    volatile int v1 = a;
    volatile float v2 = c;
    volatile double v3 = d;
    
    /* Complex mixed-type operations */
    v1 = v1 * b + (int)(v2 * 3.14f);
    v2 = v2 + (float)v3 * 2.0f;
    v3 = v3 / 1.618 + (double)v1;
    
    asm volatile ("" : : : "memory");
    return v1 + (int)v2 + (int)v3;
}

/* State machine with complex control flow */
void scheduling_stress(void) __attribute__((noinline));
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
    for (int outer = 0; outer < 50; outer = opaque_int(outer + 1)) {
        /* Complex state machine using switch with many cases */
        switch (state) {
            case 0: {
                /* Long dependency chain with mixed types */
                a = b + c;
                f1 = (float)a * f2;
                d = (int)f1 ^ e;
                d1 = (double)d * d2;
                c = (int)d1 + b;
                f3 = f1 + f2 * 3.14159f;
                a = opaque_int(a * 31 + d);
                asm volatile ("" : : : "memory");
                state = (a & 7) + 1;
                break;
            }
            case 1: {
                /* Different dependency pattern */
                b = c * d - a;
                f2 = f3 / f1 + 2.71828f;
                e = b >> (a & 3);
                d2 = d1 * 1.414 + d3;
                f1 = opaque_float(f2 * f3 - f1);
                c = e ^ d;
                asm volatile ("" : : : "memory");
                state = (b & 7) + 2;
                break;
            }
            case 2: {
                /* More complex operations */
                d = a * b + c * e;
                f3 = f1 * f2 - f3;
                d3 = d1 + d2 / 3.14159;
                a = (d << 2) | (e >> 1);
                f2 = opaque_float(f3 * 2.0f);
                b = dependency_chain_helper(a, b, f1, d1);
                asm volatile ("" : : : "memory");
                state = (d & 7) + 3;
                break;
            }
            case 3: {
                /* Integer-heavy chain */
                c = (a * b) + (c * d) - (e << 1);
                e = (c ^ d) & 0x7FFFFFFF;
                a = b * 17 - c / 3;
                d = opaque_int((e << 3) | (a & 7));
                b = c + d - e;
                asm volatile ("" : : : "memory");
                state = (c & 7) + 4;
                break;
            }
            case 4: {
                /* Floating-point intensive */
                f1 = f2 * 1.5f + f3;
                f2 = f1 / 2.0f - f3;
                f3 = opaque_float(f1 * f2);
                d1 = d2 + d3 * 0.5;
                d2 = d1 * 1.618 - d3;
                d3 = opaque_double(d2 / 3.14159);
                asm volatile ("" : : : "memory");
                state = ((int)f1 & 7) + 5;
                break;
            }
            case 5: {
                /* Memory access pattern */
                idx = (idx + a) & 31;
                int val = array[idx];
                a = val + b;
                idx = (idx + b) & 31;
                val = array[idx];
                b = val + c;
                idx = (idx + c) & 31;
                val = array[idx];
                c = val + d;
                asm volatile ("" : : : "memory");
                state = (val & 7) + 6;
                break;
            }
            case 6: {
                /* Mixed operations with function call */
                f1 = (float)a * 0.25f;
                d1 = (double)b * 0.333;
                c = dependency_chain_helper(a, b, f1, d1);
                f2 = opaque_float(f1 + (float)c);
                d = c * 2 - b;
                asm volatile ("" : : : "memory");
                state = (c & 7) + 7;
                break;
            }
            case 7: {
                /* Bit manipulation chain */
                a = (a << 3) | (b & 7);
                b = (b >> 2) ^ (c << 1);
                c = (c & 0x5555) | (d & 0xAAAA);
                d = opaque_int((e << 4) | (a & 15));
                e = (b ^ c) & d;
                asm volatile ("" : : : "memory");
                state = (e & 7);
                break;
            }
            case 8: {
                /* Another floating-point chain */
                f3 = f1 * 2.0f - f2;
                d3 = d1 * 1.5 + d2;
                f1 = opaque_float(f2 + f3 * 0.5f);
                d2 = opaque_double(d3 - d1);
                a = (int)(f1 * 10.0f);
                b = (int)(d2 * 5.0);
                asm volatile ("" : : : "memory");
                state = (a + b) & 7;
                break;
            }
            case 9: {
                /* Complex integer dependency */
                c = a * 3 + b * 5 - d * 7;
                e = (c << 2) + (d >> 1);
                a = opaque_int(b * 11 + e * 13);
                d = c ^ e ^ a;
                b = dependency_chain_helper(a, c, f1, d1);
                asm volatile ("" : : : "memory");
                state = (d & 7) + 1;
                break;
            }
            default: {
                state = 0;
                break;
            }
        }
        
        /* Inner loop with array accesses and volatile indices */
        volatile int inner_idx = outer & 31;
        for (int i = 0; i < 10; i = opaque_int(i + 1)) {
            int temp = array[inner_idx];
            array[inner_idx] = temp + a + i;
            inner_idx = (inner_idx + b) & 31;
            temp = array[inner_idx];
            array[inner_idx] = temp + c - i;
            inner_idx = (inner_idx + d) & 31;
            
            /* Small inline asm barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Update volatile counter with complex expression */
        counter = opaque_int(counter + a - b + c - d + e);
    }
    
    /* Use all variables to prevent dead code elimination */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                      "r"(f1), "r"(f2), "r"(f3),
                      "r"(d1), "r"(d2), "r"(d3),
                      "r"(counter) : "memory");
}

/* Alternative version with computed goto for even more complex CFG */
void scheduling_stress_goto(void) __attribute__((noinline));
void scheduling_stress_goto(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int x = 1, y = 2, z = 3;
    volatile float fx = 1.0f, fy = 2.0f;
    
    /* Label array for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, &&state_d,
        &&state_e, &&state_f, &&state_g, &&state_h
    };
    
    for (int i = 0; i < 30; i++) {
        if (state >= 8) state = 0;
        goto *labels[state];
        
    state_a:
        x = y * z + i;
        fx = (float)x * 0.5f;
        state = (x & 3) + 1;
        asm volatile ("" : : : "memory");
        continue;
        
    state_b:
        y = z ^ x;
        fy = fx + 1.0f;
        state = (y & 3) + 2;
        asm volatile ("" : : : "memory");
        continue;
        
    state_c:
        z = x + y * 2;
        fx = opaque_float(fy * fx);
        state = (z & 3) + 3;
        asm volatile ("" : : : "memory");
        continue;
        
    state_d:
        x = dependency_chain_helper(x, y, fx, (double)z);
        state = (x & 3) + 4;
        asm volatile ("" : : : "memory");
        continue;
        
    state_e:
        y = opaque_int(z * 3 - x);
        state = (y & 3) + 5;
        asm volatile ("" : : : "memory");
        continue;
        
    state_f:
        z = x ^ y ^ i;
        state = (z & 3) + 6;
        asm volatile ("" : : : "memory");
        continue;
        
    state_g:
        fx = fy * 2.0f - fx;
        state = ((int)fx & 3) + 7;
        asm volatile ("" : : : "memory");
        continue;
        
    state_h:
        fy = opaque_float(fx / 2.0f);
        state = ((int)fy & 3);
        asm volatile ("" : : : "memory");
        continue;
    }
    
    asm volatile ("" : : "r"(x), "r"(y), "r"(z), "r"(fx), "r"(fy) : "memory");
}

int main(void) {
    srand(time(NULL));
    
    printf("Starting scheduler stress test...\n");
    
    /* Call scheduling stress function multiple times */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Occasionally call the goto version for variety */
        if ((i % 17) == 0) {
            scheduling_stress_goto();
        }
        
        /* Use rand() to create unpredictable control flow in main */
        volatile int r = rand() & 255;
        if (r < 64) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Scheduler stress test completed.\n");
    
    /* Use rdtsc on x86 to create backend-specific scheduling requirements */
    #ifdef __x86_64__
    {
        unsigned long long tsc1, tsc2;
        asm volatile ("rdtsc" : "=a" (tsc1), "=d" (tsc2));
        asm volatile ("" : : "r"(tsc1), "r"(tsc2) : "memory");
    }
    #endif
    
    return 0;
}
