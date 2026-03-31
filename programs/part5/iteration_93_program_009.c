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
    // This should generate post-increment addressing
    while ((*dest++ = *src++) != '\0') {
        // Empty body - tight copy loop
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    // Post-increment in loop update
    for (; p < end; sum += *p++) {
        // Control flow inside loop
        if (sum > 1000) {
            // Another post-increment in conditional path
            volatile int *temp = p;
            sum += *temp++;
            p = temp;
        }
    }
    return sum;
}

/* Function 3: Search with post-increment in while condition */
int* find_value(int *arr, int size, int target) {
    int *p = arr;
    int *end = arr + size;
    
    // Post-increment in while condition
    while (p < end && *p++ != target) {
        // Nested control flow
        if (p - arr > size / 2) {
            // Early exit with another post-increment
            int *q = p;
            if (*q++ == target) return q - 1;
        }
    }
    return (p <= end && *(p-1) == target) ? (p-1) : NULL;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    float total_weight = 0.0f;
    struct Data *end = sptr + count;
    
    // Post-increment accessing structure field
    for (; sptr < end; total_weight += sptr++->weight) {
        // Switch with fall-through case
        switch (sptr->id) {
            case 'A':
                // Post-increment in comma expression
                total_weight += (sptr->value, sptr++, 0);
                // Fall through
            case 'B':
                // Another post-increment
                if (sptr->value > 50) {
                    struct Data *temp = sptr;
                    total_weight += temp++->weight * 2;
                    sptr = temp;
                }
                break;
            default:
                // Simple post-increment
                total_weight += sptr->weight;
                break;
        }
    }
    return total_weight;
}

/* Function 5: Byte buffer processing with volatile */
void process_buffer(volatile uint8_t *buf, int size) {
    volatile uint8_t *p = buf;
    volatile uint8_t *end = buf + size;
    
    // Multiple basic blocks with post-increment
    while (p < end) {
        if (*p < 128) {
            // Taken path with post-increment
            uint8_t val = *p++;
            *(p-1) = val * 2;
        } else {
            // Not-taken path with post-increment
            uint8_t val = *p++;
            *(p-1) = val / 2;
        }
        
        // Nested loop with post-increment
        volatile uint8_t *q = p;
        int i = 0;
        while (i++ < 3 && q < end) {
            *q++ = 0xFF;
        }
        p = q;
    }
}

/* Function 6: Array zeroing with post-increment */
void zero_array(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    
    // Simple post-increment loop - should generate clean auto-inc pattern
    while (p < end) {
        *p++ = 0;
    }
}

/* Function 7: Mixed pointer types in expression */
int mixed_pointers(volatile int *vptr, int *ptr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        // Using both volatile and non-volatile pointers
        result += *vptr++ + *ptr++;
        
        // Array access with zero offset pattern
        if (i == 0) {
            // This should match mem_insn.reg1_val = 0 pattern
            result += ptr[0];  // Equivalent to *(ptr + 0)
            result += vptr[0]; // Equivalent to *(vptr + 0)
        }
    }
    return result;
}

int main() {
    // Test data
    char source[] = "Test string for copy operation";
    char destination[BUFFER_SIZE];
    
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    
    struct Data struct_array[] = {
        {10, 'A', 1.5f},
        {20, 'B', 2.5f},
        {30, 'A', 3.5f},
        {40, 'C', 4.5f},
        {60, 'B', 5.5f}
    };
    
    volatile uint8_t buffer[BUFFER_SIZE];
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = i % 256;
    }
    
    printf("Starting auto-increment/decrement pattern tests...\n");
    
    // Test 1: String copy with post-increment
    copy_with_postinc(destination, source);
    printf("1. Copy test: '%s'\n", destination);
    
    // Test 2: Summation with volatile
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("2. Volatile array sum: %d\n", sum1);
    
    // Test 3: Search with post-increment
    int *found = find_value(regular_array, ARRAY_SIZE, 150);
    printf("3. Search result: %s\n", found ? "Found" : "Not found");
    
    // Test 4: Structure processing
    float total_weight = process_structs(struct_array, 
                                        sizeof(struct_array)/sizeof(struct_array[0]));
    printf("4. Total weight: %.2f\n", total_weight);
    
    // Test 5: Buffer processing
    process_buffer(buffer, BUFFER_SIZE);
    printf("5. Buffer processed\n");
    
    // Test 6: Array zeroing
    zero_array(regular_array, ARRAY_SIZE);
    printf("6. Array zeroed\n");
    
    // Test 7: Mixed pointers
    int mixed_result = mixed_pointers(volatile_array, regular_array, 10);
    printf("7. Mixed pointer result: %d\n", mixed_result);
    
    // Additional tight loops that should generate auto-inc patterns
    printf("\nAdditional tight loop tests:\n");
    
    // Tight byte copy loop
    {
        volatile uint8_t src[10] = {1,2,3,4,5,6,7,8,9,10};
        volatile uint8_t dst[10];
        volatile uint8_t *s = src;
        volatile uint8_t *d = dst;
        volatile uint8_t *end = src + 10;
        
        // This should generate ideal post-increment RTL
        while (s < end) {
            *d++ = *s++;
        }
        printf("  Tight byte copy completed\n");
    }
    
    // Loop with post-increment in condition
    {
        int arr[] = {1, 2, 3, 4, 5, 0};
        int *p = arr;
        int count = 0;
        
        while (*p++ != 0) {
            count++;
        }
        printf("  Counted %d non-zero elements\n", count);
    }
    
    // Comma expression with post-increment
    {
        int values[] = {10, 20, 30, 40};
        int *ptr = values;
        int temp;
        
        // Comma expression: access then increment
        temp = (temp = *ptr, ptr++, temp);
        printf("  Comma expression result: %d\n", temp);
    }
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
