/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    while (n-- > 0) {
        sum += *p;      /* (mem (reg p)) pattern */
        p = p + 1;      /* Subsequent increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer with explicit separate operations */
int count_chars(const char *str) {
    const char *ptr = str;
    int count = 0;
    while (*ptr != '\0') {
        char c = *ptr;  /* (mem (reg ptr)) */
        ptr = ptr + 1;  /* Separate increment */
        if (c != ' ') count++;
    }
    return count;
}

/* Pattern 3: Using restrict to help alias analysis */
void copy_ints(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) patterns */
        d = d + 1;
        s = s + 1;
    }
}

/* Pattern 4: Mixed operations in loop - testing pattern recognition */
void fill_pattern(short *buf, int size, short value) {
    short *p = buf;
    int i = size;
    
    while (i > 0) {
        short temp = *p;    /* Load with (mem (reg p)) */
        *p = value;         /* Store with (mem (reg p)) */
        p = p + 1;          /* Increment */
        i--;
    }
}

/* Pattern 5: Simple for loop with pointer */
float average_float(const float *data, int count) {
    const float *ptr = data;
    float total = 0.0f;
    int i;
    
    for (i = 0; i < count; i++) {
        total += *ptr;      /* (mem (reg ptr)) */
        ptr = ptr + 1;      /* Increment */
    }
    
    return count > 0 ? total / count : 0.0f;
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    const char *s = src;
    char *d = dst + len - 1;
    
    while (len-- > 0) {
        char val = *s;      /* (mem (reg s)) */
        s = s + 1;          /* Increment source */
        *d = val;           /* (mem (reg d)) */
        d = d - 1;          /* Decrement destination */
    }
}

/* Pattern 7: Multiple dereferences before increment */
int sum_pairs(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n/2; i++) {
        int a = *p;         /* First (mem (reg p)) */
        p = p + 1;          /* First increment */
        int b = *p;         /* Second (mem (reg p)) */
        p = p + 1;          /* Second increment */
        sum += a + b;
    }
    return sum;
}

/* Pattern 8: Direct pointer arithmetic in loop */
void memset_simple(int *buf, int value, int n) {
    int *p = buf;
    
    while (n > 0) {
        *p = value;         /* (mem (reg p)) */
        p = p + 1;          /* Increment */
        n--;
    }
}

/* Main function with various test cases */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    char char_buffer[BUFFER_SIZE];
    short short_buffer[BUFFER_SIZE];
    float float_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 2;
        float_array[i] = i * 1.5f;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = 'A' + (i % 26);
        short_buffer[i] = i;
    }
    
    /* Test Pattern 1: Simple sum with pointer */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Char counting */
    int count = count_chars(char_buffer);
    printf("Non-space chars: %d\n", count);
    
    /* Test Pattern 3: Copy with restrict */
    copy_ints(int_array2, int_array, ARRAY_SIZE);
    int sum2 = sum_array_int(int_array2, ARRAY_SIZE);
    printf("Copied array sum: %d\n", sum2);
    
    /* Test Pattern 4: Fill pattern */
    fill_pattern(short_buffer, BUFFER_SIZE, 42);
    printf("First short after fill: %d\n", (int)short_buffer[0]);
    
    /* Test Pattern 5: Float average */
    float avg = average_float(float_array, ARRAY_SIZE);
    printf("Float average: %.2f\n", avg);
    
    /* Test Pattern 6: Reverse copy */
    char reversed[BUFFER_SIZE];
    reverse_copy(reversed, char_buffer, BUFFER_SIZE);
    printf("First char reversed: %c\n", reversed[0]);
    
    /* Test Pattern 7: Sum pairs */
    int pair_sum = sum_pairs(int_array, ARRAY_SIZE);
    printf("Sum of pairs: %d\n", pair_sum);
    
    /* Test Pattern 8: Simple memset */
    memset_simple(int_array, -1, 10);
    printf("First int after memset: %d\n", int_array[0]);
    
    /* Additional pattern in main itself */
    {
        const int *ptr = int_array;
        int local_sum = 0;
        
        for (int i = 0; i < 10; i++) {
            int val = *ptr;     /* Should generate (mem (reg ptr)) */
            ptr = ptr + 1;      /* Subsequent increment */
            local_sum += val;
        }
        printf("Local sum in main: %d\n", local_sum);
    }
    
    /* Final verification */
    int total_checksum = sum1 + sum2 + count + (int)avg + pair_sum;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
