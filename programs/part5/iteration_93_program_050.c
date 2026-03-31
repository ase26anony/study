/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization in GCC's RTL passes, specifically
 * targeting the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses post-increment/decrement pointer arithmetic in various
 * contexts to create opportunities for auto-inc-dec addressing modes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ========== Function 1: Array copy with post-increment ========== */
/* This creates a tight loop with pointer post-increment in both source and destination */
void copy_array(int *dest, const int *src, size_t n) {
    /* Classic copy loop with post-increment */
    while (n--) {
        *dest++ = *src++;
    }
}

/* ========== Function 2: String copy with volatile source ========== */
/* Mixes volatile and non-volatile pointers with post-increment */
void copy_string_volatile(char *dest, volatile const char *src) {
    /* String copy loop with post-increment on volatile pointer */
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* ========== Function 3: Summation with post-increment in loop ========== */
int sum_array(const int *arr, size_t n) {
    int sum = 0;
    const int *p = arr;
    
    /* Summation loop with post-increment in update */
    for (; p < &arr[n]; sum += *p++) {
        /* Loop body can be empty - increment happens in for update */
    }
    
    return sum;
}

/* ========== Function 4: Search with post-increment in condition ========== */
/* Uses post-increment in loop condition with control flow */
size_t find_value(const int *arr, size_t n, int target) {
    size_t i = 0;
    const int *p = arr;
    
    /* Search loop with post-increment in condition */
    while (i < n && *p++ != target) {
        i++;
    }
    
    return (i < n) ? i : n;
}

/* ========== Function 5: Structure access with pointer increment ========== */
typedef struct {
    int x;
    int y;
    int z;
} Point3D;

int sum_points(Point3D *points, size_t n) {
    int sum = 0;
    Point3D *ptr = points;
    
    /* Structure access with post-increment */
    for (size_t i = 0; i < n; i++) {
        /* Access structure fields then increment pointer */
        sum += ptr->x + ptr->y + ptr->z;
        ptr++;
    }
    
    return sum;
}

/* ========== Function 6: Complex control flow with post-increment ========== */
/* Places post-increment operations in different basic blocks */
int process_buffer(volatile int *buf, int *results, size_t n) {
    int count = 0;
    volatile int *src = buf;
    int *dst = results;
    
    for (size_t i = 0; i < n; i++) {
        /* Post-increment in conditional paths */
        if (*src > 0) {
            /* Taken path: access and increment */
            *dst++ = *src++;
            count++;
        } else {
            /* Not-taken path: still increment */
            src++;
        }
    }
    
    return count;
}

/* ========== Function 7: Nested loops with inner post-increment ========== */
void matrix_copy(int dest[3][3], const int src[3][3]) {
    for (int i = 0; i < 3; i++) {
        const int *s = src[i];
        int *d = dest[i];
        
        /* Inner loop with post-increment */
        for (int j = 0; j < 3; j++) {
            *d++ = *s++;
        }
    }
}

/* ========== Function 8: Switch statement with fall-through ========== */
/* Uses post-increment in switch cases */
int decode_and_sum(const char *data, size_t len) {
    int sum = 0;
    const char *p = data;
    
    for (size_t i = 0; i < len; i++) {
        switch (*p++) {  /* Post-increment in switch expression */
            case 'A':
                sum += 1;
                /* Fall through */
            case 'B':
                sum += 2;
                break;
            case 'C':
                sum += 3;
                /* Access next element with post-increment in comma expression */
                sum += (*p++, 5);  /* Comma expression with post-increment */
                break;
            default:
                sum += 0;
        }
    }
    
    return sum;
}

/* ========== Function 9: Zero-offset access patterns ========== */
/* Direct pointer dereference with post-increment */
int sum_first_elements(int *arrays[], size_t num_arrays) {
    int sum = 0;
    
    for (size_t i = 0; i < num_arrays; i++) {
        /* Access element at offset 0, then increment pointer */
        sum += *(arrays[i]);  /* Simple dereference - zero offset */
        /* Could also be arrays[i][0] */
    }
    
    return sum;
}

/* ========== Function 10: Mixed volatile/non-volatile in same expr ========== */
int mixed_pointer_arithmetic(volatile int *vptr, int *ptr, size_t n) {
    int result = 0;
    
    /* Both pointers incremented in the same loop */
    for (size_t i = 0; i < n; i++) {
        result += *vptr++ + *ptr++;
    }
    
    return result;
}

/* ========== Main function ========== */
int main() {
    /* Test data */
    int src_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int dest_array[10] = {0};
    volatile int volatile_array[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    char src_string[] = "Hello, World!";
    char dest_string[20];
    Point3D points[5] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}};
    int matrix_a[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
    int matrix_b[3][3];
    const char encoded[] = "ABCABCA";
    int *array_ptrs[3] = {src_array, dest_array, (int*)volatile_array};
    
    printf("Auto-Inc/Dec Test Program\n");
    printf("=========================\n\n");
    
    /* Test 1: Array copy */
    copy_array(dest_array, src_array, 10);
    printf("Test 1 - Array copy: dest_array[5] = %d\n", dest_array[5]);
    
    /* Test 2: String copy with volatile */
    copy_string_volatile(dest_string, src_string);
    printf("Test 2 - String copy: \"%s\"\n", dest_string);
    
    /* Test 3: Summation */
    int sum = sum_array(src_array, 10);
    printf("Test 3 - Summation: sum = %d\n", sum);
    
    /* Test 4: Search */
    size_t found_idx = find_value(src_array, 10, 7);
    printf("Test 4 - Search: value 7 at index %zu\n", found_idx);
    
    /* Test 5: Structure access */
    int point_sum = sum_points(points, 5);
    printf("Test 5 - Structure sum: %d\n", point_sum);
    
    /* Test 6: Complex control flow */
    int results[10];
    int processed = process_buffer(volatile_array, results, 10);
    printf("Test 6 - Processed %d positive values\n", processed);
    
    /* Test 7: Nested loops */
    matrix_copy(matrix_b, matrix_a);
    printf("Test 7 - Matrix copy: matrix_b[1][1] = %d\n", matrix_b[1][1]);
    
    /* Test 8: Switch with post-increment */
    int decoded_sum = decode_and_sum(encoded, sizeof(encoded)-1);
    printf("Test 8 - Decoded sum: %d\n", decoded_sum);
    
    /* Test 9: Zero-offset access */
    int first_sum = sum_first_elements(array_ptrs, 3);
    printf("Test 9 - First elements sum: %d\n", first_sum);
    
    /* Test 10: Mixed volatile/non-volatile */
    int mixed_sum = mixed_pointer_arithmetic(volatile_array, src_array, 5);
    printf("Test 10 - Mixed pointer sum: %d\n", mixed_sum);
    
    /* Additional tight loops that should generate good RTL patterns */
    printf("\nAdditional tight loops:\n");
    
    /* Loop with post-increment in while condition */
    const char *p = src_string;
    int char_count = 0;
    while (*p++ != '\0') {
        char_count++;
    }
    printf("String length: %d\n", char_count);
    
    /* Comma expression with post-increment */
    int temp_array[5] = {1, 2, 3, 4, 5};
    int *tptr = temp_array;
    int comma_result = 0;
    for (int i = 0; i < 5; i++) {
        comma_result += (temp_array[i] = *tptr, tptr++, temp_array[i]);
    }
    printf("Comma expression result: %d\n", comma_result);
    
    return 0;
}
