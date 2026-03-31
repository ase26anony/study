/* auto_inc_test.c - Test program for auto-increment/decrement optimization */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    while (n-- > 0) {
        sum += *p;      /* (mem (reg p)) pattern */
        p = p + 1;      /* Separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) for both src and dst */
        dst = dst + 1;
        src = src + 1;
    }
}

/* Pattern 3: Explicit split operations in basic block */
int process_ints(int *ptr) {
    int a, b;
    a = *ptr;           /* (mem (reg ptr)) */
    ptr = ptr + 1;      /* Increment in next statement */
    b = *ptr;           /* Another (mem (reg ptr)) */
    ptr = ptr + 1;      /* Another increment */
    return a + b;
}

/* Pattern 4: Short type with loop */
short sum_array_short(const short *arr, int n) {
    const short *p = arr;
    short sum = 0;
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Simple dereference */
        p++;            /* Post-increment */
    }
    return sum;
}

/* Pattern 5: Mixed operations in tight loop */
void fill_sequence(int *restrict arr, int n) {
    int *p = arr;
    int value = 0;
    while (n-- > 0) {
        *p = value;     /* Store with (mem (reg p)) */
        p = p + 1;      /* Separate increment */
        value += 2;
    }
}

/* Pattern 6: Nested pointer operations */
void double_buffer(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    
    while (n-- > 0) {
        int val = *s;   /* Load with (mem (reg s)) */
        s = s + 1;      /* Increment source */
        *d = val * 2;   /* Store with (mem (reg d)) */
        d = d + 1;      /* Increment destination */
    }
}

/* Pattern 7: Pointer arithmetic in for loop update */
void reverse_copy(char *restrict dst, const char *restrict src, int n) {
    const char *s = src;
    char *d = dst + n - 1;
    
    for (int i = 0; i < n; i++) {
        *d = *s;        /* Both are (mem (reg)) patterns */
        s = s + 1;
        d = d - 1;      /* Decrement pattern */
    }
}

/* Pattern 8: Local pointer with multiple accesses */
int sum_first_four(const int *arr) {
    const int *p = arr;
    int total = 0;
    
    total += *p; p = p + 1;
    total += *p; p = p + 1;
    total += *p; p = p + 1;
    total += *p;
    
    return total;
}

/* Main function with various test cases */
int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[200];
    short short_array[50];
    int dest_array[100];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 3;
    }
    
    for (int i = 0; i < 200; i++) {
        char_buffer[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 50; i++) {
        short_array[i] = (short)(i * 2);
    }
    
    /* Test 1: Sum int array */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Copy char buffer */
    char char_copy[200];
    copy_buffer_char(char_copy, char_buffer, 200);
    printf("Char copy verification: %s\n", 
           memcmp(char_buffer, char_copy, 200) == 0 ? "PASS" : "FAIL");
    
    /* Test 3: Process ints */
    int sum2 = process_ints(int_array);
    printf("Sum of first two ints: %d\n", sum2);
    
    /* Test 4: Sum short array */
    short sum3 = sum_array_short(short_array, 50);
    printf("Sum of short array: %d\n", (int)sum3);
    
    /* Test 5: Fill sequence */
    fill_sequence(dest_array, 100);
    printf("Fill sequence first/last: %d, %d\n", dest_array[0], dest_array[99]);
    
    /* Test 6: Double buffer */
    int doubled[50];
    double_buffer(doubled, int_array, 50);
    printf("Doubled[25] = %d (expected %d)\n", doubled[25], int_array[25] * 2);
    
    /* Test 7: Reverse copy */
    char reversed[200];
    reverse_copy(reversed, char_buffer, 200);
    printf("Reverse copy check: %s\n",
           reversed[0] == char_buffer[199] ? "PASS" : "FAIL");
    
    /* Test 8: Sum first four */
    int sum4 = sum_first_four(int_array);
    printf("Sum of first four: %d\n", sum4);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *ptr;   /* Should generate (mem (reg ptr)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Final checksum */
    int total_checksum = sum1 + sum2 + sum3 + sum4 + main_sum;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
