/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        int val = *p;      /* mem access via register */
        p = p + 1;         /* increment in separate statement */
        sum += val;
    }
    return sum;
}

/* Pattern 2: char pointer with post-increment in loop */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        char c = *p;       /* simple (mem (reg)) access */
        p = p + 1;         /* separate increment */
        if (c != ' ') count++;
    }
    return count;
}

/* Pattern 3: Using restrict to help alias analysis */
void copy_ints(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    while (n-- > 0) {
        *d = *s;          /* two (mem (reg)) accesses */
        d = d + 1;        /* separate increments */
        s = s + 1;
    }
}

/* Pattern 4: Mixed operations in loop */
void process_buffer(short *buf, int n) {
    short *p = buf;
    
    for (int i = 0; i < n; i++) {
        short val = *p;    /* load via register */
        p = p + 1;         /* increment separately */
        /* Some computation to prevent other optimizations */
        val = (val > 0) ? val : -val;
        buf[i] = val;      /* store via different addressing */
    }
}

/* Pattern 5: Simple increment in for loop */
float sum_floats(const float *arr, int n) {
    const float *p = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float val = *p;    /* (mem (reg)) pattern */
        p = p + 1;         /* separate increment */
        sum += val;
    }
    return sum;
}

/* Pattern 6: Pointer arithmetic with different strides */
void fill_pattern(char *buf, int size) {
    char *p = buf;
    int i = 0;
    
    while (i < size) {
        *p = (char)(i & 0xFF);  /* store via register */
        p = p + 1;              /* increment separately */
        i++;
    }
}

/* Pattern 7: Nested pointer usage */
int sum_matrix(const int *matrix, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        const int *row_ptr = matrix + r * cols;
        
        for (int c = 0; c < cols; c++) {
            int val = *row_ptr;    /* (mem (reg)) access */
            row_ptr = row_ptr + 1; /* separate increment */
            total += val;
        }
    }
    return total;
}

/* Pattern 8: Main function with its own patterns */
int main(void) {
    /* Test data */
    int int_arr[100];
    char char_buf[256];
    short short_buf[50];
    float float_arr[25];
    int matrix[5][10];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) int_arr[i] = i;
    for (int i = 0; i < 256; i++) char_buf[i] = (char)(i % 26 + 'A');
    for (int i = 0; i < 50; i++) short_buf[i] = (short)(i * 2);
    for (int i = 0; i < 25; i++) float_arr[i] = (float)i * 1.5f;
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 10; j++)
            matrix[i][j] = i * 10 + j;
    
    /* Pattern in main: pointer traversal */
    int *p = int_arr;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        int val = *p;      /* Should generate (mem (reg)) */
        p = p + 1;         /* Separate increment */
        main_sum += val;
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_arr, 100);
    int count = count_chars(char_buf);
    
    int dest_arr[100];
    copy_ints(dest_arr, int_arr, 100);
    
    process_buffer(short_buf, 50);
    
    float sum_float = sum_floats(float_arr, 25);
    
    char test_buf[100];
    fill_pattern(test_buf, 100);
    
    int matrix_sum = sum_matrix(&matrix[0][0], 5, 10);
    
    /* Verification */
    int checksum = sum1 + count + main_sum + matrix_sum + (int)sum_float;
    
    /* Verify copy worked */
    int copy_ok = 1;
    for (int i = 0; i < 100; i++) {
        if (dest_arr[i] != int_arr[i]) {
            copy_ok = 0;
            break;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Copy verification: %s\n", copy_ok ? "PASS" : "FAIL");
    printf("All tests completed.\n");
    
    return 0;
}
