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
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
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
    
    /* Mix results to prevent elimination */
    global_sink += sum + r1 + r10;
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use variables to prevent optimization */
        c1 = *s; c2 = c1 + 1; c3 = c2 + 1; c4 = c3 + 1;
        cnt1++; cnt2 += cnt1; cnt3 += cnt2; cnt4 += cnt3;
        
        /* Opaque use of pointers */
        use_ptr(d);
        use_ptr(s);
        
        i++;
    } while (i < n);
    
    /* Ensure pointers appear used */
    asm volatile("" : : "r"(d), "r"(s));
    global_sink += cnt4;
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test_struct_array(const struct Point* points, size_t n) {
    int total = 0;
    const struct Point* p = points;
    
    /* Heavy register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        total += p->x + p->y;
        
        /* Post-increment after access */
        p++;
        
        /* Complex calculations with many live variables */
        acc1 = total * 2;
        acc2 = acc1 + p->x;
        acc3 = acc2 * 3;
        acc4 = acc3 / 2;
        acc5 = acc4 ^ total;
        acc6 = acc5 & 0xFF;
        acc7 = acc6 | 0x80;
        acc8 = acc7 << 2;
        acc9 = acc8 >> 1;
        acc10 = acc9 - total;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total));
    }
    
    global_sink += acc10;
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int matrix[][10], size_t rows) {
    int sum = 0;
    
    /* Many local variables for register pressure */
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    int t6 = 0, t7 = 0, t8 = 0, t9 = 0, t10 = 0;
    
    for (size_t i = 0; i < rows; i++) {
        int* row = matrix[i];
        int j = 0;
        
        /* Inner loop with post-increment indexing */
        while (j < 10) {
            /* Access with post-increment on index */
            sum += row[j++];
            
            /* Additional operations to prevent optimization */
            t1 = sum * i;
            t2 = t1 + j;
            t3 = t2 * matrix[i][0];
            t4 = t3 ^ sum;
            t5 = t4 & 0xFF;
            t6 = t5 | t1;
            t7 = t6 << (j & 3);
            t8 = t7 >> 1;
            t9 = t8 - t1;
            t10 = t9 + t2;
            
            /* Force pointer to stay in register */
            asm volatile("" : : "r"(row), "r"(j));
        }
        
        /* Use opaque function */
        use_ptr(matrix[i]);
    }
    
    global_sink += t10;
    return sum;
}

/* Test 5: Mixed pointer arithmetic patterns */
int test_mixed_patterns(char* data, size_t size) {
    int result = 0;
    char* p = data;
    char* end = data + size;
    
    /* Multiple pointer variables */
    char* q = p + 1;
    char* r = p + 2;
    
    /* Register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    while (p < end) {
        /* Different post-increment patterns */
        result += *p++;          /* Simple post-inc */
        result -= *(q += 2);     /* Compound assignment */
        result ^= *r++;          /* Another post-inc */
        
        /* Complex expression that may decompose to base+0 */
        if (p < end - 1) {
            result += *(p + 0);  /* Explicit plus 0 */
        }
        
        /* Use all variables */
        v1 += result; v2 += v1; v3 += v2; v4 += v3;
        v5 += v4; v6 += v5; v7 += v6; v8 += v7;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(q), "r"(r));
        barrier();
    }
    
    global_sink += v8;
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t N = 1000;
    
    /* Integer array */
    int* int_arr = malloc(N * sizeof(int));
    for (size_t i = 0; i < N; i++) {
        int_arr[i] = (int)(i * 3 + 1);
    }
    
    /* Character array */
    char* char_arr = malloc(N * sizeof(char));
    for (size_t i = 0; i < N; i++) {
        char_arr[i] = (char)(i % 256);
    }
    
    /* Struct array */
    struct Point* points = malloc(N * sizeof(struct Point));
    for (size_t i = 0; i < N; i++) {
        points[i].x = (int)i;
        points[i].y = (int)(i * 2);
        points[i].z = (int)(i * 3);
    }
    
    /* 2D matrix */
    int (*matrix)[10] = malloc(N * 10 * sizeof(int));
    for (size_t i = 0; i < N; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (int)(i * 10 + j);
        }
    }
    
    int result = 0;
    
    /* Use command-line arguments to control execution path */
    if (argc > 1) {
        int test_num = atoi(argv[1]) % 5;
        
        switch (test_num) {
            case 0:
                result = test_int_array_sum(int_arr, N);
                break;
            case 1:
                test_string_copy(char_arr, char_arr + N/2, N/2);
                result = (int)char_arr[0];
                break;
            case 2:
                result = test_struct_array(points, N);
                break;
            case 3:
                result = test_nested_loops(matrix, N/10);
                break;
            case 4:
                result = test_mixed_patterns(char_arr, N);
                break;
        }
    } else {
        /* Run all tests */
        result += test_int_array_sum(int_arr, N);
        test_string_copy(char_arr, char_arr + N/2, N/2);
        result += test_struct_array(points, N);
        result += test_nested_loops(matrix, N/10);
        result += test_mixed_patterns(char_arr, N);
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    printf("Global sink: %d\n", global_sink);
    
    /* Cleanup */
    free(int_arr);
    free(char_arr);
    free(points);
    free(matrix);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_sink += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_sink += (int)((size_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_sink += c;
}

void __attribute__((noinline)) barrier(void) {
    asm volatile("" : : : "memory");
}
