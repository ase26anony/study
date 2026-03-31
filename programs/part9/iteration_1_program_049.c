/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

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

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;  /* Separate increment */
    }
}

/* Pattern 3: Pointer dereference in while loop */
int count_zeros(const unsigned char *data, int len) {
    const unsigned char *ptr = data;
    int count = 0;
    
    while (len-- > 0) {
        if (*ptr == 0)  /* Simple (mem (reg)) */
            count++;
        ptr = ptr + 1;  /* Separate increment */
    }
    return count;
}

/* Pattern 4: Mixed types - short pointer */
int16_t sum_shorts(const int16_t *values, int n) {
    const int16_t *p = values;
    int16_t total = 0;
    
    for (int i = 0; i < n; i++) {
        total += *p;    /* mem access */
        p = p + 1;      /* increment */
    }
    return total;
}

/* Pattern 5: Explicit split operations in basic block */
int process_chunk(int *data) {
    int *ptr = data;
    int a, b, c;
    
    /* Three consecutive memory accesses with increments */
    a = *ptr;           /* (mem (reg)) */
    ptr = ptr + 1;      /* increment */
    
    b = *ptr;           /* (mem (reg)) */
    ptr = ptr + 1;      /* increment */
    
    c = *ptr;           /* (mem (reg)) */
    ptr = ptr + 1;      /* increment */
    
    return a + b + c;
}

/* Pattern 6: Fill with value (memset-like) */
void fill_sequence(int *arr, int n, int start) {
    int *p = arr;
    int value = start;
    
    for (int i = 0; i < n; i++) {
        *p = value;     /* Store via (mem (reg)) */
        p = p + 1;      /* Increment */
        value++;
    }
}

/* Pattern 7: Pointer arithmetic in for loop header */
float average_float(const float *data, int n) {
    const float *end = data + n;
    const float *p = data;
    float sum = 0.0f;
    
    for (; p < end; p = p + 1) {
        sum += *p;      /* Should be (mem (reg)) */
    }
    return n > 0 ? sum / n : 0.0f;
}

/* Pattern 8: Main function with its own pointer traversal */
int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[256];
    int16_t short_array[50];
    float float_array[40];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    char_buffer[10] = 0;  /* Put a zero in for counting */
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (int16_t)(i * 2);
    }
    
    for (int i = 0; i < 40; i++) {
        float_array[i] = i * 0.5f;
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of ints: %d\n", sum1);
    
    char dest_buffer[256];
    copy_buffer_restrict(dest_buffer, char_buffer, 256);
    printf("Copy completed, first char: %c\n", dest_buffer[0]);
    
    int zero_count = count_zeros(char_buffer, 256);
    printf("Zero bytes: %d\n", zero_count);
    
    int16_t short_sum = sum_shorts(short_array, 50);
    printf("Sum of shorts: %d\n", (int)short_sum);
    
    int chunk_sum = process_chunk(int_array);
    printf("Chunk sum: %d\n", chunk_sum);
    
    fill_sequence(int_array, 10, 100);
    printf("Fill completed, first element: %d\n", int_array[0]);
    
    float avg = average_float(float_array, 40);
    printf("Average: %.2f\n", avg);
    
    /* Additional pointer traversal in main */
    int *ptr = int_array;
    int main_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* (mem (reg)) */
        ptr = ptr + 1;      /* increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Verify results */
    int checksum = sum1 + zero_count + short_sum + chunk_sum + main_sum;
    printf("Total checksum: %d\n", checksum);
    
    return 0;
}
