/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via plain register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Using restrict to help alias analysis */
void copy_restrict(int *restrict dst, const int *restrict src, int n) {
    /* Classic memcpy pattern - should generate post-increment */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Char pointer traversal - different mode (QImode) */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == 'a')  /* Simple (mem (reg)) */
            count++;
        p = p + 1;      /* Separate increment */
    }
    return count;
}

/* Pattern 4: Mixed operations in same basic block */
void process_buffer(short *buf, int n) {
    short *p = buf;
    
    for (int i = 0; i < n; i++) {
        short val = *p;     /* Load - (mem (reg)) */
        val = val * 2;      /* Some operation */
        *p = val;           /* Store - (mem (reg)) */
        p = p + 1;          /* Increment */
    }
}

/* Pattern 5: Explicit split operations (no compound expressions) */
float sum_floats(const float *arr, int n) {
    const float *ptr = arr;
    float total = 0.0f;
    
    /* Force separate statements */
    for (int i = 0; i < n; i++) {
        float current = *ptr;   /* Load via register */
        total += current;
        ptr = ptr + 1;          /* Increment separately */
    }
    return total;
}

/* Pattern 6: Nested loops with simple pointer access */
void matrix_sum_rows(const int *matrix, int rows, int cols, int *result) {
    for (int i = 0; i < rows; i++) {
        const int *row_ptr = matrix + i * cols;
        int row_sum = 0;
        
        for (int j = 0; j < cols; j++) {
            row_sum += *row_ptr;    /* (mem (reg)) */
            row_ptr = row_ptr + 1;  /* Separate increment */
        }
        
        result[i] = row_sum;
    }
}

/* Pattern 7: Pointer increment in loop condition */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    int index = 0;
    
    /* Increment in loop header */
    while (p < arr + n) {
        if (*p == target)   /* (mem (reg)) */
            return index;
        p = p + 1;          /* Separate increment */
        index++;
    }
    return -1;
}

/* Pattern 8: Local pointer with volatile (use sparingly) */
void fill_sequence(int *buf, int n) {
    int *p = buf;
    volatile int counter = 0;  /* Prevent some optimizations */
    
    for (int i = 0; i < n; i++) {
        *p = counter;       /* Store - (mem (reg)) */
        p = p + 1;          /* Separate increment */
        counter++;
    }
}

/* Main function with various test cases */
int main() {
    /* Test data */
    int int_array[100];
    int int_array2[100];
    char test_string[] = "test string with some a characters and more a's";
    short short_buffer[50];
    float float_array[40];
    int matrix[5][10];
    int row_sums[5];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
        int_array2[i] = 100 - i;
    }
    
    for (int i = 0; i < 50; i++) {
        short_buffer[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < 40; i++) {
        float_array[i] = i * 0.5f;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test Pattern 1: Sum array */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Copy with restrict */
    copy_restrict(int_array2, int_array, 100);
    int sum2 = sum_array_int(int_array2, 100);
    printf("Sum after copy: %d\n", sum2);
    
    /* Test Pattern 3: Char counting */
    int char_count = count_chars(test_string);
    printf("Count of 'a' characters: %d\n", char_count);
    
    /* Test Pattern 4: Process short buffer */
    process_buffer(short_buffer, 50);
    printf("Processed short buffer[0]: %d\n", (int)short_buffer[0]);
    
    /* Test Pattern 5: Sum floats */
    float float_sum = sum_floats(float_array, 40);
    printf("Sum of floats: %.2f\n", float_sum);
    
    /* Test Pattern 6: Matrix row sums */
    matrix_sum_rows(&matrix[0][0], 5, 10, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* Test Pattern 7: Find value */
    int found_index = find_value(int_array, 100, 42);
    printf("Found 42 at index: %d\n", found_index);
    
    /* Test Pattern 8: Fill sequence */
    int seq_buf[20];
    fill_sequence(seq_buf, 20);
    printf("Sequence buffer[10]: %d\n", seq_buf[10]);
    
    /* Additional pattern in main itself */
    {
        const int *ptr = int_array;
        int local_sum = 0;
        
        /* Simple pointer traversal in main */
        for (int i = 0; i < 10; i++) {
            local_sum += *ptr;   /* Should generate (mem (reg)) */
            ptr = ptr + 1;       /* Separate increment */
        }
        printf("Local sum in main: %d\n", local_sum);
    }
    
    /* Verify results */
    int checksum = sum1 + sum2 + char_count + (int)short_buffer[0] + 
                   (int)float_sum + row_sums[0] + found_index + seq_buf[10];
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
