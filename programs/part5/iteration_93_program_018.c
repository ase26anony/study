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

/* Function 1: Copy with post-increment pointers (tight loop) */
void copy_with_postinc(volatile char *dest, const char *src, int n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic copy loop with post-increment */
    while (n-- > 0) {
        *d++ = *s++;  // Should generate post-increment addressing
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int size) {
    volatile int *p = arr;
    int sum = 0;
    int *end = (int*)(arr + size);  // Mixed pointer types
    
    /* Loop with post-increment in condition */
    while (p < end) {
        sum += *p++;  // Memory access with post-increment
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    int found = -1;
    int i = 0;
    
    /* Multiple basic blocks with post-increment */
    while (i < size) {
        if (*p++ == target) {  // Post-increment in condition
            found = i;
            /* Nested control flow */
            switch (target) {
                case 0:
                    /* Fall-through case with post-increment */
                    p++;  // Additional increment
                    /* FALLTHROUGH */
                case 1:
                    /* Comma expression with post-increment */
                    return (found, p--, found);  // Sequence point
                default:
                    break;
            }
        }
        i++;
        
        /* Another basic block with different pattern */
        if (i % 10 == 0) {
            /* Array access with zero offset */
            volatile int temp = arr[0];  // reg1_val = 0 pattern
            (void)temp;
        }
    }
    
    return found;
}

/* Function 4: String operations with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    /* Classic strlen-like loop */
    while (*p++ != '\0') {
        /* Empty body - post-increment in condition */
    }
    
    return (int)(p - str - 1);
}

/* Function 5: Structure array processing */
float process_structs(struct Data *sarr, int count) {
    struct Data *sptr = sarr;
    float total = 0.0f;
    int i = 0;
    
    /* Loop with structure field access and post-increment */
    while (i++ < count) {
        /* Access field then increment pointer */
        total += sptr->weight;
        sptr++;  // Post-increment of structure pointer
        
        /* Nested if with different increment pattern */
        if (i % 2 == 0) {
            /* Comma expression: access, increment, use */
            float w = (sptr->weight, sptr--, sptr++->weight);
            total += w;
        }
    }
    
    return total;
}

/* Function 6: Mixed qualifiers in same expression */
void mixed_qualifier_copy(volatile int *dest, int *src, int n) {
    volatile int *d = dest;
    int *s = src;
    
    /* Mixed volatile/non-volatile in same loop */
    for (int i = 0; i < n; i++) {
        *d++ = *s++;  // Both pointers post-increment
    }
}

/* Function 7: Byte buffer processing with post-increment */
int process_buffer(volatile uint8_t *buf, int size) {
    volatile uint8_t *ptr = buf;
    int checksum = 0;
    
    /* Tight processing loop */
    for (int i = 0; i < size; i++) {
        checksum ^= *ptr++;  // Post-increment in expression
    }
    
    /* Additional loop with different pattern */
    ptr = buf;
    while (size-- > 0) {
        if (*ptr++ == 0xFF) {  // Post-increment in condition
            checksum += 0x100;
        }
    }
    
    return checksum;
}

int main(void) {
    /* Test data arrays */
    volatile int volatile_array[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    volatile char volatile_buffer[BUFFER_SIZE];
    char regular_buffer[BUFFER_SIZE];
    struct Data struct_array[20];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile_array[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        volatile_buffer[i] = 'A' + (i % 26);
        regular_buffer[i] = 'a' + (i % 26);
    }
    
    strcpy(regular_buffer, "Test string for post-increment operations");
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].value = i;
        struct_array[i].id = 'A' + i;
        struct_array[i].weight = i * 1.5f;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(volatile_buffer, regular_buffer, BUFFER_SIZE);
    printf("Copy test completed\n");
    
    /* Test 2: Summation with post-increment */
    int sum1 = sum_with_postinc(volatile_array, ARRAY_SIZE);
    printf("Sum of volatile array: %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    int found = find_value(volatile_array, ARRAY_SIZE, 50);
    printf("Found value 50 at index: %d\n", found);
    
    /* Test 4: String length with post-increment */
    int len = string_length(regular_buffer);
    printf("String length: %d\n", len);
    
    /* Test 5: Structure processing */
    float struct_total = process_structs(struct_array, 20);
    printf("Structure total weight: %.2f\n", struct_total);
    
    /* Test 6: Mixed qualifier copy */
    mixed_qualifier_copy(volatile_array, regular_array, ARRAY_SIZE / 2);
    printf("Mixed qualifier copy completed\n");
    
    /* Test 7: Buffer processing */
    volatile uint8_t byte_buffer[64];
    for (int i = 0; i < 64; i++) {
        byte_buffer[i] = i;
    }
    int checksum = process_buffer(byte_buffer, 64);
    printf("Buffer checksum: 0x%X\n", checksum);
    
    /* Additional complex pattern: Nested loops */
    volatile int matrix[10][10];
    volatile int *row_ptr;
    int total = 0;
    
    for (int row = 0; row < 10; row++) {
        row_ptr = matrix[row];
        for (int col = 0; col < 10; col++) {
            /* Post-increment in inner loop */
            total += *row_ptr++;
        }
    }
    printf("Matrix total: %d\n", total);
    
    /* Test with zero offset pattern */
    volatile int *simple_ptr = volatile_array;
    int zero_offset_value = *simple_ptr;  // Should match mem_insn.reg1_val = 0
    printf("Zero offset value: %d\n", zero_offset_value);
    
    /* Comma expression test */
    volatile int *ptr = volatile_array;
    int result = (*ptr++, *ptr--, *ptr);  // Multiple increments in sequence
    printf("Comma expression result: %d\n", result);
    
    return 0;
}
