/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline));
float opaque_float(float x) __attribute__((noinline));
double opaque_double(double x) __attribute__((noinline));
void scheduling_barrier(void) __attribute__((noinline));

/* Helper to create complex data dependencies */
int opaque_int(int x) {
    volatile int v = x;
    /* Create backend scheduling complexity */
    asm volatile("" : "+r" (v) : : "memory");
    return v + 1;
}

float opaque_float(float x) {
    volatile float v = x;
    asm volatile("" : "+f" (v) : : "memory");
    return v * 1.001f;
}

double opaque_double(double x) {
    volatile double v = x;
    asm volatile("" : "+f" (v) : : "memory");
    return v * 1.0001;
}

void scheduling_barrier(void) {
    /* Force scheduler to save/restore context */
    asm volatile("" : : : "memory");
}

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));
int scheduling_stress(int seed) {
    volatile int state = seed % 8;
    volatile int counter = 0;
    volatile int a = seed, b = seed + 1, c = seed + 2;
    volatile float f1 = seed * 0.5f, f2 = seed * 0.25f;
    volatile double d1 = seed * 0.125, d2 = seed * 0.0625;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow with switch statement */
        switch (state) {
            case 0: {
                /* Chain of dependent integer operations */
                a = b + c;
                b = a * outer;
                c = b >> 3;
                a = c - outer;
                b = a ^ 0xABCD;
                c = b * 7;
                a = opaque_int(c);
                
                /* Mixed type operations */
                f1 = a * 0.5f;
                f2 = opaque_float(f1 + f2);
                d1 = f2 * 0.25;
                d2 = opaque_double(d1 + d2);
                
                /* Memory dependencies with volatile array */
                volatile int idx = (a + b) % 16;
                arr[idx] = arr[(idx + 1) % 16] + c;
                scheduling_barrier();
                state = (arr[idx] % 7) + 1;
                break;
            }
            case 1: {
                /* Different operation chain */
                a = b * c;
                b = a / (outer + 1);
                c = b << 2;
                a = c | 0xFF;
                b = a + 12345;
                c = b % 777;
                a = opaque_int(c * 2);
                
                f1 = b * 0.333f;
                f2 = f1 * f2;
                d1 = opaque_double(f2);
                d2 = d1 * d2 * 0.99;
                
                volatile int idx = (b + c) % 16;
                arr[(idx + 3) % 16] = arr[idx] * 2;
                scheduling_barrier();
                state = (a + outer) % 8;
                break;
            }
            case 2: {
                a = (b << 3) | (c >> 2);
                b = a ^ c;
                c = b + (outer * 17);
                a = c - b;
                b = opaque_int(a * 3);
                c = b / 5;
                a = c * 7;
                
                f2 = opaque_float(f1 * 2.0f);
                f1 = f2 + 1.5f;
                d2 = d1 * 1.1;
                d1 = opaque_double(d2 / 1.01);
                
                volatile int idx = c % 16;
                arr[idx] = arr[(idx + 5) % 16] ^ a;
                scheduling_barrier();
                state = (b + arr[idx]) % 8;
                break;
            }
            case 3: {
                a = b & c;
                b = a | 0x1234;
                c = opaque_int(b + outer);
                a = c * 11;
                b = a >> 1;
                c = b % 19;
                a = c + 255;
                
                f1 = opaque_float(f2 - 0.5f);
                f2 = f1 * 3.14159f;
                d1 = d2 + 2.71828;
                d2 = opaque_double(d1 * 0.7071);
                
                volatile int idx = (a * b) % 16;
                arr[(idx + 7) % 16] = arr[idx] + outer;
                scheduling_barrier();
                state = (c + idx) % 8;
                break;
            }
            case 4: {
                a = (b * c) + outer;
                b = opaque_int(a);
                c = b ^ a;
                a = c << 3;
                b = a >> 2;
                c = b % 23;
                a = opaque_int(c + 4096);
                
                f2 = f1 / 2.0f;
                f1 = opaque_float(f2 * 1.414f);
                d2 = d1 / 1.732;
                d1 = opaque_double(d2 + 0.577);
                
                volatile int idx = (b + outer) % 16;
                arr[idx] = arr[(idx + 11) % 16] | b;
                scheduling_barrier();
                state = (a + outer * 3) % 8;
                break;
            }
            case 5: {
                a = b + (c * 2);
                b = opaque_int(a - outer);
                c = b & 0xFFFF;
                a = c | 0x8000;
                b = a * 13;
                c = opaque_int(b % 29);
                a = c ^ 0xDEAD;
                
                f1 = opaque_float(f2 + 1.618f);
                f2 = f1 * 0.618f;
                d1 = opaque_double(d2 * 1.4142);
                d2 = d1 / 3.1416;
                
                volatile int idx = (c + a) % 16;
                arr[(idx + 13) % 16] = arr[idx] & a;
                scheduling_barrier();
                state = (b + arr[idx]) % 8;
                break;
            }
            case 6: {
                a = (b % 31) + c;
                b = opaque_int(a * outer);
                c = b ^ 0xBEEF;
                a = c + 65535;
                b = a >> 4;
                c = opaque_int(b);
                a = c * 3;
                
                f2 = opaque_float(f1 - 2.5f);
                f1 = f2 * 0.4f;
                d2 = d1 + 1.2345;
                d1 = opaque_double(d2 * 0.8765);
                
                volatile int idx = (a + c + outer) % 16;
                arr[idx] = arr[(idx + 9) % 16] ^ c;
                scheduling_barrier();
                state = (a + b + c) % 8;
                break;
            }
            case 7: {
                a = b * c + outer * 7;
                b = opaque_int(a);
                c = b % 37;
                a = c << 5;
                b = opaque_int(a >> 3);
                c = b + 1024;
                a = c ^ 0xCAFE;
                
                f1 = opaque_float(f2 * 2.718f);
                f2 = f1 / 1.414f;
                d1 = opaque_double(d2);
                d2 = d1 * 0.6931;
                
                volatile int idx = (b * c) % 16;
                arr[(idx + 15) % 16] = arr[idx] + a;
                scheduling_barrier();
                state = (outer + arr[idx]) % 8;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx1 = (a + inner) % 16;
            volatile int idx2 = (b + inner * 3) % 16;
            volatile int idx3 = (c + inner * 7) % 16;
            
            arr[idx1] = arr[idx2] + arr[idx3];
            arr[idx2] = arr[idx1] ^ inner;
            arr[idx3] = arr[idx2] * (inner + 1);
            
            /* Create floating-point dependencies */
            f1 = f1 + arr[idx1] * 0.01f;
            f2 = opaque_float(f2 - arr[idx2] * 0.005f);
            d1 = d1 + arr[idx3] * 0.001;
            d2 = opaque_double(d2 * 0.999);
        }
        
        /* Periodically call opaque functions to force scheduling boundaries */
        if (outer % 7 == 0) {
            a = opaque_int(a + b);
            f1 = opaque_float(f1 + f2);
            d1 = opaque_double(d1 + d2);
        }
        
        counter++;
    }
    
    /* Compute final checksum from all volatile variables */
    int checksum = a + b + c + (int)f1 + (int)f2 + (int)d1 + (int)d2;
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i];
    }
    checksum ^= counter;
    
    return checksum;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler's context saving/restoring */
    for (int i = 0; i < 100; i++) {
        int seed = rand();
        int result = scheduling_stress(seed);
        total_checksum ^= result;
        
        /* Print progress occasionally */
        if (i % 25 == 0) {
            printf("Iteration %d, checksum so far: %d\n", i, total_checksum);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
