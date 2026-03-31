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
        
        /* Use many variables to increase register pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent optimization with asm */
        asm volatile("" : : "r"(p), "r"(sum));
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    use_ptr(p);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_str_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd', c5 = 'e';
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        i++;
        
        /* Mix operations */
        c1 = *s; c2 = c1 + 1; c3 = c2 + 1; c4 = c3 + 1; c5 = c4 + 1;
        use_char(c5);
        
        /* Opaque function call to prevent optimization */
        if (i % 8 == 0) use_ptr(d);
        
    } while (i < n && *(s-1) != '\0');
    
    /* Force pointer usage */
    asm volatile("" : : "r"(d), "r"(s));
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_array(struct Point* points, int n) {
    struct Point* p = points;
    int total = 0;
    
    /* High register pressure */
    int a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0;
    int b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Complex calculations with pressure variables */
        a1 = total * 2; a2 = a1 + i; a3 = a2 * 3; a4 = a3 / 2; a5 = a4 - i;
        b1 = p->x; b2 = b1 * 2; b3 = b2 + a5; b4 = b3 / 3; b5 = b4 - a1;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total), "r"(a1), "r"(b1));
        
        /* Opaque function call */
        if (i % 4 == 0) use_int(total);
    }
    
    return total + a1 + a2 + a3 + a4 + a5 + b1 + b2 + b3 + b4 + b5;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        
        /* Inner loop with pointer post-increment */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(p++);
            
            /* Additional index calculations */
            k = i * cols + j;
            l = sum * k;
            m = l % 256;
            
            /* Use variables to keep them live */
            asm volatile("" : : "r"(k), "r"(l), "r"(m));
        }
        
        /* Force row pointer usage */
        use_ptr(row);
    }
    
    return sum + i + j + k + l + m;
}

/* Test 5: Mixed pointer types and arithmetic */
int test5_mixed_pointers(char* data, int size) {
    int sum = 0;
    char* cp = data;
    int* ip = (int*)data;
    
    /* Different stride patterns */
    for (int i = 0; i < size / 4; i++) {
        /* Post-increment with different pointer types */
        sum += *ip++;
        
        /* Char pointer with stride */
        char c = *cp;
        cp += 4;  /* Equivalent to *(cp += 4) pattern */
        
        /* Register pressure */
        int t1 = c * 2, t2 = t1 + i, t3 = t2 * 3;
        sum += t3;
        
        /* Opaque calls */
        if (i % 16 == 0) {
            use_int(*ip);
            use_char(*cp);
        }
    }
    
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    int count = n;
    
    /* Register pressure variables */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    
    do {
        /* Post-decrement pattern */
        sum += *p--;
        count--;
        
        /* Complex dependency chain */
        v1 = sum * 2;
        v2 = v1 + count;
        v3 = v2 * 3;
        v4 = v3 / 2;
        v5 = v4 - count;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(v1), "r"(v5));
        
    } while (count > 0);
    
    return sum + v1 + v2 + v3 + v4 + v5;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 16;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Point* struct_array = (struct Point*)malloc(ARRAY_SIZE * sizeof(struct Point));
    int** matrix = (int**)malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-constant data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = (int*)malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_sum_int_array(int_array, ARRAY_SIZE);
            break;
        case 2:
            test2_str_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_array[0] + char_array[ARRAY_SIZE-1];
            break;
        case 3:
            result = test3_struct_array(struct_array, ARRAY_SIZE/4);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_pointers(char_array, ARRAY_SIZE);
            break;
        case 6:
            result = test6_post_decrement(int_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result += test1_sum_int_array(int_array, ARRAY_SIZE);
            test2_str_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test3_struct_array(struct_array, ARRAY_SIZE/4);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_pointers(char_array, ARRAY_SIZE);
            result += test6_post_decrement(int_array, ARRAY_SIZE);
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
    global_checksum += (long)p & 0xFF;
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}
