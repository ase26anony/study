/* auto-inc-dec-test.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
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
int test1_sum_int_array(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int accum1 = 0, accum2 = 0, accum3 = 0, accum4 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Multiple uses of the loaded value to prevent optimization */
        int val = *p++;  /* This should generate mem_insn with reg+0 offset */
        
        sum += val;
        temp1 = val * 2;
        temp2 = val + temp1;
        temp3 = temp2 - val;
        temp4 = temp3 ^ val;
        
        accum1 += temp1;
        accum2 += temp2;
        accum3 += temp3;
        accum4 += temp4;
        
        /* Use asm to prevent elimination */
        asm volatile("" : : "r"(val), "r"(p));
    }
    
    /* Mix all accumulators to ensure they're used */
    sum += accum1 + accum2 + accum3 + accum4;
    
    /* Opaque function call to create aliasing concerns */
    use_int(sum);
    use_ptr((void*)p);
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_copy_string(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int checksum = 0;
    
    /* Copy loop with classic post-increment pattern */
    while (i < n) {
        c1 = *s++;  /* Should trigger find_inc() */
        *d++ = c1;
        
        /* Additional operations to prevent optimization */
        c2 = c1 + 1;
        c3 = c2 * 2;
        c4 = c3 ^ c1;
        
        checksum += c1 + c2 + c3 + c4;
        i++;
        
        /* Prevent compiler from seeing through pointer aliasing */
        asm volatile("" : : "r"(s), "r"(d));
    }
    
    /* Use results */
    use_char(c1);
    global_checksum += checksum;
}

/* Test 3: Struct array traversal with post-increment */
long test3_struct_array(const struct Data* arr, size_t n) {
    long total = 0;
    const struct Data* p = arr;
    
    /* Many live variables for register pressure */
    long t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    float f1 = 0.0f, f2 = 0.0f;
    int i1 = 0, i2 = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct through post-increment pointer */
        struct Data current = *p++;  /* Should create mem_insn with reg+0 */
        
        /* Use multiple struct members */
        total += current.value;
        t1 += current.timestamp;
        f1 += current.weight;
        i1 += current.tag;
        
        /* Additional computations */
        t2 = current.value * 2;
        t3 = current.timestamp >> 2;
        t4 = t2 + t3;
        f2 = f1 * 2.0f;
        i2 = i1 ^ current.value;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(current.value));
    }
    
    /* Combine all results */
    total += t1 + t2 + t3 + t4 + (long)f1 + (long)f2 + i1 + i2;
    use_long(total);
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* High register pressure setup */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int* row_ptr = matrix;
    
    for (int i = 0; i < rows; i++) {
        int* col_ptr = row_ptr;
        int col_sum = 0;
        
        /* Inner loop with pointer arithmetic */
        for (int j = 0; j < cols; j++) {
            /* Different access patterns that may decompose to reg+0 */
            int val1 = *(col_ptr + j);  /* Base + index */
            int val2 = *col_ptr;        /* Direct */
            col_ptr += 1;               /* Post-increment simulated */
            
            /* Complex usage to prevent optimization */
            r1 = val1 * val2;
            r2 = r1 + j;
            r3 = r2 ^ val1;
            r4 = r3 - val2;
            
            col_sum += val1 + val2 + r1 + r2 + r3 + r4;
            
            /* Force pointer to appear used */
            asm volatile("" : : "r"(col_ptr));
        }
        
        /* Alternative: post-increment in loop update */
        int* alt_ptr = row_ptr;
        for (int j = 0; j < cols; j++) {
            /* This should generate the desired pattern */
            int val = *alt_ptr;
            alt_ptr += 1;  /* Post-increment in separate statement */
            
            col_sum += val * j;
        }
        
        sum += col_sum;
        row_ptr += cols;  /* Move to next row */
        
        /* More register pressure */
        use_int(col_sum);
        asm volatile("" : : "r"(row_ptr));
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and stride access */
int test5_mixed_pointers(char* data, size_t size, int stride) {
    int sum = 0;
    char* p = data;
    int* ip = (int*)data;
    
    /* Multiple pointers with different types */
    char* p_end = data + size;
    int* ip_end = (int*)(data + size);
    
    /* First loop: char pointer with post-increment */
    while (p < p_end - 1) {
        char c1 = *p++;
        char c2 = *p++;
        
        sum += c1 * c2;
        
        /* Create address-taking pattern */
        char* temp = p;
        use_ptr(temp);
    }
    
    /* Second loop: int pointer with stride (may decompose to reg+0) */
    for (int* ptr = ip; ptr < ip_end - stride; ptr += stride) {
        /* Combined form that might generate plus(reg, const_int 0) */
        int val = *(ptr + 0);  /* Explicit zero offset */
        sum += val;
        
        /* Additional access with non-zero offset for contrast */
        if (ptr + 1 < ip_end) {
            sum += *(ptr + 1);
        }
        
        asm volatile("" : : "r"(ptr));
    }
    
    return sum;
}

/* Test 6: do-while loop with post-decrement */
int test6_post_decrement(int* arr, size_t n) {
    int sum = 0;
    int* p = arr + n - 1;  /* Start from end */
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0;
    
    if (n > 0) {
        do {
            int val = *p--;  /* Post-decrement */
            
            /* Complex usage */
            a = val + sum;
            b = a * 2;
            c = b - val;
            d = c ^ a;
            
            sum += val + a + b + c + d;
            
            /* Force p to stay in register */
            asm volatile("" : : "r"(p));
        } while (p >= arr);
    }
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1024;
    const size_t CHAR_ARRAY_SIZE = 512;
    const size_t STRUCT_ARRAY_SIZE = 256;
    const size_t MATRIX_ROWS = 32;
    const size_t MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(CHAR_ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(STRUCT_ARRAY_SIZE * sizeof(struct Data));
    int* matrix = malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 13) % 100;
    }
    
    for (size_t i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array[i] = 'A' + (i % 26);
    }
    
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        struct_array[i].value = i;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].timestamp = i * 1000;
        struct_array[i].weight = i * 0.5f;
    }
    
    for (size_t i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 7) % 50;
    }
    
    /* Select test based on command line argument */
    int test_num = 1;
    if (argc > 1) {
        test_num = atoi(argv[1]);
    }
    
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_sum_int_array(int_array, INT_ARRAY_SIZE);
            break;
        case 2:
            test2_copy_string(char_array, char_array + 128, CHAR_ARRAY_SIZE - 128);
            result = global_checksum;
            break;
        case 3:
            result = test3_struct_array(struct_array, STRUCT_ARRAY_SIZE);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_pointers(char_array, CHAR_ARRAY_SIZE, 4);
            break;
        case 6:
            result = test6_post_decrement(int_array, INT_ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test1_sum_int_array(int_array, INT_ARRAY_SIZE);
            test2_copy_string(char_array, char_array + 128, CHAR_ARRAY_SIZE - 128);
            result += test3_struct_array(struct_array, STRUCT_ARRAY_SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_pointers(char_array, CHAR_ARRAY_SIZE, 4);
            result += test6_post_decrement(int_array, INT_ARRAY_SIZE);
            break;
    }
    
    /* Cleanup */
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
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
