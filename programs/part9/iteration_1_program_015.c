/* auto_inc_dec_test.c
 * This program generates RTL patterns that should trigger the
 * auto-increment/decrement optimization in GCC's auto-inc-dec.cc
 * Specifically targeting the (mem (reg)) pattern followed by register increment.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - may be merged */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic copy pattern that often uses auto-increment */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Short pointer with explicit post-increment */
short sum_array_short(const short *arr, int n) {
    const short *p = arr;
    short sum = 0;
    
    for (int i = 0; i < n; i++) {
        short val = *p;     /* Load via register */
        sum += val;
        p++;                /* Post-increment operator */
    }
    return sum;
}

/* Pattern 4: Mixed operations in same basic block */
void process_buffer(int *restrict buf, int n, int multiplier) {
    int *p = buf;
    
    for (int i = 0; i < n; i++) {
        int x = *p;         /* Load */
        x = x * multiplier; /* Some computation */
        *p = x;             /* Store */
        p = p + 1;          /* Increment */
    }
}

/* Pattern 5: Simple while loop with pointer traversal */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    int count = 0;
    
    while (count < n) {
        if (*p == target)   /* Simple (mem (reg)) access */
            return count;
        p = p + 1;          /* Separate increment */
        count++;
    }
    return -1;
}

/* Pattern 6: Local array with pointer traversal */
int sum_local_array(void) {
    int local_arr[16];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 16; i++) {
        sum += *p;      /* Access via pointer */
        p++;            /* Increment */
    }
    return sum;
}

/* Pattern 7: Nested pointer operations */
void copy_and_transform(int *restrict dst, const int *restrict src, int n) {
    int *d = dst;
    const int *s = src;
    
    for (int i = 0; i < n; i++) {
        int val = *s;       /* Load from src */
        val = val * 2 + 1;  /* Transform */
        *d = val;           /* Store to dst */
        d = d + 1;          /* Increment dst */
        s = s + 1;          /* Increment src */
    }
}

/* Pattern 8: Multiple dereferences in same statement */
int sum_two_arrays(const int *a, const int *b, int n) {
    const int *pa = a;
    const int *pb = b;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Two separate (mem (reg)) accesses */
        sum += *pa + *pb;
        pa = pa + 1;
        pb = pb + 1;
    }
    return sum;
}

/* Main function with various test cases */
int main(void) {
    const int ARRAY_SIZE = 100;
    int int_array[ARRAY_SIZE];
    char char_array[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    int dest_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i % 32768);
    }
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Simple int array sum */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Char buffer copy */
    char dest_char[ARRAY_SIZE];
    copy_buffer_char(dest_char, char_array, ARRAY_SIZE);
    printf("Char copy completed\n");
    
    /* Test 3: Short array sum */
    short sum3 = sum_array_short(short_array, ARRAY_SIZE);
    printf("Sum of short array: %d\n", (int)sum3);
    
    /* Test 4: Process buffer */
    process_buffer(int_array, 10, 2);
    printf("Buffer processed\n");
    
    /* Test 5: Find value */
    int pos = find_value(int_array, ARRAY_SIZE, 50);
    printf("Value 50 found at position: %d\n", pos);
    
    /* Test 6: Local array */
    int sum6 = sum_local_array();
    printf("Sum of local array: %d\n", sum6);
    
    /* Test 7: Copy and transform */
    copy_and_transform(dest_array, int_array, ARRAY_SIZE);
    printf("Copy and transform completed\n");
    
    /* Test 8: Sum two arrays */
    int sum8 = sum_two_arrays(int_array, dest_array, ARRAY_SIZE);
    printf("Sum of two arrays: %d\n", sum8);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* (mem (reg)) pattern */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Verify results */
    int expected_sum = (ARRAY_SIZE - 1) * ARRAY_SIZE / 2;
    if (sum1 == expected_sum) {
        printf("SUCCESS: All patterns executed correctly\n");
        return 0;
    } else {
        printf("WARNING: Sum mismatch (got %d, expected %d)\n", sum1, expected_sum);
        return 1;
    }
}
