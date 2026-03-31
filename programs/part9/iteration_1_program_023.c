/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should be merged */
    }
    return sum;
}

/* Pattern 2: Using restrict to help alias analysis */
void copy_int_buffer(int *restrict dst, const int *restrict src, int n) {
    /* Classic pointer traversal with restrict */
    while (n-- > 0) {
        *dst = *src;    /* Simple (mem (reg)) access */
        dst = dst + 1;  /* Separate increment */
        src = src + 1;
    }
}

/* Pattern 3: Char pointer traversal - different mode (QImode) */
int count_chars(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    while (*p) {
        if (*p == target)  /* (mem (reg)) pattern */
            count++;
        p = p + 1;         /* Separate increment */
    }
    return count;
}

/* Pattern 4: Post-increment in expression */
void fill_buffer(short *buf, short value, int n) {
    short *p = buf;
    
    /* Using post-increment within the statement */
    while (n-- > 0) {
        *p++ = value;  /* Combined access+increment in C, but might split in RTL */
    }
}

/* Pattern 5: Explicit split operations in loop */
float sum_float_array(const float *arr, int n) {
    const float *ptr = arr;
    float total = 0.0f;
    int i;
    
    for (i = 0; i < n; i++) {
        float val = *ptr;   /* Load via register */
        total += val;
        ptr = ptr + 1;      /* Increment separately */
    }
    return total;
}

/* Pattern 6: Mixed operations that might still trigger the pattern */
void process_buffer(char *buf, int size) {
    char *p = buf;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Multiple accesses to same pointer location */
        char c = *p;        /* First access - (mem (reg)) */
        *p = c + 1;         /* Store back - another (mem (reg)) */
        p = p + 1;          /* Separate increment */
    }
}

/* Pattern 7: Double pointer (64-bit) for architectures with different modes */
long long sum_longs(const long long *arr, int n) {
    const long long *p = arr;
    long long sum = 0;
    
    while (n-- > 0) {
        sum += *p;      /* 64-bit memory access */
        p = p + 1;      /* 8-byte increment */
    }
    return sum;
}

/* Pattern 8: Local array with pointer traversal */
int sum_local_array(void) {
    int local_arr[100];
    int *p = local_arr;
    int i, sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (i = 0; i < 100; i++) {
        sum += *p;
        p = p + 1;
    }
    return sum;
}

/* Pattern 9: Nested loops with pointer reset */
void matrix_add(int *restrict dst, const int *restrict src1, 
                const int *restrict src2, int rows, int cols) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int *p1 = src1 + i * cols;
        const int *p2 = src2 + i * cols;
        int *p3 = dst + i * cols;
        
        for (j = 0; j < cols; j++) {
            *p3 = *p1 + *p2;  /* Multiple (mem (reg)) accesses */
            p1 = p1 + 1;
            p2 = p2 + 1;
            p3 = p3 + 1;
        }
    }
}

/* Main function with various test cases */
int main(void) {
    int int_arr[100];
    int int_arr2[100];
    char str[] = "Hello, World! This is a test string for auto-inc optimization.";
    short short_buf[50];
    float float_arr[50];
    char char_buf[100];
    long long long_arr[20];
    int matrix1[10][10], matrix2[10][10], matrix3[10][10];
    int i, result;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        int_arr[i] = i * 2;
        int_arr2[i] = i * 3;
        char_buf[i] = (char)(i % 256);
    }
    
    for (i = 0; i < 50; i++) {
        short_buf[i] = (short)(i * 10);
        float_arr[i] = (float)i * 1.5f;
    }
    
    for (i = 0; i < 20; i++) {
        long_arr[i] = 1000000000LL * i;
    }
    
    for (i = 0; i < 100; i++) {
        matrix1[i/10][i%10] = i;
        matrix2[i/10][i%10] = i * 2;
    }
    
    printf("Testing auto-increment/decrement optimization patterns...\n");
    
    /* Test 1: Simple integer array sum */
    result = sum_array_int(int_arr, 100);
    printf("Sum of int array: %d\n", result);
    
    /* Test 2: Buffer copy with restrict */
    copy_int_buffer(int_arr2, int_arr, 100);
    result = sum_array_int(int_arr2, 100);
    printf("After copy, sum: %d\n", result);
    
    /* Test 3: Char pointer traversal */
    result = count_chars(str, 'e');
    printf("Count of 'e' in string: %d\n", result);
    
    /* Test 4: Short buffer fill */
    fill_buffer(short_buf, 42, 50);
    printf("First element of short buffer: %d\n", (int)short_buf[0]);
    
    /* Test 5: Float array sum */
    float float_sum = sum_float_array(float_arr, 50);
    printf("Sum of float array: %.2f\n", float_sum);
    
    /* Test 6: Char buffer processing */
    process_buffer(char_buf, 100);
    printf("Processed char buffer, first char: %d\n", (int)char_buf[0]);
    
    /* Test 7: Long long array sum */
    long long long_sum = sum_longs(long_arr, 20);
    printf("Sum of long array: %lld\n", long_sum);
    
    /* Test 8: Local array sum */
    result = sum_local_array();
    printf("Sum of local array: %d\n", result);
    
    /* Test 9: Matrix addition */
    matrix_add(&matrix3[0][0], &matrix1[0][0], &matrix2[0][0], 10, 10);
    printf("Matrix addition completed\n");
    
    /* Additional test: Pointer traversal in main itself */
    {
        const int *p = int_arr;
        int local_sum = 0;
        for (i = 0; i < 10; i++) {  /* Small loop to keep it in one basic block */
            local_sum += *p;  /* (mem (reg)) pattern */
            p = p + 1;        /* Separate increment */
        }
        printf("Main loop sum (first 10): %d\n", local_sum);
    }
    
    printf("All tests completed successfully!\n");
    printf("Checksum: %d\n", result + (int)float_sum + (int)long_sum);
    
    return 0;
}
