/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger auto-increment/decrement addressing
 * pattern recognition in GCC's RTL passes, specifically targeting the
 * find_auto_inc function and the uncovered lines in auto-inc-dec.cc.
 * 
 * Compilation recommendations:
 *   gcc -O2 -fdump-rtl-auto_inc_dec -S auto-inc-dec-test.c
 *   gcc -O3 -funroll-loops -fdump-rtl-loop2 -S auto-inc-dec-test.c
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define BUFFER_SIZE 128

/* Structure to enable pointer arithmetic with field access */
struct DataPoint {
    int value;
    int tag;
    volatile int timestamp;
};

/* Global arrays with mixed volatile qualifiers */
volatile int volatile_buffer[ARRAY_SIZE];
int regular_buffer[ARRAY_SIZE];
struct DataPoint data_points[BUFFER_SIZE];

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(volatile int *dest, const int *src, int n) {
    if (n <= 0) return;
    
    /* Classic copy loop with post-increment */
    int i = 0;
    while (i < n) {
        dest[i++] = *src++;
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_with_postinc(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    /* Post-increment in loop update */
    for (; p < end; sum += *p++) {
        /* Empty body - all work in loop header */
    }
    
    return sum;
}

/* Function 3: String copy with post-increment (byte operations) */
void str_copy_postinc(char *dest, const char *src) {
    /* Classic K&R string copy */
    while ((*dest++ = *src++) != '\0') {
        /* Empty */
    }
}

/* Function 4: Search with post-increment in condition */
int find_value_postinc(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    /* Post-increment in while condition */
    while (p < end && *p++ != target) {
        /* Continue searching */
    }
    
    return (p - 1) - arr;  /* Return index where found, or n if not found */
}

/* Function 5: Mixed volatile/non-volatile with post-increment */
void process_mixed_pointers(volatile struct DataPoint *vptr, 
                           struct DataPoint *rptr, 
                           int count) {
    int i;
    
    /* Access structure fields with pointer increment */
    for (i = 0; i < count; i++) {
        /* Comma expression: access then increment */
        int temp = vptr->value;
        vptr++;
        
        rptr->value = temp;
        rptr->tag = i;
        
        /* Another comma expression */
        (void)(rptr->timestamp), rptr++;
    }
}

/* Function 6: Nested loops with inner post-increment */
void matrix_transpose_postinc(int dest[][4], int src[][4], int rows) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        int *d_row = dest[i];
        const int *s_col = &src[0][i];
        
        /* Inner loop with post-increment and stride */
        for (j = 0; j < 4; j++) {
            *d_row++ = *s_col;
            s_col += 4;  /* Next column */
        }
    }
}

/* Function 7: Switch statement with post-increment in cases */
int switch_with_postinc(int *arr, int index, int n) {
    int result = 0;
    
    switch (index) {
        case 0:
            /* Post-increment in case body */
            result = *arr++;
            /* Fall through */
        case 1:
            /* Another post-increment */
            result += *arr++;
            break;
        case 2:
            /* Post-increment in loop inside switch */
            while (n-- > 0) {
                result += *arr++;
            }
            break;
        default:
            /* Simple post-increment */
            result = *arr++;
    }
    
    return result;
}

/* Function 8: Pointer arithmetic with zero offset */
void zero_offset_postinc(volatile int **ptr_array, int count) {
    int i;
    
    for (i = 0; i < count; i++) {
        volatile int *ptr = ptr_array[i];
        
        /* Direct dereference with zero offset */
        int val = *ptr;  /* Should generate base + 0 addressing */
        
        /* Post-increment after use */
        ptr_array[i] = ++ptr;
        
        /* Store back through different pointer */
        regular_buffer[i] = val;
    }
}

/* Main function that exercises all patterns */
int main(void) {
    int i;
    
    /* Initialize test data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        regular_buffer[i] = i;
        volatile_buffer[i] = i * 2;
    }
    
    for (i = 0; i < BUFFER_SIZE; i++) {
        data_points[i].value = i * 3;
        data_points[i].tag = i;
        data_points[i].timestamp = i * 100;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(volatile_buffer, regular_buffer, 64);
    
    /* Test 2: Summation with post-increment */
    int sum1 = sum_with_postinc(regular_buffer, 128);
    printf("Sum of first 128 elements: %d\n", sum1);
    
    /* Test 3: String operations */
    char src_str[] = "Test string for post-increment copy";
    char dest_str[100];
    str_copy_postinc(dest_str, src_str);
    printf("Copied string: %s\n", dest_str);
    
    /* Test 4: Search with post-increment */
    int index = find_value_postinc(regular_buffer, ARRAY_SIZE, 42);
    printf("Value 42 found at index: %d\n", index);
    
    /* Test 5: Mixed volatile/non-volatile */
    process_mixed_pointers(data_points, data_points + BUFFER_SIZE/2, 32);
    
    /* Test 6: Matrix operations */
    int matrix_src[4][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
    int matrix_dest[4][4];
    matrix_transpose_postinc(matrix_dest, matrix_src, 4);
    
    /* Test 7: Switch with post-increment */
    int switch_result = switch_with_postinc(regular_buffer, 1, 5);
    printf("Switch result: %d\n", switch_result);
    
    /* Test 8: Zero offset addressing */
    volatile int *ptr_array[10];
    for (i = 0; i < 10; i++) {
        ptr_array[i] = &volatile_buffer[i * 10];
    }
    zero_offset_postinc(ptr_array, 10);
    
    /* Final verification sum */
    int final_sum = 0;
    int *final_p = regular_buffer;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += *final_p++;
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
