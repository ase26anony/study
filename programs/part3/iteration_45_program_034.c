/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent optimization */
volatile int global_sum = 0;

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

void use_int(int x) {
    asm volatile("" : : "r"(x));
}

void use_ptr(void* p) {
    asm volatile("" : : "r"(p));
}

void use_char(char c) {
    asm volatile("" : : "r"(c));
}

/* Test 1: Integer array sum with post-increment pointer in loop */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    /* Create register pressure with many live variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Loop with post-increment */
    for (int k = 0; k < n; k++) {
        /* This should generate mem_insn with (plus (reg) (const_int 0)) */
        sum += *p++;
        
        /* Use all the live variables to maintain register pressure */
        a += b; b += c; c += d; d += e; e += f;
        f += g; g += h; h += i; i += j; j += a;
        
        /* Make pointer appear used */
        asm volatile("" : : "r"(p));
    }
    
    /* Use all variables to prevent optimization */
    use_int(a); use_int(b); use_int(c); use_int(d); use_int(e);
    use_int(f); use_int(g); use_int(h); use_int(i); use_int(j);
    
    return sum;
}

/* Test 2: String copy with post-increment */
void test_string_copy(char* dst, const char* src, int n) {
    /* Create register pressure */
    char x = 'a', y = 'b', z = 'c';
    int counter = 0;
    
    /* Copy with post-increment */
    for (int i = 0; i < n; i++) {
        *dst++ = *src++;
        counter++;
        
        /* Mix in other operations */
        x = y + 1;
        y = z + 1;
        z = x + 1;
        
        /* Use pointers to prevent optimization */
        asm volatile("" : : "r"(dst), "r"(src));
    }
    
    use_char(x); use_char(y); use_char(z);
    use_int(counter);
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    
    /* Many live variables for register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    /* Loop with post-increment and member access */
    for (int i = 0; i < n; i++) {
        /* Access struct members - should create multiple memory accesses */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Maintain register pressure */
        v1 += v2; v2 += v3; v3 += v4; v4 += v5; v5 += v6;
        v6 += v7; v7 += v8; v8 += v9; v9 += v10; v10 += v1;
        
        /* Use pointer */
        use_ptr(p);
    }
    
    /* Use all variables */
    for (int k = 1; k <= 10; k++) {
        use_int(k * 10);
    }
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* High register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        
        /* Inner loop with pointer arithmetic */
        for (int j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(p++);
            
            /* Alternative: sum += *p; p += 1; */
            
            /* Complex addressing that might become (plus (reg) (const_int 0)) */
            if (j % 2 == 0) {
                sum += *(p + 0);  /* Explicit zero offset */
            }
            
            /* Maintain register pressure */
            r1 = r2 + r3; r2 = r3 + r4; r3 = r4 + r5; r4 = r5 + r6;
            r5 = r6 + r7; r6 = r7 + r8; r7 = r8 + r9; r8 = r9 + r10;
            r9 = r10 + r1; r10 = r1 + r2;
        }
        
        /* Use pointer to prevent optimization */
        asm volatile("" : : "r"(p));
    }
    
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Test 5: Mixed pointer types and arithmetic */
int test_mixed_pointers(char* data, int size) {
    int sum = 0;
    char* cp = data;
    int* ip = (int*)data;
    
    /* Different stride values */
    int char_stride = 1;
    int int_stride = sizeof(int);
    
    /* Loop with mixed pointer operations */
    for (int i = 0; i < size / 8; i++) {
        /* Post-increment with different types */
        sum += (int)(*cp++);
        sum += *ip++;
        
        /* Pointer arithmetic with explicit addition */
        cp = cp + char_stride - 1;  /* Net effect: cp++ */
        ip = ip + int_stride / sizeof(int) - 1;  /* Net effect: ip++ */
        
        /* Use both pointers */
        asm volatile("" : : "r"(cp), "r"(ip));
    }
    
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
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string_src = malloc(STRING_SIZE);
    char* string_dst = malloc(STRING_SIZE);
    struct Point* points = malloc(STRUCT_COUNT * sizeof(struct Point));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 100;
    }
    
    for (int i = 0; i < STRING_SIZE; i++) {
        string_src[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Use command-line arguments to control which tests run */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 6;
    }
    
    int result = 0;
    
    /* Run selected test(s) */
    switch (test_to_run) {
        case 0:
            result = test_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test_string_copy(string_dst, string_src, STRING_SIZE - 1);
            result = string_dst[0] + string_dst[STRING_SIZE - 2];
            break;
        case 2:
            result = test_struct_array(points, STRUCT_COUNT);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_pointers((char*)int_array, INT_ARRAY_SIZE * sizeof(int));
            break;
        default:
            /* Run all tests */
            result += test_int_array_sum(int_array, INT_ARRAY_SIZE / 2);
            test_string_copy(string_dst, string_src, STRING_SIZE / 2);
            result += test_struct_array(points, STRUCT_COUNT / 2);
            result += test_nested_loops(matrix, MATRIX_ROWS / 2, MATRIX_COLS / 2);
            result += test_mixed_pointers((char*)int_array, (INT_ARRAY_SIZE * sizeof(int)) / 2);
            break;
    }
    
    /* Update global volatile to prevent dead code elimination */
    global_sum += result;
    
    /* Cleanup */
    free(int_array);
    free(string_src);
    free(string_dst);
    free(points);
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    return 0;
}
