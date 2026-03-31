#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper function with VLA to influence scheduling */
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
        arr[i] = i * 3;
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
        array[i] = (double)rand() / RAND_MAX * 100.0;
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
            a = b + c;
            d = a * e;
            f = d / (c + 1.0);
            a = sqrt(fabs(f)) + sin(d * 0.01);
            b = cos(a) * tan(f * 0.1);
            c = b * b * b - a * a;
            d = c / (a + b + 1.0);
            e = d * d + sqrt(fabs(c));
            f = e * 0.5 + sin(e) * 0.5;
            
            /* Memory store with complex addressing */
            array[outer % ARRAY_SIZE] = f;
            temp += f;
            
            /* Inline assembly barrier */
            asm volatile ("" ::: "memory");
            
            /* Continue dependency chain */
            a = array[(outer + 10) % ARRAY_SIZE];
            b = a * 2.0 + f;
            c = b / (a + 1.0) * 3.14159;
            d = sin(c) * cos(b);
            e = d * d * d - c * c * c;
            f = sqrt(fabs(e)) + log(fabs(d) + 1.0);
            
            temp += f;
        }
        
        /* Call VLA helper between patterns */
        use_vla((outer % 50) + 10);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_bound = (rand() % 50) + 10;  /* Data-dependent */
            int int_sum = 0;
            
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < inner_bound; j++) {
                    /* Mixed integer and FP operations */
                    int idx = (i * j + outer) % ARRAY_SIZE;
                    double val = array[idx];
                    
                    /* Integer arithmetic chain */
                    int k = i * j + outer;
                    k = k * 3 + 7;
                    k = k % 100;
                    k = k * k - j;
                    
                    /* FP operations with dependencies */
                    val = val * 1.1 + sin(val * 0.01);
                    val = val / (k + 1.0);
                    val = sqrt(fabs(val)) + 0.5;
                    
                    array[idx] = val;
                    int_sum += k;
                    temp += val;
                }
                
                /* Another assembly barrier */
                asm volatile ("" ::: "memory");
            }
            
            temp += int_sum;
        }
        
        /* Use alloca */
        temp += use_alloca((outer % 20) + 5);
        
        /* PATTERN 3: __builtin_expect with cold path */
        {
            int rare_condition = (outer == 42);  /* Rare condition */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_temp = 0.0;
                for (int i = 0; i < 100; i++) {
                    double x = array[i];
                    double y = array[ARRAY_SIZE - 1 - i];
                    
                    /* Long dependency chain in cold path */
                    x = x * y + sin(x) * cos(y);
                    y = sqrt(fabs(x)) + log(fabs(y) + 1.0);
                    x = x * x - y * y;
                    y = (x + y) / (x - y + 1.0);
                    
                    cold_temp += x + y;
                    
                    /* Memory operations with pointer arithmetic */
                    double *ptr = &array[i];
                    *ptr = x;
                    *(ptr + 1) = y;
                }
                
                /* Function calls as scheduling barriers */
                cold_temp += (double)rand() / RAND_MAX;
                cold_temp = sqrt(cold_temp);
                cold_temp = sin(cold_temp * 2.0);
                
                temp += cold_temp;
                
                /* Multiple assembly barriers */
                asm volatile ("" ::: "memory");
                asm volatile ("" ::: "memory");
            } else {
                /* Hot path - simpler operations */
                temp += array[outer % ARRAY_SIZE] * 0.5;
            }
        }
        
        /* PATTERN 4: Mixed operations with function calls */
        {
            double x = temp;
            for (int i = 0; i < 20; i++) {
                /* Chain with function calls */
                x = x + (double)rand() / RAND_MAX;
                x = sqrt(fabs(x));
                x = sin(x * 0.1);
                x = x * 2.0 - 1.0;
                
                /* Integer operations mixed in */
                int n = (int)x * 100;
                n = n % 1000;
                n = n * n - i * i;
                
                x += (double)n / 1000.0;
                
                /* Memory store with index calculation */
                int idx = (i + outer * 3) % ARRAY_SIZE;
                array[idx] = x;
            }
            
            temp = x;
        }
        
        checksum += temp;
        
        /* Final assembly barrier in outer loop */
        asm volatile ("" ::: "memory");
    }
    
    /* Use the result to prevent optimization */
    printf("Final checksum: %f\n", checksum);
    
    /* Additional complex exit path */
    if (__builtin_expect(checksum > 1000000.0, 0)) {
        /* Another cold path at exit */
        double exit_temp = 0.0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            exit_temp += sqrt(array[i]) * log(array[i] + 1.0);
        }
        printf("Large checksum detected: %f\n", exit_temp);
    }
    
    return 0;
}
