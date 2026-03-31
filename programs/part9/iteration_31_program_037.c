/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline));
float opaque_float(float x) __attribute__((noinline));
double opaque_double(double x) __attribute__((noinline));

int opaque_int(int x) {
    volatile int dummy = x;
    return dummy;
}

float opaque_float(float x) {
    volatile float dummy = x;
    return dummy;
}

double opaque_double(double x) {
    volatile double dummy = x;
    return dummy;
}

/* Helper to force memory dependencies */
void memory_barrier() __attribute__((noinline));
void memory_barrier() {
    asm volatile ("" : : : "memory");
}

/* Complex function with mixed operations to stress scheduler */
void scheduling_stress(int iter) __attribute__((noinline));

void scheduling_stress(int iter) {
    volatile int state = iter % 8;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int arr[16];
    volatile int idx1, idx2, idx3;
    
    /* Initialize array with volatile indices */
    for (int i = 0; i < 16; i++) {
        arr[i] = i * iter;
    }
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < 50; outer++) {
        /* State machine using switch with many cases */
        switch (state) {
            case 0: {
                /* Chain of dependent integer operations */
                int t1 = opaque_int(counter + 1);
                int t2 = opaque_int(t1 * 3);
                int t3 = opaque_int(t2 >> 2);
                int t4 = opaque_int(t3 ^ 0xABCD);
                int t5 = opaque_int(t4 + t1);
                counter = opaque_int(t5 - t2);
                
                /* Mixed float operations */
                f1 = opaque_float(f1 * 2.0f + f2);
                f2 = opaque_float(f2 / 1.5f - f3);
                f3 = opaque_float(f3 + f1 * f2);
                
                /* Memory access with volatile indices */
                idx1 = opaque_int(counter % 16);
                idx2 = opaque_int((counter + 3) % 16);
                arr[idx1] = opaque_int(arr[idx1] + arr[idx2]);
                
                /* Scheduling barrier */
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 1) % 8);
                break;
            }
            case 1: {
                /* Different chain of operations */
                int t1 = opaque_int(counter * 7);
                int t2 = opaque_int(t1 & 0xFF);
                int t3 = opaque_int(t2 | 0x80);
                int t4 = opaque_int(t3 << 3);
                int t5 = opaque_int(t4 - t1);
                counter = opaque_int(t5 ^ t3);
                
                /* Double precision operations */
                d1 = opaque_double(d1 + d2 * 0.5);
                d2 = opaque_double(d2 - d3 / 3.0);
                d3 = opaque_double(d3 * 1.1 + d1);
                
                idx1 = opaque_int((counter + 5) % 16);
                idx2 = opaque_int((counter + 7) % 16);
                arr[idx1] = opaque_int(arr[idx1] * arr[idx2]);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 3) % 8);
                break;
            }
            case 2: {
                /* More complex integer chain */
                int t1 = opaque_int(counter + 12345);
                int t2 = opaque_int(t1 % 777);
                int t3 = opaque_int(t2 * t1);
                int t4 = opaque_int(t3 >> 4);
                int t5 = opaque_int(t4 & t2);
                int t6 = opaque_int(t5 | t1);
                counter = opaque_int(t6 ^ t4);
                
                /* Mixed int/float operations */
                f1 = opaque_float(f1 + (float)counter);
                f2 = opaque_float(f2 * f1 - 1.0f);
                f3 = opaque_float(f3 / f2 + 0.5f);
                
                idx1 = opaque_int(counter % 16);
                idx2 = opaque_int((counter + 1) % 16);
                idx3 = opaque_int((counter + 2) % 16);
                arr[idx1] = opaque_int(arr[idx2] + arr[idx3]);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 5) % 8);
                break;
            }
            case 3: {
                /* Use x86-specific builtin if available */
                #ifdef __x86_64__
                unsigned long long tsc = __builtin_ia32_rdtsc();
                counter = opaque_int(counter ^ (tsc & 0xFFFFFFFF));
                #endif
                
                /* Long dependency chain */
                int t1 = opaque_int(counter * 11);
                int t2 = opaque_int(t1 + 999);
                int t3 = opaque_int(t2 / 7);
                int t4 = opaque_int(t3 << 2);
                int t5 = opaque_int(t4 - 888);
                int t6 = opaque_int(t5 & 0xFFFF);
                int t7 = opaque_int(t6 | 0x8000);
                counter = opaque_int(t7 ^ t1);
                
                d1 = opaque_double(d1 * 1.01);
                d2 = opaque_double(d2 + d1);
                d3 = opaque_double(d3 - d2 * 0.25);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 2) % 8);
                break;
            }
            case 4: {
                /* Nested operations */
                int t1 = opaque_int(counter + arr[0]);
                for (int i = 0; i < 4; i++) {
                    t1 = opaque_int(t1 + arr[i] * (i + 1));
                }
                int t2 = opaque_int(t1 * 3);
                int t3 = opaque_int(t2 % 1023);
                counter = opaque_int(t3);
                
                f1 = opaque_float(f1 + f2 * f3);
                f2 = opaque_float(f2 - f1 / 4.0f);
                f3 = opaque_float(f3 * 1.5f + f2);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 7) % 8);
                break;
            }
            case 5: {
                /* Complex memory access pattern */
                for (int i = 0; i < 8; i += 2) {
                    idx1 = opaque_int((counter + i) % 16);
                    idx2 = opaque_int((counter + i + 1) % 16);
                    arr[idx1] = opaque_int(arr[idx1] + arr[idx2] + i);
                }
                
                int t1 = opaque_int(counter * 13);
                int t2 = opaque_int(t1 & 0xF0F0);
                int t3 = opaque_int(t2 | 0x0F0F);
                counter = opaque_int(t3 ^ t1);
                
                d1 = opaque_double(d1 + (double)counter / 100.0);
                d2 = opaque_double(d2 * d1 - 0.1);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 4) % 8);
                break;
            }
            case 6: {
                /* More mixed operations */
                int t1 = opaque_int(counter + 54321);
                int t2 = opaque_int(t1 * 5);
                int t3 = opaque_int(t2 >> 1);
                int t4 = opaque_int(t3 + 1234);
                int t5 = opaque_int(t4 % 555);
                counter = opaque_int(t5);
                
                f1 = opaque_float(f1 * 0.9f + f3);
                f2 = opaque_float(f2 / 1.1f - f1);
                f3 = opaque_float(f3 + f2 * 2.0f);
                
                /* Call to noinline function creates scheduling boundary */
                memory_barrier();
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 6) % 8);
                break;
            }
            case 7: {
                /* Final state with many operations */
                int t1 = opaque_int(counter * 17);
                int t2 = opaque_int(t1 + 32767);
                int t3 = opaque_int(t2 & 0x7FFF);
                int t4 = opaque_int(t3 | 0x8000);
                int t5 = opaque_int(t4 ^ 0xAAAA);
                int t6 = opaque_int(t5 << 1);
                int t7 = opaque_int(t6 >> 2);
                counter = opaque_int(t7);
                
                d1 = opaque_double(d1 * 1.5 + d3);
                d2 = opaque_double(d2 - d1 / 3.0);
                d3 = opaque_double(d3 + d2 * 0.75);
                
                f1 = opaque_float(f1 + (float)d1);
                f2 = opaque_float(f2 * (float)d2);
                
                asm volatile ("" : : : "memory");
                state = opaque_int((state + 1) % 8);
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            idx1 = opaque_int((counter + inner) % 16);
            idx2 = opaque_int((counter + inner * 2) % 16);
            idx3 = opaque_int((counter + inner * 3) % 16);
            
            arr[idx1] = opaque_int(arr[idx1] + arr[idx2] - arr[idx3]);
            counter = opaque_int(counter + arr[idx1] % 100);
        }
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = counter;
    for (int i = 0; i < 16; i++) {
        checksum = opaque_int(checksum ^ arr[i]);
    }
    checksum = opaque_int(checksum + (int)f1 + (int)f2 + (int)f3);
    checksum = opaque_int(checksum + (int)d1 + (int)d2 + (int)d3);
    
    /* Use checksum to affect control flow */
    if (checksum % 7 == 0) {
        state = 0;
    }
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Call scheduling_stress multiple times to increase chance of
       scheduler context saving/restoring */
    for (int i = 0; i < 100; i++) {
        scheduling_stress(i + rand() % 100);
        total += i;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
