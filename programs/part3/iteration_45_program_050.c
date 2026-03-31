/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += r2; r3 += r4; r5 += r6; r7 += r8; r9 += r10;
        r2 += r1; r4 += r3; r6 += r5; r8 += r7; r10 += r9;
    }
    
    /* Use all variables to prevent elimination */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5),
                     "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10));
    
    return sum;
}

/* Test 2: String copy with post-increment */
void test_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure variables */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    
    for (int i = 0; i < n; i++) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Mix operations to prevent over-optimization */
        c1 = c2 + 1; c2 = c3 + 1; c3 = c4 + 1; c4 = c1 + 1;
        i1 = i2 * 2; i2 = i3 * 2; i3 = i4 * 2; i4 = i1 * 2;
    }
    
    /* Opaque use of pointers */
    use_ptr(dst);
    use_ptr((void*)src);
    
    /* Force variable usage */
    asm volatile("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4),
                     "r"(i1), "r"(i2), "r"(i3), "r"(i4));
}

/* Test 3: Struct array traversal */
struct Point {
    int x;
    int y;
    int z;
};

int test_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    
    /* High register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->x + p->y + p->z;
        p++;  /* Post-increment after access */
        
        /* Complex calculations to use registers */
        v1 = v2 + v3; v4 = v5 + v6; v7 = v8 + v9;
        v10 = v11 + v12; v13 = v14 + v15;
        v2 = v1 * 2; v5 = v4 * 2; v8 = v7 * 2;
        v11 = v10 * 2; v14 = v13 * 2;
    }
    
    /* Prevent elimination */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                     "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                     "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
    
    return total;
}

/* Test 4: Nested loops with array indexing */
int test_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Many live variables */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix + i * cols;
        int j = 0;
        
        while (j < cols) {
            /* Array access with post-increment index */
            sum += row[j++];  /* This should create base+offset pattern */
            
            /* Intermix calculations */
            a1 = a2 + a3; a4 = a5 + b1;
            b2 = b3 + b4; b5 = a1 + a4;
            a2 = a1 * 3; a5 = a4 * 3;
            b3 = b2 * 3; b1 = b5 * 3;
        }
    }
    
    /* Use variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                     "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic forms */
int test_mixed_forms(char* data, int size) {
    int sum = 0;
    char* p = data;
    char* end = data + size;
    
    /* Extreme register pressure */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    int r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8, r9 = 9;
    
    do {
        /* Different forms of pointer access */
        sum += *p;          /* Direct access */
        p += 1;             /* Separate increment */
        
        if (p < end - 1) {
            /* Combined form that may decompose to base+offset */
            sum += *(p += 1);
        }
        
        /* Heavy register usage */
        r0 = r1 + r2; r3 = r4 + r5; r6 = r7 + r8;
        r1 = r0 * 2; r4 = r3 * 2; r7 = r6 * 2;
        r2 = r1 + r9; r5 = r4 + r9; r8 = r7 + r9;
        r9 = r2 + r5 + r8;
        
    } while (p < end);
    
    /* Force all registers to be used */
    asm volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3),
                     "r"(r4), "r"(r5), "r"(r6), "r"(r7),
                     "r"(r8), "r"(r9));
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int STRING_SIZE = 256;
    const int STRUCT_COUNT = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string_src = malloc(STRING_SIZE);
    char* string_dst = malloc(STRING_SIZE);
    struct Point* struct_array = malloc(STRUCT_COUNT * sizeof(struct Point));
    int* matrix = malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    char* mixed_data = malloc(STRING_SIZE);
    
    /* Initialize with non-constant data */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 100;
    }
    
    for (int i = 0; i < STRING_SIZE; i++) {
        string_src[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = i % 50;
    }
    
    for (int i = 0; i < STRING_SIZE; i++) {
        mixed_data[i] = i % 256;
    }
    
    /* Select test based on command line to prevent constant folding */
    int test_num = 0;
    if (argc > 1) {
        test_num = atoi(argv[1]) % 6;
    }
    
    int result = 0;
    
    switch (test_num) {
        case 0:
            result = test_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test_string_copy(string_dst, string_src, STRING_SIZE);
            result = string_dst[0] + string_dst[STRING_SIZE-1];
            break;
        case 2:
            result = test_struct_array(struct_array, STRUCT_COUNT);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_forms(mixed_data, STRING_SIZE);
            break;
        case 5:
            /* Run all tests */
            result = test_int_array_sum(int_array, INT_ARRAY_SIZE);
            test_string_copy(string_dst, string_src, STRING_SIZE);
            result += test_struct_array(struct_array, STRUCT_COUNT);
            result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test_mixed_forms(mixed_data, STRING_SIZE);
            break;
    }
    
    /* Update global volatile to prevent elimination */
    global_checksum = result;
    
    /* Use result to prevent dead code */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(int_array);
    free(string_src);
    free(string_dst);
    free(struct_array);
    free(matrix);
    free(mixed_data);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    asm volatile("" : : "r"(x));
}

void __attribute__((noinline)) use_ptr(void* p) {
    asm volatile("" : : "r"(p));
}

void __attribute__((noinline)) use_char(char c) {
    asm volatile("" : : "r"(c));
}
