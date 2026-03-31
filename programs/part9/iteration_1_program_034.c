/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Pointer copy with restrict qualifier */
void copy_buffer_restrict(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy-like pattern */
    while (n-- > 0) {
        *dst = *src;    /* Simple dereference */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Mixed types and operations */
void process_shorts(short *data, int count) {
    short *ptr = data;
    
    for (int i = 0; i < count; i++) {
        /* Multiple accesses to same pointer location */
        short val = *ptr;
        *ptr = val * 2;
        ptr = ptr + 1;  /* Increment after use */
    }
}

/* Pattern 4: Nested pointer operations */
int sum_matrix(const int *matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        const int *row_ptr = matrix + i * cols;
        
        for (int j = 0; j < cols; j++) {
            total += *row_ptr;  /* Simple dereference */
            row_ptr = row_ptr + 1;  /* Separate increment */
        }
    }
    return total;
}

/* Pattern 5: Pointer arithmetic with char type */
int count_chars(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == target) {  /* Simple dereference */
            count++;
        }
        p = p + 1;  /* Increment in separate statement */
    }
    return count;
}

/* Pattern 6: Local pointer with no function arguments */
void local_pointer_test(void) {
    int buffer[16];
    int *ptr = buffer;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        *ptr = i;       /* Simple store via register */
        ptr = ptr + 1;  /* Separate increment */
    }
    
    /* Read back */
    ptr = buffer;
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += *ptr;    /* Simple load via register */
        ptr = ptr + 1;  /* Separate increment */
    }
    
    /* Use sum to prevent optimization */
    printf("Local sum: %d\n", sum);
}

/* Pattern 7: Volatile pointer (use sparingly) */
void volatile_pointer_test(volatile int *vptr, int n) {
    for (int i = 0; i < n; i++) {
        int val = *vptr;    /* Volatile load */
        (void)val;          /* Use value */
        vptr = vptr + 1;    /* Separate increment */
    }
}

/* Pattern 8: Pointer to pointer */
void update_pointers(int **arr, int n) {
    for (int i = 0; i < n; i++) {
        int *ptr = arr[i];
        if (ptr) {
            *ptr = i;       /* Dereference pointer-to-pointer */
            arr[i] = ptr + 1; /* Different pointer arithmetic */
        }
    }
}

/* Main function with various test cases */
int main(void) {
    int int_array[100];
    char char_buffer[256];
    short short_data[50];
    int matrix[5][10];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i % 10;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    char_buffer[255] = '\0';
    
    for (int i = 0; i < 50; i++) {
        short_data[i] = (short)(i * 2);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test Pattern 1 */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum1: %d\n", sum1);
    
    /* Test Pattern 2 */
    char dest[256];
    copy_buffer_restrict(dest, char_buffer, 100);
    printf("Copy result: %c%c\n", dest[0], dest[99]);
    
    /* Test Pattern 3 */
    process_shorts(short_data, 50);
    printf("Short data[0]: %d\n", (int)short_data[0]);
    
    /* Test Pattern 4 */
    int sum2 = sum_matrix(&matrix[0][0], 5, 10);
    printf("Matrix sum: %d\n", sum2);
    
    /* Test Pattern 5 */
    int count = count_chars("Hello, World!", 'l');
    printf("Char count: %d\n", count);
    
    /* Test Pattern 6 */
    local_pointer_test();
    
    /* Test Pattern 7 (use volatile sparingly) */
    volatile_pointer_test(int_array, 10);
    
    /* Test Pattern 8 */
    int *ptr_array[10];
    int values[20];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &values[i * 2];
    }
    update_pointers(ptr_array, 10);
    
    /* Additional test: pointer traversal in main */
    const int *main_ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;  /* Simple dereference */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main sum: %d\n", main_sum);
    
    /* Final checksum */
    int total_checksum = sum1 + sum2 + count + main_sum;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
