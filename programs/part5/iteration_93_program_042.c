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
    for (; p < end; sum += *p++) {
        // Control flow to create multiple basic blocks
        if (sum > 1000) {
            // Another post-increment in taken path
            volatile int *q = p;
            sum += *q++;
            p = q;
        }
    }
    return sum;
}

/* Function 3: Search with post-increment in complex expression */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    volatile int *end = arr + size;
    int found = -1;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        // Nested control flow
        if ((p - arr) % 10 == 0) {
            // Comma expression with post-increment
            int temp = *(p - 1);
            p++, temp;  // p++ sequenced after access
        }
    }
    
    if (p <= end) {
        found = (int)(p - arr - 1);
    }
    return found;
}

/* Function 4: Structure array processing */
float process_structs(struct Data *sarr, int count) {
    float total = 0.0f;
    struct Data *ptr = sarr;
    struct Data *end = sarr + count;
    
    // Post-increment accessing structure field
    for (; ptr < end; total += ptr++->weight) {
        // Switch with fall-through
        switch (ptr->id) {
            case 'A':
                // Post-increment in switch case
                total += (ptr++)->value * 0.5f;
                // Fall through
            case 'B':
                // Another access with zero offset
                total += ptr->value;
                break;
            default:
                // Simple dereference
                total += *((int*)ptr);
                ptr++;
                break;
        }
    }
    return total;
}

/* Function 5: Byte buffer copy with volatile */
void copy_buffer(volatile char *dst, volatile char *src, int len) {
    volatile char *d = dst;
    volatile char *s = src;
    volatile char *end = s + len;
    
    // Tight copy loop likely to generate auto-inc
    while (s < end) {
        *d++ = *s++;
    }
}

/* Function 6: Mixed qualifier pointers */
int mixed_pointers(int *regular, volatile int *vol, int n) {
    int result = 0;
    int *r = regular;
    volatile int *v = vol;
    
    // Both pointers using post-increment
    for (int i = 0; i < n; i++) {
        // Access with zero offset then increment
        result += *r++;
        result += *v++;
        
        // Conditional with post-increment in both paths
        if (result % 2 == 0) {
            result += *(r - 1);  // Previous value
            r++;  // Extra increment
        } else {
            result += *v++;
        }
    }
    return result;
}

/* Function 7: Array access with index zero */
int access_index_zero(volatile int *arr) {
    // Direct dereference with zero offset
    int val = *arr;
    arr++;  // Post-increment separate
    
    // Array access with arr[0] - zero offset
    val += arr[0];
    arr++;
    
    return val;
}

int main() {
    // Test data
    char source[] = "Test string for post-increment operations";
    char destination[BUFFER_SIZE];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    
    struct Data struct_array[] = {
        {10, 'A', 1.5f},
        {20, 'B', 2.5f},
        {30, 'C', 3.5f},
        {40, 'A', 4.5f},
        {50, 'B', 5.5f}
    };
    
    volatile char volatile_buffer1[BUFFER_SIZE];
    volatile char volatile_buffer2[BUFFER_SIZE];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        volatile_buffer1[i] = 'A' + (i % 26);
    }
    
    printf("=== Testing Post-Increment/Decrement Patterns ===\n");
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("1. Copy result: %s\n", destination);
    
    // Test 2: Summation with volatile
    int sum = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("2. Sum of volatile array: %d\n", sum);
    
    // Test 3: Search with post-increment
    int found_idx = find_value(volatile_array, ARRAY_SIZE, 50);
    printf("3. Found 50 at index: %d\n", found_idx);
    
    // Test 4: Structure processing
    float struct_total = process_structs(struct_array, 5);
    printf("4. Structure total: %.2f\n", struct_total);
    
    // Test 5: Volatile buffer copy
    copy_buffer(volatile_buffer2, volatile_buffer1, BUFFER_SIZE);
    printf("5. Buffer copy complete, first char: %c\n", volatile_buffer2[0]);
    
    // Test 6: Mixed pointers
    int mixed_result = mixed_pointers(regular_array, volatile_array, 10);
    printf("6. Mixed pointers result: %d\n", mixed_result);
    
    // Test 7: Zero offset access
    int zero_offset_result = access_index_zero(volatile_array);
    printf("7. Zero offset access result: %d\n", zero_offset_result);
    
    // Additional tight loops for RTL generation
    printf("\n=== Additional Tight Loops ===\n");
    
    // Loop 1: Pointer arithmetic in for loop
    int loop_sum = 0;
    for (int *p = regular_array; p < &regular_array[ARRAY_SIZE]; loop_sum += *p++) {
        // All work in update
    }
    printf("Loop 1 sum: %d\n", loop_sum);
    
    // Loop 2: While with post-increment in condition
    char *str_ptr = source;
    int char_count = 0;
    while (*str_ptr++ != '\0') {
        char_count++;
    }
    printf("Loop 2 char count: %d\n", char_count);
    
    // Loop 3: Nested loops with inner post-increment
    int matrix[5][5];
    int fill_value = 0;
    for (int i = 0; i < 5; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < 5; j++) {
            *row_ptr++ = fill_value++;
        }
    }
    printf("Loop 3 matrix filled, [2][2] = %d\n", matrix[2][2]);
    
    // Loop 4: Comma expression sequencing
    volatile int *vol_ptr = volatile_array;
    int comma_result = 0;
    for (int i = 0; i < 5; i++) {
        // Memory access then increment in comma expression
        comma_result = (*vol_ptr, vol_ptr++, comma_result + 1);
    }
    printf("Loop 4 comma result: %d\n", comma_result);
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
