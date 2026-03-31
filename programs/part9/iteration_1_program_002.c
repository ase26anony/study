/* auto_inc_dec_test.c
 * This program generates RTL patterns that should trigger the
 * auto-increment/decrement optimization in GCC, specifically
 * targeting the (mem (reg)) pattern followed by register increment.
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

/* Pattern 2: Copy with restrict qualifiers to help alias analysis */
void copy_int_buffer(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    for (int i = 0; i < n; i++) {
        *d = *s;        /* Simple (mem (reg)) access */
        d++;            /* Post-increment */
        s++;            /* Post-increment */
    }
}

/* Pattern 3: Char pointer traversal - different mode (QImode) */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == 'a')  /* mem access via register */
            count++;
        p++;            /* increment */
    }
    return count;
}

/* Pattern 4: Explicit split operations in same basic block */
int process_buffer(short *buf, int n) {
    short *p = buf;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        short val = *p;     /* Load via register */
        p = p + 1;          /* Explicit increment */
        total += val;
    }
    return total;
}

/* Pattern 5: Mixed operations with local pointer */
void fill_pattern(int *buf, int n, int value) {
    int *p = buf;
    int *end = buf + n;
    
    while (p < end) {
        *p = value;     /* Store via register */
        p++;            /* Post-increment */
    }
}

/* Pattern 6: Nested pointer operations */
void copy_and_transform(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    /* Multiple dereferences in same loop */
    for (int i = 0; i < n; i++) {
        int val = *s;   /* First mem access */
        s++;            /* First increment */
        
        *d = val * 2;   /* Second mem access */
        d++;            /* Second increment */
    }
}

/* Pattern 7: Main function with its own pointer traversal */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 50;
    
    /* Stack arrays - likely to generate simple address modes */
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    char str_buffer[] = "test string with some a characters for counting";
    short short_buf[BUFFER_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = 0;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        short_buf[i] = (short)(i * 2);
    }
    
    /* Test Pattern 1: Sum array with pointer traversal */
    int sum = sum_array_int(arr1, ARRAY_SIZE);
    printf("Sum of array: %d\n", sum);
    
    /* Test Pattern 2: Copy with restrict pointers */
    copy_int_buffer(arr2, arr1, ARRAY_SIZE);
    
    /* Verify copy */
    int verify_sum = sum_array_int(arr2, ARRAY_SIZE);
    printf("Copied array sum: %d\n", verify_sum);
    
    /* Test Pattern 3: Char pointer traversal */
    int char_count = count_chars(str_buffer);
    printf("Character 'a' count: %d\n", char_count);
    
    /* Test Pattern 4: Explicit split operations */
    int short_total = process_buffer(short_buf, BUFFER_SIZE);
    printf("Short buffer total: %d\n", short_total);
    
    /* Test Pattern 5: Fill pattern */
    int fill_buf[20];
    fill_pattern(fill_buf, 20, 42);
    printf("Fill pattern test: %d\n", fill_buf[10]);
    
    /* Test Pattern 6: Copy and transform */
    int src_buf[30], dst_buf[30];
    for (int i = 0; i < 30; i++) src_buf[i] = i;
    copy_and_transform(dst_buf, src_buf, 30);
    printf("Transform test: %d -> %d\n", src_buf[10], dst_buf[10]);
    
    /* Additional test in main itself */
    int *ptr = arr1;
    int local_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* Simple (mem (reg)) */
        ptr++;              /* Post-increment */
    }
    printf("Main loop sum: %d\n", local_sum);
    
    /* Final checksum to verify all operations */
    int final_checksum = sum + verify_sum + char_count + short_total + 
                         fill_buf[10] + dst_buf[10] + local_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
