#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 16

/* Structure to test pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src, size_t n) {
    // Classic strcpy-style loop - should generate post-increment
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_with_postinc(const int *arr, size_t n) {
    int sum = 0;
    const int *end = arr + n;
    
    // Post-increment in loop update
    for (const int *p = arr; p < end; sum += *p++) {
        // Empty body - all work in loop header
    }
    return sum;
}

/* Function 3: Search with post-increment in condition */
int* find_with_postinc(int *arr, size_t n, int target) {
    int *end = arr + n;
    
    // Post-increment in while condition
    while (arr < end && *arr++ != target) {
        // Continue searching
    }
    
    // arr now points one past the found element or to end
    return (arr > end) ? NULL : arr - 1;
}

/* Function 4: Mixed volatile and non-volatile pointers */
int volatile_sum(volatile int *varr, int *arr, size_t n) {
    int sum = 0;
    
    // Use post-increment with volatile pointer
    for (size_t i = 0; i < n; i++) {
        sum += *varr++ + *arr++;
    }
    return sum;
}

/* Function 5: Structure pointer post-increment */
float sum_struct_weights(struct Data *data, size_t n) {
    float total = 0.0f;
    struct Data *end = data + n;
    
    // Post-increment accessing structure field
    for (struct Data *sptr = data; sptr < end; total += sptr++->weight) {
        // Work done in loop update
    }
    return total;
}

/* Function 6: Complex control flow with post-increment */
int conditional_postinc(int *arr, size_t n, int threshold) {
    int count = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Post-increment in if-else branches
    while (ptr < end) {
        if (*ptr > threshold) {
            // Taken path with post-increment
            count += *ptr++;
        } else {
            // Not-taken path with post-increment
            ptr++;
        }
    }
    return count;
}

/* Function 7: Switch statement with fall-through */
int switch_postinc(int *arr, size_t n) {
    int result = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    while (ptr < end) {
        switch (*ptr & 3) {
            case 0:
                // Fall through with post-increment
                result += *ptr++;
                // Fall through
            case 1:
                result -= *ptr++;
                break;
            case 2:
                ptr++;  // Skip
                break;
            case 3:
                result *= *ptr++;
                break;
        }
    }
    return result;
}

/* Function 8: Nested loops with post-increment */
void matrix_copy(int dest[][PATTERN_SIZE], int src[][PATTERN_SIZE], size_t rows) {
    for (size_t i = 0; i < rows; i++) {
        int *dptr = dest[i];
        int *sptr = src[i];
        int *end = sptr + PATTERN_SIZE;
        
        // Inner loop with post-increment
        while (sptr < end) {
            *dptr++ = *sptr++;
        }
    }
}

/* Function 9: Comma expression with post-increment */
int comma_postinc(int *arr, size_t n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    while (ptr < end) {
        // Comma expression: access then increment
        sum += (*(ptr++), *ptr);
    }
    return sum;
}

/* Function 10: Zero offset array access */
int zero_offset_postinc(int *arr, size_t n) {
    int sum = 0;
    int *ptr = arr;
    
    // Access arr[0] equivalent with pointer
    for (size_t i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset
        ptr++;          // Post-increment
    }
    return sum;
}

int main() {
    // Test data
    char source[SIZE] = "Test string for post-increment operations";
    char destination[SIZE];
    
    int data[SIZE];
    volatile int vdata[SIZE];
    
    struct Data struct_data[SIZE/4];
    
    int matrix_a[4][PATTERN_SIZE];
    int matrix_b[4][PATTERN_SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        vdata[i] = (i % 100) + 100;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        struct_data[i].value = i;
        struct_data[i].id = 'A' + (i % 26);
        struct_data[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < PATTERN_SIZE; j++) {
            matrix_a[i][j] = i * PATTERN_SIZE + j;
        }
    }
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    // Test 1: Copy with post-increment
    copy_with_postinc(destination, source, strlen(source) + 1);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Summation with post-increment
    int sum1 = sum_with_postinc(data, SIZE);
    printf("Sum test 1: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int *found = find_with_postinc(data, SIZE, 42);
    printf("Search test: Found %d at position %ld\n", 
           *found, found - data);
    
    // Test 4: Mixed volatile/non-volatile
    int sum2 = volatile_sum(vdata, data, SIZE);
    printf("Volatile sum test: %d\n", sum2);
    
    // Test 5: Structure pointer post-increment
    float weight_sum = sum_struct_weights(struct_data, SIZE/4);
    printf("Structure weight sum: %.2f\n", weight_sum);
    
    // Test 6: Conditional post-increment
    int cond_sum = conditional_postinc(data, SIZE, 50);
    printf("Conditional sum: %d\n", cond_sum);
    
    // Test 7: Switch with post-increment
    int switch_result = switch_postinc(data, SIZE/10);
    printf("Switch result: %d\n", switch_result);
    
    // Test 8: Nested loops (matrix copy)
    matrix_copy(matrix_b, matrix_a, 4);
    printf("Matrix copy complete\n");
    
    // Test 9: Comma expression
    int comma_sum = comma_postinc(data, SIZE/2);
    printf("Comma expression sum: %d\n", comma_sum);
    
    // Test 10: Zero offset access
    int zero_offset_sum = zero_offset_postinc(data, SIZE);
    printf("Zero offset sum: %d\n", zero_offset_sum);
    
    // Additional tight loops likely to generate auto-inc RTL
    printf("\nAdditional tight loop tests:\n");
    
    // Byte copy loop
    unsigned char bytes[SIZE];
    unsigned char *src_ptr = (unsigned char*)source;
    unsigned char *dst_ptr = bytes;
    unsigned char *end_ptr = src_ptr + SIZE;
    
    while (src_ptr < end_ptr) {
        *dst_ptr++ = *src_ptr++;
    }
    printf("Byte copy complete\n");
    
    // String length with post-increment
    const char *str = "Hello, World!";
    int length = 0;
    while (*str++) {
        length++;
    }
    printf("String length: %d\n", length);
    
    // Array initialization with post-increment
    int init_arr[SIZE];
    int *init_ptr = init_arr;
    int *init_end = init_arr + SIZE;
    int counter = 0;
    
    while (init_ptr < init_end) {
        *init_ptr++ = counter++;
    }
    printf("Array initialization complete\n");
    
    return 0;
}
