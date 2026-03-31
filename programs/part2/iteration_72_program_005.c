#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Memory access with zero offset
        ptr++;          // Post-increment on pointer
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *ptr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement
        sum += *ptr;    // Dereference (zero offset)
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[16];
};

int func_struct_first_member(struct Data *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  // Accesses first member (zero offset)
        ptr++;              // Post-increment
    }
    return sum;
}

/* Pattern 4: Different data types and sizes */
short func_short_type(short *ptr, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // HImode access
        ptr++;          // Post-increment
    }
    return sum;
}

char func_char_type(char *ptr, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // QImode access
        ptr++;          // Post-increment
    }
    return sum;
}

/* Pattern 5: While loop with pointer arithmetic */
float func_while_loop(float *ptr, int n) {
    float sum = 0.0f;
    int i = 0;
    while (i < n) {
        sum += ptr[0];  // Zero offset access
        ptr++;          // Post-increment
        i++;
    }
    return sum;
}

/* Pattern 6: Do-while loop ensuring execution */
long func_do_while(long *ptr, int n) {
    long sum = 0;
    int i = 0;
    if (n > 0) {
        do {
            sum += ptr[0];  // Zero offset
            ptr++;          // Post-increment
            i++;
        } while (i < n);
    }
    return sum;
}

/* Pattern 7: Memory access and arithmetic separated by trivial code */
int func_separated_ops(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  // Memory access with zero offset
        // Small independent operation
        int dummy = i * 2;  // Trivial separation
        (void)dummy;        // Prevent unused warning
        ptr++;              // Pointer increment after separation
        sum += temp;
    }
    return sum;
}

/* Pattern 8: Multiple increments in same basic block */
int func_multiple_increments(int *ptr1, int *ptr2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr1[0];  // First zero offset access
        sum += ptr2[0];  // Second zero offset access
        ptr1++;          // First increment
        ptr2++;          // Second increment
    }
    return sum;
}

/* Pattern 9: Conditional that's always taken */
int func_always_taken_conditional(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (n > 0) {  // Always true in loop
            sum += ptr[0];  // Zero offset access
        }
        ptr++;  // Post-increment
    }
    return sum;
}

/* Pattern 10: Mixed pre and post increments */
int func_mixed_increments(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Access with zero offset
        if (i % 2 == 0) {
            ++ptr;      // Pre-increment
        } else {
            ptr++;      // Post-increment
        }
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    // Initialize arrays with different types
    int int_arr[100];
    double double_arr[100];
    short short_arr[100];
    char char_arr[100];
    float float_arr[100];
    long long_arr[100];
    struct Data struct_arr[100];
    
    // Initialize arrays with values
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 100;
        char_arr[i] = i % 128;
        float_arr[i] = i * 0.5f;
        long_arr[i] = i * 10L;
        struct_arr[i].value = i * 2;
    }
    
    volatile int total = 0;  // Volatile to prevent dead code elimination
    
    // Call each function multiple times with different arguments
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(int_arr, 10);
        total += func_ptr_deref_predec(double_arr + 50, 5);
        total += func_struct_first_member(struct_arr, 8);
        total += func_short_type(short_arr, 12);
        total += func_char_type(char_arr, 15);
        total += func_while_loop(float_arr, 7);
        total += func_do_while(long_arr, 9);
        total += func_separated_ops(int_arr + 20, 6);
        total += func_multiple_increments(int_arr, int_arr + 30, 5);
        total += func_always_taken_conditional(int_arr + 40, 8);
        total += func_mixed_increments(int_arr + 60, 7);
    }
    
    // Print checksum to ensure all code executes
    printf("Checksum: %d\n", total);
    
    return 0;
}
