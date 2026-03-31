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
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access: (mem (reg p)) */
        p++;            /* increment of p */
    }
    return sum;
}

/* Pattern 2: Pointer copy with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should trigger auto-inc-dec */
    while (n-- > 0) {
        *dst++ = *src++;  /* Two (mem (reg)) accesses with increments */
    }
}

/* Pattern 3: Separate statements for dereference and increment */
void process_chars(const char *data, int len, int *result) {
    const char *ptr = data;
    int temp;
    
    /* Explicit separate operations in same basic block */
    while (len > 0) {
        temp = *ptr;      /* (mem (reg ptr)) */
        ptr = ptr + 1;    /* increment in separate statement */
        *result += temp;
        len--;
    }
}

/* Pattern 4: Fill with value using pointer */
void fill_with_value(short *buffer, short value, int count) {
    short *p = buffer;
    
    for (int i = 0; i < count; i++) {
        *p = value;      /* (mem (reg p)) store */
        p = p + 1;       /* explicit increment */
    }
}

/* Pattern 5: Mixed types and operations */
long long sum_mixed(const int *ints, const short *shorts, int n) {
    const int *ip = ints;
    const short *sp = shorts;
    long long total = 0;
    
    for (int i = 0; i < n; i++) {
        total += *ip;    /* (mem (reg ip)) */
        ip++;            /* increment ip */
        total += *sp;    /* (mem (reg sp)) */
        sp++;            /* increment sp */
    }
    return total;
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    const char *s = src;
    char *d = dst + len - 1;
    
    while (len-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        s = s + 1;      /* increment source */
        d = d - 1;      /* decrement destination */
    }
}

/* Pattern 7: Pointer arithmetic in loop condition */
int find_value(const int *haystack, int needle, int size) {
    const int *ptr = haystack;
    const int *end = haystack + size;
    
    while (ptr != end) {
        if (*ptr == needle) {  /* (mem (reg ptr)) */
            return 1;
        }
        ptr++;  /* increment after dereference */
    }
    return 0;
}

/* Pattern 8: Local pointer with volatile to prevent over-optimization */
int sum_with_volatile(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    volatile int dummy = 0;  /* Prevent reordering */
    
    for (int i = 0; i < n; i++) {
        sum += *p;    /* (mem (reg p)) */
        dummy;        /* Memory barrier effect */
        p = p + 1;    /* increment */
    }
    return sum;
}

/* Main function with various test cases */
int main(void) {
    const int ARRAY_SIZE = 100;
    const int BUFFER_SIZE = 256;
    
    /* Test data setup */
    int int_array[ARRAY_SIZE];
    char src_buffer[BUFFER_SIZE];
    char dst_buffer[BUFFER_SIZE];
    short short_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        short_array[i] = (short)(i % 1000);
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    /* Test Pattern 1: Simple pointer traversal */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test Pattern 2: Restrict pointer copy */
    copy_buffer_restrict(dst_buffer, src_buffer, BUFFER_SIZE);
    if (memcmp(src_buffer, dst_buffer, BUFFER_SIZE) == 0) {
        printf("Copy with restrict: PASS\n");
    }
    
    /* Test Pattern 3: Separate statements */
    int result = 0;
    process_chars(src_buffer, 50, &result);
    printf("Process chars result: %d\n", result);
    
    /* Test Pattern 4: Fill with value */
    fill_with_value(short_array, 42, 10);
    printf("First short after fill: %d\n", (int)short_array[0]);
    
    /* Test Pattern 5: Mixed types */
    long long mixed_sum = sum_mixed(int_array, short_array, 20);
    printf("Mixed sum: %lld\n", mixed_sum);
    
    /* Test Pattern 6: Reverse copy */
    char reversed[50];
    reverse_copy(reversed, src_buffer, 50);
    printf("Reverse copy completed\n");
    
    /* Test Pattern 7: Search with pointer */
    int found = find_value(int_array, 42, ARRAY_SIZE);
    printf("Value 42 %s\n", found ? "found" : "not found");
    
    /* Test Pattern 8: With volatile */
    int volatile_sum = sum_with_volatile(int_array, 30);
    printf("Sum with volatile: %d\n", volatile_sum);
    
    /* Additional pattern in main itself */
    int *main_ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;  /* Should generate (mem (reg main_ptr)) */
        main_ptr++;             /* Post-increment */
    }
    printf("Main loop sum: %d\n", main_sum);
    
    /* Final checksum verification */
    int final_check = sum1 + result + (int)mixed_sum + volatile_sum + main_sum;
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
