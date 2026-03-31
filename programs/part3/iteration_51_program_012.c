#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline)) 
void vla_helper(int size, int seed) {
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (i * seed) % 256;
    }
    
    /* Use the VLA to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += vla[i];
    }
    (void)sum;
}

/* Complex dependency chain with mixed operations */
__attribute__((noinline))
double complex_dependency_chain(double* arr, int idx) {
    double a = arr[idx];
    double b = arr[idx + 1];
    double c = arr[idx + 2];
    double d = arr[idx + 3];
    
    /* Long dependency chain */
    double t1 = a + b * c;
    double t2 = sqrt(fabs(t1)) + sin(d);
    double t3 = t2 * t2 - a * b;
    double t4 = t3 / (c + 1.0);
    double t5 = t4 * exp(-fabs(t3));
    
    /* Inline assembly as scheduling barrier */
    asm volatile ("" ::: "memory");
    
    /* Continue the chain */
    double t6 = t5 + cos(t4) * sin(t3);
    double t7 = t6 * t6 - t5 * t4;
    double t8 = t7 / (t6 + 2.0);
    
    /* Another scheduling barrier */
    asm volatile ("" ::: "memory");
    
    double t9 = t8 + log(fabs(t7) + 1.0);
    double t10 = t9 * t8 - t7 * t6;
    
    return t10;
}

/* Function with unlikely path using __builtin_expect */
__attribute__((noinline))
double unlikely_path_operation(double* arr, int idx, int threshold) {
    double result = arr[idx];
    
    /* Hot path - usually taken */
    if (__builtin_expect(result < threshold, 1)) {
        for (int i = 0; i < 5; ++i) {
            result = result * 1.1 + arr[idx + i];
        }
    } 
    /* Cold path - complex operations */
    else {
        /* This creates a separate scheduling region */
        double a = result;
        double b = arr[idx + 1];
        double c = arr[idx + 2];
        
        /* Complex FP operations in cold path */
        for (int i = 0; i < 10; ++i) {
            a = sin(a) + cos(b) * tan(c);
            b = sqrt(fabs(a)) + log(fabs(b) + 1.0);
            c = a * b - c / (a + 1.0);
            
            /* Memory barrier in cold path */
            if (i % 3 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        result = a + b + c;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = (double)(rand() % 1000) / 10.0;
        int_data[i] = rand() % 1000;
    }
    
    double checksum = 0.0;
    
    /* Outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        
        /* Pattern 1: Large basic block with dependency chains */
        for (int i = 0; i < ARRAY_SIZE - 10; i += 10) {
            double val1 = complex_dependency_chain(data, i);
            double val2 = complex_dependency_chain(data, i + 5);
            checksum += val1 * val2 - val1 / (val2 + 1.0);
        }
        
        /* Use VLA helper between patterns */
        vla_helper(outer % 100 + 10, outer);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 20; ++i) {
            int inner_bound = (rand() % INNER_BASE) + 10;
            
            for (int j = 0; j < inner_bound; ++j) {
                /* Mixed integer and FP operations */
                int idx = (i * j) % (ARRAY_SIZE - 5);
                
                /* Integer arithmetic chain */
                int int_val = int_data[idx];
                int_val = int_val * 3 + int_data[idx + 1];
                int_val = int_val / 2 - int_data[idx + 2];
                int_val = int_val % 100 + int_data[idx + 3];
                
                /* Convert to FP and continue chain */
                double fp_val = (double)int_val;
                fp_val = fp_val * data[idx] + sqrt(fabs(data[idx + 1]));
                fp_val = fp_val / (data[idx + 2] + 1.0) - sin(data[idx + 3]);
                
                checksum += fp_val;
                
                /* Store result back */
                data[idx] = fp_val;
            }
            
            /* Scheduling barrier in inner loop */
            if (i % 5 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Pattern 3: Operations with unlikely paths */
        for (int i = 0; i < ARRAY_SIZE - 5; i += 5) {
            /* Occasionally trigger the cold path */
            int threshold = (outer == OUTER_LOOPS - 1) ? -1000 : 900;
            double val = unlikely_path_operation(data, i, threshold);
            checksum += val;
        }
        
        /* Pattern 4: Alloca within loop */
        {
            int alloca_size = (outer % 20) + 5;
            int* dynamic = (int*)alloca(alloca_size * sizeof(int));
            
            for (int i = 0; i < alloca_size; ++i) {
                dynamic[i] = int_data[(outer + i) % ARRAY_SIZE] * i;
            }
            
            /* Use the alloca'd memory in computation */
            int temp_sum = 0;
            for (int i = 0; i < alloca_size; ++i) {
                temp_sum += dynamic[i];
            }
            checksum += (double)temp_sum / alloca_size;
        }
        
        /* Pattern 5: Function calls as scheduling barriers */
        for (int i = 0; i < 10; ++i) {
            int idx = (outer * i) % (ARRAY_SIZE - 3);
            
            /* Call rand() as scheduling barrier */
            int r = rand() % 100;
            
            /* Dependent operations after barrier */
            double base = data[idx];
            for (int j = 0; j < 5; ++j) {
                base = base * 1.5 + sin((double)r * j);
                base = sqrt(fabs(base)) + cos((double)r / (j + 1));
            }
            
            checksum += base;
        }
    }
    
    /* Final VLA usage */
    vla_helper(50, (int)checksum);
    
    printf("Final checksum: %f\n", checksum);
    
    free(data);
    free(int_data);
    
    return 0;
}
