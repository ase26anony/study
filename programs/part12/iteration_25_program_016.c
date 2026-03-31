#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

// Prevent optimization and create dependencies
volatile int global_seed = 42;

// Noinline functions to prevent optimization
__attribute__((noinline)) 
int use_value(int val) {
    return val ^ global_seed;
}

__attribute__((noinline))
void modify_value(int* ptr) {
    *ptr += global_seed;
}

// Test 1: Flow dependency (RAW) with carried dependency
__attribute__((noinline))
int test_flow_dependency(int* arr) {
    int sum = 0;
    // Classic accumulation with flow dependency
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  // Flow dependency on sum across iterations
        arr[i] = sum;   // Additional flow dependency
    }
    return sum;
}

// Test 2: Anti-dependency (WAR) pattern
__attribute__((noinline))
int test_anti_dependency(int* arr) {
    int temp = 0;
    for (int i = 0; i < SIZE; i++) {
        temp = arr[i];      // Read arr[i]
        arr[i] = i * 2;     // Write arr[i] - anti-dependency with previous read
        temp = use_value(temp); // Use temp to prevent elimination
    }
    return temp;
}

// Test 3: Output dependency (WAW) pattern
__attribute__((noinline))
int test_output_dependency(int* arr) {
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;         // First write
        arr[i] = arr[i] * 2; // Second write to same location - output dependency
        modify_value(&arr[i]); // Third write through pointer
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

// Test 4: Mixed dependencies with control flow
__attribute__((noinline))
int test_mixed_dependencies(int* arr) {
    int acc1 = 0, acc2 = 0;
    
    for (int i = 1; i < SIZE; i++) {
        // Flow dependency with distance > 0
        arr[i] = arr[i-1] + i;  // Cross-iteration flow dependency
        
        // Anti-dependency within same iteration
        int temp = arr[i];
        if (i % 3 == 0) {
            arr[i] = acc1;      // WAR: arr[i] written after being read
            acc1 = temp + 1;
        } else if (i % 3 == 1) {
            arr[i] = acc2;      // Another WAR
            acc2 = temp * 2;
        } else {
            // Output dependency
            arr[i] = i;         // First write
            arr[i] = temp + i;  // Second write to same location
        }
    }
    
    return acc1 + acc2;
}

// Test 5: Nested loops with outer-loop dependencies
__attribute__((noinline))
int test_nested_dependencies(int arr[INNER_SIZE][INNER_SIZE]) {
    int sum = 0;
    
    // Outer loop dependency carried to inner loop
    for (int i = 1; i < INNER_SIZE; i++) {
        int row_sum = 0;
        for (int j = 0; j < INNER_SIZE; j++) {
            // Flow dependency from previous row
            arr[i][j] = arr[i-1][j] + (i * j);
            
            // Anti-dependency within inner loop
            int temp = arr[i][j];
            arr[i][j] = row_sum + j;
            row_sum = temp;
            
            // Output dependency in conditional
            if (j % 4 == 0) {
                arr[i][j] = i * 100 + j;
                arr[i][j] = arr[i][j] * 2;
            }
        }
        sum += row_sum;
    }
    
    return sum;
}

// Test 6: Different data types and operations
__attribute__((noinline))
double test_mixed_data_types(double* darr, float* farr, int* iarr) {
    double dsum = 0.0;
    float fsum = 0.0f;
    
    for (int i = 1; i < SIZE; i++) {
        // Flow dependency with different types
        darr[i] = darr[i-1] * 1.01;  // Double precision flow
        
        // Anti-dependency with float
        float ftemp = farr[i];
        farr[i] = darr[i] + i;       // Mixed type operation
        fsum += ftemp;
        
        // Output dependency with int
        iarr[i] = i * 2;
        iarr[i] = (int)(darr[i] + farr[i]);
        
        dsum += darr[i];
    }
    
    return dsum + fsum;
}

// Test 7: Pointer aliasing creates ambiguous dependencies
__attribute__((noinline))
int test_pointer_aliasing(int* arr1, int* arr2) {
    // arr1 and arr2 may alias
    int sum = 0;
    
    for (int i = 1; i < SIZE; i++) {
        // Potential flow dependency if arr1 and arr2 alias
        arr1[i] = arr2[i-1] + i;
        
        // Potential anti-dependency
        int temp = arr2[i];
        arr2[i] = arr1[i] * 2;
        
        sum += temp + arr1[i];
    }
    
    return sum;
}

int main() {
    // Initialize with different patterns
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* iarr = (int*)malloc(SIZE * sizeof(int));
    int nested_arr[INNER_SIZE][INNER_SIZE];
    
    srand(time(NULL));
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 10.0;
        farr[i] = (float)(rand() % 100) / 5.0f;
        iarr[i] = rand() % 100;
    }
    
    for (int i = 0; i < INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            nested_arr[i][j] = rand() % 50;
        }
    }
    
    // Run all tests to trigger various DDG edge creations
    int result = 0;
    
    result += test_flow_dependency(arr1);
    result += test_anti_dependency(arr2);
    result += test_output_dependency(arr1);
    result += test_mixed_dependencies(arr2);
    result += test_nested_dependencies(nested_arr);
    
    double dresult = test_mixed_data_types(darr, farr, iarr);
    result += (int)dresult;
    
    // Test with potential aliasing
    result += test_pointer_aliasing(arr1, arr1 + 1);  // Overlapping arrays
    
    // Final computation to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr1[i] + arr2[i] + iarr[i];
    }
    
    printf("Result: %d, Final sum: %d\n", result, final_sum);
    
    // Cleanup
    free(arr1);
    free(arr2);
    free(darr);
    free(farr);
    free(iarr);
    
    return 0;
}
