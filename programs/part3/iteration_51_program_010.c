#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        vla[i] = sin(i * 0.1) * cos(i * 0.05);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int *arr = (int *)alloca(n * sizeof(int));
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        arr[i] = i * i - i;
        total += arr[i] % 17;
    }
    
    asm volatile ("" ::: "memory");
    return total;
}

int main(void) {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large basic block with dependency chains */
        double a = data[outer % ARRAY_SIZE];
        double b = data[(outer + 1) % ARRAY_SIZE];
        double c = data[(outer + 2) % ARRAY_SIZE];
        double d = data[(outer + 3) % ARRAY_SIZE];
        double e = data[(outer + 4) % ARRAY_SIZE];
        double f = data[(outer + 5) % ARRAY_SIZE];
        
        /* Complex FP dependency chain */
        double t1 = a + b * c;
        double t2 = sqrt(fabs(t1)) + d;
        double t3 = t2 * e - f / (a + 1.0);
        double t4 = sin(t3) * cos(t2);
        double t5 = t4 + log(fabs(t1) + 1.0);
        double t6 = t5 * t3 / (t2 + 0.001);
        
        /* Integer dependency chain mixed in */
        int i1 = (int)(t1 * 100);
        int i2 = i1 * 3 - (int)t2;
        int i3 = i2 % 17 + (int)t3;
        int i4 = i3 * i2 - i1;
        int i5 = i4 / (abs(i3) + 1) + (int)t4;
        
        /* Memory operations with varying addressing */
        data[(outer + i5) % ARRAY_SIZE] = t6;
        checksum += t6 * 0.01 - i5 * 0.001;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % INNER_BASE + 10;
        for (int j = 0; j < inner_limit; j++) {
            int idx = (outer * j) % ARRAY_SIZE;
            
            /* Mixed operations within loop */
            double val = data[idx];
            data[idx] = val * 1.1 + sin(val) * 0.1;
            
            /* Integer operations */
            int mod = (j * 17) % 31;
            data[(idx + mod) % ARRAY_SIZE] += 0.5;
            
            /* Another dependency chain */
            double x = data[idx];
            double y = data[(idx + 1) % ARRAY_SIZE];
            data[(idx + 2) % ARRAY_SIZE] = x * y - sqrt(fabs(x - y));
            
            checksum += data[idx] * 0.0001;
        }
        
        /* Call helper with VLA - influences scheduling */
        double vla_result = use_vla((outer % 20) + 5);
        checksum += vla_result * 0.001;
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 1000) == 0; /* 0.1% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            
            /* Another large dependency chain */
            for (int k = 0; k < 25; k++) {
                double x = data[(outer + k * 7) % ARRAY_SIZE];
                double y = data[(outer + k * 13) % ARRAY_SIZE];
                
                cold_sum += x * y - sqrt(x * x + y * y);
                cold_sum = sin(cold_sum) * 0.9 + cos(cold_sum) * 0.1;
                
                /* Memory barrier in cold path */
                if (k % 5 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            data[outer % ARRAY_SIZE] = cold_sum;
            checksum += cold_sum * 0.01;
            
            /* Call alloca helper in cold path */
            int alloca_res = use_alloca((outer % 15) + 3);
            checksum += alloca_res * 0.00001;
        }
        
        /* Pattern 4: More complex operations with assembly barriers */
        double acc = 0.0;
        for (int m = 0; m < 10; m++) {
            int idx1 = (outer + m * 11) % ARRAY_SIZE;
            int idx2 = (outer + m * 19) % ARRAY_SIZE;
            
            acc += data[idx1] * data[idx2];
            acc = acc / (1.0 + fabs(acc));
            
            /* Barrier every 3 iterations */
            if (m % 3 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        checksum += acc;
        
        /* More integer operations to stress scheduler */
        int int_acc = 0;
        for (int n = 0; n < 8; n++) {
            int base = (outer + n) % ARRAY_SIZE;
            int_acc += (int)data[base] * n;
            int_acc = int_acc % 97 + (int_acc / 13);
            
            /* Pointer arithmetic */
            double *ptr = &data[base];
            *ptr = *ptr * 0.99 + int_acc * 0.01;
        }
        
        checksum += int_acc * 0.000001;
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += data[i] * (i % 7);
    }
    
    checksum += final_result;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
