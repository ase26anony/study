/* Test program to trigger auto-increment/decrement optimization coverage
 * Specifically targets the uncovered block in find_inc_dec function
 * that handles simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_array[100];

/* Test 1: Simple parameter load with zero offset
 * Should generate: mem_insn.reg0 = p, mem_insn.reg1_val = 0
 */
int test_simple_param_load(int *p) {
    volatile int result = 0;
    // Simple register indirect access - target pattern
    result += *p;
    // Mixed with offset access to ensure find_inc_dec is called
    result += p[5];
    return result;
}

/* Test 2: Local pointer to array with mixed accesses
 * Includes the specific pattern: pointer[0] (zero offset)
 */
int test_local_pointer_mixed(void) {
    int local_arr[50];
    int *ptr = &local_arr[0];
    volatile int sum = 0;
    
    // Initialize array
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    // Simple register access with zero offset - target pattern
    sum += ptr[0];
    
    // Register + constant offset
    sum += ptr[10];
    
    // Loop with pointer increment (encourages auto-inc optimization)
    int *loop_ptr = &local_arr[0];
    for (int i = 0; i < 20; i++) {
        sum += *loop_ptr++;
    }
    
    // Another simple register access
    sum += *ptr;
    
    return sum;
}

/* Test 3: Global array access via local pointer
 * Multiple simple register accesses to increase coverage chance
 */
int test_global_access(void) {
    // Initialize global array
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + 1;
    }
    
    int *p = &global_array[0];
    volatile int total = 0;
    
    // Multiple simple register accesses
    total += p[0];      // Zero offset
    total += *p;        // Direct dereference
    
    // Mixed with offset
    total += p[25];
    total += p[50];
    
    // Loop with increment
    int *iter = p;
    for (int i = 0; i < 30; i++) {
        total += *iter++;
    }
    
    // Final simple access
    total += *p;
    
    return total;
}

/* Test 4: Conditional simple access inside loop
 * Creates complex control flow with simple register access pattern
 */
int test_conditional_simple_access(int *base, int n) {
    volatile int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; i++) {
        // Complex condition to prevent optimization
        if ((i % 3) == 0) {
            // Simple register access - target pattern
            sum += *ptr;
        } else if ((i % 3) == 1) {
            // Offset access
            sum += ptr[2];
        } else {
            // Pointer increment
            sum += *ptr++;
        }
        
        // Additional simple access outside condition
        if (i == n/2) {
            sum += *base;  // Simple register access
        }
    }
    
    return sum;
}

/* Test 5: Nested loops with varying access patterns
 * Increases chance of hitting the uncovered block
 */
int test_nested_loops(int *data, int rows, int cols) {
    volatile int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = data + i * cols;
        
        // Simple access at start of each row
        total += row_ptr[0];  // Zero offset
        
        for (int j = 0; j < cols; j++) {
            // Mix of access patterns
            if (j == 0) {
                total += *row_ptr;  // Simple register
            } else {
                total += row_ptr[j];  // Offset
            }
        }
        
        // Simple access at end of row
        total += *row_ptr;
    }
    
    return total;
}

/* Main driver to ensure all code is executed
 * and prevent dead code elimination
 */
int main(void) {
    volatile int final_result = 0;
    
    // Test array for parameter tests
    int test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = i * 3;
    }
    
    // Run all tests
    final_result += test_simple_param_load(&test_data[0]);
    final_result += test_local_pointer_mixed();
    final_result += test_global_access();
    final_result += test_conditional_simple_access(&test_data[0], 40);
    final_result += test_nested_loops(&test_data[0], 5, 10);
    
    // Print result to prevent optimization
    printf("Final checksum: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
