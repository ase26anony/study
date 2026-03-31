/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative flags: -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops */
/* Or: -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls */

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
    /* Inline assembly with memory clobber to force scheduling boundary */
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
    volatile int g = f - 19;
    volatile int h = g & 0x00FF00FF;
    
    /* Mix with floating point operations */
    volatile float fa = (float)h;
    volatile float fb = fa * 1.5f;
    volatile float fc = fb + 3.14159f;
    volatile int result = (int)fc;
    
    scheduling_barrier();
    return opaque_int(result);
}

/* State machine with complex control flow */
int __attribute__((noinline)) scheduling_stress(int iterations) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int checksum = 0;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex state machine using switch with many cases */
        switch (state % 12) {
            case 0: {
                /* Long dependency chain with mixed operations */
                volatile int t1 = counter * 2;
                volatile int t2 = t1 + arr[counter % 16];
                volatile float ft1 = (float)t2 * f1;
                f1 = ft1 + 0.5f;
                volatile int t3 = (int)f1 ^ t2;
                arr[t3 % 16] = t3;
                checksum += t3;
                state = (t3 & 7) + 1;
                break;
            }
            case 1: {
                volatile int t1 = checksum * 3;
                volatile double dt1 = (double)t1 * d1;
                d1 = dt1 * 0.75;
                volatile int t2 = (int)d1 | t1;
                volatile float ft1 = f2 * 1.1f;
                f2 = ft1 - 0.3f;
                checksum += t2 + (int)f2;
                state = (t2 % 5) + 2;
                break;
            }
            case 2: {
                volatile int t1 = arr[checksum % 16];
                volatile int t2 = t1 << (counter % 4);
                volatile int t3 = t2 - 17;
                volatile float ft1 = (float)t3 / f3;
                f3 = ft1 * 2.0f;
                volatile int t4 = (int)f3 ^ t3;
                checksum += t4;
                state = (t4 & 3) + 3;
                break;
            }
            case 3: {
                volatile int t1 = counter ^ checksum;
                volatile double dt1 = d2 * (double)t1;
                d2 = dt1 + 1.2345;
                volatile int t2 = (int)d2 & t1;
                volatile float ft1 = f1 * f2;
                f1 = ft1 * 0.9f;
                checksum += t2 + (int)f1;
                state = (t2 % 7) + 4;
                break;
            }
            case 4: {
                volatile int t1 = arr[(counter + 3) % 16];
                volatile int t2 = t1 * 13;
                volatile int t3 = t2 >> 1;
                volatile float ft1 = f3 * 1.7f;
                f3 = ft1 - 0.8f;
                volatile int t4 = t3 + (int)f3;
                arr[t4 % 16] = t4;
                checksum += t4;
                state = (t4 & 11) + 5;
                break;
            }
            case 5: {
                volatile int t1 = checksum * 5;
                volatile double dt1 = d3 * (double)t1;
                d3 = dt1 / 1.5;
                volatile int t2 = (int)d3 | t1;
                volatile float ft1 = f2 * f3;
                f2 = ft1 + 1.2f;
                checksum += t2 + (int)f2;
                state = (t2 % 9) + 6;
                break;
            }
            case 6: {
                volatile int t1 = arr[(checksum + 5) % 16];
                volatile int t2 = t1 + 29;
                volatile int t3 = t2 ^ 0x12345678;
                volatile float ft1 = (float)t3 * 0.3f;
                f1 = ft1 + f2;
                volatile int t4 = (int)f1 & t3;
                checksum += t4;
                state = (t4 & 15) + 7;
                break;
            }
            case 7: {
                volatile int t1 = counter * 7;
                volatile double dt1 = (double)t1 + d1;
                d1 = dt1 * 0.88;
                volatile int t2 = (int)d1 ^ t1;
                volatile float ft1 = f3 * 2.5f;
                f3 = ft1 - 1.1f;
                checksum += t2 + (int)f3;
                state = (t2 % 13) + 8;
                break;
            }
            case 8: {
                volatile int t1 = arr[counter % 16];
                volatile int t2 = t1 * 17;
                volatile int t3 = t2 - 41;
                volatile float ft1 = (float)t3 / 3.0f;
                f2 = ft1 * f1;
                volatile int t4 = (int)f2 | t3;
                arr[t4 % 16] = t4;
                checksum += t4;
                state = (t4 & 6) + 9;
                break;
            }
            case 9: {
                volatile int t1 = checksum * 11;
                volatile double dt1 = d2 * (double)t1;
                d2 = dt1 + 2.71828;
                volatile int t2 = (int)d2 & t1;
                volatile float ft1 = f1 * 0.7f;
                f1 = ft1 + 0.4f;
                checksum += t2 + (int)f1;
                state = (t2 % 11) + 10;
                break;
            }
            case 10: {
                volatile int t1 = arr[(checksum + 7) % 16];
                volatile int t2 = t1 << 2;
                volatile int t3 = t2 + 53;
                volatile float ft1 = (float)t3 * 1.3f;
                f3 = ft1 - f2;
                volatile int t4 = (int)f3 ^ t3;
                checksum += t4;
                state = (t4 & 8) + 11;
                break;
            }
            case 11: {
                volatile int t1 = counter * 19;
                volatile double dt1 = (double)t1 / d3;
                d3 = dt1 + 0.57721;
                volatile int t2 = (int)d3 | t1;
                volatile float ft1 = f2 * 0.6f;
                f2 = ft1 + 0.9f;
                checksum += t2 + (int)f2;
                state = (t2 % 17);
                break;
            }
        }
        
        /* Call to noinline function to force scheduling context save/restore */
        if (outer % 7 == 0) {
            checksum = opaque_int(checksum);
            f1 = opaque_float(f1);
            d1 = opaque_double(d1);
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 8; inner++) {
            volatile int idx = (counter + inner) % 16;
            volatile int val = arr[idx];
            volatile int new_val = complex_dependency_chain(val);
            arr[idx] = new_val;
            checksum += new_val;
        }
        
        /* Use inline assembly as scheduling barrier */
        asm volatile ("# Scheduling barrier" : : : "memory");
        
        /* Update counter with data-dependent condition */
        counter += (checksum & 1) ? 1 : 2;
        
        /* More complex dependency chain */
        volatile int tmp = checksum * 3;
        volatile float ftmp = (float)tmp * 0.25f;
        volatile int tmp2 = (int)ftmp ^ tmp;
        checksum = tmp2 + counter;
    }
    
    return checksum;
}

/* Main function with repeated calls to stress the scheduler */
int main(void) {
    srand(time(NULL));
    volatile int total_checksum = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Repeated calls to create multiple scheduling contexts */
    for (int rep = 0; rep < 100; rep++) {
        volatile int seed = rand() % 1000;
        volatile int result = scheduling_stress(50 + (seed % 20));
        total_checksum += result;
        
        /* Occasionally call opaque functions to create scheduling boundaries */
        if (rep % 13 == 0) {
            total_checksum = opaque_int(total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
