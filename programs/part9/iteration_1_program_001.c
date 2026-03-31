#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    while (n-- > 0) {
        sum += *p;      // (mem (reg p))
        p = p + 1;      // Should become post-increment
    }
    return sum;
}

/* Pattern 2: Char pointer traversal with restrict */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    while (n-- > 0) {
        *dst = *src;    // (mem (reg dst)), (mem (reg src))
        dst = dst + 1;  // Post-increment candidate
        src = src + 1;  // Post-increment candidate
    }
}

/* Pattern 3: Mixed statements in same basic block */
int process_short_data(short *data, int count) {
    short *ptr = data;
    int total = 0;
    for (int i = 0; i < count; i++) {
        short val = *ptr;       // (mem (reg ptr))
        ptr = ptr + 1;          // Separate increment
        total += val * val;
    }
    return total;
}

/* Pattern 4: Pointer arithmetic split across statements */
void fill_with_value(int *base, int value, int n) {
    int *p = base;
    while (n > 0) {
        *p = value;     // (mem (reg p))
        // Small block - keep increment close
        p = p + 1;      // Should merge with store
        n--;
    }
}

/* Pattern 5: Direct dereference with immediate increment */
int sum_first_four(int *ptr) {
    int a = *ptr;       // (mem (reg ptr))
    ptr = ptr + 1;      // Increment in next statement
    
    int b = *ptr;       // Another (mem (reg ptr))
    ptr = ptr + 1;
    
    int c = *ptr;
    ptr = ptr + 1;
    
    int d = *ptr;
    
    return a + b + c + d;
}

/* Pattern 6: Loop with pointer in update position */
float average_float(const float *array, int size) {
    const float *p = array;
    float sum = 0.0f;
    int i;
    for (i = 0; i < size; i++) {
        sum += *p;      // (mem (reg p))
        p++;            // Post-increment in loop update
    }
    return sum / size;
}

/* Pattern 7: While loop with pointer comparison */
int find_zero(const int *start, const int *end) {
    const int *p = start;
    while (p < end) {
        if (*p == 0)    // (mem (reg p))
            return 1;
        p = p + 1;      // Increment after dereference
    }
    return 0;
}

/* Pattern 8: Nested pointer operations */
void copy_reverse(short *dst, const short *src, int n) {
    const short *s = src;
    short *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;        // Two (mem (reg)) patterns
        s = s + 1;      // Increment source
        d = d - 1;      // Decrement destination
    }
}

/* Main function with various test cases */
int main(void) {
    int int_array[100];
    char char_buffer[256];
    short short_data[50];
    float float_array[20];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 2;
    }
    
    for (int i = 0; i < 256; i++) {
        char_buffer[i] = (char)(i & 0xFF);
    }
    
    for (int i = 0; i < 50; i++) {
        short_data[i] = (short)(i * 10);
    }
    
    for (int i = 0; i < 20; i++) {
        float_array[i] = i * 1.5f;
    }
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2 */
    char dest_buffer[256];
    copy_buffer_char(dest_buffer, char_buffer, 256);
    printf("Copy buffer result: %s\n", 
           memcmp(dest_buffer, char_buffer, 256) == 0 ? "OK" : "FAIL");
    
    /* Test Pattern 3 */
    int sum2 = process_short_data(short_data, 50);
    printf("Processed short data: %d\n", sum2);
    
    /* Test Pattern 4 */
    int fill_array[10];
    fill_with_value(fill_array, 42, 10);
    printf("Fill test: %d\n", fill_array[5]);
    
    /* Test Pattern 5 */
    int test_four[4] = {1, 2, 3, 4};
    int sum3 = sum_first_four(test_four);
    printf("Sum first four: %d\n", sum3);
    
    /* Test Pattern 6 */
    float avg = average_float(float_array, 20);
    printf("Average float: %.2f\n", avg);
    
    /* Test Pattern 7 */
    int zero_test[5] = {1, 2, 0, 4, 5};
    int found = find_zero(zero_test, zero_test + 5);
    printf("Found zero: %s\n", found ? "YES" : "NO");
    
    /* Test Pattern 8 */
    short src_rev[5] = {1, 2, 3, 4, 5};
    short dst_rev[5];
    copy_reverse(dst_rev, src_rev, 5);
    printf("Reverse copy: %d %d %d %d %d\n", 
           dst_rev[0], dst_rev[1], dst_rev[2], dst_rev[3], dst_rev[4]);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int local_sum = 0;
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  // (mem (reg ptr))
        ptr = ptr + 1;      // Increment in same basic block
    }
    printf("Local pointer sum: %d\n", local_sum);
    
    /* Final checksum for verification */
    int final_checksum = sum1 + sum2 + sum3 + (int)avg + found + local_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
