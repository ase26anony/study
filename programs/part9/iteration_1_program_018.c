/* auto_inc_dec_test.c
 * This program generates C code patterns that should trigger the
 * auto-increment/decrement optimization in GCC's RTL layer.
 * The goal is to create (mem (reg)) patterns followed by register increments.
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
        p++;            /* increment after access */
    }
    return sum;
}

/* Pattern 2: Copy with restrict pointers - helps alias analysis */
void copy_restrict(int *restrict dst, const int *restrict src, int n) {
    int *d = dst;
    const int *s = src;
    
    /* Direct pointer arithmetic in loop */
    for (int i = 0; i < n; i++) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment statement */
        s = s + 1;      /* Another separate increment */
    }
}

/* Pattern 3: Char buffer processing - different access size */
int count_chars(const char *str, int len) {
    const char *p = str;
    int count = 0;
    
    /* While loop with pointer increment */
    while (len-- > 0) {
        if (*p != 0) {  /* Simple dereference */
            count++;
        }
        p = p + 1;      /* Explicit increment */
    }
    return count;
}

/* Pattern 4: Mixed operations in basic block */
void process_buffer(short *buf, int n) {
    short *ptr = buf;
    short *end = buf + n;
    
    /* Multiple memory accesses with pointer increments */
    while (ptr < end) {
        short val = *ptr;   /* Load */
        *ptr = val * 2;     /* Store */
        ptr++;              /* Post-increment */
    }
}

/* Pattern 5: Simple memset-like function */
void fill_sequence(int *arr, int n, int start) {
    int *p = arr;
    int value = start;
    
    for (int i = 0; i < n; i++) {
        *p = value;         /* Store via register */
        p = p + 1;          /* Increment separately */
        value++;
    }
}

/* Pattern 6: Double pointer increment in same block */
void copy_and_transform(int *dst, const int *src, int n) {
    int *d = dst;
    const int *s = src;
    
    /* Two memory accesses with increments */
    for (int i = 0; i < n; i++) {
        int temp = *s;      /* First load */
        s++;                /* First increment */
        *d = temp + 1;      /* Then store */
        d++;                /* Second increment */
    }
}

/* Pattern 7: Local array traversal with pointer */
int sum_local_array(void) {
    int local_arr[100];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 100; i++) {
        sum += *p;  /* Simple dereference */
        p++;        /* Post-increment */
    }
    return sum;
}

/* Pattern 8: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    const char *s = src + len - 1;
    char *d = dst;
    
    while (len-- > 0) {
        *d = *s;    /* Load from one pointer, store to another */
        d = d + 1;  /* Increment destination */
        s = s - 1;  /* Decrement source (tests decrement patterns too) */
    }
}

/* Main function that exercises all patterns */
int main(void) {
    const int ARRAY_SIZE = 256;
    int src_arr[ARRAY_SIZE];
    int dst_arr[ARRAY_SIZE];
    char str_buf[ARRAY_SIZE];
    short short_buf[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_arr[i] = i * 2;
        dst_arr[i] = 0;
        str_buf[i] = (i % 2) ? 'A' : 'B';
        short_buf[i] = (short)i;
    }
    str_buf[ARRAY_SIZE - 1] = '\0';
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Simple sum with pointer traversal */
    int sum1 = sum_array_int(src_arr, ARRAY_SIZE);
    printf("Sum of array: %d\n", sum1);
    
    /* Test 2: Copy with restrict */
    copy_restrict(dst_arr, src_arr, ARRAY_SIZE);
    
    /* Verify copy */
    int sum2 = sum_array_int(dst_arr, ARRAY_SIZE);
    printf("Sum after copy: %d\n", sum2);
    
    /* Test 3: Char processing */
    int char_count = count_chars(str_buf, ARRAY_SIZE - 1);
    printf("Non-zero chars: %d\n", char_count);
    
    /* Test 4: Short buffer processing */
    process_buffer(short_buf, ARRAY_SIZE);
    
    /* Test 5: Fill sequence */
    fill_sequence(dst_arr, 100, 42);
    
    /* Test 6: Copy and transform */
    int temp_arr[100];
    copy_and_transform(temp_arr, src_arr, 100);
    
    /* Test 7: Local array */
    int local_sum = sum_local_array();
    printf("Local array sum: %d\n", local_sum);
    
    /* Test 8: Reverse copy */
    char reversed[ARRAY_SIZE];
    reverse_copy(reversed, str_buf, ARRAY_SIZE - 1);
    reversed[ARRAY_SIZE - 1] = '\0';
    
    /* Final verification */
    int final_sum = sum_array_int(dst_arr, ARRAY_SIZE) + 
                   sum_array_int(temp_arr, 100) +
                   local_sum;
    
    printf("Final checksum: %d\n", final_sum);
    
    /* Also include a pointer traversal in main itself */
    int *main_ptr = src_arr;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;  /* Should generate (mem (reg)) */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    if (sum1 == sum2 && final_sum > 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Test verification failed.\n");
        return 1;
    }
}
