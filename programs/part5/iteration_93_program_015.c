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

/* Function 1: Copy with post-increment in loop condition */
void copy_with_postinc(char *dest, const char *src) {
    // Classic K&R style copy - should generate post-increment RTL
    while ((*dest++ = *src++) != '\0') {
        // Empty body - tight loop
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

/* Function 3: Search with post-increment in condition */
int find_value(volatile int *arr, int n, int target) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    int index = -1;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        index++;
    }
    
    // Adjust for post-increment
    if (index < n - 1) {
        return index + 1;
    }
    return -1;
}

/* Function 4: Structure pointer post-increment */
void process_structs(struct Data *sptr, int count) {
    struct Data *end = sptr + count;
    
    // Post-increment accessing structure field
    for (; sptr < end; ) {
        int val = sptr->value;  // Field access
        sptr++;  // Post-increment separated
        printf("Value: %d\n", val);
    }
}

/* Function 5: Complex control flow with post-increment */
int complex_postinc(volatile int *arr, int n, int threshold) {
    int result = 0;
    volatile int *p = arr;
    
    // Nested control flow
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            // Taken path - post-increment in expression
            result += *p++;
        } else {
            // Not-taken path - different post-increment pattern
            int temp = *p;
            p++;
            result -= temp;
        }
        
        // Switch with fall-through
        switch (i % 3) {
            case 0:
                // Post-increment in switch case
                result += *p++;
                break;
            case 1:
                // Fall through with post-increment
                result += *p++;
                // Fall through
            case 2:
                result += *p++;
                break;
        }
    }
    
    return result;
}

/* Function 6: Comma expression with post-increment */
int comma_postinc(volatile int *ptr) {
    // Comma expression: memory access followed by increment
    int val = (*ptr, ptr++, *ptr);
    return val;
}

/* Function 7: String length with post-increment */
int strlen_postinc(const char *str) {
    const char *p = str;
    while (*p++ != '\0') {
        // Tight loop - post-increment in condition
    }
    return (int)(p - str - 1);
}

/* Function 8: Array zero with post-increment */
void zero_array(volatile int *arr, int n) {
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // Simple zeroing loop
    while (p < end) {
        *p++ = 0;
    }
}

/* Function 9: Mixed pointer types */
void mixed_pointers(volatile int *vptr, int *regptr, int n) {
    // Use both volatile and regular pointers
    for (int i = 0; i < n; i++) {
        *regptr++ = *vptr++;  // Both post-increment
    }
}

/* Function 10: Pointer to pointer with post-increment */
void copy_2d(int **dest, int **src, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *d = dest[i];
        int *s = src[i];
        int *end = s + cols;
        
        // Inner tight loop
        while (s < end) {
            *d++ = *s++;
        }
    }
}

int main(void) {
    // Test data - mix volatile and non-volatile
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    char source_string[] = "Test string for post-increment operations";
    char dest_string[BUFFER_SIZE];
    struct Data struct_array[10];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        struct_array[i].value = i * 10;
        struct_array[i].count = i;
        struct_array[i].id = 'A' + i;
    }
    
    printf("=== Testing Post-Increment/Decrement Patterns ===\n");
    
    // Test 1: String copy with post-increment
    printf("Test 1: String copy\n");
    copy_with_postinc(dest_string, source_string);
    printf("Copied: %s\n", dest_string);
    
    // Test 2: Summation with volatile
    printf("\nTest 2: Summation with volatile array\n");
    int sum = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("Sum: %d\n", sum);
    
    // Test 3: Search with post-increment
    printf("\nTest 3: Search value 50\n");
    int found = find_value(volatile_array, ARRAY_SIZE, 50);
    printf("Found at index: %d\n", found);
    
    // Test 4: Structure processing
    printf("\nTest 4: Structure processing\n");
    process_structs(struct_array, 10);
    
    // Test 5: Complex control flow
    printf("\nTest 5: Complex control flow\n");
    int complex_result = complex_postinc(volatile_array, 20, 10);
    printf("Complex result: %d\n", complex_result);
    
    // Test 6: Comma expression
    printf("\nTest 6: Comma expression\n");
    int comma_result = comma_postinc(volatile_array);
    printf("Comma result: %d\n", comma_result);
    
    // Test 7: String length
    printf("\nTest 7: String length\n");
    int len = strlen_postinc(source_string);
    printf("Length: %d\n", len);
    
    // Test 8: Array zeroing
    printf("\nTest 8: Array zeroing\n");
    volatile int test_zero[5] = {1, 2, 3, 4, 5};
    zero_array(test_zero, 5);
    printf("First element after zero: %d\n", test_zero[0]);
    
    // Test 9: Mixed pointers
    printf("\nTest 9: Mixed pointers\n");
    mixed_pointers(volatile_array, regular_array, 10);
    printf("First element copied: %d\n", regular_array[0]);
    
    // Test 10: 2D array copy
    printf("\nTest 10: 2D array processing\n");
    int *src_rows[3];
    int *dest_rows[3];
    int data1[5] = {1, 2, 3, 4, 5};
    int data2[5] = {6, 7, 8, 9, 10};
    int data3[5] = {11, 12, 13, 14, 15};
    int dest1[5], dest2[5], dest3[5];
    
    src_rows[0] = data1;
    src_rows[1] = data2;
    src_rows[2] = data3;
    dest_rows[0] = dest1;
    dest_rows[1] = dest2;
    dest_rows[2] = dest3;
    
    copy_2d(dest_rows, src_rows, 3, 5);
    printf("2D copy complete, first value: %d\n", dest1[0]);
    
    // Additional tight loops that should generate auto-inc RTL
    printf("\nAdditional tight loops:\n");
    
    // Loop with post-increment in array access
    volatile int *ptr = volatile_array;
    for (int i = 0; i < 10; i++) {
        printf("%d ", ptr[i]);  // Array indexing that may become *(ptr + i)
    }
    printf("\n");
    
    // While loop with post-increment
    ptr = volatile_array;
    int count = 0;
    while (count++ < 5) {
        printf("%d ", *ptr++);
    }
    printf("\n");
    
    // Do-while with post-increment
    ptr = volatile_array;
    count = 0;
    do {
        printf("%d ", *ptr++);
    } while (++count < 5);
    printf("\n");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
