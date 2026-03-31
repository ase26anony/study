#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Array access with zero index and post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Memory access with zero offset
        arr++;          // Post-increment form
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

/* Pattern 3: Loop with pointer increment in increment expression */
int func_loop_inc(char *data, int n) {
    int sum = 0;
    char *ptr = data;
    
    for (int i = 0; i < n; ptr++, i++) {
        sum += ptr[0];  // Zero offset access
    }
    return sum;
}

/* Pattern 4: Structure with first member access */
struct first_member {
    int value;
    char padding[16];
};

int func_struct_first(struct first_member *arr, int n) {
    int sum = 0;
    struct first_member *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  // Access first member (zero offset)
        ptr++;              // Post-increment
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_type(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset
        ptr--;          // Post-decrement
    }
    return sum;
}

/* Pattern 6: Different data types - long */
int func_long_type(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    // Dereference with zero offset
        ++ptr;          // Pre-increment
    }
    return (int)sum;
}

/* Pattern 7: Floating point types */
float func_float_type(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset
        ptr++;          // Post-increment
    }
    return sum;
}

/* Pattern 8: Double type */
double func_double_type(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    // Dereference with zero offset
        --ptr;          // Pre-decrement
    }
    return sum;
}

/* Pattern 9: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  // Memory access with zero offset
        
        // Independent operation
        int dummy = i * 2;
        (void)dummy;
        
        ptr++;              // Increment separated from access
        sum += temp;
    }
    return sum;
}

/* Pattern 10: While loop with post-decrement */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  // Zero offset
        ptr--;          // Post-decrement
    }
    return sum;
}

/* Pattern 11: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n > 0) {
        do {
            sum += *ptr;    // Dereference with zero offset
            ++ptr;          // Pre-increment
        } while (--n > 0);
    }
    return sum;
}

/* Pattern 12: Mixed operations in same basic block */
int func_mixed_ops(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];     // Zero offset access
        sum += ptr2[0];     // Another zero offset access
        ptr1++;             // Post-increment
        ptr2--;             // Post-decrement
    }
    return sum;
}

/* Pattern 13: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  // Always true
            sum += ptr[0];  // Zero offset
        }
        ptr++;              // Post-increment
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  // Prevent dead code elimination
    
    // Initialize arrays
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct first_member struct_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = i % 128;
        short_arr[i] = i * 2;
        long_arr[i] = i * 100L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 3;
    }
    
    // Execute each pattern multiple times
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_loop_inc(char_arr, SIZE);
        result += func_struct_first(struct_arr, SIZE);
        result += func_short_type(short_arr, SIZE);
        result += func_long_type(long_arr, SIZE);
        result += (int)func_float_type(float_arr, SIZE);
        result += (int)func_double_type(double_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_while_loop(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_mixed_ops(int_arr, SIZE);
        result += func_always_taken(int_arr, SIZE);
    }
    
    // Print result to prevent optimization
    printf("Result: %d\n", result);
    
    return 0;
}
