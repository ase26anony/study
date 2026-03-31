/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment after access - separate statement */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with post-increment in same statement */
int sum_array_char(const char *arr, int n) {
    const char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *p++;    /* Combined dereference and increment */
    }
    return sum;
}

/* Pattern 3: Memory copy with restrict pointers */
void copy_buffer(int *restrict dst, const int *restrict src, int n) {
    /* Use local pointers for clarity */
    int *d = dst;
    const int *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 4: Fill buffer with value */
void fill_buffer(short *buf, int n, short value) {
    short *p = buf;
    short *end = buf + n;
    
    while (p < end) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 5: Mixed operations in loop */
int process_buffer(int *buf, int n) {
    int *p = buf;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int val = *p;   /* Load via register */
        total += val;
        p = p + 1;      /* Increment in next statement */
    }
    return total;
}

/* Pattern 6: Double pointer increment */
void copy_and_increment(int *restrict dst, const int *restrict src, int n, int *out_dst, int *out_src) {
    int *d = dst;
    const int *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;
        s = s + 1;
    }
    
    *out_dst = (int)(d - dst);
    *out_src = (int)(s - src);
}

/* Pattern 7: Simple while loop with pointer */
int count_zeros(const unsigned char *data, int len) {
    const unsigned char *p = data;
    int count = 0;
    
    while (len--) {
        if (*p == 0)    /* Access via register */
            count++;
        p = p + 1;      /* Increment separately */
    }
    return count;
}

/* Pattern 8: Local array traversal using pointer */
int sum_local_array(void) {
    int arr[16];
    int *p = arr;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        arr[i] = i;
    }
    
    /* Traverse with pointer */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += *p;
        p = p + 1;
    }
    return sum;
}

/* Main function to exercise all patterns */
int main(void) {
    /* Test data */
    int int_array[100];
    char char_array[200];
    short short_array[150];
    unsigned char byte_array[256];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 2;
    }
    
    for (int i = 0; i < 200; i++) {
        char_array[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 150; i++) {
        short_array[i] = (short)(i * 3);
    }
    
    for (int i = 0; i < 256; i++) {
        byte_array[i] = (unsigned char)(i % 16);
        if (i % 32 == 0) byte_array[i] = 0;
    }
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(int_array, 100);
    printf("Pattern 1 sum: %d\n", sum1);
    
    /* Test Pattern 2 */
    int sum2 = sum_array_char(char_array, 200);
    printf("Pattern 2 sum: %d\n", sum2);
    
    /* Test Pattern 3 */
    int dest_array[100];
    copy_buffer(dest_array, int_array, 100);
    printf("Pattern 3 copy complete\n");
    
    /* Test Pattern 4 */
    fill_buffer(short_array, 150, 42);
    printf("Pattern 4 fill complete\n");
    
    /* Test Pattern 5 */
    int sum5 = process_buffer(int_array, 100);
    printf("Pattern 5 total: %d\n", sum5);
    
    /* Test Pattern 6 */
    int out_dst, out_src;
    copy_and_increment(dest_array, int_array, 50, &out_dst, &out_src);
    printf("Pattern 6 increments: dst=%d, src=%d\n", out_dst, out_src);
    
    /* Test Pattern 7 */
    int zeros = count_zeros(byte_array, 256);
    printf("Pattern 7 zero count: %d\n", zeros);
    
    /* Test Pattern 8 */
    int local_sum = sum_local_array();
    printf("Pattern 8 local sum: %d\n", local_sum);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* Should generate (mem (reg)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Final checksum */
    int final_checksum = sum1 + sum2 + sum5 + zeros + local_sum + main_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
