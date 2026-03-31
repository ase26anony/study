/* test_auto_inc_dec.c
 * Program designed to trigger auto-increment/decrement optimization
 * targeting uncovered lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void use_long(long) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    long timestamp;
    float weight;
};

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many local variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 ^ 0x55;
        temp4 = temp3 - temp1;
        temp5 = temp4 >> 2;
        temp6 = temp5 * 3;
        temp7 = temp6 & 0xFF;
        temp8 = temp7 | 0x80;
        
        /* Prevent optimization with asm barrier */
        asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4),
                         "r"(temp5), "r"(temp6), "r"(temp7), "r"(temp8));
    }
    
    /* Opaque function call to prevent elimination */
    use_ptr((void*)p);
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int count = 0;
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to prevent over-optimization */
        c1 = *(s - 1);
        c2 = c1 ^ 0x20;
        c3 = c2 + 1;
        c4 = c3 & 0x7F;
        
        count++;
        
        /* Use asm to make values appear used */
        asm volatile("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4), "r"(count));
    }
    
    /* Null terminate */
    *d = '\0';
    
    /* Prevent elimination */
    use_ptr(dst);
    use_ptr(src);
}

/* Test 3: Struct array traversal with post-increment */
long test_struct_array(const struct Data* arr, size_t n) {
    long total = 0;
    const struct Data* p = arr;
    const struct Data* end = arr + n;
    
    /* Many local variables for register pressure */
    int v1, v2, v3, v4;
    char t1, t2;
    float w1, w2;
    
    while (p < end) {
        /* Access struct member with post-increment */
        total += p++->value;
        
        /* Additional computations using the pointer */
        if (p > arr) {
            const struct Data* prev = p - 1;
            v1 = prev->value;
            t1 = prev->tag;
            w1 = prev->weight;
            
            v2 = v1 * 2;
            v3 = v2 + (int)t1;
            v4 = v3 ^ (int)w1;
            
            t2 = t1 + 1;
            w2 = w1 * 2.0f;
            
            asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                             "r"(t1), "r"(t2), "r"(w1), "r"(w2));
        }
    }
    
    use_long(total);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, m;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(row_ptr + j);
            
            /* Additional index manipulation */
            k = j * 2;
            m = k + i;
            
            /* Use stride-based access pattern */
            if (j < cols - 1) {
                sum += *(row_ptr += 1);  /* Combined pre-increment access */
                row_ptr--;  /* Reset for next iteration */
            }
        }
        
        /* Post-increment in loop update with additional operations */
        for (k = 0; k < cols; k++) {
            int* p = matrix[i] + k;
            sum += *p;
            
            /* Create complex addressing pattern */
            int offset = (k * 3) % cols;
            sum += *(matrix[i] + offset);
        }
    }
    
    /* Prevent optimization */
    for (i = 0; i < rows; i++) {
        use_ptr(matrix[i]);
    }
    
    return sum;
}

/* Test 5: Mixed pointer types with arithmetic */
void test_mixed_pointers(void* base, size_t size) {
    char* cptr = (char*)base;
    int* iptr = (int*)base;
    long* lptr = (long*)base;
    
    size_t char_count = size;
    size_t int_count = size / sizeof(int);
    size_t long_count = size / sizeof(long);
    
    int char_sum = 0;
    int int_sum = 0;
    long long_sum = 0;
    
    /* Process with char pointer */
    for (size_t i = 0; i < char_count; i++) {
        char_sum += *cptr++;
    }
    
    /* Process with int pointer - different stride */
    for (size_t i = 0; i < int_count; i++) {
        int_sum += *iptr++;
    }
    
    /* Process with long pointer - different stride */
    for (size_t i = 0; i < long_count; i++) {
        long_sum += *lptr++;
    }
    
    /* Mix all results */
    global_checksum += char_sum + int_sum + (int)long_sum;
    
    /* Opaque calls */
    use_int(char_sum);
    use_int(int_sum);
    use_long(long_sum);
}

/* Test 6: Do-while loop with post-decrement */
int test_do_while(int* arr, size_t n) {
    int sum = 0;
    int* p = arr + n - 1;  /* Start from end */
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0;
    
    if (n > 0) {
        do {
            /* Post-decrement access */
            sum += *p--;
            
            /* Complex computations for register pressure */
            a = sum & 0xF;
            b = (sum >> 4) & 0xF;
            c = a + b;
            d = c * 2;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        } while (p >= arr);
    }
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1024;
    const size_t CHAR_ARRAY_SIZE = 512;
    const size_t STRUCT_COUNT = 256;
    const size_t MATRIX_ROWS = 32;
    const size_t MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(CHAR_ARRAY_SIZE);
    struct Data* struct_array = malloc(STRUCT_COUNT * sizeof(struct Data));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    if (!int_array || !char_array || !struct_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) & 0xFF;
    }
    
    for (size_t i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array[i] = 'A' + (i % 26);
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].timestamp = i * 1000;
        struct_array[i].weight = i * 0.5f;
    }
    
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        if (matrix[i]) {
            for (size_t j = 0; j < MATRIX_COLS; j++) {
                matrix[i][j] = i * MATRIX_COLS + j;
            }
        }
    }
    
    /* Command-line controlled execution paths */
    int test_mask = 0xFF;  /* Run all tests by default */
    
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    int total_result = 0;
    
    /* Run selected tests */
    if (test_mask & 0x01) {
        total_result += test_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        char* dst = malloc(CHAR_ARRAY_SIZE);
        if (dst) {
            test_string_copy(dst, char_array, CHAR_ARRAY_SIZE);
            free(dst);
        }
    }
    
    if (test_mask & 0x04) {
        total_result += test_struct_array(struct_array, STRUCT_COUNT);
    }
    
    if (test_mask & 0x08) {
        total_result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (test_mask & 0x10) {
        test_mixed_pointers(int_array, INT_ARRAY_SIZE * sizeof(int));
    }
    
    if (test_mask & 0x20) {
        total_result += test_do_while(int_array, INT_ARRAY_SIZE);
    }
    
    /* Update global checksum */
    global_checksum += total_result;
    
    /* Print result to prevent elimination */
    printf("Result: %d (checksum: %d)\n", total_result, global_checksum);
    
    /* Cleanup */
    free(int_array);
    free(char_array);
    free(struct_array);
    
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((uintptr_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}

void __attribute__((noinline)) use_long(long l) {
    global_checksum += (int)(l & 0xFF);
}
