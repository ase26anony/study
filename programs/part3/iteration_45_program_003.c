/* auto-inc-dec-test.c
 * Designed to trigger find_inc() logic for post-increment/decrement patterns
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c auto-inc-dec-test.c -o test.o
 * Or: gcc -O3 -funroll-loops -fno-inline -c auto-inc-dec-test.c -o test.o
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
    int id;
    float weight;
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Pattern: *p++ - post-increment after fetch */
        sum += *p++;
        
        /* Use many variables to increase register pressure */
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
    
    /* Use all temporaries to prevent elimination */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    
    /* Opaque function call to prevent optimization */
    use_int(sum);
    
    return sum;
}

/* Test 2: String copy with dual post-increment pointers */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int count = 0;
    
    /* Classic *dst++ = *src++ pattern */
    for (int i = 0; i < n; i++) {
        *d++ = *s++;
        count++;
        
        /* Additional operations to create complex RTL */
        c1 = *(d - 1);
        c2 = *(s - 1);
        c3 = c1 ^ c2;
        c4 = c3 + 'A';
        
        /* Use pointer values to prevent optimization */
        asm volatile("" : : "r"(d), "r"(s), "r"(c1), "r"(c2));
        
        /* Opaque use of values */
        use_char(c3);
        use_char(c4);
    }
    
    /* Ensure null termination */
    if (n > 0) d[-1] = '\0';
    
    /* Use pointers after loop */
    use_ptr(dst);
    use_ptr(src);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array_sum(struct Data* arr, int n) {
    int sum = 0;
    struct Data* p = arr;
    
    /* Multiple live variables for register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    float f1 = 0.0f, f2 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        sum += p->value;
        
        /* More complex access patterns */
        v1 = p->id;
        v2 = p->tag;
        v3 = v1 * v2;
        v4 = sum ^ v3;
        
        f1 = p->weight;
        f2 = f1 * 2.0f;
        
        /* Post-increment the struct pointer */
        p++;
        
        /* Use all variables to keep them alive */
        sum += v3 + v4;
        asm volatile("" : : "r"(p), "r"(sum), "r"(v1), "r"(v2),
                         "r"(v3), "r"(v4), "r"(f1), "r"(f2));
    }
    
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form: *(p + offset) with post-increment */
            total += *(row_ptr + j);
            
            /* Additional index calculations */
            k = i * cols + j;
            l = total ^ k;
            m = l * 3;
            n = m - j;
            
            /* Use variables to prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(total), "r"(k), "r"(l));
        }
        
        /* Alternative: pointer increment in loop update */
        int* p = matrix[i];
        int* end = p + cols;
        while (p < end) {
            /* This should generate mem_insn with reg+0 offset */
            total += *p;
            
            /* Post-increment in separate expression */
            p++;
            
            /* Additional operations */
            total ^= (int)(p - matrix[i]);
        }
    }
    
    return total;
}

/* Test 5: Mixed pointer types and arithmetic */
void test5_mixed_pointers(int* int_arr, char* char_arr, int n) {
    int* ip = int_arr;
    char* cp = char_arr;
    
    /* Different stride values */
    int int_stride = 1;
    int char_stride = sizeof(int);
    
    /* Multiple simultaneous pointer operations */
    for (int i = 0; i < n; i++) {
        /* Different pointer arithmetic patterns */
        int val1 = *ip;
        char val2 = *cp;
        
        /* Post-increment with different strides */
        ip += int_stride;
        cp += char_stride;
        
        /* Combined forms that may decompose to base+0 */
        int* ip2 = ip - 1;
        char* cp2 = cp - sizeof(int);
        
        /* Use all pointers and values */
        asm volatile("" : : "r"(ip), "r"(cp), "r"(ip2), "r"(cp2),
                         "r"(val1), "r"(val2));
        
        /* Opaque function calls */
        use_int(val1);
        use_char(val2);
    }
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0;
    
    do {
        /* Post-decrement pattern */
        sum += *p--;
        
        /* Complex calculations with results */
        a = sum * 3;
        b = a ^ (int)p;
        c = b + *arr;
        d = c - a;
        
        /* Use variables */
        asm volatile("" : : "r"(p), "r"(sum), "r"(a), "r"(b), "r"(c), "r"(d));
    } while (p >= arr);
    
    return sum + a + b + c + d;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Allocate and initialize arrays */
    int* int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_arr = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_arr = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int** matrix = (int**)malloc(MATRIX_ROWS * sizeof(int*));
    
    if (!int_arr || !char_arr || !struct_arr || !matrix) {
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = 'A' + (i % 26);
        struct_arr[i].value = i * 2;
        struct_arr[i].tag = 'a' + (i % 26);
        struct_arr[i].id = i;
        struct_arr[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = (int*)malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Use command-line argument to select tests */
    int test_select = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    /* Execute selected tests to generate different patterns */
    switch (test_select % 7) {
        case 0:
            result = test1_int_array_sum(int_arr, ARRAY_SIZE);
            break;
        case 1:
            test2_string_copy(char_arr, char_arr + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_arr[0];
            break;
        case 2:
            result = test3_struct_array_sum(struct_arr, ARRAY_SIZE);
            break;
        case 3:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            test5_mixed_pointers(int_arr, char_arr, ARRAY_SIZE/4);
            result = int_arr[0] + char_arr[0];
            break;
        case 5:
            result = test6_post_decrement(int_arr, ARRAY_SIZE);
            break;
        case 6:
            /* Run all tests */
            result = test1_int_array_sum(int_arr, ARRAY_SIZE);
            test2_string_copy(char_arr, char_arr + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test3_struct_array_sum(struct_arr, ARRAY_SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            test5_mixed_pointers(int_arr, char_arr, ARRAY_SIZE/4);
            result += test6_post_decrement(int_arr, ARRAY_SIZE);
            break;
    }
    
    /* Update global volatile to prevent optimization */
    global_checksum = result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_arr);
    free(char_arr);
    free(struct_arr);
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum ^= (int)((size_t)p & 0xFFFFFFFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}
