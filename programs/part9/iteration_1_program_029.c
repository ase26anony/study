/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets the (mem (reg)) pattern in auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* Direct pointer dereference with increment - should generate (mem (reg)) */
    while (p < end) {
        sum += *p;      /* (mem (reg p)) */
        p = p + 1;      /* Increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict to help alias analysis */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic copy pattern - each iteration has two (mem (reg)) accesses */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) patterns here */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Mixed operations in loop - force different basic block patterns */
int process_short_array(short *data, int count) {
    short *ptr = data;
    int total = 0;
    int i;
    
    /* For loop with pointer increment in update */
    for (i = 0; i < count; i++) {
        short val = *ptr;   /* (mem (reg ptr)) */
        total += val;
        ptr = ptr + 1;      /* Separate increment */
    }
    return total;
}

/* Pattern 4: Multiple dereferences before increment */
void double_deref_pattern(int *a, int *b, int *result, int n) {
    int *pa = a;
    int *pb = b;
    int *pr = result;
    
    while (n-- > 0) {
        int temp_a = *pa;   /* First (mem (reg)) */
        int temp_b = *pb;   /* Second (mem (reg)) */
        *pr = temp_a + temp_b;  /* Third (mem (reg)) */
        pa = pa + 1;        /* Increment after multiple accesses */
        pb = pb + 1;
        pr = pr + 1;
    }
}

/* Pattern 5: Simple while loop with direct increment */
int sum_chars(const char *str, int len) {
    const char *p = str;
    int sum = 0;
    
    /* Very tight loop - good for basic block formation */
    while (len > 0) {
        sum += *p;      /* (mem (reg p)) */
        p = p + 1;      /* Separate increment */
        len--;
    }
    return sum;
}

/* Pattern 6: Local array traversal using pointer */
int local_array_test(void) {
    int arr[100];
    int *p = arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 100; i++) {
        sum += *p;      /* (mem (reg p)) */
        p = p + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern 7: Volatile test - sometimes prevents, sometimes enables patterns */
int volatile_pointer_test(volatile int *vptr, int n) {
    volatile int *p = vptr;
    int sum = 0;
    
    /* Volatile access - compiler must generate explicit loads */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Volatile (mem (reg)) */
        p = p + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern 8: Nested pointer operations */
void nested_pointer_ops(int **ptr_array, int *output, int n) {
    for (int i = 0; i < n; i++) {
        int *ptr = ptr_array[i];
        /* Multiple dereferences */
        int val1 = *ptr;        /* (mem (reg ptr)) */
        ptr = ptr + 1;
        int val2 = *ptr;        /* Another (mem (reg)) */
        output[i] = val1 + val2;
    }
}

/* Main function with various test cases */
int main(void) {
    /* Test buffers */
    int int_array[100];
    char char_buffer[200];
    short short_array[50];
    int result_array[100];
    volatile int volatile_array[20];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 2;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (short)(i * 3);
    }
    
    for (int i = 0; i < 20; i++) {
        volatile_array[i] = i * 5;
    }
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(int_array, 100);
    printf("Pattern 1 sum: %d\n", sum1);
    
    /* Test Pattern 2 */
    char dest_buffer[200];
    copy_buffer_char(dest_buffer, char_buffer, 200);
    printf("Pattern 2 copy complete, first char: %c\n", dest_buffer[0]);
    
    /* Test Pattern 3 */
    int sum3 = process_short_array(short_array, 50);
    printf("Pattern 3 sum: %d\n", sum3);
    
    /* Test Pattern 4 */
    int src_a[50], src_b[50], results[50];
    for (int i = 0; i < 50; i++) {
        src_a[i] = i;
        src_b[i] = i * 2;
    }
    double_deref_pattern(src_a, src_b, results, 50);
    printf("Pattern 4 first result: %d\n", results[0]);
    
    /* Test Pattern 5 */
    int sum5 = sum_chars(char_buffer, 200);
    printf("Pattern 5 sum: %d\n", sum5);
    
    /* Test Pattern 6 */
    int sum6 = local_array_test();
    printf("Pattern 6 sum: %d\n", sum6);
    
    /* Test Pattern 7 */
    int sum7 = volatile_pointer_test(volatile_array, 20);
    printf("Pattern 7 volatile sum: %d\n", sum7);
    
    /* Test Pattern 8 */
    int *ptr_arr[10];
    int out_arr[10];
    for (int i = 0; i < 10; i++) {
        ptr_arr[i] = &int_array[i * 5];
    }
    nested_pointer_ops(ptr_arr, out_arr, 10);
    printf("Pattern 8 first output: %d\n", out_arr[0]);
    
    /* Additional main loop with pointer traversal */
    int *main_ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 100; i++) {
        main_sum += *main_ptr;  /* Another (mem (reg)) in main */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Final checksum */
    int final_checksum = sum1 + sum3 + sum5 + sum6 + sum7 + main_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    /* Verify copy worked */
    if (memcmp(char_buffer, dest_buffer, 200) == 0) {
        printf("Copy verification: PASS\n");
    } else {
        printf("Copy verification: FAIL\n");
    }
    
    return 0;
}
