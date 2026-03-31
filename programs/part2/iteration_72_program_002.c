#include <stdio.h>
#include <stdlib.h>

// Volatile variable to prevent dead code elimination
volatile int global_sum = 0;

// Pattern 1: Direct array indexing at index 0 with post-increment
void func_zero_index_postinc(int *arr, int n) {
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        // Memory access with zero offset: arr[0] or ptr[0]
        int val = ptr[0];
        global_sum += val;
        // Post-increment on pointer
        ptr++;
    }
}

// Pattern 2: Pointer dereference with pre-decrement
void func_ptr_deref_predec(char *data, int n) {
    char *ptr = data + n;
    for (int i = 0; i < n; i++) {
        // Pre-decrement then memory access
        --ptr;
        char val = *ptr;  // Equivalent to ptr[0]
        global_sum += val;
    }
}

// Pattern 3: Structure with first member access
struct FirstMember {
    int first;
    int second;
    int third;
};

void func_struct_first_member(struct FirstMember *s, int n) {
    struct FirstMember *ptr = s;
    for (int i = 0; i < n; i++) {
        // Access first member (offset 0)
        int val = ptr->first;
        global_sum += val;
        // Post-increment
        ptr++;
    }
}

// Pattern 4: Different data types and sizes
void func_mixed_types(void) {
    short short_arr[100];
    long long_arr[100];
    float float_arr[100];
    double double_arr[100];
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        short_arr[i] = i;
        long_arr[i] = i * 10;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
    }
    
    // Access with different types
    short *sptr = short_arr;
    long *lptr = long_arr;
    float *fptr = float_arr;
    double *dptr = double_arr;
    
    for (int i = 0; i < 10; i++) {
        // All these have zero offset
        global_sum += sptr[0];
        global_sum += lptr[0];
        global_sum += (int)fptr[0];
        global_sum += (int)dptr[0];
        
        // Increment pointers
        sptr++;
        lptr++;
        fptr++;
        dptr++;
    }
}

// Pattern 5: While loop with pointer arithmetic in body
void func_while_loop(int *arr, int n) {
    int *ptr = arr;
    int count = 0;
    while (count < n) {
        // Memory access with zero offset
        int val = ptr[0];
        global_sum += val;
        // Pointer increment in loop body
        ptr++;
        count++;
    }
}

// Pattern 6: Do-while loop ensuring at least one execution
void func_do_while(char *data, int n) {
    if (n <= 0) return;
    
    char *ptr = data;
    int count = 0;
    do {
        // Access with zero offset
        char val = ptr[0];
        global_sum += val;
        // Post-increment
        ptr++;
        count++;
    } while (count < n);
}

// Pattern 7: Memory access and arithmetic separated by trivial code
void func_separated_ops(int *arr, int n) {
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        // Memory access with zero offset
        int val = ptr[0];
        global_sum += val;
        
        // Small amount of independent code
        int temp = val * 2;
        global_sum += temp % 3;
        
        // Then increment the pointer
        ptr++;
    }
}

// Pattern 8: Nested loops with array access
void func_nested_loops(int matrix[10][10], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < cols; j++) {
            // Access first element of current row
            int val = row_ptr[0];
            global_sum += val;
            // Move to next element in row
            row_ptr++;
        }
    }
}

// Pattern 9: Conditional branch that's always taken
void func_conditional_branch(int *arr, int n) {
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        // Memory access with zero offset
        int val = ptr[0];
        global_sum += val;
        
        // Conditional that's always true
        if (val == val) {  // Always true
            // Independent operation
            global_sum += i;
        }
        
        // Increment pointer
        ptr++;
    }
}

// Pattern 10: Multiple increments in same block
void func_multiple_increments(int *arr, int n) {
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        // Two memory accesses with zero offset
        int val1 = ptr1[0];
        int val2 = ptr2[0];
        global_sum += val1 + val2;
        
        // Increment both pointers
        ptr1++;
        ptr2++;
    }
}

int main(void) {
    // Initialize test data
    int int_arr[1000];
    char char_arr[1000];
    struct FirstMember struct_arr[100];
    int matrix[10][10];
    
    // Fill arrays with data
    for (int i = 0; i < 1000; i++) {
        int_arr[i] = i % 100;
        char_arr[i] = (i % 26) + 'A';
    }
    
    for (int i = 0; i < 100; i++) {
        struct_arr[i].first = i;
        struct_arr[i].second = i * 2;
        struct_arr[i].third = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    // Call all pattern functions multiple times
    for (int iteration = 0; iteration < 10; iteration++) {
        func_zero_index_postinc(int_arr, 100);
        func_ptr_deref_predec(char_arr, 100);
        func_struct_first_member(struct_arr, 50);
        func_mixed_types();
        func_while_loop(int_arr, 100);
        func_do_while(char_arr, 100);
        func_separated_ops(int_arr, 100);
        func_nested_loops(matrix, 10, 10);
        func_conditional_branch(int_arr, 100);
        func_multiple_increments(int_arr, 200);
    }
    
    // Print result to prevent optimization
    printf("Final sum: %d\n", global_sum);
    
    return 0;
}
