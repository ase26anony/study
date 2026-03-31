/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern followed by register increment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* Direct pointer dereference with increment - should generate (mem (reg)) */
    while (p < end) {
        int val = *p;      /* (mem (reg)) pattern */
        p = p + 1;         /* Separate increment - should be merged */
        sum += val;
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict to help alias analysis */
void copy_buffer(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    const char *end = src + n;
    
    /* Classic copy loop - each iteration has two (mem (reg)) patterns */
    while (s < end) {
        char c = *s;       /* (mem (reg)) for load */
        s = s + 1;         /* Separate increment */
        *d = c;            /* (mem (reg)) for store */
        d = d + 1;         /* Separate increment */
    }
}

/* Pattern 3: Pointer increment in for loop header */
void fill_buffer(short *buf, int n, short value) {
    short *p;
    
    /* Increment in loop header, dereference in body */
    for (p = buf; p < buf + n; p = p + 1) {
        *p = value;        /* (mem (reg)) pattern */
    }
}

/* Pattern 4: Mixed operations with local pointer variable */
int process_chars(const char *data, int len) {
    const char *ptr = data;
    int count = 0;
    int i;
    
    /* Split dereference and increment across statements */
    for (i = 0; i < len; i++) {
        char current = *ptr;   /* (mem (reg)) pattern */
        ptr = ptr + 1;         /* Separate increment */
        
        if (current > 'a') {
            count++;
        }
    }
    return count;
}

/* Pattern 5: Double pointer increment in same basic block */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    int *d = dst + n - 1;
    const int *s = src;
    
    while (n-- > 0) {
        int val = *s;      /* (mem (reg)) pattern */
        s = s + 1;         /* Separate increment */
        *d = val;          /* (mem (reg)) pattern */
        d = d - 1;         /* Decrement operation */
    }
}

/* Pattern 6: Main function with its own pointer traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Stack arrays for different patterns */
    int int_array[ARRAY_SIZE];
    char char_buffer[BUFFER_SIZE];
    short short_buffer[BUFFER_SIZE];
    int dest_array[ARRAY_SIZE];
    
    int i, total_sum, char_count;
    
    /* Initialize test data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        dest_array[i] = 0;
    }
    
    for (i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = 'a' + (i % 26);
        short_buffer[i] = (short)(i * 2);
    }
    
    /* Test Pattern 1: Simple pointer traversal */
    total_sum = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of 1..%d = %d (expected: %d)\n", 
           ARRAY_SIZE, total_sum, ARRAY_SIZE * (ARRAY_SIZE + 1) / 2);
    
    /* Test Pattern 2: Copy with restrict pointers */
    char dest_buffer[BUFFER_SIZE];
    copy_buffer(dest_buffer, char_buffer, BUFFER_SIZE);
    
    /* Verify copy */
    if (memcmp(char_buffer, dest_buffer, BUFFER_SIZE) == 0) {
        printf("Copy test PASSED\n");
    } else {
        printf("Copy test FAILED\n");
    }
    
    /* Test Pattern 3: Fill with value */
    fill_buffer(short_buffer, BUFFER_SIZE / 2, 42);
    
    /* Test Pattern 4: Char processing */
    char_count = process_chars(char_buffer, BUFFER_SIZE);
    printf("Characters > 'a': %d\n", char_count);
    
    /* Test Pattern 5: Reverse copy */
    reverse_copy(dest_array, int_array, ARRAY_SIZE);
    
    /* Verify reverse copy */
    int reverse_ok = 1;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dest_array[i] != int_array[ARRAY_SIZE - 1 - i]) {
            reverse_ok = 0;
            break;
        }
    }
    printf("Reverse copy test: %s\n", reverse_ok ? "PASSED" : "FAILED");
    
    /* Additional pointer traversal in main */
    {
        const int *ptr = int_array;
        int local_sum = 0;
        
        /* Another (mem (reg)) pattern in main */
        for (i = 0; i < 10; i++) {
            int val = *ptr;    /* (mem (reg)) pattern */
            ptr = ptr + 1;     /* Separate increment */
            local_sum += val;
        }
        printf("First 10 elements sum: %d\n", local_sum);
    }
    
    /* Final checksum */
    printf("All tests completed. Final checksum: %d\n", total_sum + char_count);
    
    return 0;
}
