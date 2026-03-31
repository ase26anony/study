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
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should generate auto-inc sequences */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Explicit split operations in basic block */
int process_chars(const char *data, int len) {
    const char *ptr = data;
    int count = 0;
    
    /* Force operations to stay in same basic block */
    if (len > 0) {
        char first = *ptr;      /* (mem (reg)) */
        ptr = ptr + 1;          /* increment in next statement */
        count = first;
        
        if (len > 1) {
            char second = *ptr; /* Another (mem (reg)) */
            ptr = ptr + 1;      /* Another increment */
            count += second;
        }
    }
    return count;
}

/* Pattern 4: Short type with post-increment in loop */
int16_t sum_shorts(const int16_t *values, int count) {
    const int16_t *p = values;
    int16_t total = 0;
    
    for (int i = 0; i < count; i++) {
        total += *p;    /* Memory access */
        p++;            /* Post-increment - should combine */
    }
    return total;
}

/* Pattern 5: Pointer arithmetic in while loop */
void fill_buffer(int *buf, int value, int size) {
    int *p = buf;
    
    /* While loop keeps operations in tight sequence */
    while (size > 0) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment separately */
        size--;
    }
}

/* Pattern 6: Mixed operations to test alias analysis */
void transform_array(int *restrict out, const int *restrict in, int n) {
    /* Use restrict to help compiler understand no aliasing */
    for (int i = 0; i < n; i++) {
        int val = *in;      /* Load */
        in = in + 1;        /* Increment */
        *out = val * 2;     /* Store */
        out = out + 1;      /* Increment */
    }
}

/* Pattern 7: Direct pointer dereference in main's loop */
int main() {
    /* Test data */
    int int_array[100];
    char char_buffer[200];
    int16_t short_array[50];
    int dest_array[100];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
        dest_array[i] = 0;
    }
    
    for (int i = 0; i < 200; i++) {
        char_buffer[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (int16_t)(i * 2);
    }
    
    /* Test Pattern 1: Sum int array */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Copy buffer with restrict */
    char dest_buffer[200];
    copy_buffer_restrict(dest_buffer, char_buffer, 200);
    
    /* Verify copy */
    if (memcmp(dest_buffer, char_buffer, 200) == 0) {
        printf("Buffer copy successful\n");
    }
    
    /* Test Pattern 3: Process chars */
    int char_sum = process_chars(char_buffer, 200);
    printf("Char process result: %d\n", char_sum);
    
    /* Test Pattern 4: Sum shorts */
    int16_t short_sum = sum_shorts(short_array, 50);
    printf("Sum of shorts: %d\n", (int)short_sum);
    
    /* Test Pattern 5: Fill buffer */
    fill_buffer(dest_array, 42, 100);
    
    /* Test Pattern 6: Transform array */
    int transformed[100];
    transform_array(transformed, int_array, 100);
    
    /* Test Pattern 7: Direct pointer traversal in main */
    const int *ptr = int_array;
    int checksum = 0;
    
    /* This loop in main should also trigger the optimization */
    for (int i = 0; i < 100; i++) {
        checksum += *ptr;   /* (mem (reg)) pattern */
        ptr = ptr + 1;      /* Separate increment */
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results */
    int expected_sum = 4950; /* Sum of 0..99 */
    if (sum1 == expected_sum && checksum == expected_sum) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Test failed!\n");
        return 1;
    }
}
