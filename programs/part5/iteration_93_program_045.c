/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger auto-increment/decrement addressing
 * pattern recognition in GCC's RTL passes, specifically targeting the
 * uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * Compilation options for coverage:
 *   -O2 -fdump-rtl-auto_inc_dec -fdump-rtl-all
 *   -O3 -funroll-loops -fdump-rtl-loop2
 *   -O2 -mtune=generic -fdump-rtl-combine
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 256

/* Structure for testing pointer post-increment with field access */
struct DataPoint {
    int value;
    int timestamp;
    volatile int status;
};

/* Function 1: Copy with post-increment pointers (tight loop) */
void copy_with_postinc(volatile char *dest, const char *src, size_t n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic K&R style copy loop */
    while (n-- > 0) {
        *d++ = *s++;
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_array_with_postinc(volatile int *arr, int size) {
    int sum = 0;
    volatile int *p = arr;
    int *end = (int *)(arr + size);  /* Mixed pointer types */
    
    /* Loop with post-increment in update statement */
    for (; p < end; sum += *p++) {
        /* Empty body - increment happens in condition */
    }
    
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value_with_postinc(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    volatile int *end = arr + size;
    int count = 0;
    
    /* Post-increment in loop condition */
    while (p < end && *p++ != target) {
        count++;
    }
    
    return (p[-1] == target) ? (count) : -1;
}

/* Function 4: String length with post-increment (comma expression style) */
size_t strlen_postinc(const char *str) {
    const char *p = str;
    
    /* Using comma expression to sequence access and increment */
    while ((*p != '\0', p++, *p != '\0')) {
        /* The actual increment happens in the comma expression */
    }
    
    return p - str;
}

/* Function 5: Structure array processing with post-increment */
int process_structs_with_postinc(struct DataPoint *points, int count) {
    struct DataPoint *ptr = points;
    int total = 0;
    int i = 0;
    
    /* Access structure field with pointer post-increment */
    while (i++ < count) {
        total += ptr->value;
        ptr->status = 1;  /* Volatile access */
        ptr++;  /* Post-increment after access */
    }
    
    return total;
}

/* Function 6: Nested loops with inner post-increment */
void matrix_copy_with_postinc(volatile int dest[][10], int src[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        volatile int *d = dest[i];
        int *s = src[i];
        int *end = s + 10;
        
        /* Inner tight loop with post-increment */
        while (s < end) {
            *d++ = *s++;
        }
    }
}

/* Function 7: Switch statement with post-increment in cases */
int switch_with_postinc(volatile int *arr, int size, int mode) {
    volatile int *p = arr;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        switch (mode) {
            case 0:
                /* Post-increment in case body */
                result += *p++;
                break;
            case 1:
                /* Different post-increment pattern */
                result -= *p++;
                /* Fall through */
            case 2:
                /* Another access with same pointer */
                result += p[-1];
                p++;  /* Explicit increment */
                break;
            default:
                /* Comma expression with post-increment */
                result = (*p++, result + p[-1]);
                break;
        }
    }
    
    return result;
}

/* Function 8: Array zeroing with post-decrement */
void zero_array_postdec(volatile int *arr, int size) {
    volatile int *p = arr + size - 1;
    
    /* Post-decrement loop */
    while (size-- > 0) {
        *p-- = 0;
    }
}

/* Function 9: Mixed qualifiers in same expression */
int mixed_qualifiers_postinc(volatile int *varr, int *arr, int size) {
    volatile int *vp = varr;
    int *p = arr;
    int sum = 0;
    
    /* Both pointers incremented in same loop */
    for (int i = 0; i < size; i++) {
        sum += *vp++ + *p++;
    }
    
    return sum;
}

/* Function 10: Complex control flow with post-increment */
int complex_flow_postinc(volatile int *arr, int size, int threshold) {
    volatile int *p = arr;
    int sum = 0;
    int i = 0;
    
    while (i < size) {
        if (*p > threshold) {
            /* Post-increment in taken path */
            sum += *p++;
            i++;
        } else if (*p < -threshold) {
            /* Post-increment in another path */
            sum -= *p++;
            i++;
        } else {
            /* Post-increment in not-taken path */
            p++;
            i++;
            continue;
        }
        
        /* Additional increment in some cases */
        if (sum > 1000 && i < size - 1) {
            sum += *p++;
            i++;
        }
    }
    
    return sum;
}

int main(void) {
    /* Test data arrays - mix volatile and non-volatile */
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    char source_string[] = "Test string for copy operations";
    char dest_buffer[BUFFER_SIZE];
    struct DataPoint data_points[20];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        data_points[i].value = i * 5;
        data_points[i].timestamp = i;
        data_points[i].status = 0;
    }
    
    /* Test 1: Copy with post-increment */
    printf("Test 1: Copy with post-increment\n");
    copy_with_postinc((volatile char *)dest_buffer, source_string, 
                     strlen(source_string) + 1);
    printf("  Copied: %s\n", dest_buffer);
    
    /* Test 2: Summation with post-increment */
    printf("\nTest 2: Summation with post-increment\n");
    int sum1 = sum_array_with_postinc(volatile_array, ARRAY_SIZE);
    printf("  Sum of volatile array: %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    printf("\nTest 3: Search with post-increment\n");
    int found_at = find_value_with_postinc(volatile_array, ARRAY_SIZE, 50);
    printf("  Value 50 found at position: %d\n", found_at);
    
    /* Test 4: String length with comma expression */
    printf("\nTest 4: String length with comma expression\n");
    size_t len = strlen_postinc(source_string);
    printf("  String length: %zu\n", len);
    
    /* Test 5: Structure processing */
    printf("\nTest 5: Structure processing with post-increment\n");
    int struct_sum = process_structs_with_postinc(data_points, 20);
    printf("  Sum of structure values: %d\n", struct_sum);
    
    /* Test 6: Mixed qualifiers */
    printf("\nTest 6: Mixed qualifiers in same loop\n");
    int mixed_sum = mixed_qualifiers_postinc(volatile_array, regular_array, 50);
    printf("  Mixed sum: %d\n", mixed_sum);
    
    /* Test 7: Complex control flow */
    printf("\nTest 7: Complex control flow with post-increment\n");
    int complex_result = complex_flow_postinc(volatile_array, ARRAY_SIZE, 25);
    printf("  Complex flow result: %d\n", complex_result);
    
    /* Test 8: Switch statement */
    printf("\nTest 8: Switch statement with post-increment\n");
    int switch_result = switch_with_postinc(volatile_array, 20, 1);
    printf("  Switch result: %d\n", switch_result);
    
    /* Test 9: Post-decrement */
    printf("\nTest 9: Array zeroing with post-decrement\n");
    volatile int test_array[10];
    for (int i = 0; i < 10; i++) test_array[i] = i + 1;
    zero_array_postdec(test_array, 10);
    printf("  Array zeroed (first element: %d)\n", test_array[0]);
    
    /* Test 10: Matrix copy (nested loops) */
    printf("\nTest 10: Nested loops with inner post-increment\n");
    volatile int dest_matrix[5][10];
    int src_matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            src_matrix[i][j] = i * 10 + j;
        }
    }
    matrix_copy_with_postinc(dest_matrix, src_matrix, 5);
    printf("  Matrix copied (sample: %d)\n", dest_matrix[2][3]);
    
    /* Verification */
    printf("\n=== Verification ===\n");
    printf("All tests completed. Check RTL dumps for auto-inc-dec patterns.\n");
    printf("Expected: Post-increment addressing modes in generated assembly.\n");
    
    return 0;
}
