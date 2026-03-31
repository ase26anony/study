#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
static void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" : : : "memory");
}

/* Another helper with complex operations */
__attribute__((noinline))
static double complex_math_helper(double a, double b, double c) {
    double t1 = a * b + c;
    double t2 = sqrt(fabs(t1));
    double t3 = sin(t2) * cos(t1);
    return t3 * t3 - t1;
}

int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    volatile int checksum = 0;
    volatile double fp_checksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* PATTERN 1: Large basic block with dependency chains */
        {
            int a = int_data[outer % ARRAY_SIZE];
            int b = int_data[(outer + 1) % ARRAY_SIZE];
            int c = int_data[(outer + 2) % ARRAY_SIZE];
            int d = int_data[(outer + 3) % ARRAY_SIZE];
            double fa = fp_data[outer % ARRAY_SIZE];
            double fb = fp_data[(outer + 1) % ARRAY_SIZE];
            
            /* Integer dependency chain */
            int t1 = a + b;
            int t2 = t1 * c;
            int t3 = t2 - d;
            int t4 = t3 % (abs(c) + 1);
            int t5 = t4 * t1;
            int t6 = t5 / (abs(b) + 1);
            
            /* Floating-point dependency chain */
            double ft1 = fa * fb;
            double ft2 = ft1 + sqrt(fabs(fa));
            double ft3 = ft2 / (fb + 0.1);
            double ft4 = sin(ft3) * cos(ft1);
            double ft5 = ft4 * ft2 - ft3;
            
            /* Mixed operations */
            checksum += t6 + (int)ft5;
            fp_checksum += ft5;
            
            /* Memory store with addressing */
            int_data[outer % ARRAY_SIZE] = t6;
            fp_data[outer % ARRAY_SIZE] = ft5;
        }
        
        /* Insert VLA helper */
        use_vla((outer % 20) + 10);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
            
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < inner_limit; j++) {
                    /* Complex addressing patterns */
                    int idx = (i * 17 + j * 13) % ARRAY_SIZE;
                    int idx2 = (i * 23 + j * 7) % ARRAY_SIZE;
                    
                    /* Mixed operations within loop */
                    int_data[idx] = int_data[idx] * 3 + int_data[idx2];
                    fp_data[idx] = fp_data[idx] * 1.5 - fp_data[idx2];
                    
                    /* Function call as scheduling barrier */
                    if (j % 7 == 0) {
                        fp_data[idx] = complex_math_helper(
                            fp_data[idx], 
                            fp_data[idx2],
                            (double)int_data[idx]
                        );
                    }
                }
                
                /* Inline assembly barrier */
                asm volatile ("" : : : "memory");
            }
        }
        
        /* PATTERN 3: __builtin_expect with cold path */
        {
            int rare_condition = (rand() % 1000) == 0; /* 0.1% probability */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                volatile int cold_sum = 0;
                volatile double cold_fp = 0.0;
                
                for (int i = 0; i < 100; i++) {
                    int idx = (outer + i * 7) % ARRAY_SIZE;
                    
                    /* Long dependency chain in cold path */
                    int val = int_data[idx];
                    val = val * 2 + 1;
                    val = val % 97;
                    val = val * 3 - 2;
                    val = val / (abs(val % 10) + 1);
                    
                    double fval = fp_data[idx];
                    fval = fval * 2.5;
                    fval = sqrt(fabs(fval));
                    fval = sin(fval) * 100;
                    
                    cold_sum += val;
                    cold_fp += fval;
                    
                    /* Another assembly barrier */
                    asm volatile ("" : : : "memory");
                }
                
                checksum += cold_sum;
                fp_checksum += cold_fp;
            } else {
                /* Hot path - simpler operations */
                checksum += int_data[outer % ARRAY_SIZE];
                fp_checksum += fp_data[outer % ARRAY_SIZE];
            }
        }
        
        /* PATTERN 4: Alloca within loop */
        {
            int alloca_size = (outer % 16) + 8;
            int* dyn_array = (int*)alloca(alloca_size * sizeof(int));
            
            for (int i = 0; i < alloca_size; i++) {
                dyn_array[i] = int_data[(outer + i) % ARRAY_SIZE] * i;
                checksum += dyn_array[i];
            }
            
            /* Force spill/reload around alloca */
            asm volatile ("" : : : "memory");
        }
        
        /* PATTERN 5: Mixed operations with memory barriers */
        {
            volatile int* ptr1 = &int_data[outer % ARRAY_SIZE];
            volatile int* ptr2 = &int_data[(outer + 100) % ARRAY_SIZE];
            volatile double* fptr1 = &fp_data[outer % ARRAY_SIZE];
            
            /* Sequence with barriers */
            int temp1 = *ptr1 + *ptr2;
            asm volatile ("" : : : "memory");
            
            double ftemp1 = *fptr1 * 2.0;
            int temp2 = temp1 * 3;
            asm volatile ("" : : : "memory");
            
            double ftemp2 = sqrt(ftemp1) + sin(ftemp1);
            int temp3 = temp2 % 17;
            asm volatile ("" : : : "memory");
            
            *ptr1 = temp3;
            *fptr1 = ftemp2;
            
            checksum += temp3;
            fp_checksum += ftemp2;
        }
    }
    
    /* Final computation to prevent elimination */
    double final_result = (double)checksum + fp_checksum;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f (checksum: %d, fp_checksum: %f)\n", 
           final_result, checksum, fp_checksum);
    
    return (final_result > 0) ? 0 : 1;
}
