#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment in a loop */
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

/* Pattern 2: Using restrict to help alias analysis */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic memcpy pattern - should trigger auto-inc-dec */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Separate increments */
        s = s + 1;
    }
}

/* Pattern 3: Mixed operations in loop */
void fill_buffer_short(short *buf, short value, int n) {
    short *p = buf;
    
    /* Post-increment in loop body */
    while (n-- > 0) {
        *p = value;     /* Store via register */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 4: Direct pointer arithmetic with dereference */
int process_chars(const char *data, int len) {
    const char *ptr = data;
    int count = 0;
    
    /* Split dereference and increment */
    for (int i = 0; i < len; i++) {
        char c = *ptr;      /* Load via register */
        ptr = ptr + 1;      /* Increment separately */
        if (c == 'A') count++;
    }
    return count;
}

/* Pattern 5: Nested pointer usage */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;        /* Two memory accesses */
        s = s + 1;      /* Increment source */
        d = d - 1;      /* Decrement destination */
    }
}

/* Pattern 6: Local array with pointer traversal */
int sum_local_array(void) {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *p = arr;
    int sum = 0;
    
    /* Simple pointer loop */
    for (int i = 0; i < 10; i++) {
        sum += *p;      /* Dereference */
        p = p + 1;      /* Increment */
    }
    return sum;
}

/* Pattern 7: Pointer increment in loop condition */
int count_zeros(const int *data, int n) {
    const int *p = data;
    int zeros = 0;
    
    while (n--) {
        if (*p == 0) zeros++;  /* Load via register */
        p = p + 1;             /* Increment */
    }
    return zeros;
}

/* Pattern 8: Multiple dereferences with increments */
void swap_arrays(int *a, int *b, int n) {
    int *pa = a;
    int *pb = b;
    
    while (n-- > 0) {
        int temp = *pa;     /* First load */
        *pa = *pb;          /* Load and store */
        *pb = temp;         /* Store */
        pa = pa + 1;        /* Increments */
        pb = pb + 1;
    }
}

int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[200];
    short short_buffer[50];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i % 10;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    char_buffer[50] = 'B';
    char_buffer[100] = 'C';
    
    for (int i = 0; i < 50; i++) {
        short_buffer[i] = (short)(i * 2);
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    char dest_buffer[200];
    copy_buffer_char(dest_buffer, char_buffer, 200);
    printf("Copy result: %s\n", dest_buffer[0] == 'A' ? "OK" : "FAIL");
    
    short fill_test[20];
    fill_buffer_short(fill_test, 42, 20);
    printf("Fill test: %d\n", fill_test[10] == 42 ? 1 : 0);
    
    int count = process_chars(char_buffer, 200);
    printf("Count of 'A': %d\n", count);
    
    int src_arr[5] = {1, 2, 3, 4, 5};
    int dst_arr[5];
    reverse_copy(dst_arr, src_arr, 5);
    printf("Reverse copy: %d\n", dst_arr[0] == 5 ? 1 : 0);
    
    int sum2 = sum_local_array();
    printf("Local array sum: %d\n", sum2);
    
    int zero_test[10] = {0, 1, 0, 2, 0, 3, 0, 4, 0, 5};
    int zeros = count_zeros(zero_test, 10);
    printf("Zero count: %d\n", zeros);
    
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {6, 7, 8, 9, 10};
    swap_arrays(a, b, 5);
    printf("Swap test: %d\n", a[0] == 6 ? 1 : 0);
    
    /* Additional pointer traversal in main */
    int *ptr = int_array;
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += *ptr;   /* Should generate (mem (reg)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
