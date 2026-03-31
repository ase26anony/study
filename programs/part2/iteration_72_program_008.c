#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Zero offset memory access
        ptr++;          // Post-increment on pointer
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *darr, int n) {
    double sum = 0.0;
    double *ptr = darr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          // Pre-decrement
        sum += *ptr;    // Memory access with implicit zero offset
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
        total += ptr->value;  // Accesses first member (zero offset)
        ptr++;                // Pointer arithmetic
    }
    return total;
}

/* Pattern 4: Loop with pointer increment in increment expression */
char func_loop_inc(char *str, int len) {
    char result = 0;
    char *p = str;
    
    for (int i = 0; i < len; i++, p++) {  // Increment in loop expression
        result ^= p[0];  // Zero offset access
    }
    return result;
}

/* Pattern 5: While loop with post-decrement */
short func_while_postdec(short *values, int n) {
    short sum = 0;
    short *ptr = values + n - 1;
    int count = n;
    
    while (count-- > 0) {
        sum += ptr[0];  // Zero offset
        ptr--;          // Post-decrement
    }
    return sum;
}

/* Pattern 6: Do-while loop ensuring at least one execution */
long func_dowhile_preinc(long *array, int n) {
    long total = 0;
    long *ptr = array;
    int i = 0;
    
    if (n <= 0) return 0;
    
    do {
        ++ptr;              // Pre-increment
        total += *(ptr-1);  // Access with offset calculation
        i++;
    } while (i < n);
    
    return total;
}

/* Pattern 7: Memory access and arithmetic separated by independent code */
float func_separated_ops(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      // Zero offset access
        
        // Independent operation that might separate the instructions
        float temp = sum * 0.5f;
        (void)temp;         // Use to prevent optimization
        
        ptr = ptr + 1;      // Pointer arithmetic (not ++ operator)
    }
    return sum;
}

/* Pattern 8: Mixed types and access sizes */
void func_mixed_types(void *buffer, int n) {
    char *cptr = (char *)buffer;
    int *iptr = (int *)(cptr + 64);
    double *dptr = (double *)(iptr + 8);
    
    for (int i = 0; i < n; i++) {
        // Different memory modes
        char c = cptr[0];   // QImode
        int val = iptr[0];  // SImode
        double d = dptr[0]; // DImode/DFmode
        
        // Prevent dead code elimination
        (void)c;
        (void)val;
        (void)d;
        
        cptr++;
        iptr++;
        dptr++;
    }
}

/* Pattern 9: Conditional branch that's always taken */
int func_conditional_branch(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset
        
        // Always-taken conditional (compiler might not know it's always true)
        if (i < n + 1) {  // Always true
            ptr++;        // Pointer increment
        }
    }
    return sum;
}

/* Pattern 10: Nested memory accesses */
int func_nested_access(int **matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row = matrix[0];  // Zero offset access
        matrix++;              // Pointer increment
        
        for (int j = 0; j < cols; j++) {
            total += row[0];   // Zero offset access
            row++;             // Pointer increment
        }
    }
    return total;
}

/* Main function that exercises all patterns */
int main(void) {
    // Initialize test data
    int int_arr[100];
    double double_arr[100];
    char char_arr[100];
    short short_arr[100];
    long long_arr[100];
    float float_arr[100];
    struct Data data_arr[50];
    int *matrix[10];
    int matrix_data[10][10];
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        char_arr[i] = 'A' + (i % 26);
        short_arr[i] = i * 2;
        long_arr[i] = i * 100L;
        float_arr[i] = i * 0.75f;
        if (i < 50) {
            data_arr[i].value = i * 3;
            strcpy(data_arr[i].name, "test");
            data_arr[i].score = i * 0.5f;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        matrix[i] = matrix_data[i];
        for (int j = 0; j < 10; j++) {
            matrix_data[i][j] = i * 10 + j;
        }
    }
    
    volatile int checksum = 0;  // Prevent optimization
    
    // Call all functions multiple times
    for (int iter = 0; iter < 10; iter++) {
        checksum += func_zero_index_postinc(int_arr, 10);
        checksum += (int)func_ptr_deref_predec(double_arr, 10);
        checksum += func_struct_first_member(data_arr, 10);
        checksum += func_loop_inc(char_arr, 10);
        checksum += func_while_postdec(short_arr, 10);
        checksum += (int)func_dowhile_preinc(long_arr, 10);
        checksum += (int)func_separated_ops(float_arr, 10);
        
        // Mixed types
        void *buffer = malloc(256);
        func_mixed_types(buffer, 5);
        free(buffer);
        
        checksum += func_conditional_branch(int_arr, 10, 100);
        checksum += func_nested_access(matrix, 5, 5);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
