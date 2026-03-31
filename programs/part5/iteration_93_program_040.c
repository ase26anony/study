#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-like loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++) {
        // Work done in update statement
    }
    return sum;
}

/* Function 3: Search with post-increment in condition */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        // Search loop
    }
    
    return (p - 1 < end && *(p - 1) == target) ? (p - arr - 1) : -1;
}

/* Function 4: Mixed volatile and non-volatile pointers */
int volatile_sum(volatile int *varr, int *regular_arr, int n) {
    int sum = 0;
    volatile int *vp = varr;
    int *rp = regular_arr;
    
    // Using post-increment on both volatile and non-volatile pointers
    for (int i = 0; i < n; i++) {
        sum += *vp++ + *rp++;
    }
    return sum;
}

/* Function 5: Structure array processing with post-increment */
float sum_structure_weights(struct Data *data, int n) {
    float total = 0.0f;
    struct Data *ptr = data;
    struct Data *end = data + n;
    
    // Post-increment on structure pointer
    for (; ptr < end; total += ptr++->weight) {
        // Work in update statement
    }
    return total;
}

/* Function 6: Complex control flow with post-increment */
void process_buffer(char *buf, int size, int threshold) {
    char *p = buf;
    char *end = buf + size;
    
    // Post-increment in if-else branches
    while (p < end) {
        if (*p >= threshold) {
            // Post-increment in taken path
            *p++ = 'A';
        } else {
            // Post-increment in not-taken path
            *p++ = 'B';
        }
    }
}

/* Function 7: Switch statement with fall-through */
int categorize_and_sum(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        switch (*p) {
            case 0:
                // Fall through with post-increment
                sum += *p++;
                // Fall through
            case 1:
                sum += *p++ * 2;
                break;
            case 2:
                // Post-increment in case body
                sum += *p++;
                break;
            default:
                // Post-increment in default
                sum += *p++ / 2;
        }
    }
    return sum;
}

/* Function 8: Nested loops with post-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int *row_end = matrix[i] + 10;
        
        // Inner loop with post-increment
        while (row_ptr < row_end) {
            *row_ptr++ *= 2;
        }
    }
}

/* Function 9: Comma expression with post-increment */
int copy_with_comma_expr(char *dest, const char *src, int n) {
    int count = 0;
    const char *s = src;
    char *d = dest;
    
    // Using comma expression to sequence access and increment
    while (count < n) {
        *d = (*s != '\0') ? (count++, *s++) : '\0';
        d++;
        if (*d == '\0') break;
    }
    return count;
}

/* Function 10: Multiple basic blocks with different increment patterns */
int mixed_increment_patterns(int *arr, int n, int pattern) {
    int result = 0;
    int *p = arr;
    int *end = arr + n;
    
    if (pattern == 0) {
        // Pattern 0: while with post-increment in condition
        while (p < end && *p++ > 0) {
            result++;
        }
    } else if (pattern == 1) {
        // Pattern 1: for with post-increment in update
        for (; p < end; p++) {
            result += *p;
        }
    } else {
        // Pattern 2: do-while with post-increment
        do {
            result -= *p++;
        } while (p < end);
    }
    
    return result;
}

int main(void) {
    // Test data
    char source[] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    int numbers[ARRAY_SIZE];
    volatile int volatile_numbers[ARRAY_SIZE];
    
    struct Data data_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i % 10;
        volatile_numbers[i] = (i % 5) + 1;
    }
    
    for (int i = 0; i < 20; i++) {
        data_array[i].value = i;
        data_array[i].id = 'A' + (i % 26);
        data_array[i].weight = i * 0.5f;
    }
    
    int matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Array summation
    int sum = sum_array(numbers, ARRAY_SIZE);
    printf("Array sum: %d\n", sum);
    
    // Test 3: Search with post-increment
    int found_index = find_value(numbers, ARRAY_SIZE, 5);
    printf("Found 5 at index: %d\n", found_index);
    
    // Test 4: Mixed volatile/non-volatile
    int volatile_sum_result = volatile_sum(volatile_numbers, numbers, ARRAY_SIZE);
    printf("Volatile sum: %d\n", volatile_sum_result);
    
    // Test 5: Structure processing
    float weight_sum = sum_structure_weights(data_array, 20);
    printf("Structure weight sum: %.2f\n", weight_sum);
    
    // Test 6: Complex control flow
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE - 1; i++) {
        buffer[i] = 'a' + (i % 26);
    }
    buffer[BUFFER_SIZE - 1] = '\0';
    
    process_buffer(buffer, BUFFER_SIZE - 1, 'm');
    printf("Processed buffer starts with: %c%c%c\n", buffer[0], buffer[1], buffer[2]);
    
    // Test 7: Switch with fall-through
    int switch_sum = categorize_and_sum(numbers, 20);
    printf("Switch sum: %d\n", switch_sum);
    
    // Test 8: Nested loops
    matrix_process(matrix, 5);
    printf("Matrix[2][3] after processing: %d\n", matrix[2][3]);
    
    // Test 9: Comma expression
    char dest2[BUFFER_SIZE];
    int copied = copy_with_comma_expr(dest2, source, 10);
    printf("Copied %d characters via comma expr: %s\n", copied, dest2);
    
    // Test 10: Multiple patterns
    int pattern_result = mixed_increment_patterns(numbers, 20, 0);
    printf("Pattern 0 result: %d\n", pattern_result);
    
    pattern_result = mixed_increment_patterns(numbers, 20, 1);
    printf("Pattern 1 result: %d\n", pattern_result);
    
    pattern_result = mixed_increment_patterns(numbers, 20, 2);
    printf("Pattern 2 result: %d\n", pattern_result);
    
    // Additional tight loops likely to generate auto-inc RTL
    // Byte-by-byte copy loop
    {
        const char *src_ptr = source;
        char *dst_ptr = destination;
        while (*src_ptr) {
            *dst_ptr++ = *src_ptr++;
        }
        *dst_ptr = '\0';
    }
    
    // Pointer arithmetic in expression
    {
        int *p = numbers;
        int total = 0;
        // Direct dereference with zero offset (ptr[0] equivalent)
        total += *p;
        p++;
        total += p[0];  // Another zero offset access
    }
    
    return 0;
}
