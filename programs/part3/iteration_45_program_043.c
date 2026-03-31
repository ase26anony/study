/* test-auto-inc-dec.c */
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

/* Struct to create complex memory accesses */
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
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many registers to increase pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3;
        r5 += r4; r6 += r5; r7 += r6; r8 += r7;
        r9 += r8; r10 += r9;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum));
    }
    
    /* Consume all the register pressure variables */
    use_int(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10);
    
    return sum;
}

/* Test 2: String copy with post-increment */
void test2_str_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    volatile int v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use volatile variables to prevent optimization */
        v1 = v2 + v3; v4 = v5 + v6; v7 = v8 + i;
        
        /* Opaque use of pointers */
        use_ptr(d);
        use_ptr((void*)s);
        
        i++;
    } while (i < n);
    
    /* Force pointer usage */
    asm volatile("" : : "r"(d), "r"(s));
}

/* Test 3: Struct array traversal with post-increment */
long test3_struct_array(struct Data* arr, int n) {
    struct Data* p = arr;
    long total = 0;
    int i = 0;
    
    /* Heavy register pressure */
    int t1 = 100, t2 = 200, t3 = 300, t4 = 400;
    int t5 = 500, t6 = 600, t7 = 700, t8 = 800;
    short s1 = 1, s2 = 2, s3 = 3, s4 = 4;
    
    for (i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value + p->count;
        p++;  /* Post-increment after access */
        
        /* Complex calculations with many variables */
        t1 += p->value; t2 += t1; t3 += t2; t4 += t3;
        t5 += p->count; t6 += t5; t7 += t6; t8 += t7;
        s1 += p->tag; s2 += s1; s3 += s2; s4 += s3;
        
        /* Prevent elimination */
        asm volatile("" : : "r"(p), "r"(total));
    }
    
    /* Use all pressure variables */
    use_int(t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8);
    use_int(s1 + s2 + s3 + s4);
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    int i, j;
    
    /* Register pressure variables */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with pointer post-increment */
        while (p < end) {
            /* Post-increment access in nested loop */
            sum += *p++;
            
            /* Update pressure variables */
            a1 += sum; a2 += a1; a3 += a2; a4 += a3;
            a5 += a4; a6 += a5; a7 += a6; a8 += a7;
            a9 += a8; a10 += a9;
            
            /* Opaque use */
            use_ptr(p);
        }
        
        /* Additional computation between loops */
        a1 ^= i; a2 ^= a1; a3 ^= a2;
    }
    
    /* Consume pressure variables */
    use_int(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10);
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test5_mixed_arithmetic(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* limit = arr + n * stride;
    
    /* Extreme register pressure */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    int r4 = 4, r5 = 5, r6 = 6, r7 = 7;
    int r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    while (p < limit) {
        /* Combined pointer arithmetic that may decompose to base+0 */
        sum += *(p += stride) - stride;
        
        /* Use all register variables */
        r0 += sum; r1 += r0; r2 += r1; r3 += r2;
        r4 += r3; r5 += r4; r6 += r5; r7 += r6;
        r8 += r7; r9 += r8; r10 += r9; r11 += r10;
        r12 += r11; r13 += r12; r14 += r13; r15 += r14;
        
        /* Memory barrier to prevent reordering */
        barrier();
        
        /* Force pointer to be in register */
        asm volatile("" : "+r"(p) : : "memory");
    }
    
    /* Use all pressure variables */
    use_int(r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7);
    use_int(r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15);
    
    return sum;
}

/* Test 6: Post-decrement pattern */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int d1 = 1, d2 = 2, d3 = 3, d4 = 4;
    int d5 = 5, d6 = 6, d7 = 7, d8 = 8;
    
    while (p >= arr) {
        /* Post-decrement access */
        sum += *p--;
        
        /* Use pressure variables */
        d1 += sum; d2 += d1; d3 += d2; d4 += d3;
        d5 += d4; d6 += d5; d7 += d6; d8 += d7;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p));
    }
    
    use_int(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int SIZE = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Integer array */
    int int_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
    }
    
    /* Character array (string) */
    char src_str[SIZE];
    char dst_str[SIZE];
    for (int i = 0; i < SIZE - 1; i++) {
        src_str[i] = 'A' + (i % 26);
    }
    src_str[SIZE - 1] = '\0';
    
    /* Struct array */
    struct Data struct_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].count = i % 100;
        struct_array[i].timestamp = i * 1000L;
    }
    
    /* Matrix */
    int* matrix[MATRIX_ROWS];
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_sum_int_array(int_array, SIZE);
            break;
        case 2:
            test2_str_copy(dst_str, src_str, SIZE - 1);
            result = (int)dst_str[SIZE / 2];
            break;
        case 3:
            result = (int)test3_struct_array(struct_array, SIZE);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_arithmetic(int_array, SIZE / 4, 4);
            break;
        case 6:
            result = test6_post_decrement(int_array, SIZE);
            break;
        default:
            /* Run all tests */
            result = test1_sum_int_array(int_array, SIZE);
            test2_str_copy(dst_str, src_str, SIZE - 1);
            result += (int)test3_struct_array(struct_array, SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_arithmetic(int_array, SIZE / 4, 4);
            result += test6_post_decrement(int_array, SIZE);
            break;
    }
    
    /* Update global volatile to prevent elimination */
    global_sum += result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    
    /* Print result to ensure code runs */
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_sum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    static volatile void* sink;
    sink = p;
}

void __attribute__((noinline)) use_char(char c) {
    global_sum += c;
}

void __attribute__((noinline)) barrier(void) {
    asm volatile("" : : : "memory");
}
