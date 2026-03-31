#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.01);
        sum += vla[i];
    }
    
    /* Inline assembly barrier */
    asm volatile ("" ::: "memory");
    
    return sum;
}

/* Another helper with complex operations */
__attribute__((noinline))
static double process_block(double* data, int start, int end) {
    double result = 0.0;
    
    /* Complex dependency chain */
    double a = data[start];
    double b = data[start + 1];
    double c = data[start + 2];
    
    for (int i = start + 3; i < end - 3; ++i) {
        /* Long dependency chain with mixed operations */
        a = b + c;
        b = c * data[i];
        c = sqrt(fabs(a - b));
        
        /* Memory store with pointer arithmetic */
        data[i - 1] = a;
        data[i] = b;
        data[i + 1] = c;
        
        result += a * b / (c + 1.0);
    }
    
    return result;
}

int main() {
    double* array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency-chain basic block */
        double temp = array[0];
        
        /* Chain of dependent FP operations */
        temp = temp * 1.1 + sin(temp);
        temp = sqrt(fabs(temp)) * 0.5;
        temp = temp / (cos(temp) + 2.0);
        temp = exp(log(temp + 1.0));
        temp = temp * temp - temp / 2.0;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* More dependent integer operations */
        int int_temp = (int)temp;
        int_temp = int_temp * 3 + 7;
        int_temp = int_temp % 97 + int_temp / 5;
        int_temp = (int_temp << 3) | (int_temp >> 5);
        
        /* Store result */
        array[outer % ARRAY_SIZE] = temp + int_temp;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = rand() % INNER_BASE + 10;
        
        for (int j = 0; j < inner_bound; ++j) {
            /* Mixed operations within nested loop */
            double* ptr = &array[j];
            
            *ptr = *ptr * 0.99 + sin(j * 0.1);
            ptr[1] = ptr[1] + cos(ptr[0]) * 0.5;
            ptr[2] = sqrt(fabs(ptr[1] - ptr[0]));
            
            /* Integer arithmetic with memory access */
            int index = (j * 7) % ARRAY_SIZE;
            array[index] = array[index] * (1.0 + (j % 3) * 0.1);
            
            /* Function call as scheduling barrier */
            checksum += rand() % 100 * 0.01;
        }
        
        /* Pattern 3: Block with inline assembly barriers */
        {
            double a = array[outer * 3 % ARRAY_SIZE];
            double b = array[(outer * 3 + 1) % ARRAY_SIZE];
            
            a = a + b * 2.5;
            b = b - a * 0.3;
            
            /* Assembly barrier between dependent operations */
            asm volatile ("" ::: "memory");
            
            a = sin(a) * cos(b);
            b = sqrt(a * a + b * b);
            
            asm volatile ("" ::: "memory");
            
            array[(outer * 3) % ARRAY_SIZE] = a;
            array[(outer * 3 + 1) % ARRAY_SIZE] = b;
        }
        
        /* Pattern 4: __builtin_expect with unlikely path */
        int rare_condition = (outer == 42) || (outer == 77);
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operation sequence */
            double cold_sum = 0.0;
            
            for (int k = 0; k < 20; ++k) {
                cold_sum += process_block(array, 
                                         k * 10, 
                                         (k + 1) * 10);
            }
            
            /* More complex FP chain in cold path */
            cold_sum = cold_sum * cold_sum - cold_sum / 3.0;
            cold_sum = sin(cold_sum) * cos(cold_sum * 2.0);
            
            checksum += cold_sum;
            
            /* VLA in cold path */
            double vla_result = use_vla(outer % 20 + 5);
            checksum += vla_result;
        } else {
            /* Hot path - simpler operations */
            checksum += array[outer % ARRAY_SIZE] * 0.5;
        }
        
        /* Call helper with VLA between patterns */
        if (outer % 10 == 0) {
            double vla_res = use_vla(15);
            checksum += vla_res * 0.1;
        }
        
        /* Additional complex block with alloca */
        if (outer % 7 == 0) {
            int dyn_size = (outer % 15) + 5;
            double* dyn_array = (double*)alloca(dyn_size * sizeof(double));
            
            for (int m = 0; m < dyn_size; ++m) {
                dyn_array[m] = array[(outer + m) % ARRAY_SIZE] * m;
                checksum += dyn_array[m];
            }
        }
        
        /* Memory-intensive pattern with varying access patterns */
        for (int n = 0; n < 5; ++n) {
            int stride = (n * 13 + 7) % 17;
            
            for (int idx = 0; idx < ARRAY_SIZE; idx += stride) {
                if (idx + 1 < ARRAY_SIZE) {
                    array[idx] = array[idx] * 0.9 + array[idx + 1] * 0.1;
                }
            }
            
            /* Assembly barrier in memory loop */
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        final_result += array[i] * (i % 3 + 1);
    }
    
    final_result += checksum;
    
    printf("Final result: %f\n", final_result);
    
    free(array);
    return 0;
}
