#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" ::: "memory");
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int *arr = (int*)alloca(n * sizeof(int));
    int sum = 0;
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100;
        sum += arr[i];
    }
    return sum;
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)(rand() % 10000) / 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        double temp = 0.0;
        
        /* PATTERN 1: Large dependency chain basic block */
        {
            double a = array[outer % ARRAY_SIZE];
            double b = array[(outer + 1) % ARRAY_SIZE];
            double c = array[(outer + 2) % ARRAY_SIZE];
            double d = array[(outer + 3) % ARRAY_SIZE];
            double e = array[(outer + 4) % ARRAY_SIZE];
            double f = array[(outer + 5) % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            double t1 = a + b * c;
            double t2 = t1 / (d + 1.0);
            double t3 = sqrt(fabs(t2)) + sin(e);
            double t4 = t3 * t3 - f * 2.0;
            double t5 = t4 / (a + b + 0.001);
            double t6 = cos(t5) * exp(-fabs(t4));
            double t7 = t6 + log(fabs(t3) + 1.0);
            double t8 = t7 * t7 - t5 * t4;
            
            temp += t8;
            
            /* Memory store with complex addressing */
            array[(outer * 7) % ARRAY_SIZE] = t8;
        }
        
        /* Call helper with VLA between patterns */
        use_vla((rand() % 50) + 10);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_bound = (rand() % 50) + 10;  /* Data-dependent */
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < inner_bound; j++) {
                    /* Mixed integer and FP operations */
                    int idx = (i * 17 + j * 13) % ARRAY_SIZE;
                    double val = array[idx];
                    
                    /* Integer arithmetic chain */
                    int int_val = (int)val;
                    int_val = int_val * 3 + 7;
                    int_val = int_val % 97;
                    int_val = int_val * int_val - 49;
                    
                    /* FP operations dependent on integer result */
                    val = val * (double)int_val + sin(val);
                    val = sqrt(fabs(val)) + cos((double)j);
                    
                    array[idx] = val;
                    temp += val / (double)(j + 1);
                }
            }
        }
        
        /* PATTERN 3: Block with inline assembly barriers */
        {
            double x = array[outer % ARRAY_SIZE];
            double y = array[(outer + 10) % ARRAY_SIZE];
            
            /* First computation group */
            x = x * x + y * y;
            y = x / (y + 0.001);
            
            /* Scheduling barrier */
            asm volatile ("" ::: "memory");
            
            /* Second group - dependent on first */
            x = sin(x) + cos(y);
            y = exp(-fabs(x * y));
            
            /* Another barrier */
            asm volatile ("" ::: "memory");
            
            /* Third group */
            x = x * 2.0 - y * 3.0;
            y = sqrt(fabs(x)) + log(fabs(y) + 1.0);
            
            temp += x + y;
            array[(outer * 13) % ARRAY_SIZE] = x;
        }
        
        /* Call helper with alloca */
        temp += (double)use_alloca((rand() % 30) + 5);
        
        /* PATTERN 4: __builtin_expect with cold path */
        {
            int rare_condition = (rand() % 10000) == 0;  /* Rarely true */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_temp = 0.0;
                for (int i = 0; i < 20; i++) {
                    double a = array[(outer + i) % ARRAY_SIZE];
                    double b = array[(outer + i + 20) % ARRAY_SIZE];
                    
                    /* Heavy dependency chain in cold path */
                    for (int k = 0; k < 5; k++) {
                        a = a * b + sin(a);
                        b = b / (a + 0.001) + cos(b);
                        a = sqrt(fabs(a)) + log(fabs(b) + 1.0);
                        b = exp(-fabs(a * b)) * 2.0;
                    }
                    
                    cold_temp += a + b;
                    array[(outer + i) % ARRAY_SIZE] = a;
                }
                temp += cold_temp * 0.01;
            } else {
                /* Hot path - simpler operations */
                temp += array[outer % ARRAY_SIZE] * 0.5;
            }
        }
        
        /* More complex operations with function calls */
        {
            double *ptr = &array[outer % ARRAY_SIZE];
            for (int i = 0; i < 3; i++) {
                *ptr = *ptr + sin(*ptr) * cos((double)i);
                ptr = &array[(outer + i * 7) % ARRAY_SIZE];
                
                /* Integer arithmetic mixed with FP */
                int int_part = (int)(*ptr * 100.0);
                int_part = (int_part * 13 + 7) % 101;
                *ptr = *ptr + (double)int_part / 100.0;
            }
        }
        
        checksum += temp;
        
        /* Occasionally call VLA helper */
        if (outer % 23 == 0) {
            use_vla((rand() % 40) + 15);
        }
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += array[i] * (i % 7 + 1);
    }
    final_result += checksum;
    
    printf("Result: %f\n", final_result);
    
    return 0;
}
