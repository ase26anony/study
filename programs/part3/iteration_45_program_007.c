/* test-auto-inc-dec.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c test-auto-inc-dec.c -o test.o
 * Or: gcc -O3 -funroll-loops -fno-inline -c test-auto-inc-dec.c -o test.o
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
int test_int_array_sum(int* arr, size_t n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use all temporaries to increase register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 += temp1 * 2;
        temp4 += temp2 / 3;
        temp5 = temp3 ^ temp4;
        temp6 = temp5 + temp1;
        temp7 = temp6 - temp2;
        temp8 = temp7 * temp3;
        
        /* Prevent pointer optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all variables to prevent elimination */
    use_int(temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    int count = 0;
    char last = 0;
    int checksum = 0;
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to prevent optimization */
        last = *(s - 1);
        checksum += last;
        count++;
        
        /* Force register usage */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Use results */
    use_int(checksum);
    use_char(last);
    *d = '\0';
}

/* Test 3: Struct array traversal with post-increment */
int test_struct_array(struct Data* arr, size_t n) {
    int total = 0;
    struct Data* p = arr;
    
    /* Many live variables for register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0;
    char tag_acc = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        tag_acc += p->tag;
        
        /* Post-increment the pointer */
        p++;
        
        /* Complex calculations with struct members */
        acc1 += arr[i].payload[0];
        acc2 += arr[i].payload[1];
        acc3 += arr[i].payload[2];
        
        /* Prevent optimization */
        use_ptr(p);
    }
    
    return total + acc1 + acc2 + acc3 + tag_acc;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Outer loop with row pointer */
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* end = row + cols;
        
        /* Inner loop with post-increment pointer */
        while (row < end) {
            /* Multiple accesses to increase chances */
            int val = *row++;
            sum += val;
            
            /* Additional operation to separate increment from use */
            sum += val * 2;
            
            /* Register pressure */
            int temp = val;
            asm volatile("" : "+r"(temp) : : "memory");
            sum += temp;
        }
        
        /* Force row pointer to be recalculated */
        asm volatile("" : : "r"(row) : "memory");
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test_mixed_arithmetic(int* arr, size_t n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Combined increment forms */
    while (p < end) {
        /* This may decompose into base+offset during RTL generation */
        sum += *(p += stride);
        
        /* Additional pointer manipulation */
        p -= stride - 1;
        
        /* Force register usage */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test_post_decrement(int* arr, size_t n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* do-while ensures at least one execution */
    do {
        sum += *p--;
        
        /* Create side effects */
        global_checksum += sum;
        
        /* Prevent tail optimization */
        asm volatile("" : : "r"(p) : "memory");
    } while (p >= arr);
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1000;
    const size_t CHAR_ARRAY_SIZE = 500;
    const size_t STRUCT_ARRAY_SIZE = 100;
    const size_t MATRIX_ROWS = 50;
    const size_t MATRIX_COLS = 20;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_src = malloc(CHAR_ARRAY_SIZE);
    char* char_dst = malloc(CHAR_ARRAY_SIZE);
    struct Data* struct_array = malloc(STRUCT_ARRAY_SIZE * sizeof(struct Data));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-zero values */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 100;
    }
    
    for (size_t i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_src[i] = 'A' + (i % 26);
    }
    
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        struct_array[i].value = i;
        struct_array[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 3; j++) {
            struct_array[i].payload[j] = i * j;
        }
    }
    
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (size_t j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Select tests based on command line to prevent constant folding */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 7;
    }
    
    int result = 0;
    
    /* Execute selected test */
    switch (test_to_run) {
        case 0:
            result = test_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test_string_copy(char_dst, char_src, CHAR_ARRAY_SIZE);
            result = char_dst[0] + char_dst[CHAR_ARRAY_SIZE - 1];
            break;
        case 2:
            result = test_struct_array(struct_array, STRUCT_ARRAY_SIZE);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_arithmetic(int_array, INT_ARRAY_SIZE, 2);
            break;
        case 5:
            result = test_post_decrement(int_array, INT_ARRAY_SIZE / 2);
            break;
        default:
            /* Run all tests */
            result += test_int_array_sum(int_array, INT_ARRAY_SIZE);
            test_string_copy(char_dst, char_src, CHAR_ARRAY_SIZE);
            result += test_struct_array(struct_array, STRUCT_ARRAY_SIZE);
            result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test_mixed_arithmetic(int_array, INT_ARRAY_SIZE, 2);
            result += test_post_decrement(int_array, INT_ARRAY_SIZE / 2);
            break;
    }
    
    /* Update global to prevent elimination */
    global_checksum += result;
    
    /* Cleanup */
    free(int_array);
    free(char_src);
    free(char_dst);
    free(struct_array);
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    printf("Result: %d, Global: %d\n", result, global_checksum);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    /* Access memory to create side effect */
    if (p) {
        global_checksum += 1;
    }
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}
