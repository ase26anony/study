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
    
    /* Make variables appear used */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
    asm volatile("" : : "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10));
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd', c5 = 'e';
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        i++;
        
        /* Mix operations */
        c1++; c2--; c3 += 2; c4 -= 1; c5 ^= 0x20;
        
        /* Opaque function call to prevent optimization */
        use_char(c1);
        
    } while (i < n && *(s-1) != '\0');
    
    /* Force pointer usage */
    use_ptr(d);
    use_ptr(s);
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
    int a = 0, b = 1, c = 2, d = 3, e = 4;
    int f = 5, g = 6, h = 7, i = 8, j = 9;
    
    for (int idx = 0; idx < n; idx++) {
        /* Access struct member with post-increment */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Complex calculations with many variables */
        a = b + c; b = c + d; c = d + e; d = e + f; e = f + g;
        f = g + h; g = h + i; h = i + j; i = j + a; j = a + b;
        
        /* Prevent elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
    }
    
    return total;
}

/* Test 4: Nested loops with array indexing */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m;
    int t1, t2, t3, t4, t5;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        j = 0;
        
        /* Inner loop with post-increment */
        while (j < cols) {
            /* Combined form that may decompose to base+offset */
            sum += *(row + j);
            j++;  /* Post-increment */
            
            /* Additional operations */
            k = i * j; l = j * j; m = i * i;
            t1 = k + l; t2 = l + m; t3 = m + k;
            t4 = t1 + t2; t5 = t3 + t4;
            
            /* Use results */
            asm volatile("" : : "r"(t4), "r"(t5));
        }
        
        /* Opaque call */
        use_ptr(row);
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic patterns */
int test_mixed_patterns(char* data, int size) {
    int result = 0;
    char* p = data;
    char* end = data + size;
    
    /* Multiple stride values */
    int stride1 = 1, stride2 = 2, stride3 = 4;
    
    while (p < end) {
        /* Different access patterns */
        result += *p;           /* Simple access */
        p += stride1;           /* Post-add with stride */
        
        if (p + stride2 < end) {
            result += *(p + stride2);  /* Offset access */
        }
        
        /* Combined increment and access */
        if (p + stride3 < end) {
            result += *(p += stride3);  /* Combined form */
        }
        
        /* Register pressure */
        stride1 = (stride1 * 3) % 7;
        stride2 = (stride2 * 5) % 11;
        stride3 = (stride3 * 7) % 13;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int STRING_SIZE = 256;
    const int STRUCT_COUNT = 500;
    const int MATRIX_ROWS = 50;
    const int MATRIX_COLS = 20;
    const int MIXED_SIZE = 1000;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string_src = malloc(STRING_SIZE);
    char* string_dst = malloc(STRING_SIZE);
    struct Point* struct_array = malloc(STRUCT_COUNT * sizeof(struct Point));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    char* mixed_data = malloc(MIXED_SIZE);
    
    /* Initialize with non-constant data */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
    }
    
    for (int i = 0; i < STRING_SIZE - 1; i++) {
        string_src[i] = 'A' + (i % 26);
    }
    string_src[STRING_SIZE - 1] = '\0';
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    for (int i = 0; i < MIXED_SIZE; i++) {
        mixed_data[i] = (i * 7) % 256;
    }
    
    /* Run tests based on command line arguments */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_to_run) {
        case 1:
            global_checksum += test_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 2:
            test_string_copy(string_dst, string_src, STRING_SIZE);
            global_checksum += string_dst[0];
            break;
        case 3:
            global_checksum += test_struct_array(struct_array, STRUCT_COUNT);
            break;
        case 4:
            global_checksum += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            global_checksum += test_mixed_patterns(mixed_data, MIXED_SIZE);
            break;
        default:
            /* Run all tests */
            global_checksum += test_int_array_sum(int_array, INT_ARRAY_SIZE);
            test_string_copy(string_dst, string_src, STRING_SIZE);
            global_checksum += test_struct_array(struct_array, STRUCT_COUNT);
            global_checksum += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            global_checksum += test_mixed_patterns(mixed_data, MIXED_SIZE);
            break;
    }
    
    /* Cleanup */
    free(int_array);
    free(string_src);
    free(string_dst);
    free(struct_array);
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(mixed_data);
    
    printf("Checksum: %d\n", global_checksum);
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
