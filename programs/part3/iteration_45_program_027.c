/* auto-inc-dec-test.c - Test program to trigger auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Opaque external functions to create aliasing concerns */
extern void use_int_ptr(int *p) __attribute__((noinline));
extern void use_char_ptr(char *p) __attribute__((noinline));
extern void use_void_ptr(void *p) __attribute__((noinline));

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    short count;
    int data[3];
};

/* Test 1: Sum integer array with post-incrementing pointer in loop */
int test1_int_array_sum(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Access with post-increment - should create (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use temp variables to increase register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 * 3;
        temp4 = temp3 - sum;
        temp5 = temp4 ^ temp1;
        temp6 = temp5 | temp2;
        temp7 = temp6 & temp3;
        temp8 = temp7 + temp4;
        
        /* Prevent optimization with asm barrier */
        asm volatile("" : : "r"(p), "r"(sum), "r"(temp1), "r"(temp2), 
                      "r"(temp3), "r"(temp4), "r"(temp5), "r"(temp6));
    }
    
    /* Use opaque function to prevent pointer elimination */
    use_int_ptr(arr);
    
    /* Mix results to prevent constant folding */
    return sum + temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src;
    int i;
    
    /* Create register pressure */
    int checksum = 0;
    char c1, c2, c3, c4;
    
    /* Copy with post-increment - classic pattern for auto-inc-dec */
    for (i = 0; i < n; i++) {
        *d++ = *s++;
        
        /* Access with different strides to create varied patterns */
        if (i % 4 == 0) {
            c1 = *(s - 1);
            c2 = *(d - 1);
            checksum += c1 + c2;
        }
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(d), "r"(s), "r"(checksum));
    }
    
    /* Force compiler to keep variables alive */
    global_checksum += checksum;
    use_char_ptr(dst);
    use_char_ptr((char *)src);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(struct Data *arr, int n) {
    struct Data *p = arr;
    int total = 0;
    
    /* Many live variables for register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    short s1 = 0, s2 = 0;
    
    /* Loop through struct array */
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        s1 += p->count;
        
        /* Complex expression to prevent simplification */
        v1 = (p->value * 3) / 2;
        v2 = v1 + p->data[0];
        v3 = v2 - p->data[1];
        v4 = v3 ^ p->data[2];
        
        /* Post-increment pointer */
        p++;
        
        /* Use all variables to keep them live */
        asm volatile("" : : "r"(p), "r"(total), "r"(v1), "r"(v2), 
                      "r"(v3), "r"(v4), "r"(s1), "r"(s2));
    }
    
    use_void_ptr(arr);
    return total + v1 + v2 + v3 + v4 + s1 + s2;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int **matrix, int rows, int cols) {
    int sum = 0;
    
    /* High register pressure variables */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row = matrix[i];
        int *p = row;
        int *end = row + cols;
        
        /* Inner loop with pointer post-increment */
        while (p < end) {
            /* Multiple accesses to increase chances */
            sum += *p++;
            r1 += *p;
            r2 += *(p - 1);
            
            /* Complex addressing patterns */
            if (p - row > 2) {
                r3 += *(p - 2);
                r4 += *(p - 3);
            }
            
            /* Use asm to prevent reordering */
            asm volatile("" : : "r"(p), "r"(sum), "r"(r1), "r"(r2), 
                          "r"(r3), "r"(r4), "r"(c1), "r"(c2));
        }
        
        /* Update outer loop variables */
        c1 += i * 2;
        c2 += i * 3;
        c3 += i * 5;
        c4 += i * 7;
    }
    
    return sum + r1 + r2 + r3 + r4 + c1 + c2 + c3 + c4;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test5_mixed_pointer_ops(int *arr, int n, int stride) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n * stride;
    
    /* Multiple pointer variables to create different addressing modes */
    int *q = arr;
    int *r = arr + stride;
    
    /* Do-while loop for different control flow */
    do {
        /* Combined pointer arithmetic that may decompose to base+0 */
        sum += *p;
        p += stride;
        
        /* Different access pattern */
        if (q < end) {
            sum += *q++;
            sum += *r;
            r += stride;
        }
        
        /* Create many intermediate values */
        int t1 = sum * 2;
        int t2 = t1 + (p - arr);
        int t3 = t2 * (q - arr);
        int t4 = t3 ^ (r - arr);
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(p), "r"(q), "r"(r), "r"(sum),
                      "r"(t1), "r"(t2), "r"(t3), "r"(t4));
        
    } while (p < end);
    
    use_int_ptr(arr);
    return sum;
}

/* Main function with command-line arguments to prevent constant folding */
int main(int argc, char *argv[]) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int STRING_SIZE = 500;
    const int STRUCT_COUNT = 200;
    const int MATRIX_ROWS = 50;
    const int MATRIX_COLS = 20;
    
    /* Allocate and initialize arrays */
    int *int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char *string1 = malloc(STRING_SIZE);
    char *string2 = malloc(STRING_SIZE);
    struct Data *struct_array = malloc(STRUCT_COUNT * sizeof(struct Data));
    int **matrix = malloc(MATRIX_ROWS * sizeof(int *));
    
    /* Initialize with non-constant data based on argv */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = rand() % 100;
    }
    
    for (int i = 0; i < STRING_SIZE; i++) {
        string1[i] = 'A' + (rand() % 26);
    }
    string1[STRING_SIZE - 1] = '\0';
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = rand() % 1000;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].count = i % 100;
        for (int j = 0; j < 3; j++) {
            struct_array[i].data[j] = rand() % 100;
        }
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    /* Run tests based on command-line arguments */
    int result = 0;
    
    if (argc < 2 || strcmp(argv[1], "1") == 0) {
        result += test1_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (argc < 2 || strcmp(argv[1], "2") == 0) {
        test2_string_copy(string2, string1, STRING_SIZE - 1);
        result += string2[0] + string2[STRING_SIZE / 2];
    }
    
    if (argc < 2 || strcmp(argv[1], "3") == 0) {
        result += test3_struct_array(struct_array, STRUCT_COUNT);
    }
    
    if (argc < 2 || strcmp(argv[1], "4") == 0) {
        result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (argc < 2 || strcmp(argv[1], "5") == 0) {
        result += test5_mixed_pointer_ops(int_array, INT_ARRAY_SIZE / 4, 4);
    }
    
    /* Update global volatile to ensure side effects */
    global_checksum += result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(string1);
    free(string2);
    free(struct_array);
    
    printf("Result: %d, Global checksum: %d\n", result, global_checksum);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int_ptr(int *p) {
    asm volatile("" : : "r"(p));
}

void __attribute__((noinline)) use_char_ptr(char *p) {
    asm volatile("" : : "r"(p));
}

void __attribute__((noinline)) use_void_ptr(void *p) {
    asm volatile("" : : "r"(p));
}
