#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper functions to prevent inlining */
__attribute__((noinline)) void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    asm volatile ("" ::: "memory");
}

__attribute__((noinline)) double complex_fp_chain(double a, double b, double c) {
    double t1 = a + b * c;
    double t2 = sqrt(fabs(t1)) + sin(b);
    double t3 = t2 * t2 - a * c;
    return t3 / (b + 1.0);
}

/* Main computation with scheduling-intensive patterns */
int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    long long checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 3.14;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large basic block with dependency chains */
        double fp_acc = 0.0;
        int int_acc = 0;
        
        for (int i = 0; i < 50; i++) {
            /* Integer dependency chain */
            int idx = (i * 7) % ARRAY_SIZE;
            int a = int_data[idx];
            int b = int_data[(idx + 1) % ARRAY_SIZE];
            int c = a + b * 3;
            int d = c - (a % 7);
            int e = d * d + b;
            int f = e / (a + 1) + (c % 13);
            int_acc += f;
            
            /* Floating-point dependency chain */
            double fa = fp_data[idx];
            double fb = fp_data[(idx + 2) % ARRAY_SIZE];
            double fc = fa * fb + sqrt(fabs(fa));
            double fd = fc / (fb + 2.0) - sin(fa);
            double fe = fd * fd + fa * fb;
            fp_acc += fe;
            
            /* Memory store with addressing computation */
            int_data[idx] = (int_acc % 1000) ^ (int)(fp_acc * 100);
        }
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = (rand() % 40) + 10;  /* 10-49 iterations */
        for (int i = 0; i < inner_loops; i++) {
            int base = rand() % (ARRAY_SIZE - 100);
            
            /* Mixed operations in inner loop */
            for (int j = 0; j < (rand() % 20 + 5); j++) {
                /* Pointer arithmetic with multiple dereferences */
                int* ptr1 = &int_data[base + j];
                int* ptr2 = &int_data[base + j + 1];
                double* fptr = &fp_data[base + j];
                
                *ptr1 = (*ptr1 + *ptr2) * (j % 7 + 1);
                *fptr = (*fptr) * 0.99 + sin((double)j * 0.1);
                
                /* Complex integer math */
                int temp = *ptr1;
                temp = (temp << 3) | (temp >> 5);
                temp = temp % 997 + (temp & 0xFF);
                *ptr1 = temp ^ (j * 31);
            }
        }
        
        /* VLA helper call - influences stack/scheduling */
        use_vla((outer % 20) + 10);
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 1000) == 0;  /* 0.1% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path: complex operations rarely executed */
            double cold_acc = 0.0;
            for (int i = 0; i < 200; i++) {
                /* Heavy FP chain with function calls */
                cold_acc += complex_fp_chain(
                    fp_data[i % ARRAY_SIZE],
                    fp_data[(i * 3) % ARRAY_SIZE],
                    fp_data[(i * 7) % ARRAY_SIZE]
                );
                
                /* Integer chain with divisions (expensive) */
                int k = int_data[i % ARRAY_SIZE];
                k = (k * 13) % 997;
                k = k / (k % 17 + 1);
                k = k + (k % 31) * (k % 7);
                int_data[i % ARRAY_SIZE] = k;
            }
            
            /* Another assembly barrier in cold path */
            asm volatile ("" ::: "memory");
            
            checksum += (long long)(cold_acc * 1000);
        } else {
            /* Hot path: simpler operations */
            for (int i = 0; i < 20; i++) {
                int idx = (outer * i) % ARRAY_SIZE;
                int_data[idx] = (int_data[idx] + 1) % 1000;
                fp_data[idx] = fp_data[idx] * 0.999;
            }
        }
        
        /* Pattern 4: Alloca within loop */
        if (outer % 3 == 0) {
            int alloca_size = (outer % 10) * 8 + 16;
            int* dynamic = (int*)alloca(alloca_size * sizeof(int));
            
            for (int i = 0; i < alloca_size; i++) {
                dynamic[i] = int_data[i % ARRAY_SIZE] + i;
                checksum += dynamic[i];
            }
            
            /* Barrier after alloca */
            asm volatile ("" ::: "memory");
        }
        
        /* Final accumulation for checksum */
        for (int i = 0; i < 10; i++) {
            checksum += int_data[(outer + i) % ARRAY_SIZE];
            checksum += (long long)(fp_data[(outer + i) % ARRAY_SIZE] * 100);
        }
    }
    
    /* Use alloca one more time */
    volatile int* final_block = (int*)alloca(64 * sizeof(int));
    for (int i = 0; i < 64; i++) {
        final_block[i] = int_data[i % ARRAY_SIZE] ^ i;
        checksum += final_block[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
