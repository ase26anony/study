#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1) * cos(i * 0.05);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * i - i;
        result += data[i] % 7;
    }
    
    return result;
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* PATTERN 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        double e = array[(outer + 4) % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2)) + sin(e);
        double t4 = t3 * t3 - t2 * t1;
        double t5 = t4 / (a + b + c + d + e);
        
        /* Integer dependency chain mixed in */
        int i1 = (int)t1;
        int i2 = i1 * 3 - 7;
        int i3 = i2 % 13 + (int)t2;
        int i4 = i3 * i2 - i1;
        
        /* Memory operations with varying addressing */
        array[(outer + 5) % ARRAY_SIZE] = t5;
        array[(outer + 6) % ARRAY_SIZE] = (double)i4;
        
        checksum += t5 + i4;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        int inner_bound = rand() % INNER_BASE + 10;
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < inner_bound; ++j) {
                /* Mixed operations with dependencies */
                double* ptr = &array[(i * j) % ARRAY_SIZE];
                double val = *ptr;
                
                val = val * 1.1 + sin(val) * 0.01;
                val = val / (cos(val) + 2.0);
                
                *ptr = val;
                checksum += val;
                
                /* Integer operations in same loop */
                int idx = (i * 17 + j * 23) % ARRAY_SIZE;
                array[idx] += (double)((i + j) % 7);
            }
            
            /* Function call as scheduling barrier */
            double temp = sqrt(fabs(array[i]));
            array[i] = temp;
        }
        
        /* Call helper with VLA */
        checksum += use_vla((outer % 20) + 5);
        
        /* PATTERN 3: Conditional with __builtin_expect */
        int rare_condition = (outer == 42 || outer == 73);
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += array[k] * array[ARRAY_SIZE - 1 - k];
                cold_sum = sqrt(cold_sum + 1.0);
                
                /* More dependency chains */
                double x = cold_sum * 0.5;
                double y = sin(x) * cos(x);
                double z = y * y - x * x;
                
                cold_sum += z / (x + 1.0);
                
                /* Memory barrier in cold path */
                asm volatile ("" ::: "memory");
            }
            checksum += cold_sum;
            
            /* Use alloca in cold path */
            int alloca_res = use_alloca(outer % 30 + 10);
            checksum += alloca_res;
        }
        
        /* PATTERN 4: Another complex block with assembly barriers */
        double* p1 = &array[outer % ARRAY_SIZE];
        double* p2 = &array[(outer + 100) % ARRAY_SIZE];
        double* p3 = &array[(outer + 200) % ARRAY_SIZE];
        
        *p1 = (*p1 + *p2) * (*p3);
        asm volatile ("" ::: "memory");
        
        *p2 = sin(*p1) * cos(*p2);
        asm volatile ("" ::: "memory");
        
        *p3 = sqrt(fabs(*p1 - *p2)) + *p3;
        
        checksum += *p1 + *p2 + *p3;
        
        /* Call helper again */
        checksum += use_vla((outer % 15) + 3);
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        final_result += array[i] * (i % 7);
    }
    final_result += checksum;
    
    printf("Final result: %f\n", final_result);
    
    return 0;
}
