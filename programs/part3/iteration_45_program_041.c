/* auto-inc-dec-test.c */
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
int test1_int_array_sum(int* arr, int n) {
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
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent optimization with asm */
        asm volatile("" : : "r"(p), "r"(sum));
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Opaque function call */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd', c5 = 'e';
    
    int i = 0;
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use pressure variables */
        c1 = *s; c2 = c1 + 1; c3 = c2 + 1; c4 = c3 + 1; c5 = c4 + 1;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d), "r"(s));
        
        i++;
    } while (i < n);
    
    /* Opaque calls */
    use_char(c1);
    use_char(c5);
    use_ptr(dst);
}

/* Test 3: Struct array traversal */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    
    /* Heavy register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Complex calculations with pressure variables */
        a1 = total & 0xFF; a2 = a1 * 2; a3 = a2 + a1;
        a4 = a3 >> 1; a5 = a4 ^ a3;
        b1 = p->x; b2 = b1 * 3; b3 = b2 - b1;
        b4 = b3 / 2; b5 = b4 | b3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total));
    }
    
    total += a1 + a2 + a3 + a4 + a5 + b1 + b2 + b3 + b4 + b5;
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        j = 0;
        
        while (j < cols) {
            /* Array access with post-increment in index */
            sum += row[j++];  /* This may generate base+offset */
            
            /* Additional operations */
            k = sum % 256;
            l = k * 2;
            m = l + i;
            
            tmp1 = row[0]; tmp2 = tmp1 + 1;
            tmp3 = tmp2 * 2; tmp4 = tmp3 - 1;
            tmp5 = tmp4 ^ tmp3;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row), "r"(j), "r"(sum));
        }
        
        /* More pressure */
        sum += tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
    }
    
    use_int(sum);
    return sum;
}

/* Test 5: Mixed pointer arithmetic */
int test5_mixed_arithmetic(char* data, int size, int stride) {
    int result = 0;
    char* p = data;
    char* end = data + size;
    
    /* Extreme register pressure */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    int r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8, r9 = 9;
    
    /* Combined forms that may decompose to base+offset */
    while (p < end) {
        /* Different access patterns */
        result += *(int*)p;
        p += stride;  /* This may become base + 0 after decomposition */
        
        /* Use all register pressure variables */
        r0 = result; r1 = r0 + 1; r2 = r1 * 2;
        r3 = r2 - r0; r4 = r3 >> 1; r5 = r4 ^ r3;
        r6 = *p; r7 = r6 * 3; r8 = r7 - r6; r9 = r8 & 0xFF;
        
        /* Force pointer to be live */
        asm volatile("" : : "r"(p), "r"(result), 
                     "r"(r0), "r"(r1), "r"(r2), "r"(r3));
    }
    
    result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    use_int(result);
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int STRING_SIZE = 256;
    const int STRUCT_COUNT = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string1 = malloc(STRING_SIZE);
    char* string2 = malloc(STRING_SIZE);
    struct Point* points = malloc(STRUCT_COUNT * sizeof(struct Point));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;  /* Non-trivial pattern */
    }
    
    for (int i = 0; i < STRING_SIZE; i++) {
        string1[i] = 'A' + (i % 26);
    }
    string1[STRING_SIZE - 1] = '\0';
    
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
    
    /* Run tests based on command line argument */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_to_run) {
        case 1:
            result = test1_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 2:
            test2_string_copy(string2, string1, STRING_SIZE - 1);
            result = string2[0] + string2[STRING_SIZE - 2];
            break;
        case 3:
            result = test3_struct_array(points, STRUCT_COUNT);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_arithmetic((char*)int_array, 
                                           INT_ARRAY_SIZE * sizeof(int), 
                                           sizeof(int));
            break;
        default:
            /* Run all tests */
            result += test1_int_array_sum(int_array, INT_ARRAY_SIZE);
            test2_string_copy(string2, string1, STRING_SIZE - 1);
            result += test3_struct_array(points, STRUCT_COUNT);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_arithmetic((char*)int_array,
                                            INT_ARRAY_SIZE * sizeof(int),
                                            sizeof(int));
            break;
    }
    
    /* Update global to prevent elimination */
    global_checksum = result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(string1);
    free(string2);
    free(points);
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (long)p & 0xFF;
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}
