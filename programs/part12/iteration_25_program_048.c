#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

// Prevent optimization and create dependencies
volatile int global_counter = 0;

// Noinline functions to prevent optimization
__attribute__((noinline)) 
int* get_array_ptr(int* arr, int idx) {
    global_counter++;
    return &arr[idx];
}

__attribute__((noinline))
void modify_through_pointer(int* ptr, int value) {
    *ptr = value;
    global_counter++;
}

// Test 1: Flow dependency (RAW) with carried dependency
__attribute__((noinline))
int test_flow_dependency(int* arr) {
    int sum = 0;
    // Flow dependency: sum from previous iteration used in current
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  // RAW: read arr[i], write sum
        arr[i] = sum;   // WAW on arr[i] if unrolled, WAR on sum
    }
    return sum;
}

// Test 2: Anti-dependency (WAR) with volatile
__attribute__((noinline))
int test_anti_dependency(volatile int* arr) {
    int temp = 0;
    for (int i = 0; i < SIZE; i++) {
        temp = arr[i];      // Read arr[i]
        arr[i] = i * 2;     // Write arr[i] - WAR dependency
        temp += arr[i];     // Read arr[i] again
    }
    return temp;
}

// Test 3: Output dependency (WAW) and mixed operations
__attribute__((noinline))
double test_output_dependency(double* arr) {
    double result = 0.0;
    for (int i = 1; i < SIZE; i++) {
        // Multiple writes to same location
        arr[i] = arr[i-1] * 1.5;    // Flow dependency from arr[i-1]
        arr[i] = arr[i] + i;        // WAW on arr[i]
        arr[i] = arr[i] / 2.0;      // Another WAW on arr[i]
        result += arr[i];
    }
    return result;
}

// Test 4: Nested loops with outer-loop dependencies
__attribute__((noinline))
int test_nested_dependency(int arr[][INNER_SIZE]) {
    int total = 0;
    // Outer loop dependency carried to inner loop
    for (int i = 1; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            // Flow dependency across outer loop iterations
            arr[i][j] = arr[i-1][j] + (i * j);
            total += arr[i][j];
        }
    }
    return total;
}

// Test 5: Control flow with dependencies
__attribute__((noinline))
int test_control_flow_dependency(int* arr) {
    int sum = 0;
    for (int i = 1; i < SIZE; i++) {
        if (i % 3 == 0) {
            // Flow dependency through sum
            sum += arr[i-1];
            arr[i] = sum;
        } else if (i % 3 == 1) {
            // Anti-dependency
            int temp = arr[i];
            arr[i] = i * 3;
            sum += temp;
        } else {
            // Output dependency
            arr[i] = i;
            arr[i] = arr[i] * 2;  // WAW
            sum += arr[i];
        }
    }
    return sum;
}

// Test 6: Pointer aliasing creates ambiguous dependencies
__attribute__((noinline))
int test_pointer_aliasing(int* arr1, int* arr2) {
    int result = 0;
    // arr1 and arr2 may alias, creating potential dependencies
    for (int i = 1; i < SIZE; i++) {
        arr1[i] = arr2[i-1] + 1;  // Possible flow if aliased
        arr2[i] = arr1[i] * 2;    // Possible anti if aliased
        result += arr1[i] + arr2[i];
    }
    return result;
}

// Test 7: Mixed data types and operations
__attribute__((noinline))
float test_mixed_types(int* int_arr, float* float_arr) {
    float sum = 0.0f;
    for (int i = 1; i < SIZE; i++) {
        // Cross-type dependencies
        float_arr[i] = (float)int_arr[i-1] * 1.5f;  // Flow
        int_arr[i] = (int)float_arr[i] + i;         // Anti
        sum += float_arr[i];
    }
    return sum;
}

int main() {
    // Initialize arrays with different patterns
    int arr1[SIZE];
    volatile int arr2[SIZE];
    double arr3[SIZE];
    int arr4[SIZE/INNER_SIZE][INNER_SIZE];
    int arr5[SIZE];
    int arr6[SIZE];
    int arr7[SIZE];
    float arr8[SIZE];
    
    srand(time(NULL));
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = (double)(rand() % 100) / 10.0;
        arr5[i] = rand() % 100;
        arr6[i] = rand() % 100;
        arr7[i] = rand() % 100;
        arr8[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            arr4[i][j] = rand() % 100;
        }
    }
    
    // Run all tests to create various DDG edges
    int result1 = test_flow_dependency(arr1);
    int result2 = test_anti_dependency(arr2);
    double result3 = test_output_dependency(arr3);
    int result4 = test_nested_dependency(arr4);
    int result5 = test_control_flow_dependency(arr5);
    int result6 = test_pointer_aliasing(arr6, arr7);
    float result7 = test_mixed_types(arr7, arr8);
    
    // Aggregate results to prevent dead code elimination
    int final_result = result1 + result2 + (int)result3 + result4 + 
                       result5 + result6 + (int)result7 + global_counter;
    
    printf("Final checksum: %d\n", final_result);
    printf("Global counter: %d\n", global_counter);
    
    return final_result != 0 ? 0 : 1;
}
