#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic strcpy-style loop with post-increment
    while ((*dest++ = *src++) != '\0') {
        // Empty body - all work in condition
    }
}

/* Function 2: Summation with post-increment in update statement */
int sum_array_postinc(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++) {
        // Work done in update statement
    }
    return sum;
}

/* Function 3: Mixed volatile and non-volatile pointers */
int find_value_postinc(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    int found = 0;
    
    // Post-increment in while condition with volatile
    while (p < end && !found) {
        if (*p++ == target) {
            found = 1;
        }
    }
    return found;
}

/* Function 4: Complex control flow with post-increment */
void process_buffer(char *buf, int size, int mode) {
    char *ptr = buf;
    char *end = buf + size;
    
    // Switch with post-increment in cases
    switch (mode) {
        case 0:
            // Simple post-increment in loop
            while (ptr < end) {
                *ptr++ = 'A';
            }
            break;
            
        case 1:
            // Post-increment with if-else
            while (ptr < end) {
                if (*ptr == '\0') {
                    *ptr++ = 'X';
                } else {
                    char temp = *ptr;
                    ptr++;
                    // Use temp
                    (void)temp;
                }
            }
            break;
            
        case 2:
            // Comma expression with post-increment
            while (ptr < end) {
                char c = (*ptr++, *ptr - 1); // Access, increment, then use
                (void)c;
            }
            break;
    }
}

/* Function 5: Structure access with pointer post-increment */
int sum_struct_values(struct Data *data, int count) {
    struct Data *ptr = data;
    struct Data *end = data + count;
    int total = 0;
    
    // Post-increment accessing structure field
    while (ptr < end) {
        total += ptr->value;  // Access field
        ptr++;                // Post-increment pointer
    }
    return total;
}

/* Function 6: Nested loops with post-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int *row_end = row_ptr + 10;
        
        // Inner loop with post-increment
        while (row_ptr < row_end) {
            // Multiple accesses with post-increment
            int val = *row_ptr++;
            *row_ptr++ = val * 2;
        }
    }
}

/* Function 7: String search with post-increment in expression */
char* find_char_postinc(const char *str, char ch) {
    const char *p = str;
    
    // Post-increment in expression
    while (*p != '\0' && *p++ != ch) {
        // Continue searching
    }
    
    return (*--p == ch) ? (char*)p : NULL;
}

/* Function 8: Byte manipulation with post-increment */
void reverse_bytes(uint8_t *data, int length) {
    uint8_t *start = data;
    uint8_t *end = data + length - 1;
    
    while (start < end) {
        // Swap using post-increment/decrement
        uint8_t temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

int main(void) {
    // Test data
    char source[] = "Test string for copying";
    char destination[50];
    
    int numbers[ARRAY_SIZE];
    volatile int volatile_numbers[ARRAY_SIZE];
    
    struct Data data_array[10];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i + 1;
        volatile_numbers[i] = (i + 1) * 2;
    }
    
    for (int i = 0; i < 10; i++) {
        data_array[i].value = i * 10;
        data_array[i].count = i;
        data_array[i].id = 'A' + i;
    }
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("Copied string: %s\n", destination);
    
    // Test 2: Summation with post-increment
    int sum = sum_array_postinc(numbers, ARRAY_SIZE);
    printf("Sum of numbers: %d\n", sum);
    
    // Test 3: Volatile array search with post-increment
    int found = find_value_postinc(volatile_numbers, ARRAY_SIZE, 42);
    printf("Found 42 in volatile array: %s\n", found ? "Yes" : "No");
    
    // Test 4: Complex control flow
    char buffer[BUFFER_SIZE];
    process_buffer(buffer, BUFFER_SIZE, 0);
    printf("Buffer[0] = %c\n", buffer[0]);
    
    // Test 5: Structure processing
    int struct_sum = sum_struct_values(data_array, 10);
    printf("Sum of struct values: %d\n", struct_sum);
    
    // Test 6: String search
    char *found_char = find_char_postinc(source, 'g');
    if (found_char) {
        printf("Found 'g' at position: %ld\n", found_char - source);
    }
    
    // Test 7: Byte reversal
    uint8_t bytes[] = {1, 2, 3, 4, 5};
    reverse_bytes(bytes, 5);
    printf("Reversed bytes: %d %d %d %d %d\n", 
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
    
    // Test 8: Mixed volatile/non-volatile in same expression
    int *regular_ptr = numbers;
    volatile int *vol_ptr = volatile_numbers;
    int mixed_sum = 0;
    
    for (int i = 0; i < 10; i++) {
        // Access both volatile and non-volatile with post-increment
        mixed_sum += *regular_ptr++ + *vol_ptr++;
    }
    printf("Mixed sum: %d\n", mixed_sum);
    
    // Test 9: Array with zero offset (triggers reg1_val = 0)
    int *ptr_to_zero = numbers;
    int zero_offset_value = *ptr_to_zero;  // Equivalent to numbers[0]
    printf("Zero offset value: %d\n", zero_offset_value);
    
    // Test 10: Post-increment in comma expression
    int *p = numbers;
    int comma_result = (*p++, *p);  // Increment, then access new value
    printf("Comma expression result: %d\n", comma_result);
    
    return 0;
}
