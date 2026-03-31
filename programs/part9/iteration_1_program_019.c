/* auto_inc_test.c - Test program for auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern for p */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Explicit split operations in basic block */
int process_short_data(const short *data, int count) {
    const short *ptr = data;
    int total = 0;
    int i = 0;
    
    while (i < count) {
        short val = *ptr;   /* Load via register */
        ptr = ptr + 1;      /* Increment in next statement */
        total += val;
        i++;
    }
    return total;
}

/* Pattern 4: Pointer traversal with different types */
float average_float(const float *values, int n) {
    const float *p = values;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Float access via register */
        p = p + 1;      /* Separate pointer increment */
    }
    return n > 0 ? sum / n : 0.0f;
}

/* Pattern 5: Mixed operations in loop - testing basic block formation */
void fill_pattern(int *restrict buf, int size, int start) {
    int *p = buf;
    int value = start;
    
    for (int i = 0; i < size; i++) {
        *p = value;         /* Store via register */
        p = p + 1;          /* Increment separately */
        value = (value * 13 + 7) & 0xFF; /* Some computation */
    }
}

/* Pattern 6: Nested pointer usage */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;        /* Simple memory access */
        s = s + 1;      /* Increment source */
        d = d - 1;      /* Decrement destination */
    }
}

/* Pattern 7: Main function with its own pointer traversal */
int main() {
    const int ARRAY_SIZE = 256;
    const int BUFFER_SIZE = 128;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    char char_buffer1[BUFFER_SIZE];
    char char_buffer2[BUFFER_SIZE];
    short short_data[BUFFER_SIZE];
    float float_data[BUFFER_SIZE];
    int pattern_buf[BUFFER_SIZE];
    int reverse_buf[BUFFER_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char_buffer1[i] = (char)(i & 0x7F);
        short_data[i] = (short)(i * 2);
        float_data[i] = (float)i * 0.5f;
        pattern_buf[i] = 0;
        reverse_buf[i] = 0;
    }
    
    /* Call pattern functions to generate RTL sequences */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    copy_buffer_char(char_buffer2, char_buffer1, BUFFER_SIZE);
    printf("Char buffer copy completed\n");
    
    int sum2 = process_short_data(short_data, BUFFER_SIZE);
    printf("Sum of short data: %d\n", sum2);
    
    float avg = average_float(float_data, BUFFER_SIZE);
    printf("Average of float data: %.2f\n", avg);
    
    fill_pattern(pattern_buf, BUFFER_SIZE, 42);
    printf("Pattern fill completed\n");
    
    reverse_copy(reverse_buf, int_array, BUFFER_SIZE);
    printf("Reverse copy completed\n");
    
    /* Additional pointer traversal in main */
    int *ptr = int_array;
    int main_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 100; i++) {
        main_sum += *ptr;   /* Simple (mem (reg)) access */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Verify results */
    int checksum = sum1 + sum2 + main_sum;
    printf("Final checksum: %d\n", checksum);
    
    /* Quick verification of copy */
    int copy_ok = memcmp(char_buffer1, char_buffer2, BUFFER_SIZE) == 0;
    printf("Copy verification: %s\n", copy_ok ? "PASS" : "FAIL");
    
    return copy_ok ? 0 : 1;
}
