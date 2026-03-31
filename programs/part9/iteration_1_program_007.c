/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern in auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(int *restrict arr, int n) {
    int *p = arr;
    int *end = arr + n;
    int sum = 0;
    
    /* Tight loop with direct pointer dereference */
    while (p < end) {
        /* This should generate (mem (reg)) pattern */
        int val = *p;
        p = p + 1;  /* Separate increment - should be merged with load */
        sum += val;
    }
    return sum;
}

/* Pattern 2: Post-increment in expression */
int sum_array_char(const char *restrict str, int len) {
    const char *p = str;
    int sum = 0;
    
    while (len-- > 0) {
        /* Direct dereference of pointer register */
        char c = *p;
        p++;  /* Post-increment in separate statement */
        sum += c;
    }
    return sum;
}

/* Pattern 3: Memory copy with separate load/store and increments */
void copy_buffer(short *restrict dst, const short *restrict src, int n) {
    const short *s = src;
    short *d = dst;
    
    for (int i = 0; i < n; i++) {
        /* Load from source - should be (mem (reg)) */
        short val = *s;
        s = s + 1;  /* Separate increment */
        
        /* Store to destination - should be (mem (reg)) */
        *d = val;
        d = d + 1;  /* Separate increment */
    }
}

/* Pattern 4: Mixed operations in loop */
void fill_and_sum(int *restrict arr, int n, int *restrict sum_out) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Store with direct pointer dereference */
        *p = i * 2;
        int val = *p;  /* Load back - both should be (mem (reg)) */
        p = p + 1;     /* Increment after both memory ops */
        sum += val;
    }
    *sum_out = sum;
}

/* Pattern 5: Nested pointer usage */
void process_buffer(char *restrict buf, int size) {
    char *p = buf;
    char *end = buf + size;
    
    while (p < end) {
        /* Multiple dereferences of same pointer */
        char current = *p;
        *p = current + 1;  /* Store back modified value */
        p = p + 1;         /* Increment after both accesses */
    }
}

/* Pattern 6: Pointer arithmetic in loop condition */
int count_zeros(const int *restrict data, int n) {
    const int *ptr = data;
    int count = 0;
    int remaining = n;
    
    while (remaining--) {
        /* Simple dereference - no offset */
        if (*ptr == 0)
            count++;
        ptr = ptr + 1;  /* Separate increment */
    }
    return count;
}

/* Pattern 7: Main function with its own pointer traversal */
int main(void) {
    const int ARRAY_SIZE = 256;
    const int BUFFER_SIZE = 128;
    
    /* Test data */
    int int_array[ARRAY_SIZE];
    char char_buffer[BUFFER_SIZE];
    short short_src[BUFFER_SIZE / 2];
    short short_dst[BUFFER_SIZE / 2];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i % 100;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < BUFFER_SIZE / 2; i++) {
        short_src[i] = i * 3;
    }
    
    /* Test each pattern */
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Pattern 1 test */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Pattern 1 (int array sum): %d\n", sum1);
    
    /* Pattern 2 test */
    int sum2 = sum_array_char(char_buffer, BUFFER_SIZE);
    printf("Pattern 2 (char buffer sum): %d\n", sum2);
    
    /* Pattern 3 test */
    copy_buffer(short_dst, short_src, BUFFER_SIZE / 2);
    printf("Pattern 3 (short copy): copied %d elements\n", BUFFER_SIZE / 2);
    
    /* Pattern 4 test */
    int sum4;
    fill_and_sum(int_array, 100, &sum4);
    printf("Pattern 4 (fill and sum): %d\n", sum4);
    
    /* Pattern 5 test */
    process_buffer(char_buffer, 64);
    printf("Pattern 5 (buffer process): processed 64 bytes\n");
    
    /* Pattern 6 test */
    int zeros = count_zeros(int_array, ARRAY_SIZE);
    printf("Pattern 6 (count zeros): %d\n", zeros);
    
    /* Additional pointer traversal in main */
    int *ptr = int_array;
    int local_sum = 0;
    for (int i = 0; i < 50; i++) {
        int val = *ptr;  /* Should generate (mem (reg)) */
        ptr = ptr + 1;   /* Separate increment */
        local_sum += val;
    }
    printf("Main loop sum: %d\n", local_sum);
    
    /* Verification */
    int total = sum1 + sum2 + sum4 + zeros + local_sum;
    printf("\nTotal checksum: %d\n", total);
    printf("All patterns executed successfully.\n");
    
    return 0;
}
