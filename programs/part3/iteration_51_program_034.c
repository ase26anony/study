#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1) * cos(i * 0.05);
        sum += vla[i];
    }
    
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * i - i;
        sum += data[i] % 17;
    }
    
    return sum;
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
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        double e = array[(outer + 4) % ARRAY_SIZE];
        double f = array[(outer + 5) % ARRAY_SIZE];
        
        /* Long chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2)) + sin(e);
        double t4 = t3 * cos(f) - tan(a);
        double t5 = t4 / (b + c) * exp(t3 * 0.01);
        double t6 = log(fabs(t5) + 1.0) + asin(fmod(f, 1.0));
        double t7 = t6 * t6 - t5 * t4 + t3 * t2;
        double t8 = pow(t7, 1.5) / (t6 + 2.0);
        
        checksum += t8;
        
        /* Inline assembly barrier between operation groups */
        asm volatile("" ::: "memory");
        
        /* More integer operations */
        int i1 = (int)t8;
        int i2 = i1 * 37 - 19;
        int i3 = i2 % 23 + i1 / 7;
        int i4 = i3 << 3 | i2 >> 2;
        int i5 = i4 ^ 0xDEADBEEF;
        
        checksum += i5;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % 50 + 10;
        for (int j = 0; j < inner_limit; ++j) {
            /* Mixed operations within inner loop */
            double* ptr = &array[(outer + j) % ARRAY_SIZE];
            *ptr = *ptr * 1.01 + sin(j * 0.1);
            
            int idx = (int)(*ptr) % ARRAY_SIZE;
            array[idx] = cos(array[idx]) * 0.99;
            
            /* Memory access with complex addressing */
            double temp = array[(idx + j * 7) % ARRAY_SIZE];
            array[(idx + j * 3) % ARRAY_SIZE] = temp * temp - sqrt(fabs(temp));
            
            /* Integer arithmetic chain */
            int k = j * 11;
            k = (k * 13) % 17;
            k = k + (k >> 2) - (k << 1);
            checksum += k * 0.001;
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 10000) == 0; /* Rarely true */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path with complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += sqrt(array[k]) * log(array[k] + 1.0);
                
                /* More dependency chains in cold path */
                double x = cold_sum;
                x = x * x - x;
                x = sin(x) * cos(x * 2.0);
                x = x / (fabs(x) + 1.0);
                cold_sum = x;
            }
            
            /* Inline assembly in cold path */
            asm volatile("" ::: "memory");
            
            checksum += cold_sum * 0.01;
            
            /* Call helper with VLA in cold path */
            double vla_result = use_vla(50 + (rand() % 50));
            checksum += vla_result;
        }
        
        /* Pattern 4: Another complex block with mixed operations */
        float fa = (float)array[outer % ARRAY_SIZE];
        float fb = (float)array[(outer + 10) % ARRAY_SIZE];
        float fc = fa * fb - fa / (fb + 0.5f);
        float fd = sqrtf(fabsf(fc)) * sinf(fa * 0.1f);
        
        /* Integer operations interleaved */
        int ia = (int)fd;
        int ib = ia * ia - ia;
        int ic = ib % 31 + ib / 11;
        int id = ic << 2 | ic >> 3;
        
        checksum += id;
        
        /* Call helper functions between patterns */
        double vla_res = use_vla(30 + (outer % 20));
        checksum += vla_res * 0.001;
        
        int alloca_res = use_alloca(20 + (outer % 15));
        checksum += alloca_res * 0.0001;
        
        /* More memory operations with pointer arithmetic */
        double* ptr1 = &array[outer % ARRAY_SIZE];
        double* ptr2 = &array[(outer + 100) % ARRAY_SIZE];
        double* ptr3 = &array[(outer + 200) % ARRAY_SIZE];
        
        *ptr1 = *ptr1 * 0.99 + *ptr2 * 0.01;
        *ptr2 = *ptr2 * 0.98 + *ptr3 * 0.02;
        *ptr3 = *ptr3 * 0.97 + *ptr1 * 0.03;
        
        /* Final barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Additional loop with function calls as scheduling barriers */
    for (int i = 0; i < 50; ++i) {
        /* Call math functions that act as scheduling barriers */
        double x = checksum * 0.01;
        checksum += sin(x) + cos(x * 2.0) + tan(x * 0.5);
        
        /* More complex addressing */
        int idx = ((int)checksum * 17) % ARRAY_SIZE;
        array[idx] = log(fabs(array[idx]) + 1.0);
        
        /* Use alloca in loop */
        int* tmp = (int*)alloca(sizeof(int) * 10);
        for (int j = 0; j < 10; ++j) {
            tmp[j] = (i * j) % 19;
            checksum += tmp[j] * 0.00001;
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
