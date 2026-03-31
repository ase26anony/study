#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    float temp[SIZE];
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float x = data[i];
        float y = x * 2.0f;
        
        /* WAR hazard: write after read */
        data[i] = y + 1.0f;
        
        /* WAW hazard: write after write */
        temp[i] = data[i] * 3.0f;
        temp[i] = temp[i] / 2.0f;  // WAW on temp[i]
        
        sum += temp[i];
    }
    return sum;
}

__attribute__((cold, noinline))
static int cold_function(int *arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                break;
            case 1:
                result += arr[i] >> 1;
                break;
            case 2:
                result += arr[i] & 0xFF;
                break;
            case 3:
                /* Conditional move pattern */
                result += (arr[i] > 0) ? arr[i] : -arr[i];
                break;
            case 4:
                result += arr[i] * arr[i];
                break;
            case 5:
                /* Early continue */
                if (arr[i] < 0) continue;
                result += arr[i];
                break;
            default:
                /* Multiple exit points */
                if (result > 1000000) return result;
                result += 1;
        }
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
    return result;
}

__attribute__((optimize("sched-pressure")))
static void vectorized_loop(float *a, float *b, float *c, int n) {
    /* SIMD-friendly loop with unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Mixed operations that should vectorize */
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) + cosf(b[i]);
        
        /* Pointer chasing pattern */
        float *ptr = &c[i];
        *ptr = t1 + t2;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Additional dependent operation */
        c[i] = c[i] * 0.5f;
    }
}

__attribute__((noinline, optimize("O3")))
static double nested_loop_scheduler_test(void) {
    double matrix[SIZE][SIZE];
    double result = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * 1.5 + j * 0.7) / (i + j + 1);
        }
    }
    
    /* Nested loops with mixed dependencies */
    for (int i = 1; i < SIZE - 1; i++) {
        for (int j = 1; j < SIZE - 1; j++) {
            /* Complex data flow with multiple hazards */
            double north = matrix[i-1][j];
            double south = matrix[i+1][j];
            double east = matrix[i][j+1];
            double west = matrix[i][j-1];
            
            /* RAW on matrix[i][j] */
            double center = matrix[i][j];
            
            /* Multiple dependent computations */
            double avg = (north + south + east + west) / 4.0;
            double diff = center - avg;
            
            /* WAR: read center, then write to it */
            matrix[i][j] = center * 0.9 + avg * 0.1;
            
            /* WAW on result */
            result += diff * diff;
            result = sqrt(result + 1.0);
            
            /* Assembly with register clobber */
            asm volatile("" ::: "%rax", "%rbx", "memory");
        }
    }
    
    return result;
}

__attribute__((optimize("O3")))
static int pointer_chasing_test(int *data, int n) {
    int sum = 0;
    int *current = data;
    
    /* Pointer chasing with computed goto */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, 
                      &&label4, &&label5, &&label6, &&label7 };
    
    for (int i = 0; i < n; i++) {
        int index = data[i] & 0x7;
        goto *labels[index];
        
    label0:
        sum += *current * 2;
        current = &data[(current - data + 1) % n];
        continue;
    label1:
        sum += *current >> 1;
        current = &data[(current - data + 2) % n];
        continue;
    label2:
        sum += *current & 0xFF;
        current = &data[(current - data + 3) % n];
        continue;
    label3:
        sum += (*current > 0) ? *current : -*current;
        current = &data[(current - data + 4) % n];
        continue;
    label4:
        sum += *current * *current;
        current = &data[(current - data + 5) % n];
        continue;
    label5:
        if (*current < 0) {
            current = &data[(current - data + 6) % n];
            continue;
        }
        sum += *current;
        current = &data[(current - data + 7) % n];
        continue;
    label6:
        sum += 1;
        current = &data[(current - data + 8) % n];
        continue;
    label7:
        sum += *current % 13;
        current = &data[(current - data + 9) % n];
        continue;
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    float float_data[SIZE];
    int int_data[SIZE];
    float a[SIZE], b[SIZE], c[SIZE];
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        int_data[i] = rand() % 1000;
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
    }
    
    double total_result = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call all test functions */
        total_result += hot_function(float_data, SIZE);
        total_result += cold_function(int_data, SIZE);
        
        vectorized_loop(a, b, c, SIZE);
        total_result += c[SIZE-1];
        
        total_result += nested_loop_scheduler_test();
        total_result += pointer_chasing_test(int_data, SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            float_data[i] += 0.1f;
            int_data[i] = (int_data[i] + 1) % 1000;
        }
    }
    
    printf("Final result: %f\n", total_result);
    return 0;
}
