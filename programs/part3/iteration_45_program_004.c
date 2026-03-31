/* auto_inc_dec_test.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c auto_inc_dec_test.c
 * Or: gcc -O3 -funroll-loops -fno-inline -c auto_inc_dec_test.c
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[3];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, size_t n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many local variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use all temporaries to create register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 += temp1 * 2;
        temp4 += temp2 / 3;
        temp5 = temp3 ^ temp4;
        temp6 = temp5 + sum;
        temp7 = temp6 - temp1;
        temp8 = temp7 | temp2;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all variables to prevent elimination */
    use_int(temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8);
    return sum;
}

/* Test 2: String copy with post-increment on both pointers */
void test2_string_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int checksum = 0;
    
    do {
        /* Classic *dst++ = *src++ pattern */
        *d++ = *s++;
        
        /* Additional operations to prevent optimization */
        c1 = *s ^ 0x55;
        c2 = c1 + (char)i;
        c3 = c2 * 2;
        c4 = c3 - c1;
        checksum += c1 + c2 + c3 + c4;
        
        /* Make pointers appear used */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
        
        i++;
    } while (i < n);
    
    /* Use results */
    use_char(c4);
    global_checksum += checksum;
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array_sum(struct Data* arr, size_t n) {
    int sum = 0;
    struct Data* p = arr;
    
    /* Many local variables for register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    int v6 = 0, v7 = 0, v8 = 0, v9 = 0, v10 = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        sum += p->value;
        
        /* Complex expression with the pointer */
        v1 = p->payload[0];
        v2 = p->payload[1];
        v3 = p->payload[2];
        v4 = v1 + v2 + v3;
        v5 = sum * v4;
        v6 = v5 ^ (int)p->tag;
        v7 = v6 + i;
        v8 = v7 - v1;
        v9 = v8 * v2;
        v10 = v9 / (v3 ? v3 : 1);
        
        /* Post-increment the struct pointer */
        p++;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all variables */
    use_int(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    /* Create many local variables */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int j = 0;
        
        /* Inner loop with pointer post-increment */
        while (j < cols) {
            /* Access with post-increment */
            int val = *p++;
            
            /* Complex calculations to use registers */
            r1 += val * i;
            r2 += val * j;
            r3 = r1 ^ r2;
            r4 = r3 + val;
            
            /* Index also increments */
            j++;
            
            /* Additional operations */
            c1 += r4 & 0xF;
            c2 += (r4 >> 4) & 0xF;
            c3 = c1 * c2;
            c4 = c3 - r4;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(p), "r"(j) : "memory");
        }
        
        /* Use pointer in opaque function */
        use_ptr(row);
    }
    
    return total + r1 + r2 + r3 + r4 + c1 + c2 + c3 + c4;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test5_mixed_arithmetic(int* arr, size_t n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Variables for register pressure */
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    int f = 0, g = 0, h = 0, k = 0, l = 0;
    
    while (p < end) {
        /* Combined pointer arithmetic that may decompose to base+0 */
        int* access_ptr = p;
        
        /* Access current position */
        sum += *access_ptr;
        
        /* Complex pointer arithmetic */
        p += stride;
        
        /* Many calculations using the accessed value */
        a = sum & 0xFF;
        b = sum >> 8;
        c = a * b;
        d = c + (int)(p - arr);
        e = d ^ sum;
        f = e * stride;
        g = f + a;
        h = g - b;
        k = h | c;
        l = k & d;
        
        /* Force pointer to be in register */
        asm volatile("" : : "r"(p), "r"(access_ptr) : "memory");
    }
    
    /* Use all temporaries */
    use_int(a + b + c + d + e + f + g + h + k + l);
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1000;
    const size_t CHAR_ARRAY_SIZE = 500;
    const size_t STRUCT_ARRAY_SIZE = 200;
    const int MATRIX_ROWS = 50;
    const int MATRIX_COLS = 20;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array1 = malloc(CHAR_ARRAY_SIZE * sizeof(char));
    char* char_array2 = malloc(CHAR_ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(STRUCT_ARRAY_SIZE * sizeof(struct Data));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    if (!int_array || !char_array1 || !char_array2 || !struct_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with non-constant values */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (int)(i * 3 + argc);
    }
    
    for (size_t i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array1[i] = (char)(i % 256);
        char_array2[i] = 0;
    }
    
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        struct_array[i].value = (int)i * 7;
        struct_array[i].tag = (char)(i % 128);
        for (int j = 0; j < 3; j++) {
            struct_array[i].payload[j] = (int)(i * j + argc);
        }
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        if (matrix[i]) {
            for (int j = 0; j < MATRIX_COLS; j++) {
                matrix[i][j] = i * MATRIX_COLS + j + argc;
            }
        }
    }
    
    /* Run tests based on command-line arguments */
    int result = 0;
    
    if (argc > 1) {
        /* Use command-line argument to control which tests run */
        int test_selector = atoi(argv[1]) % 5;
        
        switch (test_selector) {
            case 0:
                result = test1_int_array_sum(int_array, INT_ARRAY_SIZE);
                break;
            case 1:
                test2_string_copy(char_array2, char_array1, CHAR_ARRAY_SIZE);
                result = char_array2[argc % CHAR_ARRAY_SIZE];
                break;
            case 2:
                result = test3_struct_array_sum(struct_array, STRUCT_ARRAY_SIZE);
                break;
            case 3:
                result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
                break;
            case 4:
                result = test5_mixed_arithmetic(int_array, INT_ARRAY_SIZE, 2 + argc % 5);
                break;
        }
    } else {
        /* Run all tests if no argument */
        result += test1_int_array_sum(int_array, INT_ARRAY_SIZE);
        test2_string_copy(char_array2, char_array1, CHAR_ARRAY_SIZE);
        result += test3_struct_array_sum(struct_array, STRUCT_ARRAY_SIZE);
        result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
        result += test5_mixed_arithmetic(int_array, INT_ARRAY_SIZE, 3);
    }
    
    /* Cleanup */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result + global_checksum);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFFFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}
