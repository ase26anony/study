#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 50

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment pointers (tight loop) */
void copy_with_postinc(volatile char *dest, const char *src, int n) {
    // Classic strcpy-like loop with post-increment
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    // Loop with post-increment in condition
    while (p < &arr[n]) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    int found = -1;
    
    // Post-increment in loop with early exit
    for (int i = 0; i < n; i++) {
        if (*p++ == target) {
            found = i;
            // Nested control flow
            if (i % 2 == 0) {
                // Additional post-increment in taken path
                volatile int *q = arr;
                while (q < p) {
                    (*q++);
                }
            }
            break;
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
int process_structs(struct Data *sptr, int n) {
    int total = 0;
    struct Data *end = sptr + n;
    
    // Structure field access with pointer post-increment
    while (sptr < end) {
        total += sptr->value;
        // Comma expression with post-increment
        (sptr->count = 1, sptr++);
    }
    
    return total;
}

/* Function 5: Byte buffer processing with switch statement */
int process_buffer(volatile char *buf, int size) {
    int count = 0;
    volatile char *ptr = buf;
    
    for (int i = 0; i < size; i++) {
        switch (*ptr++) {  // Post-increment in switch expression
            case 'A':
                // Fall-through case with additional post-increment
            case 'B':
                count++;
                // Additional memory access with zero offset
                volatile char *tmp = ptr;
                (*tmp);
                break;
            case 'C':
                // Different path with post-increment
                count += 2;
                ptr++;  // Additional increment
                break;
            default:
                // Simple increment
                break;
        }
    }
    
    return count;
}

/* Function 6: Array initialization with post-decrement */
void init_array_reverse(int *arr, int n, int value) {
    int *p = arr + n - 1;
    
    // Post-decrement in loop
    while (p >= arr) {
        *p-- = value--;
    }
}

/* Function 7: Mixed pointer types in expression */
int mixed_pointers(volatile int *vptr, int *regptr, int n) {
    int result = 0;
    
    // Using both volatile and non-volatile pointers
    for (int i = 0; i < n; i++) {
        // Access volatile, increment regular pointer in same expression
        result += *vptr++ + *regptr++;
    }
    
    return result;
}

/* Function 8: String length with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    // Classic strlen implementation
    while (*p++ != '\0');
    
    return (int)(p - str - 1);
}

int main() {
    // Test data arrays (mix volatile and non-volatile)
    volatile char src_buffer[BUFFER_SIZE] = "Test string for auto-increment patterns";
    volatile char dest_buffer[BUFFER_SIZE];
    
    volatile int numbers[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    struct Data struct_array[20];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i + 1;
        regular_array[i] = (i + 1) * 2;
    }
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i * 3;
        struct_array[i].count = 0;
        struct_array[i].id = 'A' + (i % 26);
    }
    
    // Test 1: Copy with post-increment
    copy_with_postinc(dest_buffer, (const char*)src_buffer, BUFFER_SIZE);
    printf("Copy test: %s\n", (char*)dest_buffer);
    
    // Test 2: Summation with post-increment
    int sum = sum_with_postinc(numbers, ARRAY_SIZE);
    printf("Sum of 1..%d = %d\n", ARRAY_SIZE, sum);
    
    // Test 3: Search with post-increment
    int found_idx = find_value(numbers, ARRAY_SIZE, 42);
    printf("Found 42 at index: %d\n", found_idx);
    
    // Test 4: Structure processing
    int struct_total = process_structs(struct_array, 20);
    printf("Structure total: %d\n", struct_total);
    
    // Test 5: Buffer processing with switch
    volatile char test_buf[] = "ABCAABBC";
    int count = process_buffer(test_buf, sizeof(test_buf) - 1);
    printf("Buffer count: %d\n", count);
    
    // Test 6: Reverse initialization
    int rev_array[10];
    init_array_reverse(rev_array, 10, 100);
    printf("Reverse array first element: %d\n", rev_array[0]);
    
    // Test 7: Mixed pointers
    int mixed_result = mixed_pointers(numbers, regular_array, 10);
    printf("Mixed pointers result: %d\n", mixed_result);
    
    // Test 8: String length
    const char *test_str = "Hello, World!";
    int len = string_length(test_str);
    printf("String length: %d\n", len);
    
    // Additional complex loop with nested post-increment
    volatile int *p1 = numbers;
    int *p2 = regular_array;
    int complex_sum = 0;
    
    // Nested loops with post-increment
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            // Multiple post-increments in one expression
            complex_sum += *p1++ + *p2++;
        }
        // Reset pointers for next iteration
        p1 = numbers;
        p2 = regular_array;
    }
    printf("Complex sum: %d\n", complex_sum);
    
    // Test zero-offset access pattern
    volatile int *zero_ptr = numbers;
    int zero_test = 0;
    
    // Direct dereference with post-increment (zero offset)
    for (int i = 0; i < 5; i++) {
        zero_test += *zero_ptr;  // Simple dereference
        zero_ptr++;              // Separate increment
    }
    printf("Zero offset test: %d\n", zero_test);
    
    // Comma expression test
    volatile int *comma_ptr = numbers;
    int comma_result = 0;
    
    for (int i = 0; i < 5; i++) {
        // Comma expression: access then increment
        comma_result += (int)(*comma_ptr, comma_ptr++, 0);
    }
    printf("Comma expression test: %d\n", comma_result);
    
    return 0;
}
