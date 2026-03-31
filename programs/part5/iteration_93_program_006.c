#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 100
#define PATTERN_SIZE 50

/* Structure for testing pointer post-increment */
struct Data {
    int values[10];
    char tag;
    int count;
};

/* Function using post-increment in array access */
int sum_array_postinc(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Tight loop with post-increment - should generate auto-inc RTL */
    while (p < end) {
        sum += *p++;  /* Post-increment in memory access */
    }
    return sum;
}

/* Function using post-increment with volatile */
int sum_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    volatile int *end = arr + n;
    
    /* Mix of volatile and control flow */
    if (n > 0) {
        sum += *p++;  /* Post-increment on volatile pointer */
    }
    
    while (p < end) {
        /* Post-increment in loop with volatile */
        sum += *p++;
    }
    
    return sum;
}

/* String copy with post-increment - classic pattern */
void str_copy_postinc(char *dest, const char *src) {
    /* Tight copy loop - likely to generate auto-inc */
    while ((*dest++ = *src++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* Function with multiple basic blocks and post-increment */
int find_value_postinc(int *arr, int n, int target) {
    int *p = arr;
    int *end = arr + n;
    int found = 0;
    
    /* Search loop with post-increment */
    while (p < end && !found) {
        if (*p++ == target) {  /* Post-increment in condition */
            found = 1;
        }
    }
    
    /* Another basic block with different post-increment pattern */
    if (!found && n > 0) {
        p = arr;  /* Reset pointer */
        /* Comma expression with post-increment */
        int temp = (*p, p++, temp);  /* Access then increment */
    }
    
    return found;
}

/* Function with structure pointer post-increment */
int sum_struct_values(struct Data *data, int count) {
    int total = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < count; i++) {
        /* Access structure field with pointer, then increment */
        total += ptr->count;
        ptr++;  /* Post-increment of structure pointer */
    }
    
    return total;
}

/* Function with switch and post-increment */
int process_with_switch(int *arr, int n, int mode) {
    int result = 0;
    int *p = arr;
    
    switch (mode) {
        case 0:
            /* Post-increment in switch case */
            result = *p++;
            break;
            
        case 1:
            /* Multiple post-increments with fall-through logic */
            result += *p++;
            /* Fall through */
            
        case 2:
            result += *p++;
            break;
            
        default:
            /* Loop with post-increment in default case */
            while (p < arr + n) {
                result += *p++;
            }
    }
    
    return result;
}

/* Nested loops with post-increment */
void matrix_process(int matrix[][SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        int *end = row_ptr + cols;
        
        /* Inner loop with post-increment */
        while (row_ptr < end) {
            /* Access with zero offset (row_ptr[0]) then increment */
            int val = *row_ptr;
            row_ptr++;  /* Post-increment */
            
            /* Some computation */
            matrix[i][0] += val;  /* Access with zero offset */
        }
    }
}

/* Function with mixed volatile and non-volatile pointers */
int mixed_pointer_ops(volatile int *varr, int *arr, int n) {
    int sum = 0;
    volatile int *vp = varr;
    int *p = arr;
    
    /* Alternate between volatile and non-volatile accesses */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *vp++;  /* Post-increment volatile */
        } else {
            sum += *p++;   /* Post-increment non-volatile */
        }
    }
    
    return sum;
}

/* Main function with various post-increment patterns */
int main() {
    /* Test data */
    int array[SIZE];
    volatile int varray[SIZE];
    char source[PATTERN_SIZE] = "Test string for copy operation";
    char destination[PATTERN_SIZE];
    struct Data data_items[5];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
        varray[i] = i * 2;
    }
    
    /* Initialize structure array */
    for (int i = 0; i < 5; i++) {
        data_items[i].count = i * 10;
        for (int j = 0; j < 10; j++) {
            data_items[i].values[j] = i * 10 + j;
        }
    }
    
    printf("Testing post-increment/decrement patterns for auto-inc-dec pass\n\n");
    
    /* Test 1: Basic array sum with post-increment */
    int sum1 = sum_array_postinc(array, SIZE);
    printf("Sum of array (post-increment): %d\n", sum1);
    
    /* Test 2: Volatile array sum with post-increment */
    int sum2 = sum_volatile_postinc(varray, SIZE);
    printf("Sum of volatile array (post-increment): %d\n", sum2);
    
    /* Test 3: String copy with post-increment */
    str_copy_postinc(destination, source);
    printf("Copied string: %s\n", destination);
    
    /* Test 4: Find value with post-increment */
    int found = find_value_postinc(array, SIZE, 42);
    printf("Value 42 %s in array\n", found ? "found" : "not found");
    
    /* Test 5: Structure processing */
    int struct_sum = sum_struct_values(data_items, 5);
    printf("Sum of structure counts: %d\n", struct_sum);
    
    /* Test 6: Switch with post-increment */
    int switch_result = process_with_switch(array, 10, 1);
    printf("Switch processing result: %d\n", switch_result);
    
    /* Test 7: Mixed volatile/non-volatile pointers */
    int mixed_sum = mixed_pointer_ops(varray, array, 20);
    printf("Mixed pointer operations sum: %d\n", mixed_sum);
    
    /* Test 8: Matrix processing */
    int matrix[5][SIZE];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    matrix_process(matrix, 5, SIZE);
    printf("Matrix processed, first element: %d\n", matrix[0][0]);
    
    /* Additional tight loop patterns */
    /* Pattern 1: while with post-increment in condition */
    int *ptr = array;
    int count = 0;
    while (count < SIZE && *ptr++ > -1) {
        count++;
    }
    
    /* Pattern 2: for loop with post-increment */
    int total = 0;
    for (int *p = array; p < &array[SIZE]; total += *p++) {
        /* All work in update expression */
    }
    printf("For loop with post-increment sum: %d\n", total);
    
    /* Pattern 3: Comma expression with post-increment */
    ptr = array;
    int temp;
    for (int i = 0; i < 10; i++) {
        temp = (*ptr, ptr++, temp);  /* Access, increment, use result */
    }
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
