#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
static int use_vla(int size) {
    int vla[size];
    int sum = 0;
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
        sum += vla[i];
    }
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static float dynamic_alloca_math(int n) {
    float* data = (float*)alloca(n * sizeof(float));
    float result = 0.0f;
    
    for (int i = 0; i < n; i++) {
        data[i] = sinf(i * 0.1f);
        result += data[i] * (i + 1);
    }
    return result;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    double double_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    volatile double final_result = 0.0; /* Prevent optimization */
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        {
            int a = int_data[outer % ARRAY_SIZE];
            int b = int_data[(outer + 1) % ARRAY_SIZE];
            int c = int_data[(outer + 2) % ARRAY_SIZE];
            float f1 = float_data[outer % ARRAY_SIZE];
            float f2 = float_data[(outer + 1) % ARRAY_SIZE];
            double d1 = double_data[outer % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            int t1 = a + b;
            int t2 = t1 * c;
            int t3 = t2 % (b + 1);
            int t4 = t3 - a;
            int t5 = t4 / (c + 1);
            
            float ft1 = f1 + f2;
            float ft2 = ft1 * f1;
            float ft3 = ft2 / (f2 + 1.0f);
            float ft4 = sqrtf(fabsf(ft3));
            
            double dt1 = d1 * 2.0;
            double dt2 = dt1 + sin(d1);
            double dt3 = dt2 * cos(d1);
            
            /* Memory operations with varying addressing */
            int_data[(outer + 3) % ARRAY_SIZE] = t5;
            float_data[(outer + 3) % ARRAY_SIZE] = ft4;
            double_data[(outer + 3) % ARRAY_SIZE] = dt3;
            
            final_result += t5 + ft4 + dt3;
        }
        
        /* Call helper with VLA between patterns */
        use_vla((outer % 20) + 10);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
            int local_sum = 0;
            float local_float = 0.0f;
            
            for (int i = 0; i < 5; i++) { /* Outer fixed loop */
                for (int j = 0; j < inner_limit; j++) { /* Data-dependent inner loop */
                    /* Mixed operations within nested loop */
                    int idx = (i * inner_limit + j) % ARRAY_SIZE;
                    local_sum += int_data[idx] * (j + 1);
                    local_float += float_data[idx] * sinf(j * 0.1f);
                    
                    /* Memory store with complex addressing */
                    double_data[idx] = double_data[idx] * 0.99 + 
                                      (double)local_float / (j + 2);
                }
                
                /* Inline assembly barrier - forces scheduler boundary */
                asm volatile ("" ::: "memory");
                
                /* More operations after barrier */
                for (int j = 0; j < inner_limit / 2; j++) {
                    int idx = (i * 10 + j) % ARRAY_SIZE;
                    local_sum -= int_data[idx] % (j + 3);
                    local_float *= 0.9f + float_data[idx] * 0.01f;
                }
            }
            
            final_result += local_sum + local_float;
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        {
            int rare_condition = (rand() % 1000) == 0; /* Rarely true */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_result = 0.0;
                for (int i = 0; i < 100; i++) {
                    cold_result += sqrt(double_data[i % ARRAY_SIZE]) * 
                                  cos(i * 0.01);
                }
                
                /* Another assembly barrier in cold path */
                asm volatile ("" ::: "memory");
                
                /* More cold path computations */
                for (int i = 0; i < 50; i++) {
                    int idx = (outer + i) % ARRAY_SIZE;
                    cold_result -= log(fabs(double_data[idx]) + 1.0);
                }
                
                final_result += cold_result;
                
                /* Call helper with alloca in cold path */
                dynamic_alloca_math(30);
            } else {
                /* Hot path - simpler operations */
                final_result += int_data[outer % ARRAY_SIZE] * 0.01;
            }
        }
        
        /* Pattern 4: Mixed operations with inline assembly barriers */
        {
            /* First computation block */
            float block1_result = 0.0f;
            for (int i = 0; i < 20; i++) {
                block1_result += float_data[(outer + i) % ARRAY_SIZE] * 
                               sinf(i * 0.2f);
            }
            
            /* Assembly barrier between computation blocks */
            asm volatile ("" ::: "memory");
            
            /* Second computation block */
            int block2_result = 0;
            for (int i = 0; i < 15; i++) {
                block2_result += int_data[(outer + i * 2) % ARRAY_SIZE] * 
                               (i % 7 + 1);
            }
            
            /* Another barrier */
            asm volatile ("" ::: "memory");
            
            /* Third computation block with function call */
            double block3_result = 0.0;
            for (int i = 0; i < 10; i++) {
                block3_result += sqrt(fabs(double_data[(outer + i * 3) % ARRAY_SIZE]));
            }
            
            final_result += block1_result + block2_result + block3_result;
        }
        
        /* Final helper call in each outer iteration */
        use_vla(15);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %f\n", final_result);
    
    return 0;
}
