/* auto-inc-dec-test.c
 * Program designed to trigger auto-increment/decrement optimization patterns
 * Specifically targeting find_inc() logic in auto-inc-dec.cc lines 1352-1358
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
    int id;
    float weight;
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access - should create (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use many variables to increase register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 = temp1 * temp2;
        temp4 = temp3 ^ temp1;
        temp5 = temp4 + temp2;
        temp6 = temp5 - temp3;
        temp7 = temp6 * 7;
        temp8 = temp7 / 3;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all temps to prevent elimination */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    
    /* Opaque function call to create aliasing concerns */
    use_ptr(arr);
    
    return sum;
}

/* Test 2: String copy with post-increment on both pointers */
int test2_copy_string(char* dst, const char* src, int max_len) {
    int len = 0;
    char* d = dst;
    const char* s = src;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int i1 = 0, i2 = 0, i3 = 0, i4 = 0;
    
    do {
        /* Classic *dst++ = *src++ pattern */
        *d++ = *s++;
        len++;
        
        /* Interleave with other operations */
        c1 = *(s - 1);
        c2 = c1 ^ 0x55;
        c3 = c2 + 1;
        c4 = c3 * 2;
        
        i1 += c1;
        i2 += c2;
        i3 += c3;
        i4 += c4;
        
        /* Prevent optimization */
        if (len % 8 == 0) {
            asm volatile("" : : "r"(d), "r"(s) : "memory");
        }
    } while (*(s - 1) != '\0' && len < max_len - 1);
    
    *d = '\0';
    
    /* Use all computed values */
    return len + i1 + i2 + i3 + i4;
}

/* Test 3: Struct array traversal with post-increment */
int test3_sum_struct_array(struct Data* data, int count) {
    int total = 0;
    struct Data* p = data;
    struct Data* end = data + count;
    
    /* High register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    float f1 = 0.0f, f2 = 0.0f;
    
    for (; p < end; p++) {
        /* Access struct member via post-incremented pointer */
        total += p->value;
        
        /* Additional struct member accesses */
        v1 += p->id;
        v2 += p->tag;
        f1 += p->weight;
        
        /* Complex calculations to use registers */
        v3 = (v1 * v2) ^ p->id;
        v4 = v3 + p->value;
        f2 = f1 * 2.0f - p->weight;
        
        /* Opaque use to prevent elimination */
        use_ptr(p);
    }
    
    /* Mix integer and float results */
    return total + v1 + v2 + v3 + v4 + (int)f1 + (int)f2;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k = 0, l = 0;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(row_ptr + j);
            
            /* Alternative: *(row_ptr += 1) pattern */
            k += *row_ptr;
            row_ptr += 1;  /* This creates separate increment instruction */
            
            /* More register pressure */
            l = (l << 1) ^ k;
            
            /* Prevent optimization */
            if ((i * cols + j) % 16 == 0) {
                asm volatile("" : : "r"(row_ptr) : "memory");
            }
        }
        
        /* Reset pointer with offset calculation */
        row_ptr = matrix[i] + cols;
        use_ptr(row_ptr);
    }
    
    return sum + k + l;
}

/* Test 5: Mixed pointer types and arithmetic */
int test5_mixed_pointers(char* char_arr, int* int_arr, int n) {
    int sum = 0;
    char* cp = char_arr;
    int* ip = int_arr;
    
    /* Multiple simultaneous pointer traversals */
    for (int i = 0; i < n; i++) {
        /* Different pointer types with post-increment */
        sum += (int)(*cp++);
        sum += *ip++;
        
        /* Additional pointer arithmetic */
        char* cp2 = cp - 1;
        int* ip2 = ip - 1;
        
        /* Use them to prevent optimization */
        sum += (int)(*cp2);
        sum += *ip2;
        
        /* Complex addressing modes */
        sum += *(cp + (i & 3));
        sum += *(ip + (i % 2));
        
        /* Register pressure */
        int t1 = *cp, t2 = *ip;
        sum += t1 * t2;
        
        /* Opaque barrier */
        barrier();
    }
    
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    int count = n;
    
    /* Register pressure variables */
    int a = 0, b = 0, c = 0, d = 0;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        count--;
        
        /* Complex calculations using multiple registers */
        a = sum ^ count;
        b = a * 3;
        c = b + *p;
        d = c - a;
        
        /* Use all variables */
        sum += d;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p) : "memory");
    } while (count > 0);
    
    return sum + a + b + c + d;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 10;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int (*matrix)[MATRIX_COLS] = (int(*)[MATRIX_COLS])malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 100;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].value = i;
        struct_array[i].id = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    int result = 0;
    
    /* Use command-line arguments to select tests */
    /* This prevents the compiler from optimizing away entire tests */
    int test_mask = (argc > 1) ? atoi(argv[1]) : 0x3F; /* Run all tests by default */
    
    if (test_mask & 0x01) result += test1_sum_int_array(int_array, ARRAY_SIZE);
    if (test_mask & 0x02) result += test2_copy_string(char_array, char_array + 100, 100);
    if (test_mask & 0x04) result += test3_sum_struct_array(struct_array, ARRAY_SIZE / 4);
    if (test_mask & 0x08) result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    if (test_mask & 0x10) result += test5_mixed_pointers(char_array, int_array, ARRAY_SIZE / 8);
    if (test_mask & 0x20) result += test6_post_decrement(int_array, ARRAY_SIZE / 2);
    
    /* Update global volatile to prevent elimination */
    global_sum = result;
    
    /* Print result to ensure code runs */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    /* Access memory to create aliasing */
    asm volatile("" : : "r"(p) : "memory");
}

void use_char(char c) {
    global_sum += (int)c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}
