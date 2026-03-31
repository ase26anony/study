/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increments
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
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer traversal with restrict */
void copy_buffer(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *d = *s;        /* (mem (reg)) for both load and store */
        d = d + 1;      /* separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Pointer increment in loop condition */
int count_zeros(const int *ptr, int len) {
    const int *p = ptr;
    int count = 0;
    
    while (len-- > 0) {
        if (*p == 0)    /* (mem (reg)) access */
            count++;
        p = p + 1;      /* increment after use */
    }
    return count;
}

/* Pattern 4: Mixed types to test different memory modes */
short sum_shorts(const short *data, int n) {
    const short *p = data;
    short total = 0;
    
    for (int i = 0; i < n; i++) {
        total += *p;    /* HImode memory access */
        p = p + 1;      /* separate increment */
    }
    return total;
}

/* Pattern 5: Explicit split operations in basic block */
int process_pair(int *restrict a, int *restrict b) {
    int x, y;
    
    /* Two separate (mem (reg)) accesses followed by increments */
    x = *a;             /* first mem access */
    a = a + 1;          /* increment */
    
    y = *b;             /* second mem access */
    b = b + 1;          /* increment */
    
    return x + y;
}

/* Pattern 6: Nested pointer usage */
void fill_pattern(int *restrict buf, int n, int value) {
    int *p = buf;
    
    while (n-- > 0) {
        *p = value;     /* store via register */
        p = p + 1;      /* increment */
    }
}

/* Pattern 7: Pointer arithmetic with different strides */
void stride_copy(int *restrict dst, const int *restrict src, int n) {
    int *d = dst;
    const int *s = src;
    
    /* Force simple (mem (reg)) pattern by avoiding index calculation */
    for (int i = 0; i < n; i++) {
        *d = *s;        /* simple dereference */
        d = d + 1;      /* explicit increment */
        s = s + 1;
    }
}

/* Pattern 8: Main function with its own pointer traversal */
int main(void) {
    /* Test buffers of different types */
    int int_arr[100];
    char char_buf[256];
    short short_arr[50];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i % 10;
    }
    
    for (int i = 0; i < 256; i++) {
        char_buf[i] = (char)(i & 0xFF);
    }
    
    for (int i = 0; i < 50; i++) {
        short_arr[i] = (short)(i * 2);
    }
    
    /* Test Pattern 1: Simple array sum */
    int sum1 = sum_array_int(int_arr, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Buffer copy */
    char dest_buf[256];
    copy_buffer(dest_buf, char_buf, 256);
    
    /* Verify copy */
    if (memcmp(dest_buf, char_buf, 256) == 0) {
        printf("Buffer copy successful\n");
    }
    
    /* Test Pattern 3: Count zeros */
    int zero_count = count_zeros(int_arr, 100);
    printf("Zero count: %d\n", zero_count);
    
    /* Test Pattern 4: Short array sum */
    short sum_shorts_result = sum_shorts(short_arr, 50);
    printf("Sum of shorts: %d\n", sum_shorts_result);
    
    /* Test Pattern 5: Process pair */
    int pair_result = process_pair(&int_arr[0], &int_arr[50]);
    printf("Pair sum: %d\n", pair_result);
    
    /* Test Pattern 6: Fill pattern */
    int fill_buf[20];
    fill_pattern(fill_buf, 20, 42);
    
    /* Test Pattern 7: Stride copy */
    int src_buf[30], dst_buf[30];
    for (int i = 0; i < 30; i++) src_buf[i] = i * 3;
    stride_copy(dst_buf, src_buf, 30);
    
    /* Additional pointer traversal in main */
    int *p = int_arr;
    int main_sum = 0;
    
    /* This loop in main should also generate (mem (reg)) patterns */
    for (int i = 0; i < 10; i++) {
        main_sum += *p;     /* direct dereference */
        p = p + 1;          /* separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Final checksum for verification */
    int final_checksum = sum1 + zero_count + sum_shorts_result + pair_result + main_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
