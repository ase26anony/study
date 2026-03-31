#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 100
#define BUFFER_SIZE 256

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    char id;
    float weight;
};

/* Function 1: Copy with post-increment pointers (tight loop) */
void copy_with_postinc(volatile char *dest, const char *src, size_t n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic strcpy-like loop with post-increment */
    while (n-- > 0) {
        *d++ = *s++;  /* Should generate post-increment addressing */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *arr, int count) {
    volatile int *p = arr;
    int sum = 0;
    
    /* Loop with post-increment in condition */
    while (count-- > 0) {
        sum += *p++;  /* Memory access followed by pointer increment */
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    int i = 0;
    
    /* Search loop with post-increment */
    while (i < size && *p++ != target) {
        i++;
    }
    
    /* If not found, continue with different pattern */
    if (i == size) {
        p = arr;
        /* Another loop with post-increment */
        for (i = 0; i < size; i++) {
            if (*p++ == -target) {
                return i;
            }
        }
    }
    
    return (i < size) ? i : -1;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    struct Data *ptr = sptr;
    float total_weight = 0.0f;
    int i = 0;
    
    /* Loop through structures */
    while (i++ < count) {
        /* Access structure field with pointer, then increment */
        total_weight += ptr->weight;
        ptr++;  /* Post-increment of structure pointer */
    }
    
    return total_weight;
}

/* Function 5: Nested loops with post-increment addressing */
void matrix_operation(volatile int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = matrix + i * cols;
        
        /* Inner loop with post-increment */
        for (int j = 0; j < cols; j++) {
            /* Multiple memory operations with post-increment */
            int val = *row_ptr++;
            *row_ptr = val * 2;  /* Store to next location */
            row_ptr++;  /* Additional increment */
        }
    }
}

/* Function 6: Comma expression with post-increment */
int comma_postinc(volatile int *ptr) {
    int result;
    
    /* Comma expression: access then increment */
    result = (*ptr, ptr++, *ptr);  /* Access ptr[0], increment, access ptr[1] */
    
    return result;
}

/* Function 7: Switch case with post-increment */
int switch_postinc(volatile char *buf, int mode) {
    volatile char *p = buf;
    int count = 0;
    
    switch (mode) {
        case 0:
            /* Post-increment in loop condition */
            while (*p++ != '\0') {
                count++;
            }
            break;
            
        case 1:
            /* Post-increment in update statement */
            for (; *p != '\0'; p++) {
                count += *p;
            }
            break;
            
        case 2:
            /* Fall-through case with post-increment */
            count = *p++;
            /* Fall through */
        case 3:
            count += *p++;
            break;
    }
    
    return count;
}

/* Function 8: Pointer arithmetic with zero offset */
void zero_offset_access(volatile int *ptr) {
    /* Direct dereference - should generate base + 0 offset */
    int val = *ptr;
    
    /* Array access with index 0 */
    val = ptr[0];
    
    /* Post-increment after access */
    val = *ptr;
    ptr++;
}

int main() {
    /* Test data arrays - mix volatile and non-volatile */
    volatile char source[BUFFER_SIZE] = "Test string for post-increment operations";
    volatile char dest[BUFFER_SIZE] = {0};
    
    volatile int numbers[ARRAY_SIZE];
    int non_volatile[ARRAY_SIZE];
    
    struct Data struct_array[50];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2;
        non_volatile[i] = i * 3;
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].value = i;
        struct_array[i].id = 'A' + (i % 26);
        struct_array[i].weight = i * 0.5f;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(dest, (const char*)source, strlen((const char*)source) + 1);
    printf("Copy test: %s\n", (char*)dest);
    
    /* Test 2: Summation with post-increment */
    int sum = sum_with_postinc(numbers, ARRAY_SIZE);
    printf("Sum of numbers: %d\n", sum);
    
    /* Test 3: Search with post-increment */
    int found_idx = find_value(numbers, ARRAY_SIZE, 50);
    printf("Found 50 at index: %d\n", found_idx);
    
    /* Test 4: Structure processing */
    float total_weight = process_structs(struct_array, 50);
    printf("Total weight: %.2f\n", total_weight);
    
    /* Test 5: Matrix operation */
    volatile int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_operation(&matrix[0][0], 10, 10);
    
    /* Test 6: Comma expression */
    volatile int test_arr[5] = {10, 20, 30, 40, 50};
    int comma_result = comma_postinc(test_arr);
    printf("Comma expression result: %d\n", comma_result);
    
    /* Test 7: Switch with post-increment */
    volatile char test_buf[] = "HelloWorld";
    int switch_result = switch_postinc(test_buf, 0);
    printf("Switch post-increment count: %d\n", switch_result);
    
    /* Test 8: Zero offset access */
    volatile int single = 42;
    zero_offset_access(&single);
    
    /* Additional tight loop patterns */
    /* Pattern 1: String length with post-increment */
    volatile char *str = "TestString";
    int len = 0;
    while (*str++ != '\0') {
        len++;
    }
    printf("String length: %d\n", len);
    
    /* Pattern 2: Array initialization with post-increment */
    volatile int *init_ptr = numbers;
    for (int i = 0; i < 10; i++) {
        *init_ptr++ = i * 10;
    }
    
    /* Pattern 3: Mixed pointer types in expression */
    volatile short *short_ptr = (volatile short*)numbers;
    int *regular_ptr = non_volatile;
    for (int i = 0; i < 10; i++) {
        *regular_ptr++ = *short_ptr++;
    }
    
    return 0;
}
