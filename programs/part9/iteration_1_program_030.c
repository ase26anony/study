/* Program to trigger auto-inc-dec optimization for (mem (reg)) patterns */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg p)) followed by p = p + 1 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access: (mem (reg p)) */
        p = p + 1;      /* increment: p = p + 1 */
    }
    return sum;
}

/* Pattern 2: Using restrict to help alias analysis */
void copy_int_buffer(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    /* Each iteration: two (mem (reg)) accesses followed by increments */
    for (int i = 0; i < n; i++) {
        *d = *s;        /* (mem (reg d)) and (mem (reg s)) */
        d = d + 1;      /* increment d */
        s = s + 1;      /* increment s */
    }
}

/* Pattern 3: Char pointer traversal - simpler addressing mode */
int count_chars(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    /* Char access often uses simpler (mem (reg)) patterns */
    while (*p != '\0') {
        if (*p == target)  /* (mem (reg p)) */
            count++;
        p = p + 1;          /* increment p */
    }
    return count;
}

/* Pattern 4: Split operations across statements */
void process_short_buffer(short *buf, int n, short multiplier) {
    short *ptr = buf;
    
    for (int i = 0; i < n; i++) {
        short val = *ptr;           /* (mem (reg ptr)) - load */
        val *= multiplier;
        *ptr = val;                 /* (mem (reg ptr)) - store */
        ptr = ptr + 1;              /* increment ptr */
    }
}

/* Pattern 5: While loop with post-increment style */
void fill_buffer_char(char *buf, int size, char value) {
    char *p = buf;
    int remaining = size;
    
    while (remaining-- > 0) {
        *p = value;         /* (mem (reg p)) */
        p = p + 1;          /* increment p */
    }
}

/* Pattern 6: Direct pointer arithmetic in loop body */
float average_float(const float *data, int count) {
    const float *ptr = data;
    float sum = 0.0f;
    
    for (int i = 0; i < count; i++) {
        sum += *ptr;        /* (mem (reg ptr)) */
        ptr = ptr + 1;      /* increment ptr */
    }
    return (count > 0) ? sum / count : 0.0f;
}

/* Pattern 7: Mixed operations - trying to hit different modes */
void transform_buffer(int *buf, int n) {
    int *p = buf;
    
    for (int i = 0; i < n; i++) {
        /* Multiple (mem (reg)) accesses with increment */
        int x = *p;         /* (mem (reg p)) - load */
        x = x * 2 + 1;
        *p = x;             /* (mem (reg p)) - store */
        p = p + 1;          /* increment p */
    }
}

/* Pattern 8: Main function with its own traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    char char_buffer[BUFFER_SIZE];
    short short_buffer[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 2;
        int_array2[i] = 0;
        short_buffer[i] = (short)(i % 100);
        float_array[i] = i * 0.5f;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = 'A' + (i % 26);
    }
    
    /* Call pattern functions */
    int sum = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum);
    
    copy_int_buffer(int_array2, int_array, ARRAY_SIZE);
    printf("Copied %d elements\n", ARRAY_SIZE);
    
    int count = count_chars(char_buffer, 'A');
    printf("Count of 'A' in buffer: %d\n", count);
    
    process_short_buffer(short_buffer, ARRAY_SIZE, 2);
    printf("Processed short buffer\n");
    
    fill_buffer_char(char_buffer, 50, 'X');
    printf("Filled buffer with 'X'\n");
    
    float avg = average_float(float_array, ARRAY_SIZE);
    printf("Average of float array: %.2f\n", avg);
    
    transform_buffer(int_array, ARRAY_SIZE);
    printf("Transformed int array\n");
    
    /* Additional pattern in main itself */
    {
        int *ptr = int_array;
        int checksum = 0;
        
        /* This loop in main should also generate (mem (reg)) patterns */
        for (int i = 0; i < 10; i++) {
            checksum += *ptr;   /* (mem (reg ptr)) */
            ptr = ptr + 1;      /* increment ptr */
        }
        printf("Checksum of first 10 elements: %d\n", checksum);
    }
    
    /* Verify copy worked */
    int verify_sum = sum_array_int(int_array2, ARRAY_SIZE);
    printf("Verification sum: %d\n", verify_sum);
    
    return 0;
}
