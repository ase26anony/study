/* auto_inc_test.c - Test program for auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with explicit post-increment */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Mixed operations with pointer traversal */
void fill_buffer_short(short *buf, short value, int count) {
    short *p = buf;
    
    for (int i = 0; i < count; i++) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 4: Multiple dereferences in same loop */
int sum_two_arrays(const int *a, const int *b, int n) {
    const int *pa = a;
    const int *pb = b;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *pa;     /* First (mem (reg)) */
        sum += *pb;     /* Second (mem (reg)) */
        pa = pa + 1;    /* Increment */
        pb = pb + 1;    /* Increment */
    }
    return sum;
}

/* Pattern 5: Pointer arithmetic in loop condition */
int count_zeros(const char *data, int len) {
    const char *p = data;
    int zeros = 0;
    
    while (len > 0) {
        if (*p == 0) {  /* Dereference pointer */
            zeros++;
        }
        p = p + 1;      /* Increment after use */
        len--;
    }
    return zeros;
}

/* Pattern 6: Local pointer with restrict in small function */
void increment_array(int *restrict arr, int n) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        *p += 1;        /* Load-modify-store pattern */
        p = p + 1;      /* Separate increment */
    }
}

/* Pattern 7: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int n) {
    char *d = dst + n - 1;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d - 1;      /* Decrement instead of increment */
        s = s + 1;      /* Increment */
    }
}

/* Pattern 8: Main function with its own pointer traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    char char_buffer[BUFFER_SIZE];
    short short_buffer[BUFFER_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = (char)(i % 256);
        short_buffer[i] = (short)(i % 1000);
    }
    
    /* Test Pattern 1: Simple sum with pointer */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Char buffer copy */
    char char_copy[BUFFER_SIZE];
    copy_buffer_char(char_copy, char_buffer, BUFFER_SIZE);
    
    /* Verify copy */
    if (memcmp(char_buffer, char_copy, BUFFER_SIZE) == 0) {
        printf("Char buffer copy successful\n");
    }
    
    /* Test Pattern 3: Fill buffer with shorts */
    short short_filled[BUFFER_SIZE];
    fill_buffer_short(short_filled, 42, BUFFER_SIZE);
    
    /* Test Pattern 4: Sum two arrays */
    int sum2 = sum_two_arrays(int_array, int_array, ARRAY_SIZE / 2);
    printf("Sum of two arrays: %d\n", sum2);
    
    /* Test Pattern 5: Count zeros */
    int zero_count = count_zeros(char_buffer, BUFFER_SIZE);
    printf("Zero count in buffer: %d\n", zero_count);
    
    /* Test Pattern 6: Increment array */
    int mutable_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        mutable_array[i] = i;
    }
    increment_array(mutable_array, ARRAY_SIZE);
    
    /* Test Pattern 7: Reverse copy */
    char reversed[BUFFER_SIZE];
    reverse_copy(reversed, char_buffer, BUFFER_SIZE);
    
    /* Main function's own pointer traversal */
    int *main_ptr = int_array;
    int main_sum = 0;
    
    /* This loop in main should also trigger the optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        main_sum += *main_ptr;  /* Direct dereference */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    
    printf("Main pointer traversal sum: %d\n", main_sum);
    
    /* Final checksum verification */
    int final_checksum = sum1 + sum2 + zero_count + main_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    /* Additional test: volatile pointer (use sparingly) */
    volatile int *vol_ptr = (volatile int *)int_array;
    int volatile_sum = 0;
    
    for (int i = 0; i < 10; i++) {
        volatile_sum += *vol_ptr;
        vol_ptr = vol_ptr + 1;
    }
    printf("Volatile pointer sum: %d\n", volatile_sum);
    
    return 0;
}
