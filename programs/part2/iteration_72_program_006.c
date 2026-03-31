#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Memory access with zero offset
        arr++;          // Pointer arithmetic in same basic block
    }
    
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n;
    
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement before access
        sum += *ptr;    // Memory access via pointer (implicit offset 0)
    }
    
    return sum;
}

/* Pattern 3: Structure with first member access */
struct first_member {
    int value;
    char data[32];
    double extra;
};

int func_struct_first_member(struct first_member *s, int n) {
    int sum = 0;
    struct first_member *ptr = s;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  // Accesses first member (offset 0)
        ptr++;              // Pointer arithmetic
    }
    
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_zero_offset(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // char access with zero offset
        arr++;          // Post-increment
    }
    
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_zero_offset(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // short access with zero offset
        arr++;          // Post-increment
    }
    
    return sum;
}

/* Pattern 6: Different data types - long */
int func_long_zero_offset(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // long access with zero offset
        arr++;          // Post-increment
    }
    
    return (int)sum;
}

/* Pattern 7: Floating point types - float */
float func_float_zero_offset(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // float access with zero offset
        arr++;          // Post-increment
    }
    
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_zero_offset(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // double access with zero offset
        arr++;          // Post-increment
    }
    
    return sum;
}

/* Pattern 9: While loop with pointer arithmetic */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    while (count-- > 0) {
        sum += *ptr;    // Dereference pointer (offset 0)
        ptr++;          // Post-increment in loop body
    }
    
    return sum;
}

/* Pattern 10: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    if (count <= 0) return 0;
    
    do {
        sum += ptr[0];  // Zero offset access
        ptr++;          // Post-increment
    } while (--count > 0);
    
    return sum;
}

/* Pattern 11: Memory access and arithmetic separated by trivial code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = arr[0];  // Memory access with zero offset
        
        // Small amount of independent code
        int dummy = i * 2;
        (void)dummy;  // Prevent unused variable warning
        
        arr++;              // Pointer arithmetic after separation
        sum += temp;
    }
    
    return sum;
}

/* Pattern 12: Conditional that's always taken */
int func_always_taken_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (n > 0) {  // Always true
            sum += ptr[0];  // Zero offset access
        }
        ptr++;  // Post-increment
    }
    
    return sum;
}

/* Pattern 13: Multiple increments in same block */
int func_multiple_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i += 2) {
        sum += ptr[0];  // First access
        ptr++;          // First increment
        
        if (i + 1 < n) {
            sum += ptr[0];  // Second access
            ptr++;          // Second increment
        }
    }
    
    return sum;
}

/* Pattern 14: Mixed pre and post increments */
int func_mixed_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    // Access current
        ++ptr;          // Pre-increment
        
        if (i + 1 < n) {
            sum += ptr[0];  // Access next with zero offset
            ptr++;          // Post-increment
        }
    }
    
    return sum;
}

/* Pattern 15: Nested array access */
int func_nested_zero_offset(int arr[][10], int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[i][0];  // Zero offset in second dimension
    }
    
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  // Volatile to prevent optimization
    
    // Initialize arrays
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct first_member struct_arr[SIZE];
    int nested_arr[10][10];
    
    // Fill arrays with data
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 2L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            nested_arr[i][j] = i * 10 + j;
        }
    }
    
    // Execute all patterns multiple times
    for (int iter = 0; iter < 3; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE / 2);
        result += func_ptr_deref_predec(int_arr + SIZE/2, SIZE / 2);
        result += func_struct_first_member(struct_arr, SIZE / 4);
        result += func_char_zero_offset(char_arr, SIZE);
        result += func_short_zero_offset(short_arr, SIZE);
        result += func_long_zero_offset(long_arr, SIZE / 2);
        result += (int)func_float_zero_offset(float_arr, SIZE / 2);
        result += (int)func_double_zero_offset(double_arr, SIZE / 2);
        result += func_while_loop(int_arr, SIZE / 2);
        result += func_do_while(int_arr + SIZE/2, SIZE / 2);
        result += func_separated_ops(int_arr, SIZE / 4);
        result += func_always_taken_branch(int_arr, SIZE / 4);
        result += func_multiple_increments(int_arr, SIZE / 2);
        result += func_mixed_increments(int_arr + SIZE/2, SIZE / 2);
        result += func_nested_zero_offset(nested_arr, 10);
    }
    
    // Print result to prevent dead code elimination
    printf("Result checksum: %d\n", result);
    
    return 0;
}
