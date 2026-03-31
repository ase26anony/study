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

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; ) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(int *array, int size, int target) {
    int *ptr = array;
    int *end = array + size;
    int found = 0;
    
    // Post-increment in while condition with early exit
    while (ptr < end && !found) {
        if (*ptr++ == target) {
            found = 1;
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
float sum_weights(struct Data *data, int count) {
    float total = 0.0f;
    struct Data *ptr = data;
    struct Data *end = data + count;
    
    // Post-increment accessing structure field
    for (; ptr < end; ) {
        total += ptr++->weight;
    }
    
    return total;
}

/* Function 5: Nested loops with post-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int *row_end = matrix[i] + 10;
        
        // Inner loop with post-increment
        while (row_ptr < row_end) {
            // Access with zero offset then increment
            int val = *row_ptr++;
            
            // Use val to prevent optimization
            if (val < 0) {
                *(&val) = -val;  // Prevent dead code elimination
            }
        }
    }
}

/* Function 6: Switch case with post-increment */
int process_buffer(char *buf, int len) {
    char *p = buf;
    char *end = buf + len;
    int count = 0;
    
    while (p < end) {
        // Post-increment in switch expression
        switch (*p++) {
            case 'A':
            case 'a':
                count++;
                // Fall through with another increment
                if (p < end) {
                    char c = *p++;
                    if (c == 'B') count++;
                }
                break;
            case '0':
            case '1':
            case '2':
                // Multiple increments in same basic block
                if (p + 1 < end) {
                    char c1 = *p++;
                    char c2 = *p++;
                    count += (c1 - '0') + (c2 - '0');
                }
                break;
            default:
                // Simple increment
                break;
        }
    }
    
    return count;
}

/* Function 7: Comma expression with post-increment */
int copy_with_comma(char *dest, char *src, int n) {
    int i;
    char *d = dest;
    char *s = src;
    
    for (i = 0; i < n; i++) {
        // Comma expression: access then increment
        *d++ = *s++, (void)0;
    }
    
    return i;
}

/* Function 8: Mixed volatile and non-volatile pointers */
void mixed_pointers_operation(volatile int *vptr, int *regptr, int n) {
    volatile int *vend = vptr + n;
    int *rend = regptr + n;
    
    // Both pointers use post-increment
    while (vptr < vend && regptr < rend) {
        *regptr++ = *vptr++;
    }
}

int main(void) {
    // Test data arrays
    char source[BUFFER_SIZE] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    int search_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    struct Data data_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        data_array[i].value = i;
        data_array[i].id = 'A' + (i % 26);
        data_array[i].weight = i * 1.5f;
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("Copy test: %s\n", destination);
    
    // Test 2: Summation with volatile array
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("Sum of volatile array: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int found = find_value(search_array, 10, 7);
    printf("Search for 7: %s\n", found ? "Found" : "Not found");
    
    // Test 4: Structure processing
    float weight_sum = sum_weights(data_array, 20);
    printf("Total weight: %.2f\n", weight_sum);
    
    // Test 5: Matrix processing
    int matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_process(matrix, 5);
    
    // Test 6: Buffer processing with switch
    char buffer[] = "AAB123CD0";
    int count = process_buffer(buffer, sizeof(buffer) - 1);
    printf("Buffer process count: %d\n", count);
    
    // Test 7: Comma expression copy
    char src[] = "Comma test";
    char dest[20];
    int copied = copy_with_comma(dest, src, strlen(src) + 1);
    printf("Comma copy: %s (%d chars)\n", dest, copied);
    
    // Test 8: Mixed pointers
    mixed_pointers_operation(volatile_array, regular_array, ARRAY_SIZE);
    
    // Verify the copy worked
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += regular_array[i];
    }
    printf("Mixed pointers verify sum: %d\n", verify_sum);
    
    // Additional tight loop with pointer arithmetic
    int *ptr1 = regular_array;
    int *ptr2 = regular_array + ARRAY_SIZE;
    int final_sum = 0;
    
    // Very tight loop likely to generate auto-inc
    while (ptr1 < ptr2) {
        final_sum += *ptr1++;
    }
    printf("Final tight loop sum: %d\n", final_sum);
    
    return 0;
}
