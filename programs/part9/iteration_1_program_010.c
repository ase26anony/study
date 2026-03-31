/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Direct dereference and increment */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Short pointer in for loop */
int sum_array_short(const short *arr, int n) {
    const short *p = arr;
    int sum = 0;
    int i;
    
    /* For loop with pointer traversal */
    for (i = 0; i < n; i++) {
        sum += *p;      /* Simple dereference */
        p++;            /* Post-increment */
    }
    return sum;
}

/* Pattern 4: Mixed operations in basic block */
void process_buffer(int *restrict buf, int n, int mul) {
    int *p = buf;
    
    while (n-- > 0) {
        int val = *p;           /* Load via register */
        *p = val * mul;         /* Store via same register */
        p = p + 1;              /* Increment separately */
    }
}

/* Pattern 5: Nested pointer operations */
void fill_sequence(int *restrict arr, int n, int start) {
    int *p = arr;
    int value = start;
    
    while (n-- > 0) {
        *p = value++;           /* Store and increment value */
        p = p + 1;              /* Pointer increment separate */
    }
}

/* Pattern 6: Main function with its own traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    int int_array[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    char src_buffer[ARRAY_SIZE];
    char dst_buffer[ARRAY_SIZE];
    int i;
    int total = 0;
    
    /* Initialize arrays */
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        short_array[i] = (short)((i + 1) * 2);
        src_buffer[i] = (char)('A' + (i % 26));
    }
    
    /* Test Pattern 1: Simple int pointer traversal */
    total += sum_array_int(int_array, ARRAY_SIZE);
    
    /* Test Pattern 2: Char buffer copy */
    copy_buffer_char(dst_buffer, src_buffer, ARRAY_SIZE);
    
    /* Verify copy */
    if (memcmp(src_buffer, dst_buffer, ARRAY_SIZE) != 0) {
        printf("Copy failed!\n");
        return 1;
    }
    
    /* Test Pattern 3: Short pointer traversal */
    total += sum_array_short(short_array, ARRAY_SIZE);
    
    /* Test Pattern 4: Mixed operations */
    process_buffer(int_array, ARRAY_SIZE, 2);
    
    /* Test Pattern 5: Fill sequence */
    fill_sequence(int_array, 10, 100);
    
    /* Additional pattern in main: Pointer traversal */
    {
        int *ptr = int_array;
        int local_sum = 0;
        
        /* This loop in main should also generate the pattern */
        for (i = 0; i < 10; i++) {
            local_sum += *ptr;  /* Dereference */
            ptr = ptr + 1;      /* Separate increment */
        }
        total += local_sum;
    }
    
    /* Verify results */
    printf("Checksum: %d\n", total);
    printf("All patterns executed successfully.\n");
    
    return 0;
}
