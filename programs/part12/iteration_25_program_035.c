#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

/* Prevent optimization and create dependencies */
volatile int volatile_var = 0;

/* Functions to prevent optimization */
__attribute__((noinline)) int use_value(int val) {
    volatile_var = val;
    return val + 1;
}

__attribute__((noinline)) void modify_array(int* arr, int idx, int val) {
    arr[idx] = val;
    volatile_var = idx;
}

/* Test 1: Flow (RAW) dependency - cumulative sum pattern */
__attribute__((noinline)) int test_flow_dependency() {
    int array[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i + 1;
    }
    
    /* Flow dependency: sum depends on previous iteration's sum */
    for (int i = 0; i < SIZE; i++) {
        sum += array[i];  // RAW: read array[i], write sum
        array[i] = sum;   // WAW on array[i] from initialization
    }
    
    /* Additional flow dependency with distance > 0 */
    int result = 0;
    for (int i = 2; i < SIZE; i++) {
        array[i] = array[i-2] + array[i-1];  // Distance 1 and 2 flow dependencies
        result += array[i];
    }
    
    return result + sum;
}

/* Test 2: Anti (WAR) dependency */
__attribute__((noinline)) int test_anti_dependency() {
    int a[SIZE], b[SIZE];
    int temp;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
    }
    
    /* Anti-dependency: read then write to same location through different names */
    for (int i = 0; i < SIZE - 1; i++) {
        temp = a[i];        // Read a[i]
        a[i] = b[i] + 1;    // Write a[i] - WAR with previous read
        b[i] = temp * 2;    // Write b[i]
    }
    
    /* Complex anti-dependency with pointer aliasing */
    int* p1 = &a[10];
    int* p2 = &b[10];
    for (int i = 0; i < 100; i++) {
        int val = *p1;      // Read through p1
        *p2 = val + i;      // Write through p2 (potential WAR if p1 == p2)
        p1++;
        p2++;
    }
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i];
    }
    return sum;
}

/* Test 3: Output (WAW) dependency */
__attribute__((noinline)) int test_output_dependency() {
    int array[SIZE];
    int result = 0;
    
    /* Multiple writes to same location */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;               // Write 1
        array[i] = array[i] * 2;    // Write 2 - WAW
        array[i] = array[i] + 1;    // Write 3 - WAW
        
        /* Conditional WAW */
        if (i % 3 == 0) {
            array[i] = array[i] * 3;
        } else if (i % 3 == 1) {
            array[i] = array[i] / 2;
        } else {
            array[i] = array[i] + 100;
        }
    }
    
    /* WAW with different data types */
    float farray[SIZE];
    for (int i = 0; i < SIZE; i++) {
        farray[i] = i * 1.5f;
        farray[i] = farray[i] * 2.0f;  // WAW on float
        result += (int)farray[i];
    }
    
    return result;
}

/* Test 4: Nested loops with dependencies */
__attribute__((noinline)) int test_nested_dependency() {
    int matrix[N][N];
    int sum = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * N + j;
        }
    }
    
    /* Nested loop with flow dependency across outer iterations */
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < N; j++) {
            /* Flow dependency on previous row */
            matrix[i][j] = matrix[i][j] + matrix[i-1][j];
        }
    }
    
    /* Another nested pattern with anti-dependency */
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N; j++) {
            int temp = matrix[i][j];
            matrix[i][j] = matrix[i+1][j] * 2;
            matrix[i+1][j] = temp + 1;
        }
    }
    
    /* Calculate checksum */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += matrix[i][j];
        }
    }
    
    return sum;
}

/* Test 5: Mixed dependencies with control flow */
__attribute__((noinline)) int test_mixed_dependencies() {
    int data[SIZE];
    int accum = 0;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
    }
    
    /* Loop with mixed dependencies and control flow */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency */
        int prev = data[i-1];
        
        /* Control flow creates complex dependency graph */
        if (i % 4 == 0) {
            data[i] = prev * 2;          // Flow
            accum += data[i];            // Anti on accum
        } else if (i % 4 == 1) {
            accum = data[i] + accum;     // RAW and WAW on accum
            data[i] = accum;             // WAW on data[i]
        } else if (i % 4 == 2) {
            int temp = data[i];
            data[i] = data[i-1];         // WAW on data[i]
            data[i-1] = temp;            // WAW on data[i-1]
        } else {
            data[i] = data[i] * 3;       // WAW on data[i]
            accum = accum - data[i];     // RAW on accum, RAW on data[i]
        }
    }
    
    return accum;
}

/* Test 6: Pointer-based dependencies with aliasing */
__attribute__((noinline)) int test_pointer_dependencies() {
    int buffer[2 * SIZE];
    int* p1 = &buffer[0];
    int* p2 = &buffer[SIZE/2];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 2 * SIZE; i++) {
        buffer[i] = i;
    }
    
    /* Pointer chasing with dependencies */
    for (int i = 0; i < SIZE; i++) {
        /* Potential aliasing creates complex dependencies */
        *p1 = *p2 + 1;      // Flow from p2 to p1
        sum += *p1;         // Anti on sum, Flow from p1
        p1++;
        p2++;
        
        /* Interleaved dependencies */
        if (i % 2 == 0) {
            buffer[i] = buffer[i] + sum;  // WAW on buffer[i]
        } else {
            sum = buffer[i] * 2;          // WAW on sum
        }
    }
    
    return sum;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    printf("Running DDG edge creation tests...\n");
    
    /* Run all tests to trigger various DDG edge creations */
    total += test_flow_dependency();
    total += test_anti_dependency();
    total += test_output_dependency();
    total += test_nested_dependency();
    total += test_mixed_dependencies();
    total += test_pointer_dependencies();
    
    printf("Total checksum: %d\n", total);
    printf("Volatile var: %d\n", volatile_var);
    
    return total % 256;  /* Return non-zero to indicate execution */
}
