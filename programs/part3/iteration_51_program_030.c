#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define MAX_INNER 50

/* Prevent inlining to create scheduling barriers */
__attribute__((noinline)) 
static void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    asm volatile ("" ::: "memory");
}

/* Another noinline helper with complex operations */
__attribute__((noinline))
static double process_chunk(double *arr, int start, int end) {
    double result = 0.0;
    volatile double temp[end - start];
    
    for (int i = start; i < end; i++) {
        temp[i - start] = arr[i] * 1.5 - sqrt(fabs(arr[i]) + 1.0);
        result += temp[i - start];
    }
    
    return result;
}

int main(void) {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = data[outer % ARRAY_SIZE];
        double b = data[(outer + 1) % ARRAY_SIZE];
        double c = data[(outer + 2) % ARRAY_SIZE];
        double d = data[(outer + 3) % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sin(t2) * cos(t1);
        double t4 = t3 * t3 - sqrt(fabs(t2));
        double t5 = t4 + exp(t3 * 0.1);
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* More dependent integer operations */
        int i1 = (int)t5;
        int i2 = i1 * 3 + 7;
        int i3 = i2 % 17 + (i1 & 0xFF);
        int i4 = i3 * i3 - i2 * 2;
        
        /* Store results back */
        data[outer % ARRAY_SIZE] = t5 + i4;
        checksum += t5;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = rand() % MAX_INNER + 10;
        for (int j = 0; j < inner_bound; j++) {
            int idx = (outer * j) % ARRAY_SIZE;
            
            /* Mixed integer/FP operations */
            double val = data[idx];
            int ival = (int)val;
            
            /* Complex addressing modes */
            data[(idx + 1) % ARRAY_SIZE] += val * 0.5;
            data[(idx + ival) % ARRAY_SIZE] *= 1.01;
            data[idx] = sin(val) + cos(val * 0.5);
            
            /* Integer arithmetic chain */
            ival = ival * 3 + 7;
            ival = ival % 19 + (ival >> 2);
            ival = ival * ival - ival / 3;
            
            checksum += ival * 0.001;
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* 0.01% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_sum += process_chunk(data, k * 10, (k + 1) * 10);
            }
            
            /* More barriers in cold path */
            asm volatile ("" ::: "memory");
            
            /* VLA in cold path */
            int vla_size = rand() % 50 + 10;
            volatile int cold_vla[vla_size];
            for (int k = 0; k < vla_size; k++) {
                cold_vla[k] = k * k - k * 3 + 7;
            }
            
            checksum += cold_sum * 0.01;
        }
        
        /* Pattern 4: Another dependency chain with barriers */
        double x = data[outer % ARRAY_SIZE];
        double y = data[(outer + 100) % ARRAY_SIZE];
        
        x = x * y + sqrt(x * x + y * y);
        asm volatile ("" ::: "memory");
        y = sin(x) * cos(y) + tan(x * 0.1);
        asm volatile ("" ::: "memory");
        x = x / (y + 1.0) + log(fabs(x) + 1.0);
        
        data[outer % ARRAY_SIZE] = x;
        checksum += y;
        
        /* Call VLA helper between patterns */
        use_vla((outer % 20) + 5);
        
        /* Pattern 5: Memory-intensive loop with pointer arithmetic */
        double *ptr = data;
        for (int m = 0; m < ARRAY_SIZE / 4; m++) {
            double v1 = *ptr;
            double v2 = *(ptr + 1);
            double v3 = *(ptr + 2);
            
            /* Dependent operations */
            v1 = v1 * 1.1 + v2 * 0.9;
            v2 = v2 / (v3 + 2.0) * 0.8;
            v3 = sqrt(v1 * v1 + v2 * v2);
            
            *ptr = v1;
            *(ptr + 1) = v2;
            *(ptr + 2) = v3;
            
            ptr += 3;
            checksum += v3;
        }
    }
    
    /* Final computation to prevent elimination */
    double final = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final += data[i] * (i % 7 + 1);
    }
    checksum += final;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
