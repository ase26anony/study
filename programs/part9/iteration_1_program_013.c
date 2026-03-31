/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access via register */
        p++;            /* increment of same register */
    }
    return sum;
}

/* Pattern 2: Copy with restrict pointers to avoid aliasing issues */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern that often uses auto-increment */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst++;          /* Post-increment */
        src++;          /* Post-increment */
    }
}

/* Pattern 3: Fill with value using pointer traversal */
void fill_buffer_char(char *buf, char value, int n) {
    char *p = buf;
    char *end = buf + n;
    
    while (p < end) {
        *p = value;     /* Store via register */
        p++;            /* Increment register */
    }
}

/* Pattern 4: Mixed operations in same basic block */
int process_short_array(short *data, int count) {
    short *ptr = data;
    int total = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        short val = *ptr;   /* Load via register */
        ptr++;              /* Increment register */
        total += val;
        
        /* Small basic block keeps operations together */
        if (val < 0) {
            total -= val;   /* Extra operation, but still in same block */
        }
    }
    return total;
}

/* Pattern 5: Direct pointer arithmetic split across statements */
int sum_every_other(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i += 2) {
        int temp = *p;      /* Load via register */
        sum += temp;
        p = p + 2;          /* Pointer arithmetic - might become increment */
    }
    return sum;
}

/* Pattern 6: Nested pointer usage */
void matrix_sum_rows(int *restrict dest, const int *restrict src, int rows, int cols) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int *row_ptr = src + i * cols;
        int row_sum = 0;
        
        for (j = 0; j < cols; j++) {
            row_sum += *row_ptr;    /* Load via register */
            row_ptr++;              /* Increment register */
        }
        
        *dest = row_sum;    /* Store result */
        dest++;             /* Increment destination pointer */
    }
}

/* Pattern 7: Character buffer processing with explicit increments */
int count_chars(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == target) { /* Load via register */
            count++;
        }
        p++;                /* Increment register */
    }
    return count;
}

/* Pattern 8: Local pointer with simple dereference */
int simple_deref_and_inc(int *ptr) {
    int a = *ptr;   /* Load via register */
    ptr++;          /* Increment register */
    int b = *ptr;   /* Another load */
    return a + b;
}

/* Main function to exercise all patterns */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    char src_buffer[BUFFER_SIZE];
    char dst_buffer[BUFFER_SIZE];
    short short_array[ARRAY_SIZE];
    int matrix[10][10];
    int dest_rows[10];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        short_array[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        src_buffer[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Pattern 1: Sum array with pointer traversal */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Pattern 2: Copy with restrict pointers */
    copy_buffer_restrict(dst_buffer, src_buffer, BUFFER_SIZE);
    
    /* Verify copy */
    if (memcmp(src_buffer, dst_buffer, BUFFER_SIZE) == 0) {
        printf("Copy successful\n");
    }
    
    /* Pattern 3: Fill buffer */
    fill_buffer_char(dst_buffer, 'A', 50);
    
    /* Pattern 4: Process short array */
    int sum2 = process_short_array(short_array, ARRAY_SIZE);
    printf("Processed short array total: %d\n", sum2);
    
    /* Pattern 5: Sum every other element */
    int sum3 = sum_every_other(int_array, ARRAY_SIZE);
    printf("Sum of every other element: %d\n", sum3);
    
    /* Pattern 6: Matrix row sums */
    matrix_sum_rows(dest_rows, &matrix[0][0], 10, 10);
    printf("Matrix row sums calculated\n");
    
    /* Pattern 7: Count characters */
    const char *test_str = "Hello, World!";
    int count = count_chars(test_str, 'l');
    printf("Count of 'l' in '%s': %d\n", test_str, count);
    
    /* Pattern 8: Simple dereference and increment */
    int test_vals[2] = {42, 17};
    int sum4 = simple_deref_and_inc(test_vals);
    printf("Simple deref sum: %d\n", sum4);
    
    /* Additional pattern in main: Pointer traversal */
    int *ptr = int_array;
    int checksum = 0;
    for (int i = 0; i < 10; i++) {  /* Small loop for tight basic block */
        checksum += *ptr;    /* Load via register */
        ptr++;               /* Increment register */
    }
    printf("Main loop checksum: %d\n", checksum);
    
    /* Final verification */
    int final_total = sum1 + sum2 + sum3 + sum4 + checksum;
    printf("Final total checksum: %d\n", final_total);
    
    return 0;
}
