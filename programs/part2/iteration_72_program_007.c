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
        ptr++;          // Post-increment on pointer
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *darr, int n) {
    double sum = 0.0;
    double *ptr = &darr[n-1];
    
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement
        sum += *ptr;    // Dereference with zero offset
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char name[20];
    float score;
};

int func_struct_first_member(struct Data *data, int n) {
    int total = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        total += ptr->value;  // Access first member (zero offset)
        ptr++;                // Increment pointer
    }
    return total;
}

/* Pattern 4: Different data types and sizes */
short func_mixed_types(short *sarr, char *carr, int n) {
    short result = 0;
    short *sptr = sarr;
    char *cptr = carr;
    
    for (int i = 0; i < n; i++) {
        result += sarr[0];  // 16-bit access
        result += carr[0];  // 8-bit access
        sptr++;
        cptr++;
    }
    return result;
}

/* Pattern 5: Loop with increment in increment expression */
float func_loop_increment(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    
    for (int i = 0; i < n; ptr++, i++) {  // Increment in loop expression
        sum += ptr[0];  // Zero offset access
    }
    return sum;
}

/* Pattern 6: While loop with post-decrement */
long func_while_postdec(long *larr, int n) {
    long total = 0;
    long *ptr = &larr[n-1];
    int count = n;
    
    while (count-- > 0) {
        total += *ptr;  // Dereference
        ptr--;          // Post-decrement
    }
    return total;
}

/* Pattern 7: Do-while with separated operations */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    do {
        sum += arr[0];      // Memory access with zero offset
        
        // Small independent calculation
        int temp = i * 2;
        (void)temp;         // Use to prevent optimization
        
        ptr++;              // Increment separated by code
        i++;
    } while (i < n);
    
    return sum;
}

/* Pattern 8: Nested access patterns */
void func_nested_patterns(int *arr1, int *arr2, int n) {
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < n; i++) {
        // Multiple zero-offset accesses
        int val1 = ptr1[0];
        int val2 = ptr2[0];
        
        // Increment both pointers
        ptr1++;
        ptr2++;
        
        // Use values to prevent dead code elimination
        arr1[i] = val1 + val2;
    }
}

/* Pattern 9: Conditional with always-taken branch */
int func_conditional_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset access
        
        // Always-taken conditional
        if (i < n) {    // Always true
            ptr++;      // Increment in conditional block
        }
    }
    return sum;
}

/* Pattern 10: Multiple increments in same block */
double func_multiple_increments(double *darr, int n) {
    double sum = 0.0;
    double *ptr1 = darr;
    double *ptr2 = &darr[n/2];
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];  // First zero offset
        sum += ptr2[0];  // Second zero offset
        
        ptr1++;  // First increment
        ptr2++;  // Second increment
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int checksum = 0;  // Volatile to prevent optimization
    
    // Initialize arrays with different types
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    // Fill arrays with values
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 100;
        char_arr[i] = 'A' + (i % 26);
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 1000L;
        struct_arr[i].value = i * 2;
        strcpy(struct_arr[i].name, "test");
        struct_arr[i].score = i * 0.5f;
    }
    
    // Call each function multiple times with different arguments
    for (int iter = 0; iter < 10; iter++) {
        int n = SIZE - iter;  // Vary size slightly
        
        checksum += func_zero_index_postinc(int_arr, n);
        checksum += (int)func_ptr_deref_predec(double_arr, n);
        checksum += func_struct_first_member(struct_arr, n);
        checksum += func_mixed_types(short_arr, char_arr, n);
        checksum += (int)func_loop_increment(float_arr, n);
        checksum += (int)func_while_postdec(long_arr, n);
        checksum += func_separated_ops(int_arr, n);
        
        func_nested_patterns(int_arr, &int_arr[SIZE/2], n/2);
        
        checksum += func_conditional_branch(int_arr, n);
        checksum += (int)func_multiple_increments(double_arr, n);
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
