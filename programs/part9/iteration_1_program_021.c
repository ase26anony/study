/* auto_inc_test.c - Test program for auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += *p;      /* (mem (reg p)) pattern */
        p = p + 1;      /* Separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer traversal with restrict */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) for both load and store */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Pointer increment in loop condition */
int count_zeros(const int *ptr, int len) {
    const int *p = ptr;
    int count = 0;
    
    while (len--) {
        if (*p == 0)    /* (mem (reg p)) */
            count++;
        p = p + 1;      /* Separate increment */
    }
    return count;
}

/* Pattern 4: Mixed operations with simple pointer access */
void fill_alternating(short *buf, int size, short val1, short val2) {
    short *p = buf;
    for (int i = 0; i < size; i++) {
        *p = (i & 1) ? val2 : val1;  /* Store with (mem (reg p)) */
        p = p + 1;                   /* Separate increment */
    }
}

/* Pattern 5: Multiple dereferences in same block */
int sum_first_last(const int *arr, int n) {
    const int *p = arr;
    int first, last;
    
    if (n > 0) {
        first = *p;     /* First (mem (reg p)) */
        p = p + (n - 1);
        last = *p;      /* Second (mem (reg p)) with different offset */
        return first + last;
    }
    return 0;
}

/* Pattern 6: Pointer arithmetic split across statements */
void process_bytes(uint8_t *data, int length) {
    uint8_t *ptr = data;
    int i = 0;
    
    while (i < length) {
        uint8_t val = *ptr;  /* Load with (mem (reg ptr)) */
        val ^= 0x55;         /* Some operation */
        *ptr = val;          /* Store with (mem (reg ptr)) */
        ptr = ptr + 1;       /* Separate increment */
        i = i + 1;
    }
}

/* Pattern 7: Nested pointer usage */
int sum_matrix(const int *matrix, int rows, int cols) {
    const int *p = matrix;
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            total += *p;    /* (mem (reg p)) */
            p = p + 1;      /* Separate increment */
        }
    }
    return total;
}

/* Pattern 8: Main function with its own pointer traversal */
int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[256];
    short short_buffer[50];
    uint8_t byte_data[128];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 2;
    }
    
    for (int i = 0; i < 256; i++) {
        char_buffer[i] = (char)(i & 0xFF);
    }
    
    for (int i = 0; i < 50; i++) {
        short_buffer[i] = (short)(i * 10);
    }
    
    for (int i = 0; i < 128; i++) {
        byte_data[i] = (uint8_t)i;
    }
    
    /* Test Pattern 1: Simple pointer dereference with increment */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Char buffer copy with restrict */
    char dest_buffer[256];
    copy_buffer_char(dest_buffer, char_buffer, 256);
    int copy_ok = memcmp(dest_buffer, char_buffer, 256) == 0;
    printf("Copy buffer %s\n", copy_ok ? "OK" : "FAILED");
    
    /* Test Pattern 3: Counting zeros */
    int zeros[20];
    for (int i = 0; i < 20; i++) zeros[i] = (i % 5 == 0) ? 0 : 1;
    int zero_count = count_zeros(zeros, 20);
    printf("Zero count: %d\n", zero_count);
    
    /* Test Pattern 4: Fill alternating */
    fill_alternating(short_buffer, 50, 100, 200);
    printf("Short buffer[0] = %d, [1] = %d\n", short_buffer[0], short_buffer[1]);
    
    /* Test Pattern 5: First and last */
    int first_last_sum = sum_first_last(int_array, 100);
    printf("First + last sum: %d\n", first_last_sum);
    
    /* Test Pattern 6: Byte processing */
    process_bytes(byte_data, 128);
    printf("Byte data processed, first byte: %u\n", byte_data[0]);
    
    /* Test Pattern 7: Matrix sum */
    int matrix[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
    int matrix_sum = sum_matrix(&matrix[0][0], 3, 4);
    printf("Matrix sum: %d\n", matrix_sum);
    
    /* Additional pattern in main: Pointer traversal */
    int *ptr = int_array;
    int local_sum = 0;
    for (int i = 0; i < 10; i++) {  /* Small loop for tight basic block */
        local_sum += *ptr;          /* (mem (reg ptr)) */
        ptr = ptr + 1;              /* Separate increment */
    }
    printf("Local pointer sum: %d\n", local_sum);
    
    /* Final checksum verification */
    int final_checksum = sum1 + zero_count + first_last_sum + matrix_sum + local_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
