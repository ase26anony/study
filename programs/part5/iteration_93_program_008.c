#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 16

/* Structure for testing pointer post-increment */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment - tight loop */
void copy_with_postinc(volatile char *dest, const char *src, size_t n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic copy loop with post-increment */
    while (n--) {
        *d++ = *s++;  // Should generate post-increment addressing
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int count) {
    volatile int *p = arr;
    int sum = 0;
    
    /* Summation loop with post-increment in condition */
    while (count-- > 0) {
        sum += *p++;  // Memory access followed by increment
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_pattern(volatile uint8_t *data, size_t len, uint8_t pattern) {
    volatile uint8_t *p = data;
    volatile uint8_t *end = data + len;
    int position = -1;
    int i = 0;
    
    /* Search loop with post-increment in condition */
    while (p < end) {
        if (*p++ == pattern) {  // Post-increment in condition check
            position = i;
            /* Nested control flow */
            if (i % 2 == 0) {
                /* Additional post-increment in taken path */
                volatile uint8_t *q = p;
                if (q < end && *q++ == pattern) {
                    return position * 2;
                }
            }
            break;
        }
        i++;
    }
    
    return position;
}

/* Function 4: Structure access with pointer post-increment */
float process_structs(volatile struct Data *structs, int count) {
    volatile struct Data *sptr = structs;
    float total = 0.0f;
    int i = 0;
    
    /* Loop with structure field access and pointer increment */
    while (i++ < count) {
        /* Access structure field with zero offset */
        total += sptr->weight;  // sptr->field with implicit offset 0
        
        /* Post-increment separated by comma expression */
        volatile struct Data *temp = sptr;
        sptr++, temp->value = i;  // Increment after access
        
        /* Switch with fall-through */
        switch (i % 3) {
            case 0:
                /* Post-increment in switch case */
                (void)*((volatile char *)sptr + 1);  // Access with offset
                sptr++;  // Increment pointer
                break;
            case 1:
                /* Fall through with post-increment */
            case 2:
                sptr->id = 'A' + (i % 26);
                sptr++;  // Post-increment in fall-through case
                break;
        }
    }
    
    return total;
}

/* Function 5: String operations with post-increment */
size_t string_length(const char *str) {
    const char *p = str;
    
    /* Classic strlen implementation */
    while (*p++ != '\0') {  // Post-increment in loop condition
        /* Empty body - tight loop */
    }
    
    return p - str - 1;
}

/* Function 6: Array processing with multiple basic blocks */
void process_array(volatile int *arr, int size) {
    volatile int *ptr = arr;
    int *regular_ptr = (int *)arr;  // Mixed volatile/non-volatile
    
    for (int i = 0; i < size; i++) {
        /* If-else with post-increment in both paths */
        if (i % 2 == 0) {
            /* Taken path */
            int val = *ptr++;  // Post-increment with assignment
            *regular_ptr++ = val * 2;
        } else {
            /* Not-taken path */
            int val = *ptr;
            ptr++;  // Separate increment
            *regular_ptr = val / 2;
            regular_ptr++;
        }
        
        /* Nested loop with post-increment */
        if (i % 10 == 0) {
            volatile int *inner = ptr;
            for (int j = 0; j < 3; j++) {
                *inner++ = j;  // Inner loop post-increment
            }
            ptr = inner;
        }
    }
}

/* Function 7: Byte buffer processing */
int checksum(volatile uint8_t *data, size_t len) {
    volatile uint8_t *p = data;
    int sum = 0;
    
    /* Unrolled loop with post-increment */
    while (len >= 4) {
        sum += *p++;  // Post-increment
        sum += *p++;  // Another post-increment
        sum += *p++;
        sum += *p++;
        len -= 4;
    }
    
    /* Remainder handling */
    while (len-- > 0) {
        sum += *p++;  // Final post-increments
    }
    
    return sum;
}

int main(void) {
    /* Test data - mix volatile and non-volatile */
    volatile char source[SIZE];
    volatile char dest[SIZE];
    volatile int numbers[SIZE];
    volatile struct Data structs[PATTERN_SIZE];
    volatile uint8_t buffer[SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < SIZE; i++) {
        source[i] = 'A' + (i % 26);
        numbers[i] = i * 2;
        buffer[i] = (uint8_t)(i % 256);
    }
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        structs[i].value = i;
        structs[i].id = 'A' + i;
        structs[i].weight = i * 1.5f;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(dest, (const char *)source, SIZE);
    
    /* Verify copy */
    int copy_ok = 1;
    for (int i = 0; i < SIZE; i++) {
        if (dest[i] != source[i]) {
            copy_ok = 0;
            break;
        }
    }
    printf("Copy test: %s\n", copy_ok ? "PASS" : "FAIL");
    
    /* Test 2: Summation with post-increment */
    int sum = sum_with_postinc(numbers, SIZE);
    int expected_sum = (SIZE - 1) * SIZE;  // Sum of 0..(SIZE-1)*2
    printf("Sum test: %d (expected: %d) %s\n", 
           sum, expected_sum, sum == expected_sum ? "PASS" : "FAIL");
    
    /* Test 3: Pattern search */
    int pos = find_pattern(buffer, SIZE, 42);
    printf("Pattern search: found at position %d\n", pos);
    
    /* Test 4: Structure processing */
    float struct_total = process_structs(structs, PATTERN_SIZE);
    printf("Structure total weight: %.2f\n", struct_total);
    
    /* Test 5: String length */
    const char *test_str = "Hello, auto-inc-dec!";
    size_t len = string_length(test_str);
    printf("String length: %zu (actual: %zu)\n", len, strlen(test_str));
    
    /* Test 6: Array processing */
    process_array(numbers, PATTERN_SIZE);
    
    /* Test 7: Checksum */
    int csum = checksum(buffer, SIZE);
    printf("Checksum: %d\n", csum);
    
    /* Additional complex test with nested loops */
    volatile int matrix[10][10];
    volatile int *row_ptr = &matrix[0][0];
    
    /* Nested loop with pointer arithmetic */
    for (int i = 0; i < 10; i++) {
        volatile int *col_ptr = row_ptr;
        for (int j = 0; j < 10; j++) {
            *col_ptr++ = i * 10 + j;  // Post-increment in inner loop
        }
        row_ptr += 10;  // Move to next row
    }
    
    /* Verify matrix */
    int matrix_ok = 1;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != i * 10 + j) {
                matrix_ok = 0;
                break;
            }
        }
    }
    printf("Matrix test: %s\n", matrix_ok ? "PASS" : "FAIL");
    
    return 0;
}
