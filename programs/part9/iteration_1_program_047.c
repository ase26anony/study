/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via plain register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Direct dereference and increment */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Increment dst pointer */
        s = s + 1;      /* Increment src pointer */
    }
}

/* Pattern 3: Short pointer in for loop */
int16_t sum_array_short(const int16_t *arr, int n) {
    const int16_t *p = arr;
    int16_t sum = 0;
    int i;
    
    /* Loop with pointer increment in update */
    for (i = 0; i < n; i++) {
        sum += *p;      /* Simple dereference */
        p++;            /* Post-increment */
    }
    return sum;
}

/* Pattern 4: Mixed operations to create basic block sequences */
void fill_and_sum(int *arr, int n, int value) {
    int *p = arr;
    int i;
    
    /* Fill array */
    for (i = 0; i < n; i++) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Separate increment */
    }
    
    /* Reset and sum */
    p = arr;
    int sum = 0;
    while (n-- > 0) {
        sum += *p;      /* Load via register */
        p++;            /* Post-increment */
    }
    
    /* Use sum to prevent dead code elimination */
    arr[0] = sum;
}

/* Pattern 5: Pointer arithmetic split across statements */
void process_buffer(uint8_t *buf, int size) {
    uint8_t *ptr = buf;
    int count = size;
    
    /* Explicit split pattern */
    while (count > 0) {
        uint8_t val = *ptr;     /* Load: (mem (reg ptr)) */
        ptr = ptr + 1;          /* Increment in next statement */
        
        /* Simple operation to use the value */
        *buf = val ^ 0x55;      /* Store with different pointer */
        buf = buf + 1;
        
        count--;
    }
}

/* Pattern 6: Nested pointer access */
void matrix_sum_rows(const int *matrix, int rows, int cols, int *result) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int *row_ptr = matrix + i * cols;
        int row_sum = 0;
        
        /* Inner loop with simple pointer traversal */
        for (j = 0; j < cols; j++) {
            row_sum += *row_ptr;    /* (mem (reg row_ptr)) */
            row_ptr = row_ptr + 1;  /* Separate increment */
        }
        
        result[i] = row_sum;
    }
}

/* Pattern 7: Volatile test - use sparingly */
int volatile_sum(const int *arr, int n) {
    const int *p = arr;
    volatile int sum = 0;  /* volatile to prevent some optimizations */
    
    while (n-- > 0) {
        sum += *p;      /* Load from (mem (reg p)) */
        p = p + 1;      /* Pointer increment */
    }
    
    return sum;
}

/* Main function with various test cases */
int main() {
    /* Test data */
    int int_arr[100];
    char char_buf1[256], char_buf2[256];
    int16_t short_arr[50];
    uint8_t byte_buf[128];
    int matrix[10][10];
    int row_sums[10];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
    }
    
    for (int i = 0; i < 256; i++) {
        char_buf1[i] = (char)(i & 0xFF);
    }
    
    for (int i = 0; i < 50; i++) {
        short_arr[i] = (int16_t)(i * 2);
    }
    
    for (int i = 0; i < 128; i++) {
        byte_buf[i] = (uint8_t)i;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute pattern functions */
    int sum1 = sum_array_int(int_arr, 100);
    printf("Sum of int array: %d\n", sum1);
    
    copy_buffer_char(char_buf2, char_buf1, 256);
    printf("Char buffer copy done, first char: %d\n", char_buf2[0]);
    
    int16_t sum2 = sum_array_short(short_arr, 50);
    printf("Sum of short array: %d\n", (int)sum2);
    
    fill_and_sum(int_arr, 50, 42);
    printf("Fill and sum result: %d\n", int_arr[0]);
    
    process_buffer(byte_buf, 128);
    printf("Buffer processed, first byte: %d\n", byte_buf[0]);
    
    matrix_sum_rows((const int *)matrix, 10, 10, row_sums);
    printf("Matrix row sums calculated, first row: %d\n", row_sums[0]);
    
    int volatile_sum_result = volatile_sum(int_arr, 100);
    printf("Volatile sum: %d\n", volatile_sum_result);
    
    /* Additional pointer traversal in main itself */
    int *ptr = int_arr;
    int local_sum = 0;
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* Should generate (mem (reg ptr)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Local pointer sum: %d\n", local_sum);
    
    /* Final checksum */
    int total_checksum = sum1 + sum2 + local_sum + volatile_sum_result;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
