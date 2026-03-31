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
    /* This should generate post-increment addressing */
    while (n-- > 0) {
        *dest++ = *src++;
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int count) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Loop with post-increment in condition */
    while (count-- > 0) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(volatile int *arr, int size, int target) {
    volatile int *ptr = arr;
    int found = -1;
    
    /* Multiple basic blocks with post-increment */
    for (int i = 0; i < size; i++) {
        if (*ptr++ == target) {
            found = i;
            /* Nested control flow */
            switch (target) {
                case 0:
                    /* Fall-through case with post-increment */
                    ptr++;  /* Extra increment */
                    /* FALLTHROUGH */
                case 1:
                    /* Another memory access with different offset */
                    if (*(ptr - 1) == 1) {
                        return found;
                    }
                    break;
                default:
                    break;
            }
            break;
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    float total_weight = 0.0f;
    struct Data *current = sptr;
    
    /* Access structure field with pointer post-increment */
    for (int i = 0; i < count; i++) {
        total_weight += current->weight;
        current++;  /* Post-increment equivalent: current = &current[1] */
    }
    
    return total_weight;
}

/* Function 5: Comma expression with post-increment */
int comma_postinc(volatile int *ptr) {
    int temp;
    
    /* Comma expression: access then increment */
    temp = (*ptr, ptr++, temp);
    
    /* Another comma expression variant */
    return (temp = *ptr, ptr++, temp);
}

/* Function 6: String length with post-increment (classic example) */
int strlen_postinc(const char *str) {
    const char *p = str;
    
    /* Classic post-increment in loop condition */
    while (*p++ != '\0') {
        /* Empty body */
    }
    
    return (int)(p - str - 1);
}

/* Function 7: Mixed qualifiers in same expression */
void mixed_qualifiers(volatile int *vptr, int *regptr, int n) {
    /* Use both volatile and non-volatile pointers */
    for (int i = 0; i < n; i++) {
        *regptr++ = *vptr++;  /* Mixed qualifiers */
    }
}

/* Function 8: Nested loops with post-increment */
void matrix_process(volatile int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = matrix[i];
        int *dest = (int *)row_ptr;  /* Cast away volatile for destination */
        
        /* Inner loop with post-increment */
        for (int j = 0; j < 10; j++) {
            *dest++ = *row_ptr++ + 1;
        }
    }
}

/* Main function that exercises all patterns */
int main() {
    /* Test data */
    volatile char source[BUFFER_SIZE] = "Test string for post-increment operations";
    volatile char dest[BUFFER_SIZE] = {0};
    
    volatile int numbers[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    
    struct Data structs[20];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i % 10;  /* Values 0-9 repeating */
        regular_array[i] = i;
    }
    
    for (int i = 0; i < 20; i++) {
        structs[i].value = i;
        structs[i].id = 'A' + (i % 26);
        structs[i].weight = i * 0.5f;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(dest, (const char *)source, strlen((const char *)source) + 1);
    printf("Copy test: %s\n", (const char *)dest);
    
    /* Test 2: Summation with post-increment */
    int sum = sum_with_postinc(numbers, ARRAY_SIZE);
    printf("Sum test: %d\n", sum);
    
    /* Test 3: Search with post-increment */
    int found_idx = find_value(numbers, ARRAY_SIZE, 5);
    printf("Search test: Found 5 at index %d\n", found_idx);
    
    /* Test 4: Structure processing */
    float total_weight = process_structs(structs, 20);
    printf("Structure test: Total weight = %.2f\n", total_weight);
    
    /* Test 5: Comma expression */
    volatile int test_val = 42;
    int comma_result = comma_postinc(&test_val);
    printf("Comma expression test: %d\n", comma_result);
    
    /* Test 6: String length */
    const char *test_str = "Hello, World!";
    int len = strlen_postinc(test_str);
    printf("String length test: %d\n", len);
    
    /* Test 7: Mixed qualifiers */
    mixed_qualifiers(numbers, regular_array, 10);
    printf("Mixed qualifiers test: First element = %d\n", regular_array[0]);
    
    /* Test 8: Matrix processing */
    volatile int matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_process(matrix, 5);
    printf("Matrix test complete\n");
    
    /* Additional tight loop patterns likely to generate auto-inc RTL */
    
    /* Pattern A: Array zeroing with post-increment */
    volatile int clear_array[50];
    volatile int *clear_ptr = clear_array;
    int clear_count = 50;
    while (clear_count-- > 0) {
        *clear_ptr++ = 0;
    }
    
    /* Pattern B: Pointer difference with post-increment */
    const char *str1 = "Test1";
    const char *str2 = "Test2";
    const char *p1 = str1;
    const char *p2 = str2;
    
    while (*p1 && *p2 && *p1++ == *p2++) {
        /* Compare strings with post-increment */
    }
    
    /* Pattern C: Multiple post-increments in same expression */
    volatile int multi[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int *mp = multi;
    int a, b, c;
    
    /* Sequence of post-increment accesses */
    a = *mp++;
    b = *mp++;
    c = *mp++;
    
    printf("Multiple increments: a=%d, b=%d, c=%d\n", a, b, c);
    
    /* Pattern D: Post-increment with zero offset (targeting the uncovered lines) */
    volatile int *zero_offset_ptr = &numbers[0];
    /* Direct dereference with zero offset */
    int zero_offset_val = *zero_offset_ptr;
    zero_offset_ptr++;  /* Post-increment */
    
    printf("Zero offset test: %d\n", zero_offset_val);
    
    return 0;
}
