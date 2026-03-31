#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static void vla_helper(int size, int iter) {
    volatile int vla[size];  /* VLA forces stack adjustments */
    for (int i = 0; i < size; i++) {
        vla[i] = (i * iter) % 256;
    }
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
}

/* Another helper with mixed operations */
__attribute__((noinline))
static double complex_math(double a, double b, double c, double d) {
    double t1 = a + b;
    double t2 = c - d;
    double t3 = t1 * t2;
    double t4 = sqrt(fabs(t3));
    double t5 = sin(t4);
    return t5 * cos(t3);
}

int main(void) {
    /* Initialize with random data */
    srand(time(NULL));
    
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    double total_sum = 0.0;
    long int_checksum = 0;
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        double chain_result = fp_data[0];
        for (int i = 1; i < 100; i++) {
            /* Long dependency chain with mixed operations */
            chain_result = chain_result + fp_data[i];
            chain_result = chain_result * 1.01;
            chain_result = chain_result - fp_data[i-1];
            chain_result = chain_result / 1.5;
            chain_result = fmod(chain_result, 100.0);
            
            /* Memory access with varying addressing */
            int idx = (i * outer) % ARRAY_SIZE;
            chain_result += int_data[idx] * 0.5;
            
            /* Inline assembly barrier every 10 operations */
            if (i % 10 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        total_sum += chain_result;
        
        /* Call VLA helper between patterns */
        vla_helper(50 + (outer % 20), outer);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 20; i++) {
            int inner_bound = (rand() % INNER_BASE) + 10;  /* Data-dependent */
            for (int j = 0; j < inner_bound; j++) {
                /* Mixed integer and FP operations */
                int idx = (i * 31 + j * 17) % ARRAY_SIZE;
                double temp = fp_data[idx];
                
                /* Integer arithmetic chain */
                int_data[idx] = int_data[idx] + j;
                int_data[idx] = int_data[idx] * 3;
                int_data[idx] = int_data[idx] % 997;
                
                /* FP operations with function calls */
                temp = sqrt(fabs(temp));
                temp = sin(temp) * cos(temp);
                temp = temp + complex_math(temp, fp_data[j], 
                                          fp_data[i], fp_data[idx]);
                
                fp_data[idx] = temp;
                
                /* Memory store with pointer dereference */
                volatile double* ptr = &fp_data[(idx + 1) % ARRAY_SIZE];
                *ptr = temp * 0.99;
            }
            
            /* Another assembly barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (outer == 42);  /* Rare case */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int i = 0; i < 200; i++) {
                /* Alloca inside loop - influences scheduling */
                int* dynamic = (int*)alloca(sizeof(int) * 4);
                for (int k = 0; k < 4; k++) {
                    dynamic[k] = int_data[(i + k) % ARRAY_SIZE];
                }
                
                /* Complex FP chain */
                double a = fp_data[i % ARRAY_SIZE];
                double b = fp_data[(i + 1) % ARRAY_SIZE];
                double c = fp_data[(i + 2) % ARRAY_SIZE];
                
                a = a + b * c;
                b = b - c / a;
                c = c * sqrt(fabs(a + b));
                
                cold_sum += a + b + c + dynamic[0];
                
                /* Barrier in cold path */
                if (i % 25 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            total_sum += cold_sum * 0.01;
        }
        
        /* Pattern 4: Mixed operations with pointer chasing */
        int* ptr1 = int_data;
        double* ptr2 = fp_data;
        for (int i = 0; i < 50; i++) {
            /* Pointer arithmetic with loads/stores */
            int val1 = *ptr1;
            double val2 = *ptr2;
            
            val1 = val1 * 7 + 13;
            val2 = val2 * 1.1 - 0.5;
            
            *ptr1 = val1 % 1000;
            *ptr2 = fmod(val2, 50.0);
            
            ptr1 = &int_data[(ptr1 - int_data + 7) % ARRAY_SIZE];
            ptr2 = &fp_data[(ptr2 - fp_data + 11) % ARRAY_SIZE];
            
            /* Integer division/modulo - expensive operations */
            if (val1 > 0) {
                int_checksum += 1000 / (val1 + 1);
                int_checksum %= 1000000;
            }
        }
        
        /* Final VLA call in each outer iteration */
        vla_helper(30 + (outer % 15), outer * 2);
    }
    
    /* Compute final checksum to prevent elimination */
    double final_result = total_sum;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += fp_data[i] * 0.001;
        final_result += int_data[i] * 0.000001;
    }
    
    printf("Result: %f, Checksum: %ld\n", final_result, int_checksum);
    return 0;
}
