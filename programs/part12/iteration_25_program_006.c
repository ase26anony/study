#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

// Prevent inlining to maintain dependencies
__attribute__((noinline)) 
void init_array(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i % 100;
    }
}

__attribute__((noinline))
int test_flow_dependency() {
    volatile int sum = 0;
    int array[SIZE];
    
    init_array(array, SIZE);
    
    // Flow dependency (RAW) across iterations
    for (int i = 1; i < SIZE; i++) {
        array[i] = array[i-1] + array[i];  // Carried flow dependency
        sum += array[i];
    }
    
    // Prevent dead code elimination
    return sum % 1000;
}

__attribute__((noinline))
int test_anti_dependency() {
    int temp[SIZE];
    int array[SIZE];
    
    init_array(array, SIZE);
    init_array(temp, SIZE);
    
    int result = 0;
    
    // Anti-dependency (WAR) within and across iterations
    for (int i = 0; i < SIZE - 1; i++) {
        int read_val = array[i];          // Read array[i]
        array[i] = temp[i] + i;           // Overwrite array[i] - anti-dependency
        temp[i] = read_val * 2;           // Use read value
        result += array[i] + temp[i];
    }
    
    return result % 1000;
}

__attribute__((noinline))
int test_output_dependency() {
    volatile int output[SIZE];
    int result = 0;
    
    // Output dependency (WAW) - multiple writes to same location
    for (int i = 0; i < SIZE; i++) {
        output[i] = i;                    // First write
        output[i] = output[i] * 2;        // Second write - output dependency
        output[i] = output[i] + 1;        // Third write - output dependency
        
        // Flow dependency mixed with output dependency
        if (i > 0) {
            output[i] = output[i-1] + output[i];
        }
        
        result += output[i];
    }
    
    return result % 1000;
}

__attribute__((noinline))
int test_nested_dependency() {
    int matrix[INNER_SIZE][INNER_SIZE];
    int result = 0;
    
    // Initialize matrix
    for (int i = 0; i < INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            matrix[i][j] = (i * INNER_SIZE + j) % 100;
        }
    }
    
    // Nested loops with cross-iteration dependencies
    for (int i = 1; i < INNER_SIZE; i++) {
        for (int j = 1; j < INNER_SIZE; j++) {
            // Flow dependency from previous row and column
            matrix[i][j] = matrix[i-1][j] + matrix[i][j-1] + matrix[i][j];
            result += matrix[i][j];
        }
    }
    
    return result % 1000;
}

__attribute__((noinline))
int test_mixed_types_dependency() {
    float f_array[SIZE];
    double d_array[SIZE];
    int i_array[SIZE];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        f_array[i] = i * 0.5f;
        d_array[i] = i * 0.25;
        i_array[i] = i;
    }
    
    float f_sum = 0.0f;
    double d_sum = 0.0;
    int i_sum = 0;
    
    // Mixed data types with dependencies
    for (int i = 1; i < SIZE; i++) {
        // Flow dependency with type conversion
        f_array[i] = f_array[i-1] + (float)i_array[i];
        
        // Anti-dependency with different type
        double temp = d_array[i];
        d_array[i] = f_array[i] * 2.0;
        d_sum += temp;
        
        // Output dependency on integer array
        i_array[i] = (int)f_array[i];
        i_array[i] = i_array[i] * 3;
        
        f_sum += f_array[i];
        i_sum += i_array[i];
    }
    
    return ((int)f_sum + (int)d_sum + i_sum) % 1000;
}

__attribute__((noinline))
int test_control_flow_dependency() {
    int array[SIZE];
    int result = 0;
    
    init_array(array, SIZE);
    
    // Control flow creating complex dependencies
    for (int i = 1; i < SIZE; i++) {
        if (i % 3 == 0) {
            // Flow dependency path
            array[i] = array[i-1] + array[i];
            result += array[i];
        } else if (i % 3 == 1) {
            // Anti-dependency path
            int temp = array[i];
            array[i] = result;
            result = temp;
        } else {
            // Output dependency path
            array[i] = i * 2;
            array[i] = array[i] + 1;
            result += array[i];
        }
        
        // Additional dependency across all paths
        if (i > 10) {
            array[i] = array[i] + array[i-10];
        }
    }
    
    return result % 1000;
}

__attribute__((noinline))
int test_pointer_aliasing_dependency() {
    int data[SIZE];
    int* volatile ptr1 = data;
    int* volatile ptr2 = data + SIZE/2;
    
    init_array(data, SIZE);
    
    int sum = 0;
    
    // Pointer aliasing creates ambiguous dependencies
    for (int i = 0; i < SIZE/2; i++) {
        // Potential aliasing between ptr1 and ptr2
        ptr1[i] = ptr2[i] + i;           // Could be flow/anti dependency
        ptr2[i] = ptr1[i] * 2;           // Creates dependency chain
        
        // Additional output dependency
        if (i % 4 == 0) {
            ptr1[i] = ptr1[i] + 1;
        }
        
        sum += ptr1[i] + ptr2[i];
    }
    
    return sum % 1000;
}

int main() {
    srand(time(NULL));
    
    int total = 0;
    
    // Run all test functions to create various dependency patterns
    total += test_flow_dependency();
    total += test_anti_dependency();
    total += test_output_dependency();
    total += test_nested_dependency();
    total += test_mixed_types_dependency();
    total += test_control_flow_dependency();
    total += test_pointer_aliasing_dependency();
    
    printf("Total checksum: %d\n", total);
    
    // Additional loop in main to ensure DDG analysis
    int final_array[SIZE];
    init_array(final_array, SIZE);
    
    volatile int final_sum = 0;
    for (int i = 1; i < SIZE; i++) {
        // Complex dependency pattern
        final_array[i] = final_array[i-1] + final_array[i];
        if (i % 2 == 0) {
            final_array[i] = final_array[i] * 2;
        }
        final_sum += final_array[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
