#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Memory access with zero offset
        ptr++;          // Post-increment on pointer
    }
    return sum;
}

/* Helper function 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement on pointer
        sum += *ptr;    // Memory access with zero offset (implied)
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int value;
    char padding[12];
    double extra;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  // Accesses first member (zero offset)
        ptr++;              // Pointer arithmetic
    }
    return sum;
}

/* Helper function 4: Different data types - char */
int func_char_access(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // char access with zero offset
        ptr++;          // Post-increment
    }
    return sum;
}

/* Helper function 5: Different data types - short */
int func_short_access(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // short access with zero offset
        ptr++;          // Post-increment
    }
    return sum;
}

/* Helper function 6: Different data types - double */
double func_double_access(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // double access with zero offset
        ptr++;          // Post-increment
    }
    return sum;
}

/* Helper function 7: While loop with separated operations */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        int temp = ptr[0];  // Memory access with zero offset
        
        // Small independent calculation
        int dummy = i * 2;
        (void)dummy;  // Prevent unused variable warning
        
        sum += temp;
        ptr++;        // Post-increment separated by code
        i++;
    }
    return sum;
}

/* Helper function 8: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  // Memory access with zero offset
        ptr++;          // Post-increment
        i++;
    } while (i < n);
    
    return sum;
}

/* Helper function 9: Nested zero offset access */
int func_nested_access(int **arr, int n) {
    int sum = 0;
    int **ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {  // First level: pointer array access
            sum += ptr[0][0];   // Second level: integer access
        }
        ptr++;
    }
    return sum;
}

/* Helper function 10: Mixed increment/decrement patterns */
int func_mixed_patterns(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n - 1;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];  // Access with zero offset
        ptr1++;          // Post-increment
        
        sum += ptr2[0];  // Access with zero offset
        ptr2--;          // Post-decrement
    }
    return sum;
}

/* Helper function 11: Control flow that might inhibit fusion */
int func_conditional_inc(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Memory access with zero offset
        
        // Conditional that's always true in our test
        if (i < n + 1) {  // Always true condition
            // Independent operation
            int dummy = threshold * 3;
            (void)dummy;
        }
        
        ptr++;  // Post-increment after conditional
    }
    return sum;
}

/* Helper function 12: Float type with pre-increment */
float func_float_preinc(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ++ptr;          // Pre-increment
        sum += ptr[0];  // Access with zero offset
    }
    return sum;
}

/* Helper function 13: Long type with index variable */
long func_long_index(long *arr, int n) {
    long sum = 0;
    int idx = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[idx];  // Access with zero offset (idx == 0 in first iteration)
        idx++;            // Post-increment on index
    }
    return sum;
}

/* Helper function 14: Multiple zero-offset accesses */
int func_multiple_accesses(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int a = ptr[0];  // First access
        int b = ptr[0];  // Second access (same offset)
        sum += a + b;
        ptr++;           // Post-increment
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  // volatile to prevent optimization
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    double double_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    struct Data struct_arr[SIZE];
    int *ptr_arr[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i * 2);
        double_arr[i] = i * 1.5;
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 100L;
        struct_arr[i].value = i * 3;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_access(char_arr, SIZE);
        result += func_short_access(short_arr, SIZE);
        result += (int)func_double_access(double_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_nested_access(ptr_arr, SIZE/10);
        result += func_mixed_patterns(int_arr, SIZE);
        result += func_conditional_inc(int_arr, SIZE, 50);
        result += (int)func_float_preinc(float_arr, SIZE);
        result += (int)func_long_index(long_arr, SIZE);
        result += func_multiple_accesses(int_arr, SIZE);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", result);
    
    return 0;
}
