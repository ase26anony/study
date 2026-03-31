/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in auto-inc-dec.cc
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

/* Pattern 2: Char pointer with restrict to help alias analysis */
void copy_buffer(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic memcpy pattern - should trigger auto-inc */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Mixed operations in loop - testing different basic blocks */
int find_first(const char *str, char target) {
    const char *p = str;
    
    while (*p != '\0') {
        if (*p == target) {  /* (mem (reg)) access */
            return (int)(p - str);
        }
        p = p + 1;           /* Separate increment */
    }
    return -1;
}

/* Pattern 4: Pointer arithmetic split across statements */
void fill_sequence(short *arr, int n) {
    short *ptr = arr;
    short value = 0;
    
    for (int i = 0; i < n; i++) {
        *ptr = value;        /* Store via register */
        ptr = ptr + 1;       /* Increment separately */
        value += 2;
    }
}

/* Pattern 5: Nested pointer operations */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    for (int i = 0; i < n; i++) {
        *d = *s;            /* Two (mem (reg)) accesses */
        s = s + 1;          /* Increment source */
        d = d - 1;          /* Decrement destination */
    }
}

/* Pattern 6: Local pointer with no function arguments */
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
        sum += *p;      /* (mem (reg)) */
        p = p + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern 7: Using volatile to prevent over-optimization but keep pattern */
int sum_with_volatile(volatile int *arr, int n) {
    volatile int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Volatile access - still (mem (reg)) */
        p = p + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern 8: Different data types */
long long sum_large_array(const long long *arr, int n) {
    const long long *p = arr;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *p;      /* 64-bit memory access */
        p = p + 1;      /* Pointer increment */
    }
    return sum;
}

/* Main function with various test cases */
int main(void) {
    int test_int[100];
    char test_char[200];
    short test_short[50];
    long long test_llong[25];
    
    int total_checksum = 0;
    
    /* Initialize test arrays */
    for (int i = 0; i < 100; i++) {
        test_int[i] = i * 3;
    }
    
    for (int i = 0; i < 200; i++) {
        test_char[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 50; i++) {
        test_short[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < 25; i++) {
        test_llong[i] = (long long)i * 1000;
    }
    
    /* Test 1: Simple int array sum */
    int sum1 = sum_array_int(test_int, 100);
    total_checksum += sum1;
    printf("Sum1 (int array): %d\n", sum1);
    
    /* Test 2: Buffer copy */
    char dest[200];
    copy_buffer(dest, test_char, 200);
    
    /* Verify copy */
    int copy_ok = memcmp(dest, test_char, 200) == 0;
    total_checksum += copy_ok ? 1 : 0;
    printf("Copy test: %s\n", copy_ok ? "PASS" : "FAIL");
    
    /* Test 3: Char search */
    int pos = find_first("Hello, World!", 'W');
    total_checksum += pos;
    printf("Found 'W' at position: %d\n", pos);
    
    /* Test 4: Fill sequence */
    fill_sequence(test_short, 50);
    
    /* Test 5: Reverse copy */
    int src_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int dst_arr[10];
    reverse_copy(dst_arr, src_arr, 10);
    
    /* Test 6: Local array sum */
    int sum6 = sum_local_array();
    total_checksum += sum6;
    printf("Local array sum: %d\n", sum6);
    
    /* Test 7: Volatile access */
    int sum7 = sum_with_volatile(test_int, 50);
    total_checksum += sum7;
    printf("Volatile sum: %d\n", sum7);
    
    /* Test 8: 64-bit array sum */
    long long sum8 = sum_large_array(test_llong, 25);
    total_checksum += (int)sum8;
    printf("64-bit array sum: %lld\n", sum8);
    
    /* Final pointer traversal in main itself */
    int *main_ptr = test_int;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;  /* (mem (reg)) in main */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    total_checksum += main_sum;
    
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
