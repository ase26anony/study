#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Prevent inlining to create scheduling barriers */
__attribute__((noinline)) 
void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    asm volatile ("" ::: "memory");
}

/* Another noinline function with mixed operations */
__attribute__((noinline))
float complex_float_ops(float a, float b, float c, float d) {
    float t1 = a * b + c;
    float t2 = sinf(t1) * d;
    float t3 = sqrtf(fabsf(t2));
    return t3 * t1 - d / (c + 1.0f);
}

int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    double double_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (rand() % 1000) / 100.0f;
        double_data[i] = (rand() % 1000) / 50.0;
    }
    
    volatile float total_result = 0.0f;
    volatile int int_result = 0;
    
    /* Outer loop - driver for scheduling contexts */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        
        /* PATTERN 1: Large basic block with dependency chains */
        {
            float f1 = float_data[outer % ARRAY_SIZE];
            float f2 = float_data[(outer + 1) % ARRAY_SIZE];
            float f3 = float_data[(outer + 2) % ARRAY_SIZE];
            float f4 = float_data[(outer + 3) % ARRAY_SIZE];
            
            /* Long dependency chain */
            float chain1 = f1 * f2 + f3;
            float chain2 = chain1 / (f4 + 0.1f);
            float chain3 = chain2 * chain1 - f3;
            float chain4 = sinf(chain3) * cosf(chain2);
            float chain5 = chain4 + sqrtf(fabsf(chain3));
            float chain6 = chain5 * 2.5f - chain1 / (chain2 + 1.0f);
            float chain7 = chain6 + powf(chain4, 2.0f);
            float chain8 = chain7 * 0.75f + tanf(chain5 * 0.1f);
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
            
            /* Continue chain */
            float chain9 = chain8 / (1.0f + fabsf(chain6));
            float chain10 = chain9 * chain7 - chain5;
            float chain11 = logf(fabsf(chain10) + 1.0f);
            float chain12 = chain11 * chain8 + chain9;
            
            total_result += chain12;
            
            /* Use VLA between patterns */
            use_vla((outer % 20) + 10);
        }
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
            int local_sum = 0;
            float local_float = 0.0f;
            
            for (int i = 0; i < inner_limit; i++) {
                /* Mixed integer operations */
                int idx = (outer * i) % ARRAY_SIZE;
                int val1 = int_data[idx];
                int val2 = int_data[(idx + 1) % ARRAY_SIZE];
                int val3 = int_data[(idx + 2) % ARRAY_SIZE];
                
                int temp1 = val1 * val2 + val3;
                int temp2 = temp1 % (val2 + 1);
                int temp3 = (temp1 * temp2) / (val3 + 1);
                local_sum += temp3 - temp2 + val1;
                
                /* Floating point in same loop */
                float fval1 = float_data[idx];
                float fval2 = float_data[(idx + 3) % ARRAY_SIZE];
                local_float += fval1 * fval2 - sinf(fval1) + sqrtf(fval2);
                
                /* Memory operation with addressing mode variation */
                double_data[idx] = double_data[idx] * 0.99 + local_float * 0.01;
                
                /* Scheduling barrier every 5 iterations */
                if (i % 5 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            int_result += local_sum;
            total_result += local_float;
            
            use_vla(inner_limit % 15 + 5);
        }
        
        /* PATTERN 3: __builtin_expect with cold path */
        {
            int rare_condition = (rand() % 1000) < 2; /* 0.2% probability */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                float cold_result = 0.0f;
                for (int k = 0; k < 30; k++) {
                    float a = float_data[(outer + k) % ARRAY_SIZE];
                    float b = float_data[(outer + k + 10) % ARRAY_SIZE];
                    cold_result += complex_float_ops(a, b, a * b, a / (b + 1.0f));
                    
                    /* Alloca in cold path */
                    volatile int* dyn_array = alloca(sizeof(int) * (k % 5 + 2));
                    for (int m = 0; m < (k % 5 + 2); m++) {
                        dyn_array[m] = m * k + outer;
                    }
                }
                total_result += cold_result * 0.1f;
            } else {
                /* Hot path - simpler operations */
                int_result += int_data[outer % ARRAY_SIZE] * 3;
            }
        }
        
        /* PATTERN 4: Mixed operations with inline assembly barriers */
        {
            double d1 = double_data[outer % ARRAY_SIZE];
            double d2 = double_data[(outer + 10) % ARRAY_SIZE];
            double d3 = double_data[(outer + 20) % ARRAY_SIZE];
            
            /* Group 1 */
            double g1 = d1 * d2 + d3;
            double g2 = g1 / (d2 + 1.0);
            asm volatile ("" ::: "memory");
            
            /* Group 2 */
            double g3 = sin(g1) * cos(g2);
            double g4 = sqrt(fabs(g3)) + g2;
            asm volatile ("" ::: "memory");
            
            /* Group 3 */
            double g5 = g3 * g4 - g1;
            double g6 = log(fabs(g5) + 1.0) * g4;
            asm volatile ("" ::: "memory");
            
            /* Group 4 - memory operations */
            int idx1 = (outer * 7) % ARRAY_SIZE;
            int idx2 = (outer * 13) % ARRAY_SIZE;
            double_data[idx1] = g6 * 0.5;
            double_data[idx2] = g5 * 0.3;
            
            total_result += (float)(g6 + g5);
        }
        
        /* VLA at loop end */
        use_vla((outer % 25) + 1);
    }
    
    /* Final computation to prevent elimination */
    float final_checksum = total_result + int_result * 0.001f;
    
    /* Additional complex block at end */
    {
        volatile float last_calc = 0.0f;
        for (int i = 0; i < 50; i++) {
            float a = float_data[i];
            float b = float_data[ARRAY_SIZE - 1 - i];
            last_calc += a * b - sqrtf(a) + sinf(b) * cosf(a);
            
            /* VLA in final computation */
            volatile int final_vla[i % 10 + 1];
            for (int j = 0; j < i % 10 + 1; j++) {
                final_vla[j] = j * i + (int)last_calc;
            }
        }
        final_checksum += last_calc;
    }
    
    printf("Result: %f\n", final_checksum);
    return 0;
}
