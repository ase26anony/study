/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access via register */
        p++;            /* increment of same register */
    }
    return sum;
}

/* Pattern 2: Pointer copy with restrict qualifier */
void copy_buffer(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *dst = *src;    /* mem access via register */
        dst++;          /* increment dst register */
        src++;          /* increment src register */
    }
}

/* Pattern 3: Fill buffer with value */
void fill_buffer(short *buf, short value, int n) {
    short *p = buf;
    short *end = buf + n;
    
    while (p < end) {
        *p = value;     /* mem store via register */
        p++;            /* increment register */
    }
}

/* Pattern 4: Mixed operations in loop */
int process_chars(const char *data, int len) {
    const char *ptr = data;
    int total = 0;
    int i;
    
    /* Split operations to create separate mem access and increment */
    for (i = 0; i < len; i++) {
        char c = *ptr;  /* mem load via register */
        ptr = ptr + 1;  /* explicit increment */
        total += c;
    }
    return total;
}

/* Pattern 5: Array traversal with pointer arithmetic */
float average_float(const float *array, int count) {
    const float *p = array;
    float sum = 0.0f;
    int i;
    
    for (i = 0; i < count; i++) {
        float val = *p; /* mem access via register */
        p = p + 1;      /* increment in separate statement */
        sum += val;
    }
    
    return count > 0 ? sum / count : 0.0f;
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;        /* Two mem accesses via registers */
        s++;            /* increment source register */
        d--;            /* decrement destination register */
    }
}

/* Pattern 7: Main function with its own pointer traversal */
int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[256];
    short short_buffer[128];
    float float_array[50];
    int reversed[100];
    
    int i, total, char_sum;
    float avg;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        int_array[i] = i + 1;
    }
    
    for (i = 0; i < 256; i++) {
        char_buffer[i] = (char)(i % 128);
    }
    
    for (i = 0; i < 128; i++) {
        short_buffer[i] = (short)(i * 2);
    }
    
    for (i = 0; i < 50; i++) {
        float_array[i] = i * 1.5f;
    }
    
    /* Test Pattern 1: Sum array with pointer traversal */
    total = sum_array_int(int_array, 100);
    printf("Sum of ints: %d (expected: 5050)\n", total);
    
    /* Test Pattern 2: Copy buffer */
    char dest_buffer[256];
    copy_buffer(dest_buffer, char_buffer, 256);
    
    /* Verify copy */
    if (memcmp(char_buffer, dest_buffer, 256) == 0) {
        printf("Buffer copy successful\n");
    } else {
        printf("Buffer copy failed\n");
    }
    
    /* Test Pattern 3: Fill buffer */
    fill_buffer(short_buffer, 42, 128);
    
    /* Verify fill */
    int fill_ok = 1;
    for (i = 0; i < 128; i++) {
        if (short_buffer[i] != 42) {
            fill_ok = 0;
            break;
        }
    }
    printf("Buffer fill: %s\n", fill_ok ? "successful" : "failed");
    
    /* Test Pattern 4: Process chars */
    char_sum = process_chars(char_buffer, 256);
    printf("Char sum: %d\n", char_sum);
    
    /* Test Pattern 5: Float average */
    avg = average_float(float_array, 50);
    printf("Float average: %.2f\n", avg);
    
    /* Test Pattern 6: Reverse copy */
    reverse_copy(reversed, int_array, 100);
    
    /* Verify reverse copy */
    int reverse_ok = 1;
    for (i = 0; i < 100; i++) {
        if (reversed[i] != int_array[99 - i]) {
            reverse_ok = 0;
            break;
        }
    }
    printf("Reverse copy: %s\n", reverse_ok ? "successful" : "failed");
    
    /* Pattern in main itself: Pointer traversal */
    {
        const int *ptr = int_array;
        int local_sum = 0;
        
        /* This loop in main should also generate the pattern */
        for (i = 0; i < 10; i++) {  /* Small loop for demonstration */
            local_sum += *ptr;      /* (mem (reg)) access */
            ptr++;                  /* register increment */
        }
        printf("Main loop partial sum: %d\n", local_sum);
    }
    
    /* Final checksum */
    int final_checksum = total + char_sum + (int)avg + fill_ok + reverse_ok;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
