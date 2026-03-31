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
        --ptr;          // Pre-decrement
        sum += *ptr;    // Dereference with implicit zero offset
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int first;
    int second;
    char third;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->first;  // First member = zero offset
        ptr++;              // Pointer arithmetic
    }
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_zero_index(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // char access with zero offset
        arr++;          // Post-increment
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_zero_index(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // short access with zero offset
        arr++;          // Post-increment
    }
    return sum;
}

/* Pattern 6: Different data types - long */
long func_long_zero_index(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // long access with zero offset
        arr++;          // Post-increment
    }
    return sum;
}

/* Pattern 7: Floating point types - float */
float func_float_zero_index(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // float access with zero offset
        arr++;          // Post-increment
    }
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_zero_index(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // double access with zero offset
        arr++;          // Post-increment
    }
    return sum;
}

/* Pattern 9: While loop with pointer arithmetic in body */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        sum += ptr[0];  // Zero offset access
        ptr++;          // Post-increment in loop body
        i++;
    }
    return sum;
}

/* Pattern 10: Do-while loop ensuring at least one execution */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    if (n > 0) {
        do {
            sum += ptr[0];  // Zero offset access
            ptr++;          // Post-increment
            i++;
        } while (i < n);
    }
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
        (void)dummy;
        
        arr++;              // Pointer arithmetic after separation
        sum += temp;
    }
    return sum;
}

/* Pattern 12: Conditional that's always taken */
int func_conditional_always(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  // Always true in practice
            sum += ptr[0];  // Zero offset access
        }
        ptr++;              // Post-increment
    }
    return sum;
}

/* Pattern 13: Nested zero offset access */
int func_nested_access(int **arr, int n) {
    int sum = 0;
    int **ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {      // First level: ptr[0]
            sum += ptr[0][0];      // Second level: ptr[0][0] - zero offset
        }
        ptr++;                     // Pointer arithmetic
    }
    return sum;
}

/* Pattern 14: Mixed pre and post increments */
int func_mixed_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    // Dereference with implicit zero offset
        ++ptr;          // Pre-increment
    }
    
    ptr = arr;  // Reset
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset access
        ptr--;          // Pre-decrement (will access wrong memory but pattern is valid)
    }
    
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  // Volatile to prevent dead code elimination
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 10L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].first = i;
        struct_arr[i].second = i * 2;
        struct_arr[i].third = (char)(i % 26 + 'A');
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        int n = SIZE - iter;
        
        result += func_zero_index_postinc(int_arr, n);
        result += func_ptr_deref_predec(int_arr, n);
        result += func_struct_first_member(struct_arr, n);
        result += func_char_zero_index(char_arr, n);
        result += func_short_zero_index(short_arr, n);
        result += func_long_zero_index(long_arr, n);
        result += func_float_zero_index(float_arr, n);
        result += func_double_zero_index(double_arr, n);
        result += func_while_loop(int_arr, n);
        result += func_do_while(int_arr, n);
        result += func_separated_ops(int_arr, n);
        result += func_conditional_always(int_arr, n);
        result += func_nested_access(ptr_arr, n);
        result += func_mixed_increments(int_arr, n);
    }
    
    /* Print checksum to ensure all code executes */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
