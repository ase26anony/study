#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Array access with zero index and post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Memory access with zero offset
        ptr++;          // Post-increment on pointer
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *arr, int n) {
    double sum = 0.0;
    double *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement
        sum += *ptr;    // Dereference with zero offset
    }
    return sum;
}

/* Pattern 3: Loop with pointer increment in increment expression */
char func_loop_inc(char *str, int n) {
    char result = 0;
    char *ptr = str;
    
    for (int i = 0; i < n; i++) {
        result ^= ptr[0];  // Zero offset access
        ptr++;             // Increment in loop body
    }
    return result;
}

/* Pattern 4: Structure with first member access */
struct first_member {
    int value;
    char padding[16];
};

int func_struct_first_member(struct first_member *arr, int n) {
    int sum = 0;
    struct first_member *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  // Accesses first member (zero offset)
        ptr++;              // Pointer arithmetic
    }
    return sum;
}

/* Pattern 5: Memory access and arithmetic separated by trivial code */
short func_separated_ops(short *arr, int n) {
    short sum = 0;
    int index = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];      // Zero offset access
        
        // Small independent operation
        int temp = i * 2;
        (void)temp;         // Use to prevent optimization
        
        index++;            // Increment separated from access
    }
    return sum;
}

/* Pattern 6: Different data types and sizes */
long func_mixed_types(void *data, int n) {
    long sum = 0;
    char *cptr = (char*)data;
    int *iptr = (int*)data;
    float *fptr = (float*)data;
    
    // Access different types with zero offset
    for (int i = 0; i < n; i++) {
        sum += cptr[0];     // QImode access
        cptr++;
        
        if (i % 2 == 0) {
            sum += iptr[0]; // SImode access
            iptr++;
        }
        
        sum += (long)fptr[0]; // SFmode access
        fptr++;
    }
    return sum;
}

/* Pattern 7: While loop with post-decrement */
float func_while_postdec(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = &arr[n-1];
    int count = n;
    
    while (count > 0) {
        sum += ptr[0];  // Zero offset
        ptr--;          // Post-decrement
        count--;
    }
    return sum;
}

/* Pattern 8: Do-while loop ensuring execution */
unsigned long func_do_while(unsigned long *arr, int n) {
    unsigned long sum = 0;
    unsigned long *ptr = arr;
    int i = 0;
    
    if (n > 0) {
        do {
            sum += ptr[0];  // Zero offset
            ptr++;          // Increment
            i++;
        } while (i < n);
    }
    return sum;
}

/* Pattern 9: Nested pointer arithmetic */
int func_nested_ptr(int **ptr_arr, int n) {
    int sum = 0;
    int **ptr = ptr_arr;
    
    for (int i = 0; i < n; i++) {
        sum += (*ptr)[0];  // Double dereference with zero offset
        ptr++;             // Pointer increment
    }
    return sum;
}

/* Pattern 10: Volatile to prevent elimination */
volatile int global_counter = 0;

int func_volatile_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];          // Zero offset access
        global_counter = sum;   // Volatile write
        ptr++;                  // Increment
    }
    return sum;
}

/* Main function that exercises all patterns */
int main(void) {
    const int SIZE = 100;
    
    // Initialize arrays of different types
    int int_arr[SIZE];
    double double_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    unsigned long ulong_arr[SIZE];
    struct first_member struct_arr[SIZE];
    int *ptr_arr[SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        char_arr[i] = 'A' + (i % 26);
        short_arr[i] = i * 2;
        long_arr[i] = i * 1000L;
        float_arr[i] = i * 0.5f;
        ulong_arr[i] = i * 2000UL;
        struct_arr[i].value = i * 3;
        ptr_arr[i] = &int_arr[i];
    }
    
    // Mixed data buffer
    void *mixed_data = malloc(SIZE * sizeof(double));
    
    int total_sum = 0;
    
    // Call each function multiple times
    for (int iter = 0; iter < 10; iter++) {
        total_sum += func_zero_index_postinc(int_arr, SIZE);
        total_sum += (int)func_ptr_deref_predec(double_arr, SIZE);
        total_sum += func_loop_inc(char_arr, SIZE);
        total_sum += func_struct_first_member(struct_arr, SIZE);
        total_sum += func_separated_ops(short_arr, SIZE);
        total_sum += func_mixed_types(mixed_data, SIZE / 4);
        total_sum += (int)func_while_postdec(float_arr, SIZE);
        total_sum += func_do_while(ulong_arr, SIZE);
        total_sum += func_nested_ptr(ptr_arr, SIZE);
        total_sum += func_volatile_access(int_arr, SIZE);
    }
    
    free(mixed_data);
    
    // Print result to prevent dead code elimination
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
