/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets (mem (reg)) patterns followed by register increment
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
        sum += *p;      /* mem access via plain register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict to help alias analysis */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Tight loop with dereference and increment */
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Mixed operations in loop - should still trigger */
int find_first_zero(const int *arr, int n) {
    const int *p = arr;
    int i;
    
    for (i = 0; i < n; i++) {
        if (*p == 0)    /* mem access via register */
            return i;
        p = p + 1;      /* increment after use */
    }
    return -1;
}

/* Pattern 4: Short type with explicit pointer arithmetic */
int16_t sum_shorts(const int16_t *data, int count) {
    const int16_t *ptr = data;
    int16_t total = 0;
    
    while (count > 0) {
        total += *ptr;  /* Access via register */
        ptr = ptr + 1;  /* Increment separately */
        count--;
    }
    return total;
}

/* Pattern 5: Fill buffer with value using pointer */
void fill_with_value(int *buffer, int size, int value) {
    int *p = buffer;
    
    for (int i = 0; i < size; i++) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment pointer */
    }
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    char *d = dst + len - 1;
    const char *s = src;
    
    while (len-- > 0) {
        *d = *s;        /* Both are (mem (reg)) accesses */
        d = d - 1;      /* Decrement instead of increment */
        s = s + 1;      /* Increment source */
    }
}

/* Pattern 7: Multiple dereferences in same basic block */
int sum_product(const int *a, const int *b, int n) {
    const int *pa = a;
    const int *pb = b;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int val_a = *pa;    /* First access */
        int val_b = *pb;    /* Second access */
        sum += val_a * val_b;
        pa = pa + 1;        /* Increment after both accesses */
        pb = pb + 1;
    }
    return sum;
}

/* Pattern 8: Main function with its own pointer traversal */
int main() {
    /* Test data */
    int int_array[100];
    char char_buffer[200];
    int16_t short_array[50];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i % 10;
    }
    
    for (int i = 0; i < 200; i++) {
        char_buffer[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (int16_t)(i * 2);
    }
    
    /* Test Pattern 1: Sum array */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Copy buffer */
    char dest_buffer[200];
    copy_buffer_char(dest_buffer, char_buffer, 200);
    
    /* Verify copy */
    if (memcmp(dest_buffer, char_buffer, 200) == 0) {
        printf("Copy successful\n");
    }
    
    /* Test Pattern 3: Find zero */
    int_array[42] = 0;
    int zero_pos = find_first_zero(int_array, 100);
    printf("First zero at position: %d\n", zero_pos);
    
    /* Test Pattern 4: Sum shorts */
    int16_t short_sum = sum_shorts(short_array, 50);
    printf("Sum of shorts: %d\n", (int)short_sum);
    
    /* Test Pattern 5: Fill with value */
    int fill_array[50];
    fill_with_value(fill_array, 50, 42);
    
    /* Test Pattern 6: Reverse copy */
    char src_str[] = "HelloWorld";
    char rev_dest[20] = {0};
    reverse_copy(rev_dest, src_str, strlen(src_str));
    printf("Reverse copy: %s -> %s\n", src_str, rev_dest);
    
    /* Test Pattern 7: Sum product */
    int array_a[20], array_b[20];
    for (int i = 0; i < 20; i++) {
        array_a[i] = i;
        array_b[i] = 20 - i;
    }
    int prod_sum = sum_product(array_a, array_b, 20);
    printf("Sum of products: %d\n", prod_sum);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int local_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* (mem (reg)) access */
        ptr = ptr + 1;      /* increment in next statement */
    }
    printf("Local sum from main: %d\n", local_sum);
    
    /* Final checksum for verification */
    int final_checksum = sum1 + zero_pos + short_sum + prod_sum + local_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
