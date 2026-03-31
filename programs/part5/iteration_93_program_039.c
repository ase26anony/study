/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger auto-increment/decrement addressing
 * pattern recognition in GCC's RTL passes, specifically targeting the
 * uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 *
 * Compile with: gcc -O2 -fdump-rtl-auto_inc_dec -S auto-inc-dec-test.c
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure for testing pointer post-increment with field access */
struct DataPoint {
    int value;
    int timestamp;
    volatile int status;
};

/* Function 1: Copy with post-increment in loop (classic strcpy pattern) */
void copy_with_postinc(char *dest, const char *src) {
    /* This should generate post-increment addressing */
    while ((*dest++ = *src++) != '\0')
        ;
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* Post-increment in loop condition */
    while (p < end) {
        sum += *p++;
    }
    
    /* Additional post-increment in if statement */
    if (n > 0) {
        p = arr;
        if (*p++ > 100) {
            sum += 1000;
        }
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int search_with_postinc(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    int found = -1;
    int i = 0;
    
    /* Post-increment in loop with break */
    while (p < end) {
        if (*p++ == target) {
            found = i;
            break;
        }
        i++;
    }
    
    /* Post-increment in switch statement */
    switch (found) {
        case 0:
            p = arr;
            /* Comma expression with post-increment */
            return (*p++, 0);
        case 1:
            p = arr + 1;
            return (*p++, 1);
        default:
            return found;
    }
}

/* Function 4: Nested loops with post-increment addressing */
void process_matrix(volatile int matrix[][4], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = matrix[i];
        volatile int *row_end = row_ptr + cols;
        
        /* Inner loop with post-increment */
        while (row_ptr < row_end) {
            /* Access with zero offset */
            int val = *row_ptr;
            row_ptr++;
            
            /* Another access in conditional */
            if (val > 0 && *row_ptr++ < 0) {
                matrix[i][0] = -1;
            }
        }
    }
}

/* Function 5: Structure access with post-increment */
int process_structs(struct DataPoint *points, int n) {
    struct DataPoint *ptr = points;
    struct DataPoint *end = ptr + n;
    int total = 0;
    
    /* Post-increment with structure field access */
    while (ptr < end) {
        total += ptr->value;
        ptr->status = 1;  /* volatile access */
        ptr++;  /* Post-increment after access */
    }
    
    /* Another pattern: comma expression */
    ptr = points;
    if (n > 0) {
        int first = (ptr->value, ptr++, 0);
        total += first;
    }
    
    return total;
}

/* Function 6: Byte buffer processing with post-increment */
int checksum(const unsigned char *data, int length) {
    const unsigned char *p = data;
    const unsigned char *end = data + length;
    uint32_t sum = 0;
    
    /* Tight loop with post-increment */
    while (p < end) {
        sum += *p++;
    }
    
    /* Handle remaining bytes if any */
    if (length & 1) {
        p = data + length - 1;
        sum += *p;
    }
    
    return sum & 0xFF;
}

/* Function 7: Mixed qualifiers in same expression */
void mixed_qualifier_test(volatile int *vptr, int *ptr, int n) {
    /* Using both volatile and non-volatile pointers */
    for (int i = 0; i < n; i++) {
        *ptr++ = *vptr++ + 1;
    }
    
    /* Reset and use in conditional */
    vptr -= n;
    ptr -= n;
    
    if (n > 2) {
        int a = *vptr++;
        int b = *ptr++;
        vptr[0] = a + b;  /* Zero offset access */
    }
}

int main(void) {
    /* Test data */
    char source[BUFFER_SIZE] = "Test string for auto-increment pattern matching";
    char destination[BUFFER_SIZE] = {0};
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    int search_array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    volatile int matrix[4][4];
    struct DataPoint points[10];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        points[i].value = i * 10;
        points[i].timestamp = i;
        points[i].status = 0;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 4 + j;
        }
    }
    
    /* Test 1: String copy with post-increment */
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    /* Test 2: Summation with volatile */
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("Volatile sum: %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    int found = search_with_postinc(search_array, 10, 50);
    printf("Search for 50: found at index %d\n", found);
    
    /* Test 4: Matrix processing */
    process_matrix(matrix, 4, 4);
    printf("Matrix[0][0] = %d\n", matrix[0][0]);
    
    /* Test 5: Structure processing */
    int struct_sum = process_structs(points, 10);
    printf("Structure sum: %d\n", struct_sum);
    
    /* Test 6: Checksum */
    unsigned char data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int csum = checksum(data, sizeof(data));
    printf("Checksum: %d\n", csum);
    
    /* Test 7: Mixed qualifiers */
    mixed_qualifier_test(volatile_array, regular_array, 10);
    printf("Mixed test: regular_array[0] = %d\n", regular_array[0]);
    
    /* Additional tight loop patterns */
    /* Pattern 1: while with post-increment in condition */
    {
        const char *p = source;
        int count = 0;
        while (*p++ != '\0') {
            count++;
        }
        printf("String length: %d\n", count);
    }
    
    /* Pattern 2: for loop with post-increment */
    {
        int total = 0;
        for (const int *p = search_array; p < &search_array[10]; total += *p++)
            ;
        printf("For loop sum: %d\n", total);
    }
    
    /* Pattern 3: do-while with post-increment */
    {
        volatile int *vp = volatile_array;
        volatile int *end_vp = volatile_array + 5;
        do {
            *vp = (*vp) * 2;
        } while (++vp < end_vp);
        printf("Do-while modified first 5 elements\n");
    }
    
    /* Pattern 4: Nested loops with pointer arithmetic */
    {
        int nested_sum = 0;
        for (int i = 0; i < 3; i++) {
            volatile int *row = matrix[i];
            for (int j = 0; j < 4; j++) {
                nested_sum += *row++;
            }
        }
        printf("Nested loop sum: %d\n", nested_sum);
    }
    
    return 0;
}
