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
    asm volatile("" : "+r"(dummy));
    return dummy; 
}

float __attribute__((noinline)) opaque_float(float x) { 
    volatile float dummy = x;
    asm volatile("" : "+f"(dummy));
    return dummy; 
}

double __attribute__((noinline)) opaque_double(double x) { 
    volatile double dummy = x;
    asm volatile("" : "+f"(dummy));
    return dummy; 
}

void __attribute__((noinline)) scheduling_barrier(void) {
    /* Memory clobber forces scheduler to save/restore state */
    asm volatile("" ::: "memory");
}

/* Complex state machine with many basic blocks */
void __attribute__((noinline)) scheduling_stress(int iterations) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    volatile double d1 = 1.25, d2 = 2.25, d3 = 3.25;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - forces scheduler to manage loop context */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex switch with many cases creates multiple basic blocks */
        switch (state % 12) {
            case 0: {
                /* Long dependency chain with mixed types */
                a = b + c;
                f1 = (float)a * f2;
                d = (int)f1 ^ e;
                d1 = (double)d * d2;
                c = (int)d1 + a;
                f3 = f1 + f2 + (float)c;
                arr[a & 0xF] = d;
                scheduling_barrier();
                state = (state + (a & 3)) % 12;
                break;
            }
            case 1: {
                /* Another dependency chain */
                b = c * d - e;
                f2 = f1 * 1.7f - f3;
                a = b ^ (c << 2);
                d2 = d1 / 1.3 + d3;
                e = a | (b & 0xFF);
                f1 = opaque_float(f2 + f3);
                arr[b & 0xF] = e;
                scheduling_barrier();
                state = (state + (b & 1)) % 12;
                break;
            }
            case 2: {
                /* Integer-heavy chain */
                c = a + b + d + e;
                d = c * 3 - a;
                e = d >> (b & 3);
                a = e ^ (c & 0xAAAA);
                b = opaque_int(a * 7);
                arr[c & 0xF] = b;
                scheduling_barrier();
                state = (state + (c & 7)) % 12;
                break;
            }
            case 3: {
                /* Floating-point intensive */
                f1 = f2 * 2.3f + f3;
                f3 = f1 / 1.7f - f2;
                f2 = opaque_float(f3 * 3.1f);
                d1 = (double)f1 + d2;
                d3 = d1 * 0.9 - d2;
                a = (int)(f1 + f2 + f3);
                arr[d & 0xF] = a;
                scheduling_barrier();
                state = (state + (a & 3)) % 12;
                break;
            }
            case 4: {
                /* Mixed operations with memory dependencies */
                volatile int idx = (a + b) & 0xF;
                a = arr[idx] + c;
                b = arr[(idx + 1) & 0xF] * d;
                f1 = (float)arr[(idx + 2) & 0xF] * 1.5f;
                arr[idx] = a + b;
                scheduling_barrier();
                state = (state + (idx & 3)) % 12;
                break;
            }
            case 5: {
                /* Complex integer chain */
                e = ((a * b) + (c << 3)) ^ d;
                d = (e >> 2) & 0x3F;
                c = opaque_int(d * 11);
                b = c + a - e;
                a = b | (d & 0xF0F0);
                arr[e & 0xF] = a;
                scheduling_barrier();
                state = (state + (e & 5)) % 12;
                break;
            }
            case 6: {
                /* Double precision chain */
                d1 = d2 * 1.7 + d3;
                d3 = opaque_double(d1 / 2.3);
                d2 = d3 * 0.89 - d1;
                f1 = (float)(d1 + d2 + d3);
                a = (int)(d1 * 100);
                arr[(a >> 1) & 0xF] = a;
                scheduling_barrier();
                state = (state + (a & 3)) % 12;
                break;
            }
            case 7: {
                /* Memory-intensive operations */
                for (int i = 0; i < 8; i++) {
                    volatile int idx = (i + a) & 0xF;
                    arr[idx] = arr[idx] + b + (i * c);
                    b = arr[idx] ^ a;
                }
                scheduling_barrier();
                state = (state + (b & 7)) % 12;
                break;
            }
            case 8: {
                /* Long dependency chain */
                a = b * c + d - e;
                b = (a << 2) | (c >> 1);
                c = opaque_int(b ^ d);
                d = c * 3 + a * 7;
                e = d & 0x7F;
                f1 = (float)(a + b + c + d + e) / 5.0f;
                arr[e & 0xF] = (int)f1;
                scheduling_barrier();
                state = (state + (d & 3)) % 12;
                break;
            }
            case 9: {
                /* Mixed float/int chain */
                f2 = f1 * 3.14f;
                a = (int)f2 + b;
                f3 = (float)a * 0.5f + f2;
                c = opaque_int((int)f3 ^ d);
                d1 = (double)c + d2;
                e = (int)d1 * 2;
                arr[c & 0xF] = e;
                scheduling_barrier();
                state = (state + (c & 3)) % 12;
                break;
            }
            case 10: {
                /* Complex bit operations */
                a = (b & c) | (d ^ e);
                b = (a << 1) + (c >> 2);
                c = opaque_int(b * 13);
                d = (c & 0xAA) | (b & 0x55);
                e = d ^ 0xFF;
                arr[d & 0xF] = e;
                scheduling_barrier();
                state = (state + (e & 7)) % 12;
                break;
            }
            case 11: {
                /* Final complex state */
                f1 = f2 + f3;
                d1 = d2 - d3;
                a = (int)(f1 * 10) + (int)(d1 * 20);
                b = opaque_int(a * c);
                c = b ^ d ^ e;
                d = (c * 3) & 0x3FF;
                e = arr[(d >> 4) & 0xF] + a;
                scheduling_barrier();
                state = (state + (outer & 3)) % 12;
                break;
            }
        }
        
        /* Inner loop with data-dependent trip count */
        int inner_loop = (a + b + c) & 7;
        for (int i = 0; i < inner_loop + 2; i++) {
            volatile int idx = (i * 3 + d) & 0xF;
            int temp = arr[idx];
            arr[idx] = temp + e + (i * a);
            e = arr[idx] ^ b;
            /* Small dependency chain inside inner loop */
            c = c + (temp & 0xF);
            d = opaque_int(d + (c >> 1));
        }
        
        /* Use x86-specific builtin if available */
        #ifdef __x86_64__
        unsigned long long tsc = __builtin_ia32_rdtsc();
        a ^= (int)(tsc & 0xFFFFFFFF);
        #endif
        
        counter++;
    }
    
    /* Force use of all volatile variables to prevent optimization */
    volatile int checksum = a + b + c + d + e + (int)f1 + (int)f2 + (int)f3 
                          + (int)d1 + (int)d2 + (int)d3 + counter;
    
    /* Use checksum to affect control flow */
    if (checksum & 1) {
        state = opaque_int(state + 1);
    }
}

int main(void) {
    srand(time(NULL));
    
    printf("Starting scheduler stress test...\n");
    
    /* Multiple calls to stress scheduler context management */
    for (int rep = 0; rep < 100; rep++) {
        /* Varying iteration counts create different scheduling contexts */
        int iterations = 30 + (rand() % 40);
        scheduling_stress(iterations);
        
        /* Small computation between calls */
        volatile int temp = 0;
        for (int i = 0; i < 100; i++) {
            temp += i * (rep & 0xF);
        }
        
        if (rep % 20 == 0) {
            printf("Completed repetition %d\n", rep);
        }
    }
    
    printf("Scheduler stress test completed.\n");
    return 0;
}
