/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* Pattern 2: Char pointer with post-increment in while loop */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == 'a')  /* Simple (mem (reg)) access */
            count++;
        p = p + 1;      /* Separate increment */
    }
    return count;
}

/* Pattern 3: Use restrict to help alias analysis */
void copy_ints(int *restrict dst, const int *restrict src, int n) {
    /* Local pointers for clarity */
    int *d = dst;
    const int *s = src;
    
    for (int i = 0; i < n; i++) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 4: Mixed operations in loop */
void fill_and_sum(short *data, int n, short *out_sum) {
    short *p = data;
    short sum = 0;
    
    for (int i = 0; i < n; i++) {
        short val = *p;     /* Load via (mem (reg)) */
        sum += val;
        *p = val * 2;       /* Store via (mem (reg)) */
        p = p + 1;          /* Separate increment */
    }
    *out_sum = sum;
}

/* Pattern 5: Pointer arithmetic split across basic block */
int process_buffer(char *buf, int size) {
    char *ptr = buf;
    int total = 0;
    int i = 0;
    
    while (i < size) {
        /* Multiple accesses to same pointer location */
        char c1 = *ptr;     /* First access */
        total += c1;
        
        char c2 = *ptr;     /* Second access - same address */
        total += c2;
        
        ptr = ptr + 1;      /* Increment after both accesses */
        i++;
    }
    return total;
}

/* Pattern 6: Nested pointer usage */
void transform_array(int *arr, int n) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        int x = *p;         /* Load */
        x = x * 3 + 7;
        *p = x;             /* Store */
        p = p + 1;          /* Increment */
    }
}

/* Pattern 7: Simple memcpy-like with byte access */
void byte_copy(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;    /* Both are (mem (reg)) patterns */
        d = d + 1;
        s = s + 1;
    }
}

/* Pattern 8: Main function with its own patterns */
int main() {
    const int ARR_SIZE = 100;
    const int BUF_SIZE = 256;
    
    /* Test data */
    int int_array[ARR_SIZE];
    char char_buffer[BUF_SIZE];
    short short_array[ARR_SIZE];
    int dest_array[ARR_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARR_SIZE; i++) {
        int_array[i] = i * 2;
        short_array[i] = i;
    }
    
    for (int i = 0; i < BUF_SIZE; i++) {
        char_buffer[i] = (i % 26) + 'a';
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_array, ARR_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    int count = count_chars(char_buffer);
    printf("Count of 'a' chars: %d\n", count);
    
    copy_ints(dest_array, int_array, ARR_SIZE);
    
    short short_sum;
    fill_and_sum(short_array, ARR_SIZE, &short_sum);
    printf("Sum of short array: %d\n", short_sum);
    
    int buf_total = process_buffer(char_buffer, BUF_SIZE);
    printf("Buffer total: %d\n", buf_total);
    
    transform_array(int_array, ARR_SIZE);
    
    char dest_buffer[BUF_SIZE];
    byte_copy(dest_buffer, char_buffer, BUF_SIZE);
    
    /* Verify copy worked */
    if (memcmp(char_buffer, dest_buffer, BUF_SIZE) == 0) {
        printf("Copy verification: PASSED\n");
    } else {
        printf("Copy verification: FAILED\n");
    }
    
    /* Final checksum calculation using pointer traversal in main */
    int final_checksum = 0;
    int *ptr = int_array;
    for (int i = 0; i < ARR_SIZE; i++) {
        final_checksum += *ptr;  /* Should generate (mem (reg)) */
        ptr = ptr + 1;           /* Separate increment */
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
