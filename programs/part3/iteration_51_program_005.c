#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - forces stack adjustments */
__attribute__((noinline))
static void vla_helper(int size, int iter) {
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (i * iter) % 256;
    }
    /* Use the VLA to prevent optimization */
    asm volatile ("" : : "r"(vla[size/2]) : "memory");
}

/* Another helper with mixed operations */
__attribute__((noinline))
static double compute_complex(double a, double b, double c) {
    double t1 = a * b + c;
    double t2 = sin(t1) * cos(b);
    double t3 = sqrt(fabs(t2)) + 1.0;
    return t3 * t3 - t2;
}

int main(void) {
    int i, j, k;
    double sum = 0.0;
    long int_checksum = 0;
    
    /* Initialize arrays with random data */
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; ++i) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency-chain basic block */
        double chain_result = fp_array[0];
        for (k = 1; k < 20; ++k) {
            /* Long dependency chain */
            chain_result = chain_result * 1.1 + fp_array[k];
            chain_result = sqrt(fabs(chain_result)) + 0.5;
            chain_result = sin(chain_result) * cos(fp_array[k-1]);
            chain_result = chain_result / (1.0 + fabs(fp_array[k]));
        }
        sum += chain_result;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = (rand() % INNER_BASE) + 10;
        for (j = 0; j < inner_bound; ++j) {
            /* Mixed integer and FP operations */
            int idx = (j * outer) % ARRAY_SIZE;
            double temp = fp_array[idx] * 2.0;
            
            /* Integer arithmetic chain */
            int val = int_array[idx];
            val = (val * 3 + 7) % 100;
            val = (val << 2) | (val >> 6);
            val = val * val - val;
            
            /* Memory store with addressing mode */
            int_array[(idx + 1) % ARRAY_SIZE] = val;
            fp_array[(idx + 2) % ARRAY_SIZE] = temp + sin(val);
            
            /* Function call as scheduling barrier */
            temp = compute_complex(temp, val, sum);
            sum += temp * 0.01;
        }
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 20) + 10, outer);
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 10000) == 0; /* Rare condition */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (k = 0; k < 100; ++k) {
                /* Alloca inside cold path */
                int* dyn = (int*)alloca(sizeof(int) * 10);
                for (int m = 0; m < 10; ++m) {
                    dyn[m] = k * m + outer;
                }
                
                /* More complex math */
                cold_sum += sqrt(k * outer + 1.0);
                cold_sum *= 1.0001;
                
                /* Memory operations */
                int_array[(k + outer) % ARRAY_SIZE] = dyn[k % 10];
            }
            sum += cold_sum * 0.001;
            
            /* Another assembly barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 4: Mixed operations with varying types */
        for (k = 0; k < 15; ++k) {
            /* Integer arithmetic with dependencies */
            int a = int_array[(outer + k) % ARRAY_SIZE];
            int b = a * 3 - 7;
            int c = b % 13 + 5;
            int d = c * c - b;
            int e = d / (abs(c) + 1);
            
            /* Floating-point with dependencies */
            double x = fp_array[(outer + k + 1) % ARRAY_SIZE];
            double y = x * 1.5 + sin(x);
            double z = y / (cos(x) + 2.0);
            double w = sqrt(fabs(z)) * 0.5;
            
            /* Store results */
            int_array[(outer + k + 2) % ARRAY_SIZE] = e;
            fp_array[(outer + k + 3) % ARRAY_SIZE] = w;
            
            /* Update checksums */
            int_checksum += e;
            sum += w;
        }
        
        /* Final VLA call in loop */
        vla_helper(25, outer * 2);
    }
    
    /* Additional complex block at end */
    double final_result = 0.0;
    for (i = 0; i < 50; ++i) {
        /* Long dependency chain */
        final_result = final_result + fp_array[i % ARRAY_SIZE];
        final_result = final_result * 1.01 - 0.5;
        final_result = sin(final_result) + cos(final_result * 0.5);
        
        /* Integer operations intertwined */
        int_checksum = (int_checksum * 1103515245 + 12345) & 0x7fffffff;
        int_array[i % ARRAY_SIZE] = int_checksum % 1000;
    }
    sum += final_result;
    
    /* Ensure result is used */
    printf("Final sum: %f, Checksum: %ld\n", sum, int_checksum);
    
    return 0;
}
