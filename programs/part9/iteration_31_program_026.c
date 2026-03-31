#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, noipa));
float opaque_float(float x) __attribute__((noinline, noipa));
double opaque_double(double x) __attribute__((noinline, noipa));

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

/* Memory barrier function */
void memory_barrier(void) __attribute__((noinline, noipa));
void memory_barrier(void) {
    asm volatile ("" : : : "memory");
}

/* Complex helper function that forces scheduler to save context */
int complex_helper(volatile int a, volatile float b, volatile double c) 
    __attribute__((noinline, noipa));

int complex_helper(volatile int a, volatile float b, volatile double c) {
    int r1 = a + (int)b;
    float r2 = b * (float)c;
    double r3 = c / (double)a;
    
    /* Mixed operations create scheduling complexity */
    int r4 = r1 ^ (int)r2;
    float r5 = r2 + (float)r3;
    double r6 = r3 - (double)r4;
    
    /* Inline assembly as scheduling barrier */
    asm volatile ("# Complex helper barrier" : : : "memory");
    
    return r4 + (int)r5 + (int)r6;
}

/* State machine implementation */
void scheduling_stress(void) __attribute__((noinline, noipa));

void scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int var1 = 1, var2 = 2, var3 = 3, var4 = 4;
    volatile float fvar1 = 1.5f, fvar2 = 2.5f, fvar3 = 3.5f;
    volatile double dvar1 = 1.25, dvar2 = 2.25, dvar3 = 3.25;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 2;
    }
    
    /* Outer loop - creates scheduling region */
    for (int outer = 0; outer < 50; outer++) {
        /* Update state based on complex condition */
        state = (var1 + var2 * var3 - var4) % 10;
        
        /* Switch with many cases - creates multiple basic blocks */
        switch (state) {
            case 0: {
                /* Long dependency chain */
                var1 = var2 + var3;
                var2 = var1 * var4;
                var3 = var2 >> var1;
                var4 = var3 ^ var1;
                fvar1 = fvar2 + (float)var1;
                fvar2 = fvar1 * fvar3;
                dvar1 = dvar2 / (double)var2;
                var1 = (int)(fvar1 + fvar2) + var3;
                
                /* Call helper to force scheduling boundary */
                var2 = complex_helper(var1, fvar1, dvar1);
                
                /* Memory barrier */
                asm volatile ("# Case 0 barrier" : : : "memory");
                break;
            }
            case 1: {
                /* Different dependency pattern */
                fvar1 = opaque_float(fvar2 + fvar3);
                fvar2 = fvar1 * 2.0f;
                var1 = (int)fvar1 + var2;
                var2 = var1 * var3;
                dvar1 = dvar2 + dvar3;
                dvar2 = dvar1 * 2.0;
                var3 = (int)dvar1 ^ var4;
                
                /* Array access with volatile index */
                volatile int idx = var1 % 16;
                var4 = arr[idx] + var2;
                
                asm volatile ("# Case 1 barrier" : : : "memory");
                break;
            }
            case 2: {
                /* Mixed integer/float operations */
                var1 = var2 * var3 + var4;
                fvar1 = (float)var1 / fvar2;
                var2 = (int)(fvar1 * 100.0f);
                dvar1 = (double)var2 / dvar2;
                fvar2 = (float)dvar1 * fvar3;
                var3 = opaque_int(var2 + (int)fvar2);
                
                /* Complex condition for state transition */
                if ((var1 ^ var2) > (var3 | var4)) {
                    state = 3;
                }
                
                asm volatile ("# Case 2 barrier" : : : "memory");
                break;
            }
            case 3: {
                /* Another dependency chain */
                dvar1 = dvar2 * dvar3;
                var1 = (int)dvar1 + var2;
                fvar1 = fvar2 - fvar3;
                var2 = var1 * (int)fvar1;
                dvar2 = dvar1 / 3.14159;
                var3 = var2 ^ var4;
                
                /* Call helper */
                var4 = complex_helper(var1, fvar1, dvar1);
                
                asm volatile ("# Case 3 barrier" : : : "memory");
                break;
            }
            case 4: {
                /* Bit manipulation chain */
                var1 = var2 & var3;
                var2 = var1 | var4;
                var3 = var2 ^ var1;
                var4 = ~var3;
                fvar1 = (float)(var1 + var2 + var3 + var4);
                dvar1 = (double)fvar1 * dvar2;
                
                asm volatile ("# Case 4 barrier" : : : "memory");
                break;
            }
            case 5: {
                /* Floating point intensive */
                fvar1 = fvar2 * fvar3;
                fvar2 = fvar1 / 2.0f;
                fvar3 = fvar2 + fvar1;
                dvar1 = (double)fvar3 * dvar3;
                dvar2 = dvar1 - dvar3;
                var1 = (int)(fvar1 + fvar2 + fvar3);
                
                asm volatile ("# Case 5 barrier" : : : "memory");
                break;
            }
            case 6: {
                /* Memory intensive */
                for (int i = 0; i < 8; i++) {
                    volatile int idx1 = (var1 + i) % 16;
                    volatile int idx2 = (var2 + i) % 16;
                    arr[idx1] = arr[idx2] + var3;
                }
                var1 = arr[var1 % 16];
                var2 = arr[var2 % 16];
                
                asm volatile ("# Case 6 barrier" : : : "memory");
                break;
            }
            case 7: {
                /* Long chain with calls */
                var1 = opaque_int(var2);
                var2 = opaque_int(var3);
                var3 = opaque_int(var4);
                var4 = opaque_int(var1);
                fvar1 = opaque_float(fvar2);
                fvar2 = opaque_float(fvar3);
                
                asm volatile ("# Case 7 barrier" : : : "memory");
                break;
            }
            case 8: {
                /* Complex arithmetic */
                var1 = (var2 * var3) / (var4 + 1);
                var2 = (var1 << 3) | (var3 >> 2);
                fvar1 = (float)var1 * (float)var2;
                dvar1 = (double)fvar1 / dvar3;
                var3 = (int)dvar1 % 256;
                
                asm volatile ("# Case 8 barrier" : : : "memory");
                break;
            }
            case 9: {
                /* Mixed operations with barrier */
                var1 = var2 + var3;
                memory_barrier();
                var2 = var1 * var4;
                memory_barrier();
                fvar1 = fvar2 + (float)var2;
                memory_barrier();
                dvar1 = dvar2 * (double)fvar1;
                
                asm volatile ("# Case 9 barrier" : : : "memory");
                break;
            }
            default: {
                /* Default case with computation */
                var1 = var2 - var3;
                var2 = var4 * var1;
                fvar1 = fvar3 - fvar2;
                dvar1 = dvar3 + dvar2;
                
                asm volatile ("# Default barrier" : : : "memory");
                break;
            }
        }
        
        /* Inner loop with data-dependent condition */
        int inner_limit = (var1 % 8) + 2;
        for (int inner = 0; inner < inner_limit; inner++) {
            /* Dependent operations in inner loop */
            volatile int idx = (var1 + inner) % 16;
            arr[idx] = arr[idx] + var2 + inner;
            var3 = var3 ^ arr[idx];
            fvar1 = fvar1 + (float)arr[idx];
        }
        
        /* Update counter with complex condition */
        counter = (counter + var1 + var2 + var3 + (int)fvar1) % 1000;
        
        /* Occasionally call helper */
        if ((outer % 7) == 0) {
            var4 = complex_helper(var1, fvar1, dvar1);
        }
    }
    
    /* Final computation to use all variables */
    volatile int result = var1 + var2 + var3 + var4 + 
                         (int)fvar1 + (int)fvar2 + (int)fvar3 +
                         (int)dvar1 + (int)dvar2 + (int)dvar3;
    
    /* Prevent dead code elimination */
    asm volatile ("# Final result: %0" : : "r"(result));
}

int main(void) {
    srand(time(NULL));
    
    printf("Starting scheduling stress test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        if ((i % 10) == 0) {
            printf("Iteration %d\n", i);
        }
    }
    
    printf("Scheduling stress test completed.\n");
    return 0;
}
