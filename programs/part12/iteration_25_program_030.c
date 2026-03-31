/* test_ddg_coverage.c - Program to trigger DDG edge creation in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

/* Prevent optimization of helper functions */
__attribute__((noinline)) 
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i % 100;
    }
}

__attribute__((noinline))
void init_array_float(float *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (float)(i % 100) * 1.5f;
    }
}

/* Volatile pointer to prevent dependency elimination */
volatile int *volatile_ptr;

/* Test 1: Flow dependency (RAW) with carried dependency across iterations */
__attribute__((noinline))
int test_flow_dependency() {
    int array[SIZE];
    init_array(array, SIZE);
    
    int sum = 0;
    /* Classic flow dependency: sum depends on previous iteration */
    for (int i = 0; i < SIZE; i++) {
        sum += array[i];  /* Flow dependency on sum */
    }
    
    /* Additional flow dependency with distance > 0 */
    int result = 0;
    for (int i = 2; i < SIZE; i++) {
        array[i] = array[i-2] + array[i-1];  /* Flow with distance 1 and 2 */
        result += array[i];
    }
    
    return sum + result;
}

/* Test 2: Anti-dependency (WAR) */
__attribute__((noinline))
int test_anti_dependency() {
    int array[SIZE];
    init_array(array, SIZE);
    
    int temp = 0;
    for (int i = 0; i < SIZE - 1; i++) {
        temp = array[i];      /* Read array[i] */
        array[i] = array[i+1]; /* Write to array[i] - anti-dependency with previous read */
        array[i+1] = temp;     /* Write to array[i+1] */
    }
    
    /* Complex anti-dependency with control flow */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        int read_val = array[i];  /* Read */
        if (i % 2 == 0) {
            array[i] = read_val * 2;  /* Write - anti-dependency */
        } else {
            array[i] = read_val / 2;  /* Write - anti-dependency */
        }
        sum += array[i];
    }
    
    return sum;
}

/* Test 3: Output dependency (WAW) */
__attribute__((noinline))
int test_output_dependency() {
    int array[SIZE];
    init_array(array, SIZE);
    
    /* Multiple writes to same location */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;          /* First write */
        array[i] = array[i] * 2; /* Second write - output dependency */
        if (i % 3 == 0) {
            array[i] = array[i] + 5; /* Third write - output dependency */
        }
    }
    
    /* WAW with different data types */
    float farray[SIZE];
    init_array_float(farray, SIZE);
    
    float fsum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        farray[i] = farray[i] * 1.1f;    /* Write 1 */
        farray[i] = farray[i] + 0.5f;    /* Write 2 - output dependency */
        fsum += farray[i];
    }
    
    return (int)fsum;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline))
int test_nested_dependency() {
    int matrix[64][64];
    
    /* Initialize matrix */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    /* Nested loop with flow dependency on outer loop */
    int total = 0;
    for (int i = 1; i < 63; i++) {
        for (int j = 1; j < 63; j++) {
            /* Dependencies on previous iteration of outer loop */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j-1];
            total += matrix[i][j];
        }
    }
    
    /* Another nested pattern with anti-dependency */
    for (int i = 0; i < 63; i++) {
        for (int j = 0; j < 63; j++) {
            int temp = matrix[i][j];          /* Read */
            matrix[i][j] = matrix[i+1][j+1];  /* Write - anti-dependency */
            matrix[i+1][j+1] = temp;          /* Write */
        }
    }
    
    return total;
}

/* Test 5: Mixed dependencies with volatile and pointer aliasing */
__attribute__((noinline))
int test_mixed_dependencies() {
    int data[SIZE];
    int buffer[SIZE];
    init_array(data, SIZE);
    init_array(buffer, SIZE);
    
    /* Use volatile pointer to prevent optimization */
    volatile_ptr = data;
    
    int sum = 0;
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency */
        int flow_val = data[i-1] + 1;
        
        /* Anti-dependency */
        int anti_val = buffer[i];
        buffer[i] = flow_val;
        
        /* Output dependency */
        data[i] = flow_val + anti_val;
        data[i] = data[i] * 2;  /* Second write to same location */
        
        /* Control flow creating complex dependencies */
        if (i % 4 == 0) {
            sum += data[i] + buffer[i];
        } else if (i % 4 == 1) {
            sum += data[i] - buffer[i];
        } else {
            sum += data[i] * buffer[i];
        }
        
        /* Access through volatile pointer */
        sum += *(volatile_ptr + (i % 16));
    }
    
    return sum;
}

/* Test 6: Loop with multiple dependency distances */
__attribute__((noinline))
int test_variable_distance() {
    int array[SIZE * 2];
    init_array(array, SIZE * 2);
    
    int result = 0;
    /* Different dependency distances */
    for (int i = 4; i < SIZE; i++) {
        /* Distance 1 */
        array[i] = array[i-1] + 1;
        
        /* Distance 2 */
        array[i] += array[i-2];
        
        /* Distance 4 */
        if (i % 3 == 0) {
            array[i] += array[i-4];
        }
        
        result += array[i];
    }
    
    /* Another loop with computed indices creating dependencies */
    for (int i = 0; i < SIZE; i++) {
        int idx1 = (i * 7) % SIZE;
        int idx2 = (i * 3) % SIZE;
        int idx3 = (i * 5) % SIZE;
        
        /* Complex dependency pattern */
        array[idx1] = array[idx2] + array[idx3];
        result += array[idx1];
    }
    
    return result;
}

/* Main function that runs all tests */
int main() {
    printf("Starting DDG coverage test...\n");
    
    int total_result = 0;
    
    /* Run each test multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        total_result += test_flow_dependency();
        total_result += test_anti_dependency();
        total_result += test_output_dependency();
        total_result += test_nested_dependency();
        total_result += test_mixed_dependencies();
        total_result += test_variable_distance();
    }
    
    printf("Final checksum: %d\n", total_result);
    printf("DDG coverage test completed.\n");
    
    return total_result != 0 ? 0 : 1;
}
