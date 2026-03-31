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
    /* Classic strcpy-like loop with post-increment */
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
    volatile int *p = arr;
    int found = -1;
    
    /* Post-increment in loop with early exit */
    for (int i = 0; i < size; i++) {
        if (*p++ == target) {
            found = i;
            /* Nested condition with another post-increment */
            if (i + 1 < size && *p++ == target * 2) {
                found = i + 1000;  /* Special marker */
            }
            break;
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
float process_structs(struct Data *sptr, int count) {
    float total_weight = 0.0f;
    
    /* Access structure field with pointer post-increment */
    for (int i = 0; i < count; i++) {
        total_weight += sptr->weight;
        sptr++;  /* Post-increment after access */
    }
    
    return total_weight;
}

/* Function 5: Comma expression with post-increment */
int comma_postinc(volatile int *ptr) {
    int temp;
    
    /* Memory access followed by increment in comma expression */
    temp = (temp = *ptr, ptr++, temp);
    
    /* Another comma expression variant */
    return (temp = *ptr, ptr++, temp * 2);
}

/* Function 6: Switch case with post-increment */
int switch_postinc(volatile char *str, int mode) {
    int count = 0;
    
    switch (mode) {
        case 0:
            /* Post-increment in while condition */
            while (*str++ != '\0') {
                count++;
            }
            break;
            
        case 1:
            /* Post-increment with fall-through */
            count = *str++;
            /* Fall through */
            
        case 2:
            /* Another post-increment in fall-through case */
            count += *str++;
            break;
            
        default:
            /* Post-increment in default case */
            count = *str++ * 2;
    }
    
    return count;
}

/* Function 7: Nested loops with post-increment */
void matrix_process(volatile int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = matrix[i];
        
        /* Inner loop with post-increment pointer */
        for (int j = 0; j < 10; j++) {
            /* Access with zero offset (base + 0) */
            *row_ptr = *row_ptr * 2;
            row_ptr++;  /* Post-increment */
        }
    }
}

/* Function 8: Mixed qualifiers in same expression */
int mixed_qualifiers_postinc(volatile int *vptr, int *regptr, int n) {
    int result = 0;
    
    /* Using both volatile and non-volatile pointers */
    for (int i = 0; i < n; i++) {
        *regptr = *vptr++;  /* Post-increment on volatile pointer */
        result += *regptr++;
    }
    
    return result;
}

int main() {
    /* Test data arrays - mix volatile and non-volatile */
    volatile char src_buffer[BUFFER_SIZE] = "Test string for post-increment operations";
    volatile char dest_buffer[BUFFER_SIZE];
    
    volatile int numbers[ARRAY_SIZE];
    int regular_array[ARRAY_SIZE];
    
    volatile int matrix[5][10];
    struct Data data_array[20];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2;
        regular_array[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        data_array[i].value = i;
        data_array[i].id = 'A' + (i % 26);
        data_array[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(dest_buffer, (const char*)src_buffer, strlen((const char*)src_buffer) + 1);
    printf("Copy test: %s\n", (char*)dest_buffer);
    
    /* Test 2: Summation with post-increment */
    int sum = sum_with_postinc(numbers, ARRAY_SIZE);
    printf("Sum test: %d\n", sum);
    
    /* Test 3: Search with post-increment */
    numbers[42] = 999;  /* Insert target value */
    int found = find_value(numbers, ARRAY_SIZE, 999);
    printf("Search test: found at %d\n", found);
    
    /* Test 4: Structure processing */
    float total_weight = process_structs(data_array, 20);
    printf("Structure test: total weight = %.2f\n", total_weight);
    
    /* Test 5: Comma expression */
    int comma_result = comma_postinc(numbers);
    printf("Comma expression test: %d\n", comma_result);
    
    /* Test 6: Switch with post-increment */
    volatile char test_str[] = "Hello";
    int switch_result = switch_postinc(test_str, 0);
    printf("Switch test: %d\n", switch_result);
    
    /* Test 7: Nested loops */
    matrix_process(matrix, 5);
    printf("Matrix processed\n");
    
    /* Test 8: Mixed qualifiers */
    int mixed_result = mixed_qualifiers_postinc(numbers, regular_array, 10);
    printf("Mixed qualifiers test: %d\n", mixed_result);
    
    /* Additional tight loops likely to generate auto-inc RTL */
    
    /* Loop 1: String length with post-increment */
    volatile char *p = src_buffer;
    int length = 0;
    while (*p++ != '\0') {
        length++;
    }
    printf("String length: %d\n", length);
    
    /* Loop 2: Array initialization with post-increment */
    volatile int *init_ptr = numbers;
    for (int i = 0; i < 10; i++) {
        *init_ptr++ = i * 100;
    }
    
    /* Loop 3: Conditional post-increment in both paths */
    volatile int *cond_ptr = numbers;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            *cond_ptr = *cond_ptr * 3;
            cond_ptr++;  /* Post-increment in taken path */
        } else {
            cond_ptr++;  /* Post-increment in not-taken path */
            *cond_ptr = *cond_ptr / 2;
        }
    }
    
    /* Test zero-offset access patterns */
    volatile int *zero_ptr = &numbers[0];
    int zero_test = *zero_ptr;  /* Direct dereference with zero offset */
    zero_ptr++;
    
    /* Array access with index 0 */
    int zero_index_test = numbers[0];
    
    return 0;
}
