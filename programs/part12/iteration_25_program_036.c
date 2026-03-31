#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

// Prevent inlining to maintain dependencies
__attribute__((noinline)) 
int test_flow_dependency(int* arr, int n) {
    volatile int sum = 0;
    // Flow dependency: sum from previous iteration used in current iteration
    for (int i = 0; i < n; i++) {
        sum += arr[i];  // RAW: read arr[i], write sum (carried across iterations)
    }
    return sum;
}

__attribute__((noinline))
int test_anti_dependency(int* arr1, int* arr2, int n) {
    // Anti-dependency: read then write to same location
    for (int i = 0; i < n - 1; i++) {
        int temp = arr1[i];      // Read arr1[i]
        arr1[i] = arr2[i];       // Write arr1[i] - WAR with previous read
        arr2[i] = temp;          // Additional dependency chain
    }
    return arr1[n-1] + arr2[n-1];
}

__attribute__((noinline))
void test_output_dependency(volatile int* arr, int n) {
    // Output dependency: multiple writes to same location
    for (int i = 0; i < n; i++) {
        arr[i] = i;          // First write
        arr[i] = i * 2;      // Second write - WAW with first
        arr[i] = arr[i] + 1; // Third write with flow dependency
    }
}

__attribute__((noinline))
int test_nested_dependency(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    // Nested loops with cross-iteration dependencies
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // Flow dependency on previous i iteration
            arr[i][j] = arr[i-1][j] + arr[i][j];
            sum += arr[i][j];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_dependencies(int* arr, float* farr, double* darr, int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    // Mixed data types with dependencies
    for (int i = 1; i < n; i++) {
        // Integer flow dependency
        int_sum += arr[i];
        arr[i] = int_sum;
        
        // Float anti-dependency
        float temp = farr[i];
        farr[i] = float_sum;
        float_sum = temp + float_sum;
        
        // Double output dependency
        darr[i] = double_sum;
        darr[i] = darr[i] * 1.5;
        double_sum = darr[i];
    }
    return int_sum + (int)float_sum + (int)double_sum;
}

__attribute__((noinline))
int test_control_flow_dependency(int* arr, int n) {
    int sum = 0;
    // Control flow creates complex dependencies
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            // Flow dependency through sum
            sum += arr[i];
            arr[i] = sum;
        } else {
            // Anti-dependency: read then modify
            int temp = arr[i];
            arr[i] = sum;
            sum = temp;
        }
        
        // Additional output dependency
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2;  // WAW with previous write
        }
    }
    return sum;
}

__attribute__((noinline))
int test_pointer_aliasing(int* restrict a, int* restrict b, int* c, int n) {
    // Potential aliasing creates conservative dependencies
    int sum = 0;
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + 1;      // Flow in b[i]
        c[i] = a[i] * 2;      // Flow from a[i]
        sum += c[i];
        
        // Potential WAR if b and c alias
        b[i] = sum % 256;
    }
    return sum;
}

__attribute__((noinline))
int test_reduction_with_dependency(double* arr, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    // Multiple accumulators with dependencies
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        prod *= (sum + 1.0);  // Flow dependency on sum
        arr[i] = prod;        // Output dependency
    }
    return (int)(sum + prod);
}

int main() {
    // Initialize with different patterns
    int arr1[SIZE];
    int arr2[SIZE];
    volatile int varr[SIZE];
    int matrix[SIZE][SIZE];
    float farr[SIZE];
    double darr[SIZE];
    
    srand(time(NULL));
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        varr[i] = i;
        farr[i] = (float)rand() / RAND_MAX;
        darr[i] = (double)rand() / RAND_MAX;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    int total = 0;
    
    // Execute all test functions
    total += test_flow_dependency(arr1, N);
    total += test_anti_dependency(arr1, arr2, N);
    test_output_dependency(varr, N);
    total += test_nested_dependency(matrix, 32);  // Smaller size for matrix
    
    // Mixed type test
    total += test_mixed_dependencies(arr1, farr, darr, N);
    
    // Control flow test
    total += test_control_flow_dependency(arr2, N);
    
    // Pointer aliasing test
    total += test_pointer_aliasing(arr1, arr2, arr1, N);  // arr1 aliased
    
    // Reduction test
    total += test_reduction_with_dependency(darr, N);
    
    // Final computation to prevent optimization
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i] + varr[i] + (int)farr[i] + (int)darr[i];
    }
    
    printf("Total: %d, Checksum: %d\n", total, checksum);
    return total + checksum;
}
