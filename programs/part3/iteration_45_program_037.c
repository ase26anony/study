/* test-auto-inc-dec.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358 in auto-inc-dec.cc: find_inc(true) path for mem_insn with reg+0 addressing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    short count;
    long timestamp;
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Use asm to prevent pointer elimination */
    asm volatile("" : : "r"(p), "r"(end) : "memory");
    
    while (p < end) {
        /* Post-increment access: *p++ creates reg+0 addressing */
        sum += *p++;
        
        /* Use the temporary variables to increase register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 * 3;
        temp4 = temp3 - sum;
        
        acc1 += temp1;
        acc2 += temp2;
        acc3 += temp3;
        acc4 += temp4;
        
        /* Barrier to prevent reordering */
        barrier();
    }
    
    /* Mix all accumulators to ensure they're used */
    sum += acc1 + acc2 + acc3 + acc4;
    
    /* Use opaque function to prevent optimization */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_copy_string(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Create register pressure */
    char c1, c2, c3, c4;
    int checksum = 0;
    
    /* Use asm to mark pointers as used */
    asm volatile("" : : "r"(d), "r"(s) : "memory");
    
    /* Copy with post-increment: *d++ = *s++ */
    while (i++ < n && *s) {
        *d++ = *s++;
        
        /* Access the copied values in different ways */
        c1 = *(d-1);
        c2 = *(s-1);
        c3 = c1 ^ c2;
        c4 = c1 + c2;
        
        checksum += c1 + c2 + c3 + c4;
        
        /* More register pressure */
        for (int j = 0; j < 4; j++) {
            checksum += (c1 << j) + (c2 >> j);
        }
    }
    
    *d = '\0';
    
    /* Use results to prevent elimination */
    global_sum += checksum;
    use_char(*(dst + n/2));
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(struct Data* data, int count) {
    struct Data* p = data;
    int total = 0;
    
    /* Many local variables for register pressure */
    long t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    short s1 = 0, s2 = 0;
    char ch1 = 0, ch2 = 0;
    
    /* Mark pointer as used */
    asm volatile("" : : "r"(p) : "memory");
    
    for (int i = 0; i < count; i++) {
        /* Access struct members via post-increment pointer */
        total += p->value;
        ch1 += p->tag;
        s1 += p->count;
        t1 += p->timestamp;
        
        /* Post-increment the pointer */
        p++;
        
        /* Complex calculations with all variables */
        t2 = t1 * 2 + total;
        t3 = t2 - p[-1].timestamp;
        t4 = t3 ^ t1;
        
        s2 = s1 + ch1;
        ch2 = ch1 ^ s2;
        
        /* Use opaque function periodically */
        if (i % 8 == 0) {
            use_ptr((void*)&p[-1]);
        }
    }
    
    /* Combine all results */
    total += (int)(t1 + t2 + t3 + t4) + s1 + s2 + ch1 + ch2;
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][16], int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Outer loop with row pointer */
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* end_row = row + cols;
        
        /* Inner loop with post-increment pointer */
        j = 0;
        while (row < end_row) {
            /* Post-increment access in loop body */
            sum += *row++;
            
            /* Additional calculations for register pressure */
            tmp1 = sum * i;
            tmp2 = tmp1 + j;
            tmp3 = tmp2 * matrix[i][j];
            tmp4 = tmp3 - sum;
            
            /* Use all temporaries */
            for (k = 0; k < 2; k++) {
                for (l = 0; l < 2; l++) {
                    sum += (tmp1 >> k) + (tmp2 << l) - tmp3 + tmp4;
                }
            }
            
            j++;
            
            /* Barrier to prevent optimization across iterations */
            barrier();
        }
        
        /* Opaque function call */
        use_int(sum);
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
void test5_mixed_pointers(void* base, int n) {
    char* cp = (char*)base;
    int* ip = (int*)base;
    short* sp = (short*)base;
    
    int char_sum = 0, int_sum = 0, short_sum = 0;
    
    /* Create aliasing by using different pointer types on same memory */
    for (int i = 0; i < n; i++) {
        /* Post-increment with different strides */
        char_sum += *cp++;
        
        if (i % 2 == 0) {
            int_sum += *ip++;
        } else {
            short_sum += *sp++;
        }
        
        /* Complex address calculation that may become reg+0 */
        *(cp + 0) = (char)(int_sum & 0xFF);
        *(ip - 1) = short_sum;
        
        /* Use asm to prevent reordering */
        asm volatile("" : : "r"(cp), "r"(ip), "r"(sp) : "memory");
    }
    
    global_sum += char_sum + int_sum + short_sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int* p = arr + n - 1;
    int sum = 0;
    int count = n;
    
    /* Register pressure variables */
    int a = 0, b = 0, c = 0, d = 0;
    
    if (count <= 0) return 0;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Use multiple variables */
        a = sum * 3;
        b = a + count;
        c = b ^ sum;
        d = c - a;
        
        /* Combine them */
        sum += (a + b + c + d) >> 2;
        
        count--;
        
        /* Barrier */
        asm volatile("" : : "r"(p) : "memory");
    } while (count > 0);
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 256;
    const int MATRIX_ROWS = 8;
    const int MATRIX_COLS = 16;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int (*matrix)[MATRIX_COLS] = (int(*)[MATRIX_COLS])malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 13 + 7) & 0xFF;
        char_array[i] = (char)((i * 17 + 11) & 0x7F);
        struct_array[i].value = i * 3;
        struct_array[i].tag = (char)(i & 0xFF);
        struct_array[i].count = (short)(i % 100);
        struct_array[i].timestamp = i * 1000L;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = (i * MATRIX_COLS + j) * 7;
        }
    }
    
    /* Use command-line argument to select test, preventing constant folding */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 7;
    }
    
    int result = 0;
    
    /* Run selected test */
    switch (test_to_run) {
        case 0:
            result = test1_sum_int_array(int_array, ARRAY_SIZE);
            break;
        case 1:
            test2_copy_string(char_array, "Test string for copy operation", ARRAY_SIZE);
            result = char_array[5];
            break;
        case 2:
            result = test3_struct_array(struct_array, ARRAY_SIZE / 4);
            break;
        case 3:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            test5_mixed_pointers(int_array, ARRAY_SIZE / 8);
            result = global_sum;
            break;
        case 5:
            result = test6_post_decrement(int_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test1_sum_int_array(int_array, ARRAY_SIZE);
            test2_copy_string(char_array, "Default string", ARRAY_SIZE);
            result += test3_struct_array(struct_array, ARRAY_SIZE / 4);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            test5_mixed_pointers(int_array, ARRAY_SIZE / 8);
            result += test6_post_decrement(int_array, ARRAY_SIZE);
            break;
    }
    
    /* Clean up */
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    /* Return result to prevent optimization */
    return result & 0xFF;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_sum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    static volatile void* last_ptr = NULL;
    last_ptr = p;
}

void __attribute__((noinline)) use_char(char c) {
    global_sum += c;
}

void __attribute__((noinline)) barrier(void) {
    asm volatile("" : : : "memory");
}
