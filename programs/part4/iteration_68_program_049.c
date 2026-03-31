/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumping in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 * Or for more aggressive scheduling: gcc -O3 -fsel-sched-pipelining -funroll-loops -march=native -fdump-rtl-all sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
double complex_kernel(double* data, int size, int start) {
    double sum = 0.0;
    double prod = 1.0;
    int i;
    
    /* Nested loops with data dependencies */
    for (i = start; i < size - 1; i++) {
        /* Data dependency: current iteration depends on previous */
        double dep = data[i] * data[i - 1];
        
        /* Mixed-width operations */
        int int_val = (int)dep;
        long long_val = (long)dep * 7LL;
        
        /* Complex conditional with both branches having computation */
        if (int_val % 3 == 0) {
            /* Branch 1: Floating point intensive */
            sum += dep * 2.5;
            prod *= (dep > 0) ? dep : 1.0;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
        } else {
            /* Branch 2: Integer intensive */
            sum += (double)(int_val * 2);
            prod *= (double)(long_val % 100);
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Non-constant division to create complex scheduling */
        if (int_val != 0) {
            sum /= (double)((int_val % 10) + 1);
        }
    }
    
    return sum + prod;
}

/* Matrix-vector multiplication kernel */
void matrix_vector_multiply(double matrix[4][4], double vector[4], double result[4]) {
    int i, j;
    
    #pragma GCC unroll 4
    for (i = 0; i < 4; i++) {
        result[i] = 0.0;
        for (j = 0; j < 4; j++) {
            /* Strided access pattern */
            result[i] += matrix[i][j] * vector[j];
            
            /* Conditional move simulation */
            double temp = (matrix[i][j] > vector[j]) ? matrix[i][j] : vector[j];
            result[i] += temp * 0.1;
        }
        
        /* Volatile dependency in loop bound */
        if (i < volatile_bound) {
            /* More mixed operations */
            result[i] = result[i] * 1.1 - 0.5;
        }
    }
}

/* Pointer chasing pattern */
double pointer_chase(double* array, int size) {
    double* ptr = array;
    double sum = 0.0;
    int steps = size / 2;
    
    while (steps-- > 0) {
        /* Chase pointer with stride */
        sum += *ptr;
        ptr += (*(int*)ptr % 8) + 1;
        
        /* Bound check with volatile */
        if ((ptr - array) >= size) {
            ptr = array;
        }
        
        /* Complex arithmetic with dependency chain */
        sum = sum * 1.01 + 0.001;
    }
    
    return sum;
}

/* Switch statement with multiple computation paths */
double switch_computation(double x, int mode) {
    double result = x;
    
    switch (mode % 5) {
        case 0:
            result = x * x + 2.0 * x + 1.0;
            /* Inline assembly in switch case */
            asm volatile ("" : : : "memory");
            break;
        case 1:
            result = x / (x + 1.0) * 3.14;
            break;
        case 2:
            result = (long)(x * 1000.0) % 100 / 10.0;
            break;
        case 3:
            result = x > 0 ? x * 2.0 : -x * 0.5;
            break;
        case 4:
            result = x * x * x - x * x + x - 1.0;
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
            break;
    }
    
    return result;
}

int main() {
    const int ARRAY_SIZE = 1024;
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double matrix[4][4];
    double vector[4], result[4];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)(rand() % 1000) / 100.0;
    }
    
    /* Initialize matrix and vector */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (double)(rand() % 100) / 10.0;
        }
        vector[i] = (double)(rand() % 100) / 10.0;
    }
    
    double total = 0.0;
    
    /* Main computation loop with volatile bound */
    int iterations = volatile_bound;
    if (iterations > ARRAY_SIZE / 2) {
        iterations = ARRAY_SIZE / 2;
    }
    
    /* Multiple nested loops with different computation patterns */
    for (int outer = 0; outer < 10; outer++) {
        /* Kernel 1: Complex data-dependent computation */
        double kernel1_result = complex_kernel(data, ARRAY_SIZE, outer * 10);
        
        /* Kernel 2: Matrix-vector multiplication */
        matrix_vector_multiply(matrix, vector, result);
        double kernel2_result = 0.0;
        for (int i = 0; i < 4; i++) {
            kernel2_result += result[i];
        }
        
        /* Kernel 3: Pointer chasing */
        double kernel3_result = pointer_chase(data, ARRAY_SIZE);
        
        /* Kernel 4: Switch-based computation */
        double kernel4_result = 0.0;
        for (int i = 0; i < 100; i++) {
            kernel4_result += switch_computation(data[i], i);
        }
        
        /* Combine results with non-trivial reduction */
        total += kernel1_result * 0.3 + kernel2_result * 0.2 + 
                kernel3_result * 0.25 + kernel4_result * 0.25;
        
        /* Modify data slightly for next iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            data[i] *= 1.0001;
        }
        
        /* External function call to prevent optimization */
        if (outer % 3 == 0) {
            volatile_seed = rand();
        }
    }
    
    /* Final reduction with XOR-like pattern */
    long long final_check = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= *(long long*)&data[i];
    }
    
    printf("Result: %f (checksum: %llx)\n", total, final_check);
    
    free(data);
    return 0;
}
