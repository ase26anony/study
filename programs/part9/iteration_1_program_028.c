/* Program to trigger auto-inc-dec optimization for (mem (reg)) patterns */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    while (n-- > 0) {
        /* Should generate: (mem (reg p)) followed by p = p + 1 */
        sum += *p;
        p = p + 1;  /* Separate increment statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with post-increment in expression */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    while (*p) {
        /* Should generate: (mem (reg p)) pattern */
        if (*p == 'a') count++;
        p++;  /* Post-increment */
    }
    return count;
}

/* Pattern 3: Restrict pointers for better alias analysis */
void copy_ints(int *restrict dst, const int *restrict src, int n) {
    /* Local pointers to ensure simple (mem (reg)) patterns */
    int *d = dst;
    const int *s = src;
    
    while (n-- > 0) {
        /* Separate dereference and increment */
        *d = *s;
        d = d + 1;
        s = s + 1;
    }
}

/* Pattern 4: Mixed operations in loop */
void fill_pattern(short *buf, int size) {
    short *p = buf;
    int i = 0;
    while (i < size) {
        /* Simple (mem (reg p)) access */
        *p = (short)(i & 0xFF);
        p = p + 1;  /* Explicit increment */
        i++;
    }
}

/* Pattern 5: Pointer arithmetic with different strides */
void process_bytes(char *data, int len) {
    char *ptr = data;
    int remaining = len;
    
    while (remaining >= 4) {
        /* Multiple simple accesses to trigger pattern */
        char temp = *ptr;
        ptr = ptr + 1;
        
        temp += *ptr;
        ptr = ptr + 1;
        
        temp += *ptr;
        ptr = ptr + 1;
        
        temp += *ptr;
        ptr = ptr + 1;
        
        remaining -= 4;
    }
}

/* Pattern 6: Nested pointer usage */
int sum_matrix(const int *matrix, int rows, int cols) {
    int total = 0;
    const int *row_ptr = matrix;
    
    for (int i = 0; i < rows; i++) {
        const int *col_ptr = row_ptr;
        for (int j = 0; j < cols; j++) {
            /* Simple dereference pattern */
            total += *col_ptr;
            col_ptr = col_ptr + 1;  /* Separate increment */
        }
        row_ptr += cols;
    }
    return total;
}

/* Pattern 7: Volatile test case (use sparingly) */
int sum_with_volatile(const int *arr, int n) {
    const int *p = arr;
    volatile int sum = 0;  /* volatile to prevent some optimizations */
    
    while (n-- > 0) {
        sum += *p;
        p = p + 1;
    }
    return sum;
}

/* Main function with various test cases */
int main() {
    /* Test data setup */
    int int_array[100];
    char char_buffer[256];
    short short_array[50];
    int matrix[5][10];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
    }
    
    strcpy(char_buffer, "test string with some a characters for counting");
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute pattern functions */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    int count = count_chars(char_buffer);
    printf("Count of 'a' chars: %d\n", count);
    
    int dest_array[100];
    copy_ints(dest_array, int_array, 100);
    printf("Copy verification: %s\n", 
           memcmp(int_array, dest_array, 100 * sizeof(int)) == 0 ? "PASS" : "FAIL");
    
    fill_pattern(short_array, 50);
    printf("Fill pattern completed\n");
    
    process_bytes(char_buffer, strlen(char_buffer));
    printf("Byte processing completed\n");
    
    int matrix_sum = sum_matrix(&matrix[0][0], 5, 10);
    printf("Matrix sum: %d\n", matrix_sum);
    
    int volatile_sum = sum_with_volatile(int_array, 100);
    printf("Volatile sum: %d\n", volatile_sum);
    
    /* Additional pointer traversal in main itself */
    int *ptr = int_array;
    int local_sum = 0;
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* Simple (mem (reg)) pattern */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Local pointer sum: %d\n", local_sum);
    
    /* Final checksum for verification */
    int final_checksum = sum1 + count + matrix_sum + volatile_sum + local_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
