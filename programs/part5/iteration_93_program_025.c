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

/* ========== Function 1: String copy with post-increment ========== */
char* custom_strcpy(char* dest, const char* src) {
    char* d = dest;
    const char* s = src;
    
    /* Classic K&R strcpy using post-increment */
    while ((*d++ = *s++) != '\0')
        ; /* empty loop body */
    
    return dest;
}

/* ========== Function 2: Array sum with mixed volatile/non-volatile ========== */
int sum_array(volatile int* arr, int size) {
    int sum = 0;
    volatile int* p = arr;
    int* end = (int*)(arr + size);  /* Mixed pointer types */
    
    /* Loop with post-increment in condition */
    while (p < end) {
        sum += *p++;
    }
    
    return sum;
}

/* ========== Function 3: Byte buffer copy with post-increment ========== */
void copy_buffer(volatile uint8_t* dst, const uint8_t* src, size_t len) {
    volatile uint8_t* d = dst;
    const uint8_t* s = src;
    const uint8_t* end = s + len;
    
    /* Tight copy loop likely to generate auto-inc RTL */
    while (s < end) {
        *d++ = *s++;
    }
}

/* ========== Function 4: Search with post-increment in complex CFG ========== */
int find_value(volatile int* arr, int size, int target) {
    volatile int* p = arr;
    int count = 0;
    
    /* Loop with post-increment and early exit */
    while (count < size) {
        if (*p++ == target) {
            return count;
        }
        count++;
        
        /* Additional control flow to create multiple basic blocks */
        if (count % 10 == 0) {
            /* Do something different every 10 iterations */
            volatile int* temp = p - 1;
            if (*temp < 0) {
                /* Nested condition with pointer access */
                return -2;
            }
        }
    }
    
    return -1;
}

/* ========== Function 5: Structure access with post-increment ========== */
typedef struct {
    int id;
    volatile float data;
    char tag;
} DataPoint;

float sum_data_points(DataPoint* points, int count) {
    float total = 0.0f;
    DataPoint* ptr = points;
    
    /* Structure access with post-increment */
    for (int i = 0; i < count; i++) {
        total += ptr->data;
        ptr++;  /* Post-increment after access */
    }
    
    return total;
}

/* ========== Function 6: Comma expression with post-increment ========== */
int process_with_comma(volatile int* values, int n) {
    volatile int* v = values;
    int result = 0;
    
    /* Using comma expression to sequence access and increment */
    for (int i = 0; i < n; i++) {
        result += (int)(*v, v++, *v);  /* Access v, increment, access again */
    }
    
    return result;
}

/* ========== Function 7: Switch statement with post-increment ========== */
int switch_with_inc(volatile char* str) {
    volatile char* p = str;
    int score = 0;
    
    while (*p != '\0') {
        switch (*p++) {  /* Post-increment in switch expression */
            case 'A':
            case 'a':
                score += 1;
                break;
            case 'B':
            case 'b':
                score += 2;
                /* Fall through */
            case 'C':
            case 'c':
                score += *p++;  /* Post-increment in case body */
                break;
            default:
                p++;  /* Skip next character */
                break;
        }
    }
    
    return score;
}

/* ========== Function 8: Nested loops with post-increment ========== */
void matrix_multiply(volatile int* a, volatile int* b, int* result, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            volatile int* a_row = a + i * n;
            volatile int* b_col = b + j;
            
            /* Inner loop with post-increment on both pointers */
            for (int k = 0; k < n; k++) {
                sum += *a_row++ * *b_col;
                b_col += n;  /* Move to next row in column */
            }
            
            result[i * n + j] = sum;
        }
    }
}

/* ========== Main function ========== */
int main() {
    printf("Auto-Inc/Dec Test Program\n");
    printf("=========================\n\n");
    
    /* Test data */
    char source[] = "Test string for auto-inc-dec optimization";
    char destination[100];
    
    volatile int numbers[50];
    for (int i = 0; i < 50; i++) {
        numbers[i] = i + 1;
    }
    
    uint8_t buffer1[100];
    uint8_t buffer2[100];
    for (int i = 0; i < 100; i++) {
        buffer1[i] = (uint8_t)(i % 256);
    }
    
    DataPoint points[5];
    for (int i = 0; i < 5; i++) {
        points[i].id = i;
        points[i].data = (float)i * 1.5f;
        points[i].tag = 'A' + i;
    }
    
    volatile int matrix_a[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    volatile int matrix_b[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    int matrix_result[9];
    
    /* Test 1: String copy */
    printf("Test 1: String copy\n");
    custom_strcpy(destination, source);
    printf("   Copied: %s\n", destination);
    
    /* Test 2: Array sum with volatile */
    printf("\nTest 2: Array sum\n");
    int sum = sum_array(numbers, 50);
    printf("   Sum of 1..50 = %d (expected: 1275)\n", sum);
    
    /* Test 3: Buffer copy */
    printf("\nTest 3: Buffer copy\n");
    copy_buffer(buffer2, buffer1, 100);
    printf("   Buffer copied, first byte = %u\n", buffer2[0]);
    
    /* Test 4: Search with complex CFG */
    printf("\nTest 4: Search in array\n");
    int found_at = find_value(numbers, 50, 42);
    printf("   Found 42 at index: %d (expected: 41)\n", found_at);
    
    /* Test 5: Structure access */
    printf("\nTest 5: Structure data sum\n");
    float data_sum = sum_data_points(points, 5);
    printf("   Sum of data points = %.2f (expected: 22.50)\n", data_sum);
    
    /* Test 6: Comma expression */
    printf("\nTest 6: Comma expression processing\n");
    int comma_result = process_with_comma(numbers, 10);
    printf("   Comma result on first 10 elements = %d\n", comma_result);
    
    /* Test 7: Switch with increment */
    printf("\nTest 7: Switch with post-increment\n");
    char test_str[] = "aBcDeF";
    int switch_score = switch_with_inc(test_str);
    printf("   Switch score for '%s' = %d\n", test_str, switch_score);
    
    /* Test 8: Matrix multiplication */
    printf("\nTest 8: Matrix multiplication\n");
    matrix_multiply(matrix_a, matrix_b, matrix_result, 3);
    printf("   Result[0][0] = %d (expected: 30)\n", matrix_result[0]);
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
