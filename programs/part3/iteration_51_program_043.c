#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper with VLA - forces stack adjustments */
__attribute__((noinline))
static void vla_helper(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" ::: "memory");
}

/* Complex dependency chain in single basic block */
static double complex_block(double* arr, int idx) {
    double a = arr[idx];
    double b = arr[idx + 1];
    double c = arr[idx + 2];
    double d = arr[idx + 3];
    
    /* Long dependency chain */
    double t1 = a + b * c;
    double t2 = t1 / (d + 1.0);
    double t3 = sqrt(fabs(t2));
    double t4 = t3 * sin(t2);
    double t5 = t4 - cos(t3);
    double t6 = t5 * exp(-fabs(t4));
    
    /* Memory barrier */
    asm volatile ("" ::: "memory");
    
    /* More dependencies */
    double t7 = t6 + log(fabs(t5) + 1.0);
    double t8 = t7 * tan(t6 * 0.1);
    double t9 = t8 / (1.0 + pow(t7, 2.0));
    
    return t9;
}

/* Mixed integer/FP operations with dependencies */
static int mixed_operations(int* int_arr, double* dbl_arr, int start) {
    int i1 = int_arr[start];
    int i2 = int_arr[start + 1];
    double d1 = dbl_arr[start];
    double d2 = dbl_arr[start + 1];
    
    /* Integer chain */
    int r1 = i1 * i2 + (i1 % (abs(i2) + 1));
    int r2 = r1 ^ (r1 >> 3);
    int r3 = r2 * 1103515245 + 12345;
    
    /* FP chain dependent on integer results */
    double f1 = d1 * r3;
    double f2 = f1 + d2 * sin(f1);
    double f3 = f2 / (1.0 + fabs(f1));
    
    /* Convert back to integer */
    return (int)(f3 * 1000.0) ^ r3;
}

int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    double* dbl_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dbl_array[i] = (rand() % 1000) / 100.0;
        int_array[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int checksum = 0;
    
    /* Outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        for (int i = 0; i < 10; i++) {
            int idx = (outer * 10 + i) % (ARRAY_SIZE - 10);
            double result = complex_block(dbl_array, idx);
            total_sum += result;
            
            /* Inline assembly barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 20) + 10);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 5; i++) {
            int inner_bound = (rand() % INNER_BASE) + 10;
            
            for (int j = 0; j < inner_bound; j++) {
                /* Mixed operations with memory accesses */
                int idx = (i * j + outer) % (ARRAY_SIZE - 2);
                int res = mixed_operations(int_array, dbl_array, idx);
                checksum ^= res;
                
                /* Complex addressing modes */
                double* ptr1 = &dbl_array[idx];
                double* ptr2 = ptr1 + 1;
                *ptr1 = *ptr1 * 0.99 + *ptr2 * 0.01;
                
                int* iptr = &int_array[idx];
                *iptr = (*iptr + res) % 1000;
            }
            
            /* Another barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 37 || outer == 73); /* Rare cases */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            for (int i = 0; i < ARRAY_SIZE / 2; i++) {
                /* Heavy computation in cold path */
                double a = dbl_array[i];
                double b = dbl_array[ARRAY_SIZE - i - 1];
                dbl_array[i] = sqrt(a * a + b * b);
                
                /* Dependency chain */
                for (int k = 0; k < 3; k++) {
                    dbl_array[i] = sin(dbl_array[i]) * cos(dbl_array[i]);
                }
            }
            checksum += 0x12345678; /* Mark cold path taken */
        }
        
        /* Pattern 4: Alloca within loop */
        {
            int alloca_size = (outer % 16) + 8;
            int* dynamic = (int*)alloca(alloca_size * sizeof(int));
            
            for (int i = 0; i < alloca_size; i++) {
                dynamic[i] = int_array[(outer + i) % ARRAY_SIZE];
                checksum += dynamic[i] * i;
            }
            
            /* Barrier after alloca block */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 5: Function calls as scheduling barriers */
        for (int i = 0; i < 3; i++) {
            int r = rand() % 100;
            total_sum += (double)r / 100.0;
            
            /* Call to external function */
            double s = sqrt(total_sum + 1.0);
            total_sum = fmod(s * 1000.0, 100.0);
        }
        
        /* More VLA usage */
        vla_helper((outer % 15) + 5);
    }
    
    /* Final computation to prevent elimination */
    double final_result = total_sum + checksum;
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f (checksum: %d)\n", final_result, checksum);
    
    free(dbl_array);
    free(int_array);
    
    return 0;
}
