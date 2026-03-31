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
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 = sum ^ 0x55;
        r2 = r1 + *p;
        r3 = r2 * 2;
        r4 = r3 - r1;
        r5 = r4 & 0xFF;
        r6 = r5 | 0xAA;
        r7 = r6 << 1;
        r8 = r7 >> 1;
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(r1), "r"(r2), 
                     "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8));
    }
    
    /* Opaque function call */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to create complex RTL */
        c1 = *(s - 1);
        c2 = *(d - 1);
        t1 = c1 + c2;
        t2 = t1 * 2;
        t3 = t2 - c1;
        t4 = t3 & 0xFF;
        
        /* Use pointer values */
        asm volatile("" : : "r"(d), "r"(s), "r"(t1), "r"(t2));
        
        i++;
    } while (i < n && *s != '\0');
    
    /* Force pointer usage */
    use_ptr(dst);
    use_ptr((void*)src);
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_array_sum(struct Point* points, int n) {
    int sum = 0;
    struct Point* p = points;
    struct Point* end = points + n;
    
    /* Heavy register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8;
    int b1, b2, b3, b4, b5, b6, b7, b8;
    
    for (; p < end; p++) {
        /* Access struct member with pointer - may create base+0 offset */
        sum += p->x;
        sum += p->y;
        sum += p->z;
        
        /* Complex calculations for register pressure */
        a1 = p->x * 2;
        a2 = p->y + a1;
        a3 = p->z - a2;
        a4 = a3 & 0xFF;
        a5 = a4 | 0x55;
        a6 = a5 << 1;
        a7 = a6 >> 1;
        a8 = a7 ^ 0xAA;
        
        b1 = sum + a1;
        b2 = b1 * a2;
        b3 = b2 / (a3 ? a3 : 1);
        b4 = b3 - a4;
        b5 = b4 & a5;
        b6 = b5 | a6;
        b7 = b6 ^ a7;
        b8 = b7 + a8;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(a1), "r"(b1),
                     "r"(a2), "r"(b2), "r"(a3), "r"(b3));
    }
    
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n, o, p;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* row_end = row + cols;
        int* ptr = row;
        
        /* Inner loop with pointer post-increment */
        while (ptr < row_end) {
            /* Target pattern: *(ptr + 0) with post-increment */
            total += *ptr++;
            
            /* Additional index calculations */
            j = i * cols;
            k = (int)(ptr - row);
            l = j + k;
            m = l * 2;
            n = m - i;
            o = n & 0xFF;
            p = o | 0x55;
            
            /* Use all variables */
            asm volatile("" : : "r"(ptr), "r"(total), "r"(j), "r"(k),
                         "r"(l), "r"(m), "r"(n), "r"(o), "r"(p));
        }
        
        /* Mixed pointer arithmetic */
        int* temp = matrix[i];
        temp += cols / 2;  /* Creates (plus (reg) (const_int)) patterns */
        total += *temp;
    }
    
    return total;
}

/* Test 5: Mixed pointer types and arithmetic */
void test5_mixed_pointers(char* data, int size) {
    char* cptr = data;
    int* iptr = (int*)data;
    short* sptr = (short*)data;
    
    int char_sum = 0;
    int int_sum = 0;
    int short_sum = 0;
    
    /* Multiple simultaneous pointer traversals */
    for (int i = 0; i < size; i += 8) {
        /* Different pointer types with post-increment */
        char_sum += *cptr++;
        int_sum += *iptr++;
        short_sum += *sptr++;
        
        /* Combined forms that may decompose to base+0 */
        cptr += 1;  /* Simple increment */
        iptr = iptr + 1;  /* Another form */
        sptr = &sptr[1];  /* Array indexing form */
        
        /* Force register usage */
        int t1 = *cptr;
        int t2 = *iptr;
        int t3 = *sptr;
        int t4 = t1 + t2 + t3;
        int t5 = t4 * 2;
        int t6 = t5 - t1;
        int t7 = t6 & 0xFF;
        
        asm volatile("" : : "r"(cptr), "r"(iptr), "r"(sptr),
                     "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    }
    
    use_int(char_sum + int_sum + short_sum);
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 16;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Point* struct_array = (struct Point*)malloc(ARRAY_SIZE/4 * sizeof(struct Point));
    int** matrix = (int**)malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = (int*)malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = (i * MATRIX_COLS + j) % 127;
        }
    }
    
    /* Use command-line arguments to control execution path */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 6;
    }
    
    int result = 0;
    
    /* Execute different tests based on input */
    switch (test_to_run) {
        case 0:
            result = test1_int_array_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test2_string_copy(char_array, "Test string for copy operation", ARRAY_SIZE);
            result = char_array[0];
            break;
        case 2:
            result = test3_struct_array_sum(struct_array, ARRAY_SIZE/4);
            break;
        case 3:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            test5_mixed_pointers(char_array, ARRAY_SIZE);
            result = 1;
            break;
        default:
            /* Run all tests */
            result = test1_int_array_sum(int_array, ARRAY_SIZE);
            test2_string_copy(char_array, "Test string", ARRAY_SIZE);
            result += test3_struct_array_sum(struct_array, ARRAY_SIZE/4);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            test5_mixed_pointers(char_array, ARRAY_SIZE);
            break;
    }
    
    /* Update global volatile to prevent optimization */
    global_checksum = result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((long)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}
