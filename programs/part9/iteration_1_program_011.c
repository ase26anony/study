/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_chars(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Direct dereference and increment */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Mixed operations in loop */
void fill_alternating(short *buf, int n, short val1, short val2) {
    short *p = buf;
    int i;
    
    for (i = 0; i < n; i++) {
        if (i & 1) {
            *p = val2;  /* Simple store */
        } else {
            *p = val1;  /* Simple store */
        }
        p = p + 1;      /* Increment after store */
    }
}

/* Pattern 4: Pointer arithmetic in small basic block */
int process_buffer(int *buf, int n) {
    int *p = buf;
    int total = 0;
    int temp;
    
    /* Multiple dereferences with intervening increments */
    while (n >= 4) {
        temp = *p;      /* Load via register */
        p = p + 1;      /* Increment */
        total += temp;
        
        temp = *p;      /* Another load */
        p = p + 1;      /* Another increment */
        total += temp;
        
        temp = *p;
        p = p + 1;
        total += temp;
        
        temp = *p;
        p = p + 1;
        total += temp;
        
        n -= 4;
    }
    
    /* Remainder */
    while (n-- > 0) {
        total += *p;
        p = p + 1;
    }
    
    return total;
}

/* Pattern 5: Local pointer with no function arguments */
void local_pointer_test(void) {
    int buffer[16];
    int *p = buffer;
    int i;
    
    /* Initialize */
    for (i = 0; i < 16; i++) {
        *p = i * 2;
        p = p + 1;
    }
    
    /* Process */
    p = buffer;
    for (i = 0; i < 16; i++) {
        buffer[i] = *p + 1;  /* Load via p, store via array index */
        p = p + 1;
    }
}

/* Pattern 6: Volatile test - use sparingly */
int volatile_sum(const int *arr, int n) {
    const int *p = arr;
    volatile int sum = 0;  /* Prevent some optimizations */
    
    while (n-- > 0) {
        sum += *p;
        p = p + 1;
    }
    return sum;
}

/* Pattern 7: Different data types */
unsigned short sum_shorts(const unsigned short *data, int count) {
    const unsigned short *ptr = data;
    unsigned short total = 0;
    
    while (count-- > 0) {
        total += *ptr;
        ptr = ptr + 1;
    }
    return total;
}

/* Main test driver */
int main(void) {
    int int_array[100];
    char char_src[200], char_dst[200];
    short short_buf[50];
    int i;
    int checksum = 0;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        int_array[i] = i;
    }
    
    for (i = 0; i < 200; i++) {
        char_src[i] = (char)(i % 128);
    }
    
    for (i = 0; i < 50; i++) {
        short_buf[i] = (short)(i * 3);
    }
    
    /* Test 1: Integer array sum */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    checksum += sum1;
    
    /* Test 2: Char copy */
    copy_chars(char_dst, char_src, 200);
    
    /* Verify copy */
    if (memcmp(char_src, char_dst, 200) == 0) {
        printf("Char copy successful\n");
        checksum += 1;
    }
    
    /* Test 3: Fill alternating */
    fill_alternating(short_buf, 50, 100, 200);
    
    /* Test 4: Process buffer */
    int sum2 = process_buffer(int_array, 100);
    printf("Process buffer result: %d\n", sum2);
    checksum += sum2;
    
    /* Test 5: Local pointer */
    local_pointer_test();
    
    /* Test 6: Volatile sum */
    int sum3 = volatile_sum(int_array, 100);
    printf("Volatile sum: %d\n", sum3);
    checksum += sum3;
    
    /* Test 7: Short sum */
    unsigned short short_sum = sum_shorts((unsigned short*)short_buf, 50);
    printf("Short sum: %u\n", short_sum);
    checksum += short_sum;
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Additional test: Pointer traversal in main */
    {
        int *ptr = int_array;
        int local_sum = 0;
        
        for (i = 0; i < 10; i++) {
            local_sum += *ptr;  /* Should generate (mem (reg)) */
            ptr = ptr + 1;      /* Separate increment */
        }
        printf("Main loop sum: %d\n", local_sum);
    }
    
    return 0;
}
