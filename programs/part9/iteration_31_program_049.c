#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

int opaque_int(int x) {
    volatile int temp = x;
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

/* Helper to create memory dependencies */
void memory_barrier() __attribute__((noinline));
void memory_barrier() {
    asm volatile ("" : : : "memory");
}

/* Complex state machine with scheduling boundaries */
void scheduling_stress(int iter) __attribute__((noinline));
void scheduling_stress(int iter) {
    volatile int state = iter % 10;
    volatile int counter = 0;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    volatile int arr[16];
    
    /* Initialize array with volatile indices */
    for (volatile int i = 0; i < 16; i++) {
        arr[i] = i * iter;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (volatile int outer = 0; outer < 50; outer++) {
        /* Complex switch statement creating many basic blocks */
        switch (state) {
            case 0: {
                /* Long chain of dependent integer operations */
                int t1 = opaque_int(counter);
                int t2 = opaque_int(t1 + arr[0]);
                int t3 = opaque_int(t2 * arr[1]);
                int t4 = opaque_int(t3 >> (arr[2] & 7));
                int t5 = opaque_int(t4 ^ arr[3]);
                int t6 = opaque_int(t5 - arr[4]);
                int t7 = opaque_int(t6 & arr[5]);
                counter = opaque_int(t7 | arr[6]);
                
                /* Mixed floating point operations */
                f1 = opaque_float(f1 + f2);
                f2 = opaque_float(f2 * f3);
                f3 = opaque_float(f3 - f1);
                
                /* Scheduling barrier */
                asm volatile ("" : : : "memory");
                state = (counter + outer) % 10;
                break;
            }
            case 1: {
                /* Different pattern of dependencies */
                int t1 = opaque_int(arr[7] + counter);
                int t2 = opaque_int(t1 - arr[8]);
                int t3 = opaque_int(t2 * 3);
                int t4 = opaque_int(t3 / (arr[9] | 1));
                counter = opaque_int(t4 ^ 0x55AA);
                
                /* Double precision chain */
                d1 = opaque_double(d1 * d2);
                d2 = opaque_double(d2 + d3);
                d3 = opaque_double(d3 - d1);
                
                asm volatile ("" : : : "memory");
                state = (counter ^ outer) % 10;
                break;
            }
            case 2: {
                /* Memory-intensive operations */
                for (volatile int i = 0; i < 8; i++) {
                    arr[i] = opaque_int(arr[i] + arr[i+8]);
                    arr[i+8] = opaque_int(arr[i+8] * (counter + 1));
                }
                
                /* Complex floating chain */
                float ftmp = opaque_float(f1 * f2 + f3);
                f1 = opaque_float(f2 - ftmp);
                f2 = opaque_float(ftmp * f3);
                f3 = opaque_float(f1 / (f2 + 1.0f));
                
                asm volatile ("" : : : "memory");
                state = (arr[0] + outer) % 10;
                break;
            }
            case 3: {
                /* Mixed integer/float dependencies */
                int t1 = opaque_int(counter * arr[1]);
                float ft1 = opaque_float((float)t1 / 7.0f);
                int t2 = opaque_int((int)ft1 ^ arr[2]);
                double dt1 = opaque_double((double)t2 * 0.3);
                counter = opaque_int((int)dt1 + arr[3]);
                
                asm volatile ("" : : : "memory");
                state = (counter * outer) % 10;
                break;
            }
            case 4: {
                /* Use builtin for x86-specific scheduling */
                #ifdef __x86_64__
                unsigned long long tsc = __builtin_ia32_rdtsc();
                counter = opaque_int(counter ^ (tsc & 0xFFFFFFFF));
                #endif
                
                /* Chain with array dependencies */
                for (volatile int i = 0; i < 4; i++) {
                    arr[i] = opaque_int(arr[i] + arr[i+4] + arr[i+8] + arr[i+12]);
                }
                
                asm volatile ("" : : : "memory");
                state = (arr[0] ^ outer) % 10;
                break;
            }
            /* Additional cases to create more basic blocks */
            case 5: {
                int t1 = opaque_int(counter + arr[5]);
                int t2 = opaque_int(t1 * 7);
                counter = opaque_int(t2 >> 2);
                state = (counter + arr[6]) % 10;
                break;
            }
            case 6: {
                f1 = opaque_float(f1 * 1.1f);
                f2 = opaque_float(f2 + 0.5f);
                counter = opaque_int((int)f1 + (int)f2);
                state = (counter & outer) % 10;
                break;
            }
            case 7: {
                d1 = opaque_double(d1 * 0.99);
                d2 = opaque_double(d2 + 0.01);
                counter = opaque_int((int)(d1 * d2));
                state = (counter | arr[7]) % 10;
                break;
            }
            case 8: {
                for (volatile int i = 0; i < 16; i += 2) {
                    arr[i] = opaque_int(arr[i] + counter);
                }
                counter = opaque_int(arr[0] + arr[8]);
                state = (counter % 9) + 1;
                break;
            }
            case 9: {
                /* Complex final state */
                int t1 = opaque_int(counter);
                for (volatile int i = 0; i < 4; i++) {
                    t1 = opaque_int(t1 + arr[i*4]);
                }
                float ft1 = opaque_float((float)t1 * 0.25f);
                double dt1 = opaque_double((double)ft1 * 1.5);
                counter = opaque_int((int)dt1);
                state = 0;
                break;
            }
        }
        
        /* Inner loop with memory dependencies */
        for (volatile int inner = 0; inner < 10; inner++) {
            volatile int idx = (counter + inner) & 15;
            arr[idx] = opaque_int(arr[idx] + inner);
            
            /* Small unrolled section to fill instruction queue */
            arr[(idx+1)&15] = opaque_int(arr[(idx+1)&15] * 2);
            arr[(idx+2)&15] = opaque_int(arr[(idx+2)&15] - 1);
            arr[(idx+3)&15] = opaque_int(arr[(idx+3)&15] ^ 0xFF);
            arr[(idx+4)&15] = opaque_int(arr[(idx+4)&15] >> 1);
        }
        
        /* Call noinline function to force scheduling boundary */
        counter = opaque_int(counter + outer);
    }
    
    /* Final checksum computation */
    volatile int checksum = counter;
    for (volatile int i = 0; i < 16; i++) {
        checksum = opaque_int(checksum ^ arr[i]);
    }
    checksum = opaque_int(checksum + (int)f1 + (int)f2 + (int)d1);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (checksum) : "memory");
}

int main() {
    srand(time(NULL));
    volatile int total = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        scheduling_stress(i);
        total = opaque_int(total + i);
        
        /* Vary control flow to prevent optimization */
        if (i % 7 == 0) {
            memory_barrier();
        }
    }
    
    printf("Result: %d\n", total);
    return total & 255;
}
