#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Simple pointer dereference followed by increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* This should generate (mem (reg)) pattern */
        int val = *p;
        p = p + 1;  /* Separate increment - target for optimization */
        sum += val;
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        /* Direct dereference of pointer register */
        char c = *s;
        s = s + 1;  /* Separate increment */
        *d = c;
        d = d + 1;  /* Separate increment for dst */
    }
}

/* Pattern 3: Pointer arithmetic split across statements */
int process_short_data(short *data, int count) {
    short *ptr = data;
    int total = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Simple (mem (reg)) access */
        short value = *ptr;
        ptr = ptr + 1;  /* Post-increment as separate statement */
        total += value;
    }
    return total;
}

/* Pattern 4: While loop with pointer traversal - classic pattern */
void fill_with_value(int *buffer, int size, int value) {
    int *p = buffer;
    int *end = buffer + size;
    
    while (p < end) {
        *p = value;
        p = p + 1;  /* Separate increment statement */
    }
}

/* Pattern 5: Mixed operations to test different modes */
long long sum_mixed(char *chars, short *shorts, int *ints, int n) {
    long long total = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Different sized accesses all with simple (mem (reg)) */
        char c = *chars;
        chars = chars + 1;
        
        short s = *shorts;
        shorts = shorts + 1;
        
        int val = *ints;
        ints = ints + 1;
        
        total += c + s + val;
    }
    return total;
}

/* Pattern 6: Nested loops with pointer traversal */
void matrix_sum_rows(int *matrix, int rows, int cols, int *result) {
    int *row_ptr = matrix;
    int r, c;
    
    for (r = 0; r < rows; r++) {
        int *col_ptr = row_ptr;
        int row_sum = 0;
        
        for (c = 0; c < cols; c++) {
            /* Simple dereference followed by increment */
            int elem = *col_ptr;
            col_ptr = col_ptr + 1;
            row_sum += elem;
        }
        
        *result = row_sum;
        result = result + 1;
        row_ptr = row_ptr + cols;
    }
}

/* Pattern 7: Pointer to pointer traversal */
int sum_through_indirect(int **ptrs, int n) {
    int total = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int *p = ptrs[i];
        /* Dereference the pointer */
        int val = *p;
        total += val;
    }
    return total;
}

/* Pattern 8: Main function with its own pointer traversal */
int main() {
    const int ARRAY_SIZE = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Test data allocations */
    int int_array[ARRAY_SIZE];
    char char_array[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    int matrix[MATRIX_ROWS * MATRIX_COLS];
    int row_sums[MATRIX_ROWS];
    int *indirect_ptrs[ARRAY_SIZE];
    
    /* Initialize test data */
    int i, j;
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i % 1000);
        indirect_ptrs[i] = &int_array[i];
    }
    
    for (i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = i;
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    char dest[ARRAY_SIZE];
    copy_buffer_char(dest, char_array, ARRAY_SIZE);
    printf("Copy completed, first char: %d\n", dest[0]);
    
    int sum2 = process_short_data(short_array, ARRAY_SIZE);
    printf("Sum of short array: %d\n", sum2);
    
    fill_with_value(int_array, ARRAY_SIZE, 42);
    printf("Fill completed, first element: %d\n", int_array[0]);
    
    long long mixed_sum = sum_mixed(char_array, short_array, int_array, ARRAY_SIZE/4);
    printf("Mixed sum: %lld\n", mixed_sum);
    
    matrix_sum_rows(matrix, MATRIX_ROWS, MATRIX_COLS, row_sums);
    printf("Matrix row sums calculated, first: %d\n", row_sums[0]);
    
    int indirect_sum = sum_through_indirect(indirect_ptrs, ARRAY_SIZE);
    printf("Indirect sum: %d\n", indirect_sum);
    
    /* Additional pointer traversal in main */
    int *ptr = int_array;
    int *end_ptr = int_array + ARRAY_SIZE;
    int main_sum = 0;
    
    while (ptr < end_ptr) {
        /* This should also generate the target pattern */
        int val = *ptr;
        ptr = ptr + 1;  /* Separate increment */
        main_sum += val;
    }
    
    printf("Main traversal sum: %d\n", main_sum);
    printf("Total checksum: %lld\n", (long long)sum1 + sum2 + mixed_sum + main_sum);
    
    return 0;
}
