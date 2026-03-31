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

/* Pattern 2: Pointer copy with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Fill with value using pointer */
void fill_buffer_short(short *buf, short value, int n) {
    short *p = buf;
    
    /* memset-like pattern */
    for (int i = 0; i < n; i++) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 4: Mixed operations in loop */
int process_buffer(const unsigned char *data, int n) {
    const unsigned char *p = data;
    int checksum = 0;
    
    /* Multiple memory accesses with pointer increments */
    while (n >= 4) {
        checksum += *p;         /* Load byte */
        p = p + 1;              /* Increment */
        
        checksum += *p << 8;    /* Another load */
        p = p + 1;              /* Another increment */
        
        checksum += *p << 16;
        p = p + 1;
        
        checksum += *p << 24;
        p = p + 1;
        
        n -= 4;
    }
    
    /* Handle remaining bytes */
    while (n-- > 0) {
        checksum += *p;
        p = p + 1;
    }
    
    return checksum;
}

/* Pattern 5: Pointer arithmetic in separate statements */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    int *d = dst + n - 1;
    const int *s = src;
    
    for (int i = 0; i < n; i++) {
        int val = *s;       /* Load via register */
        s = s + 1;          /* Increment source */
        *d = val;           /* Store via different register */
        d = d - 1;          /* Decrement destination */
    }
}

/* Pattern 6: Simple while loop with pointer */
int count_zeros(const char *buf, int n) {
    const char *p = buf;
    int count = 0;
    
    while (n-- > 0) {
        if (*p == 0) {      /* Load via register */
            count++;
        }
        p = p + 1;          /* Increment after load */
    }
    return count;
}

/* Pattern 7: Direct pointer manipulation in main */
int main(void) {
    /* Test buffers */
    int int_array[100];
    char char_buffer[256];
    short short_buffer[128];
    unsigned char data_buffer[512];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 3;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    memset(short_buffer, 0x55, sizeof(short_buffer));
    
    for (int i = 0; i < 512; i++) {
        data_buffer[i] = (unsigned char)(i % 256);
    }
    
    /* Test Pattern 1: Sum array */
    int sum = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum);
    
    /* Test Pattern 2: Copy buffer */
    char dest_buffer[256];
    copy_buffer_char(dest_buffer, char_buffer, 256);
    printf("Copy completed, first char: %c\n", dest_buffer[0]);
    
    /* Test Pattern 3: Fill buffer */
    fill_buffer_short(short_buffer, 0x1234, 128);
    printf("Fill completed, first short: 0x%04x\n", short_buffer[0]);
    
    /* Test Pattern 4: Process buffer */
    int checksum = process_buffer(data_buffer, 512);
    printf("Checksum of data buffer: %d\n", checksum);
    
    /* Test Pattern 5: Reverse copy */
    int src_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int dst_array[10];
    reverse_copy(dst_array, src_array, 10);
    printf("Reverse copy completed, first dst: %d\n", dst_array[0]);
    
    /* Test Pattern 6: Count zeros */
    char zero_test[50];
    memset(zero_test, 0, 10);  /* First 10 bytes are zero */
    int zero_count = count_zeros(zero_test, 50);
    printf("Zero count: %d\n", zero_count);
    
    /* Additional pattern in main itself */
    const int *ptr = int_array;
    int local_sum = 0;
    
    /* Simple pointer traversal in main - should also trigger the pattern */
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* (mem (reg)) access */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Local sum from main: %d\n", local_sum);
    
    /* Verify results */
    int total = sum + checksum + zero_count + local_sum;
    printf("Total checksum: %d\n", total);
    
    return (total > 0) ? 0 : 1;
}
