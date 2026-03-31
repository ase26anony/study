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
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += r2; r3 += r4; r5 += r6; r7 += r8; r9 += r10;
        r2 += r1; r4 += r3; r6 += r5; r8 += r7; r10 += r9;
    }
    
    /* Use all variables to prevent optimization */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5),
                     "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10));
    
    use_ptr(p);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure variables */
    int t1 = 1, t2 = 2, t3 = 3, t4 = 4;
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use variables to prevent spilling of pointers */
        t1 += t2; t3 += t4;
        c1 += c2; c3 += c4;
        t2 += *s; t4 += *d;
        
        i++;
    } while (i < n);
    
    /* Make pointers appear used */
    asm volatile("" : : "r"(d), "r"(s));
    use_ptr(dst);
    use_ptr((void*)src);
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_array_sum(struct Point* points, int n) {
    int sum = 0;
    struct Point* p = points;
    struct Point* end = points + n;
    
    /* High register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10;
    int a11 = 11, a12 = 12, a13 = 13, a14 = 14, a15 = 15;
    
    for (; p < end; p++) {
        /* Access struct member with pointer post-increment in loop update */
        sum += p->x + p->y + p->z;
        
        /* Complex calculations to use many registers */
        a1 = a2 * a3 + a4;
        a5 = a6 * a7 - a8;
        a9 = a10 * a11 / (a12 + 1);
        a13 = a14 * a15 % (a1 + 1);
        a2 = a3 + a4 * a5;
        a6 = a7 - a8 * a9;
    }
    
    /* Force all variables to be live */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                     "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                     "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15));
    
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables */
    int i, j;
    int* row_ptr;
    
    /* Register pressure variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        int* p = row_ptr;
        int* end = row_ptr + cols;
        
        /* Inner loop with post-increment pointer */
        for (j = 0; p < end; j++) {
            /* Combined form that may decompose to base+offset */
            total += *(p++);
            
            /* Index update in loop condition creates separate increment */
            v1 += v2 * v3;
            v4 += v5 / (v6 + 1);
            v7 += v8 % (v1 + 1);
            v2 += matrix[i][j] * v4;
        }
        
        /* Use stride-based pointer arithmetic */
        p = row_ptr;
        for (j = 0; j < cols; j += 2) {
            /* *(p += stride) pattern */
            total += *(p += 2) / 2;
        }
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8));
    
    return total;
}

/* Test 5: Mixed pointer types and operations */
void test5_mixed_operations() {
    char char_arr[100];
    int int_arr[50];
    struct Point struct_arr[25];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        char_arr[i] = (char)(i % 256);
    }
    for (int i = 0; i < 50; i++) {
        int_arr[i] = i * 2;
    }
    for (int i = 0; i < 25; i++) {
        struct_arr[i].x = i;
        struct_arr[i].y = i * 2;
        struct_arr[i].z = i * 3;
    }
    
    /* Char pointer with post-increment */
    char* cp = char_arr;
    int char_sum = 0;
    for (int i = 0; i < 100; i++) {
        char_sum += *cp++;
    }
    
    /* Int pointer with post-decrement */
    int* ip = int_arr + 49;
    int int_sum = 0;
    for (int i = 0; i < 50; i++) {
        int_sum += *ip--;
    }
    
    /* Struct pointer with post-increment */
    struct Point* sp = struct_arr;
    int struct_sum = 0;
    for (int i = 0; i < 25; i++) {
        struct_sum += sp->x;
        sp++;
    }
    
    /* Mix operations to confuse optimizer */
    cp = char_arr;
    ip = int_arr;
    sp = struct_arr;
    
    int mixed = 0;
    for (int i = 0; i < 25; i++) {
        mixed += *cp++ + *ip++ + sp->x;
        sp++;
        
        /* Inline asm to prevent elimination */
        asm volatile("" : : "r"(cp), "r"(ip), "r"(sp));
    }
    
    global_checksum += char_sum + int_sum + struct_sum + mixed;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_arr[100];
    char char_arr[100];
    struct Point struct_arr[50];
    int* matrix[10];
    
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i + 1;
        char_arr[i] = 'A' + (i % 26);
    }
    for (int i = 0; i < 50; i++) {
        struct_arr[i].x = i * 2;
        struct_arr[i].y = i * 3;
        struct_arr[i].z = i * 4;
    }
    for (int i = 0; i < 10; i++) {
        matrix[i] = malloc(10 * sizeof(int));
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command-line arguments to control execution path */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 6;
    }
    
    int result = 0;
    
    switch (test_to_run) {
        case 0:
            result = test1_int_array_sum(int_arr, 100);
            break;
        case 1:
            test2_string_copy(char_arr, "Test string for copy operation", 30);
            result = char_arr[0];
            break;
        case 2:
            result = test3_struct_array_sum(struct_arr, 50);
            break;
        case 3:
            result = test4_nested_loops(matrix, 10, 10);
            break;
        case 4:
            test5_mixed_operations();
            result = global_checksum;
            break;
        default:
            /* Run all tests */
            result = test1_int_array_sum(int_arr, 100);
            test2_string_copy(char_arr, "Test string", 11);
            result += test3_struct_array_sum(struct_arr, 50);
            result += test4_nested_loops(matrix, 10, 10);
            test5_mixed_operations();
            result += global_checksum;
            break;
    }
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(matrix[i]);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((long)p & 0xFFFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}
