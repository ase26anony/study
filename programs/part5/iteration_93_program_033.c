/* auto-inc-dec-test.c
 * 
 * This program is specifically designed to trigger the uncovered lines
 * 1352-1358 in auto-inc-dec.cc by generating RTL patterns with
 * post-increment/decrement memory addressing.
 *
 * Compilation recommendations for coverage:
 *   gcc -O2 -fdump-rtl-auto_inc_dec -fdump-rtl-all -o auto-inc-dec-test auto-inc-dec-test.c
 *   gcc -O3 -funroll-loops -fdump-rtl-loop2 -o auto-inc-dec-test auto-inc-dec-test.c
 *   gcc -O2 -mtune=generic -fdump-rtl-combine -o auto-inc-dec-test auto-inc-dec-test.c
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure for testing pointer arithmetic with field access */
struct DataPoint {
    int value;
    int timestamp;
    char label[16];
};

/* Function 1: Copy with post-increment in loop condition */
/* This should generate RTL with mem access followed by pointer increment */
void copy_with_postinc(char *dest, const char *src) {
    /* Classic K&R strcpy pattern */
    while ((*dest++ = *src++) != '\0')
        ;
}

/* Function 2: Summation with post-increment in loop update */
/* Uses volatile pointer to test different RTL patterns */
int sum_array_volatile(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* Post-increment in loop update statement */
    for (; p < end; sum += *p++)
        ;
    
    return sum;
}

/* Function 3: Search with post-increment in loop condition */
/* Mixes control flow with auto-increment patterns */
int find_value(int *array, int size, int target) {
    int *p = array;
    int *end = array + size;
    
    /* Post-increment in loop condition */
    while (p < end && *p++ != target) {
        /* Empty body - increment happens in condition */
    }
    
    return (p - 1) - array;  /* Return index where found or size */
}

/* Function 4: Nested loops with post-increment addressing */
/* Creates complex control flow graph */
void matrix_transpose(int src[4][4], int dst[4][4]) {
    for (int i = 0; i < 4; i++) {
        int *src_row = src[i];
        int *dst_col = &dst[0][i];
        
        /* Inner loop with post-increment on both pointers */
        for (int j = 0; j < 4; j++) {
            *dst_col = *src_row++;
            dst_col += 4;  /* Move to next row in same column */
        }
    }
}

/* Function 5: Structure access with post-increment */
/* Tests pointer-to-struct with field access */
int sum_struct_values(struct DataPoint *points, int count) {
    int total = 0;
    struct DataPoint *ptr = points;
    struct DataPoint *end = points + count;
    
    /* Access structure field with pointer increment */
    while (ptr < end) {
        total += ptr->value;  /* mem_insn.mem_loc = address_of_x */
        ptr++;                /* Should trigger find_inc(true) */
    }
    
    return total;
}

/* Function 6: Comma expression with post-increment */
/* Explicitly sequences memory access and increment */
int copy_with_comma(char *dest, const char *src, int max) {
    int i = 0;
    char ch;
    
    /* Comma expression: access then increment */
    while (i < max && (ch = *src, src++, ch) != '\0') {
        *dest++ = ch;
        i++;
    }
    
    if (i < max) {
        *dest = '\0';
    }
    
    return i;
}

/* Function 7: Switch statement with post-increment */
/* Tests auto-increment in different control flow paths */
int process_buffer(char *buf, int size) {
    int count = 0;
    char *p = buf;
    char *end = buf + size;
    
    while (p < end) {
        switch (*p) {
            case 'A':
            case 'a':
                /* Post-increment in taken path */
                count += (*p++ == 'A') ? 2 : 1;
                break;
                
            case 'B':
            case 'b':
                /* Post-increment with different operation */
                count -= (*p++ == 'B') ? 1 : 0;
                break;
                
            default:
                /* Post-increment in not-taken path */
                p++;
                count++;
        }
    }
    
    return count;
}

/* Function 8: Mixed volatile and non-volatile pointers */
/* Tests different RTL patterns based on qualifiers */
int compare_buffers(volatile char *buf1, char *buf2, int size) {
    volatile char *p1 = buf1;
    char *p2 = buf2;
    int differences = 0;
    
    /* Both pointers use post-increment */
    for (int i = 0; i < size; i++) {
        if (*p1++ != *p2++) {
            differences++;
        }
    }
    
    return differences;
}

/* Function 9: Pointer arithmetic with zero offset */
/* Direct dereference to trigger reg1_is_const = true, reg1_val = 0 */
int process_with_zero_offset(int **ptr_array, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* *ptr_array[i] has offset 0 */
        sum += *ptr_array[i];
        
        /* Post-increment the pointer value itself */
        (*ptr_array[i])++;
    }
    
    return sum;
}

/* Function 10: Complex loop with multiple post-increment operations */
void transform_array(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    int *end = src + size;
    
    /* Multiple post-increment operations in loop */
    while (s < end) {
        /* Comma expression with post-increment */
        int val = (*s++, *s++);  /* Skip one, use next */
        
        /* Conditional post-increment */
        *d++ = (val > 0) ? val : -val;
        
        /* Additional control flow */
        if (val == 0) {
            /* Another post-increment in conditional block */
            *d++ = *s++;
        }
    }
}

int main(void) {
    /* Test data setup */
    char source[BUFFER_SIZE] = "Test string for auto-increment/decrement patterns";
    char destination[BUFFER_SIZE];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    int *ptr_array[10];
    
    struct DataPoint points[5] = {
        {10, 1001, "Point1"},
        {20, 1002, "Point2"},
        {30, 1003, "Point3"},
        {40, 1004, "Point4"},
        {50, 1005, "Point5"}
    };
    
    int matrix_a[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    int matrix_b[4][4];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &regular_array[i * 5];
    }
    
    /* Test 1: String copy with post-increment */
    printf("Test 1: String copy\n");
    copy_with_postinc(destination, source);
    printf("  Copied: %s\n", destination);
    
    /* Test 2: Summation with volatile pointer */
    printf("\nTest 2: Sum with volatile pointer\n");
    int sum1 = sum_array_volatile(volatile_array, ARRAY_SIZE);
    printf("  Sum of volatile array: %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    printf("\nTest 3: Search in array\n");
    int index = find_value(regular_array, ARRAY_SIZE, 150);
    printf("  Found 150 at index: %d\n", index);
    
    /* Test 4: Matrix transpose */
    printf("\nTest 4: Matrix transpose\n");
    matrix_transpose(matrix_a, matrix_b);
    printf("  Transposed element [0][0]: %d\n", matrix_b[0][0]);
    
    /* Test 5: Structure access */
    printf("\nTest 5: Structure field summation\n");
    int struct_sum = sum_struct_values(points, 5);
    printf("  Sum of structure values: %d\n", struct_sum);
    
    /* Test 6: Comma expression */
    printf("\nTest 6: Copy with comma expression\n");
    char dest2[50];
    int copied = copy_with_comma(dest2, source, 50);
    printf("  Copied %d characters: %s\n", copied, dest2);
    
    /* Test 7: Switch with post-increment */
    printf("\nTest 7: Switch statement processing\n");
    char test_buffer[] = "AaBbCcDdEe";
    int switch_result = process_buffer(test_buffer, sizeof(test_buffer) - 1);
    printf("  Processed buffer result: %d\n", switch_result);
    
    /* Test 8: Mixed volatile/non-volatile */
    printf("\nTest 8: Mixed pointer comparison\n");
    char buf1[] = "Hello";
    volatile char buf2[] = "Hello";
    int diff = compare_buffers(buf2, buf1, 5);
    printf("  Differences found: %d\n", diff);
    
    /* Test 9: Zero offset pointer arithmetic */
    printf("\nTest 9: Zero offset processing\n");
    int zero_offset_sum = process_with_zero_offset(ptr_array, 10);
    printf("  Sum with zero offset: %d\n", zero_offset_sum);
    
    /* Test 10: Complex transformation */
    printf("\nTest 10: Complex array transformation\n");
    int src_array[10] = {1, -2, 3, 0, -4, 5, 6, -7, 8, 9};
    int dst_array[15];
    transform_array(src_array, dst_array, 10);
    printf("  Transformed first element: %d\n", dst_array[0]);
    
    /* Verification */
    printf("\n=== Verification ===\n");
    printf("All tests completed. Check RTL dumps for auto-inc-dec patterns.\n");
    printf("Look for mem_insn.mem_loc = address_of_x with reg1_is_const = true\n");
    
    return 0;
}
