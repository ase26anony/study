/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in auto-inc-dec.cc
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
        p++;            /* increment after use */
    }
    return sum;
}

/* Pattern 2: Copy with restrict pointers - helps alias analysis */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    const char *s = src;
    char *d = dst;
    
    /* Classic memcpy pattern */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d++;            /* Post-increment of dst */
        s++;            /* Post-increment of src */
    }
}

/* Pattern 3: Fill with pointer increment */
void fill_buffer_char(char *buf, char value, int n) {
    char *p = buf;
    
    /* Simple fill pattern */
    while (n-- > 0) {
        *p = value;     /* Store with (mem (reg p)) */
        p = p + 1;      /* Explicit increment in separate statement */
    }
}

/* Pattern 4: Mixed operations in loop - testing basic block formation */
int process_short_array(short *data, int count) {
    short *ptr = data;
    int total = 0;
    int i;
    
    /* For loop with pointer traversal */
    for (i = 0; i < count; i++) {
        short val = *ptr;   /* Load: (mem (reg ptr)) */
        total += val;
        ptr = ptr + 1;      /* Increment in separate statement */
    }
    return total;
}

/* Pattern 5: Double dereference pattern */
void swap_int_blocks(int *a, int *b, int n) {
    int *pa = a;
    int *pb = b;
    
    while (n-- > 0) {
        int temp = *pa;     /* Load from pa: (mem (reg pa)) */
        *pa = *pb;          /* Load from pb, store to pa */
        *pb = temp;         /* Store to pb */
        pa++;               /* Post-increment */
        pb++;               /* Post-increment */
    }
}

/* Pattern 6: Local pointer with simple arithmetic */
int sum_first_elements(int *matrix, int rows, int cols) {
    int sum = 0;
    int *p = matrix;
    int i;
    
    /* Access every first element of row with stride */
    for (i = 0; i < rows; i++) {
        sum += *p;          /* (mem (reg p)) */
        p += cols;          /* Pointer arithmetic with stride */
    }
    return sum;
}

/* Pattern 7: Char pointer with explicit increment in loop body */
int count_char_occurrences(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == target) { /* (mem (reg p)) in comparison */
            count++;
        }
        p = p + 1;          /* Explicit increment */
    }
    return count;
}

/* Pattern 8: Multiple dereferences in same statement */
void add_arrays(int *restrict dst, const int *restrict src1, 
                const int *restrict src2, int n) {
    int *d = dst;
    const int *s1 = src1;
    const int *s2 = src2;
    
    while (n-- > 0) {
        *d = *s1 + *s2;    /* Three (mem (reg)) accesses */
        d++;
        s1++;
        s2++;
    }
}

/* Main function that exercises all patterns */
int main(void) {
    /* Test data setup */
    int int_array[100];
    char char_buffer[200];
    short short_array[50];
    int matrix[5][10];
    const char *test_string = "test string for character counting";
    
    int i, j;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        int_array[i] = i % 10;
    }
    
    for (i = 0; i < 200; i++) {
        char_buffer[i] = 'A' + (i % 26);
    }
    
    for (i = 0; i < 50; i++) {
        short_array[i] = (short)(i * 2);
    }
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute pattern tests */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    char dest_buffer[200];
    copy_buffer_restrict(dest_buffer, char_buffer, 200);
    
    fill_buffer_char(dest_buffer, 'Z', 100);
    
    int sum2 = process_short_array(short_array, 50);
    printf("Sum of short array: %d\n", sum2);
    
    int array1[10] = {1,2,3,4,5,6,7,8,9,10};
    int array2[10] = {10,20,30,40,50,60,70,80,90,100};
    swap_int_blocks(array1, array2, 5);
    
    int sum3 = sum_first_elements(&matrix[0][0], 5, 10);
    printf("Sum of first elements: %d\n", sum3);
    
    int count = count_char_occurrences(test_string, 't');
    printf("Count of 't' in string: %d\n", count);
    
    int src1[20], src2[20], dst[20];
    for (i = 0; i < 20; i++) {
        src1[i] = i;
        src2[i] = i * 2;
    }
    add_arrays(dst, src1, src2, 20);
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int local_sum = 0;
    for (i = 0; i < 10; i++) {
        local_sum += *ptr;  /* (mem (reg ptr)) */
        ptr++;              /* Post-increment */
    }
    printf("Local sum from main: %d\n", local_sum);
    
    /* Verification */
    int checksum = sum1 + sum2 + sum3 + count + local_sum;
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
