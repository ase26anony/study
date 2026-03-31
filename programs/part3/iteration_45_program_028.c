/* auto-inc-dec-test.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Specifically targets lines 1352-1358 of auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern int get_value(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int a = get_value();
    int b = get_value();
    int c = get_value();
    int d = get_value();
    int e = get_value();
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use all live variables to maintain register pressure */
        a ^= sum;
        b += a;
        c -= b;
        d *= c + 1;
        e = d ^ e;
        
        /* Prevent pointer optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use variables to prevent elimination */
    global_sum += a + b + c + d + e;
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char* dst, const char* src, int n) {
    /* Multiple live pointers for register pressure */
    char* d = dst;
    const char* s = src;
    char* d2 = dst + n/2;
    const char* s2 = src + n/2;
    
    int i = 0;
    while (i < n) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        i++;
        
        /* Alternate pointer usage */
        if (i % 3 == 0) {
            *d2++ = *s2++;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d), "r"(s), "r"(d2), "r"(s2) : "memory");
    }
    
    /* Force pointer usage */
    use_ptr(dst);
    use_ptr(src);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(struct Data* arr, int n) {
    int total = 0;
    struct Data* p = arr;
    
    /* Create register pressure with struct members */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0, temp5 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct through post-increment pointer */
        total += p->value;
        temp1 += p->tag;
        temp2 ^= p->payload[0];
        temp3 |= p->payload[1];
        
        /* Post-increment */
        p++;
        
        /* Complex operations to maintain live variables */
        temp4 = (temp4 + temp1) * 3;
        temp5 = temp5 ^ temp2 ^ temp3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    global_sum += temp1 + temp2 + temp3 + temp4 + temp5;
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Many live variables for register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with post-increment pointer */
        while (p < end) {
            /* Target pattern: *(p + 0) with post-increment */
            int val = *p;
            p++;  /* Post-increment separated */
            
            sum += val;
            
            /* Update live variables */
            r1 += val;
            r2 ^= r1;
            r3 = r3 * 2 + val;
            r4 = (r4 << 1) | (val & 1);
            r5 = r5 - val + i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(p) : "memory");
        }
        
        /* Additional pointer arithmetic that might decompose to base+0 */
        int* q = matrix[i];
        sum += *(q + 0);  /* Could become (plus (reg) (const_int 0)) */
        
        /* Use asm to force register usage */
        asm volatile("" : : "r"(q) : "memory");
    }
    
    global_sum += r1 + r2 + r3 + r4 + r5;
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
void test5_mixed_pointers(char* data, int size) {
    char* p = data;
    int* ip = (int*)data;
    struct Data* sp = (struct Data*)data;
    
    int char_sum = 0;
    int int_sum = 0;
    int struct_sum = 0;
    
    /* Loop with multiple pointer types */
    for (int i = 0; i < size; i += 8) {
        /* Post-increment on char pointer */
        char c = *p++;
        char_sum += c;
        
        /* Post-increment on int pointer */
        if (i + 4 < size) {
            int val = *ip++;
            int_sum += val;
        }
        
        /* Complex pointer expression that might simplify to base+0 */
        char* p2 = p + 0;  /* Explicit (plus (reg) (const_int 0)) */
        char_sum += *p2;
        
        /* Use all pointers to keep them live */
        asm volatile("" : : "r"(p), "r"(ip), "r"(sp), "r"(p2) : "memory");
    }
    
    global_sum += char_sum + int_sum + struct_sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Live variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Update live variables */
        a = (a * b) ^ sum;
        b = (b + c) | sum;
        c = (c - d) & sum;
        d = (d ^ e) + sum;
        e = (e * a) - sum;
        
        /* Memory barrier */
        asm volatile("" : : "r"(p) : "memory");
    } while (p >= arr);
    
    global_sum += a + b + c + d + e;
    return sum;
}

/* Opaque function implementations */
void use_int(int x) {
    global_sum ^= x;
}

void use_ptr(void* p) {
    /* Access through volatile to prevent optimization */
    volatile int* vp = (volatile int*)&p;
    global_sum += *vp;
}

void use_char(char c) {
    global_sum += c;
}

int get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int CHAR_ARRAY_SIZE = 2000;
    const int STRUCT_COUNT = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 20;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(CHAR_ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(STRUCT_COUNT * sizeof(struct Data));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array[i] = (i % 26) + 'a';
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].payload[0] = i;
        struct_array[i].payload[1] = i * i;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Select tests based on command line arguments */
    int test_mask = 0x3F; /* Run all tests by default */
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    int total_result = 0;
    
    if (test_mask & 0x01) {
        total_result ^= test1_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        test2_string_copy(char_array, char_array + 500, 500);
    }
    
    if (test_mask & 0x04) {
        total_result ^= test3_struct_array(struct_array, STRUCT_COUNT);
    }
    
    if (test_mask & 0x08) {
        total_result ^= test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (test_mask & 0x10) {
        test5_mixed_pointers(char_array, CHAR_ARRAY_SIZE);
    }
    
    if (test_mask & 0x20) {
        total_result ^= test6_post_decrement(int_array, INT_ARRAY_SIZE);
    }
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    /* Use result to prevent optimization */
    printf("Result: %d (global_sum: %d)\n", total_result, global_sum);
    
    return total_result != 0 ? 0 : 1;
}
