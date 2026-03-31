/* auto-inc-dec-test.c - Test program to trigger auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent optimization */
volatile int global_checksum = 0;

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

void use_int(int x) {
    global_checksum ^= x;
}

void use_ptr(void* p) {
    global_checksum ^= (int)(long)p;
}

void use_char(char c) {
    global_checksum ^= c;
}

/* Test 1: Sum integer array with post-increment pointer in loop */
int test1_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        a ^= sum; b += a; c ^= b; d += c;
        e ^= d; f += e; g ^= f; h += g;
        
        /* Prevent elimination of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all pressure variables */
    use_int(a + b + c + d + e + f + g + h);
    return sum;
}

/* Test 2: String copy with post-increment */
void test2_string_copy(char* dst, const char* src, int n) {
    int i;
    
    /* Create register pressure */
    char x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 10, y2 = 20, y3 = 30, y4 = 40;
    
    for (i = 0; i < n; i++) {
        /* Classic post-increment copy pattern */
        *dst++ = *src++;
        
        /* Use pressure variables */
        x1 ^= *dst; x2 += *src;
        y1 ^= x1; y2 += x2; y3 ^= y1; y4 += y2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(dst), "r"(src) : "memory");
    }
    
    /* Force use of variables */
    use_char(x1 + x2 + x3 + x4);
    use_int(y1 + y2 + y3 + y4);
}

/* Test 3: Struct traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_traversal(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    int i;
    
    /* High register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8;
    int s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    
    for (i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Complex calculations with pressure variables */
        r1 ^= p->x; r2 += p->y;
        r3 ^= r1; r4 += r2;
        r5 ^= r3; r6 += r4;
        r7 ^= r5; r8 += r6;
        s1 ^= r7; s2 += r8;
        s3 ^= s1; s4 += s2;
        
        /* Make pointer appear used */
        use_ptr(p);
    }
    
    /* Use all variables */
    use_int(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8);
    use_int(s1 + s2 + s3 + s4);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    int i, j;
    
    /* Extreme register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int w1 = 10, w2 = 20, w3 = 30, w4 = 40, w5 = 50, w6 = 60;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with pointer post-increment */
        while (p < end) {
            /* Should generate: (mem (plus (reg) (const_int 0))) */
            sum += *p++;
            
            /* Massive register pressure calculations */
            v1 ^= sum; v2 += v1; v3 ^= v2; v4 += v3;
            v5 ^= v4; v6 += v5; v7 ^= v6; v8 += v7;
            w1 ^= v8; w2 += w1; w3 ^= w2; w4 += w3;
            w5 ^= w4; w6 += w5;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(p) : "memory");
        }
        
        /* Use variables between loops */
        use_int(v1 + v2 + v3 + v4);
        use_int(w1 + w2 + w3 + w4);
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic forms */
int test5_mixed_forms(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Register pressure */
    int t1 = 1, t2 = 2, t3 = 3, t4 = 4;
    
    /* Combined forms that may decompose to base+offset */
    while (p < end) {
        /* Different forms of pointer arithmetic */
        sum += *p;           /* Simple dereference */
        p += 1;              /* Separate increment */
        
        /* Alternative: *(p += stride) form */
        if (p < end - stride) {
            sum += *(p += stride);
        }
        
        /* More pressure */
        t1 ^= sum; t2 += t1; t3 ^= t2; t4 += t3;
        
        /* Force pointer to stay in register */
        use_ptr(p);
    }
    
    use_int(t1 + t2 + t3 + t4);
    return sum;
}

/* Test 6: do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int u1 = 1, u2 = 2, u3 = 3, u4 = 4;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Pressure calculations */
        u1 ^= sum; u2 += u1; u3 ^= u2; u4 += u3;
        
        /* Prevent elimination */
        asm volatile("" : : "r"(p) : "memory");
    } while (p >= arr);
    
    use_int(u1 + u2 + u3 + u4);
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    const int N = 100;
    const int ROWS = 10;
    const int COLS = 10;
    
    /* Initialize test data */
    int* int_array = malloc(N * sizeof(int));
    char* char_array1 = malloc(N * sizeof(char));
    char* char_array2 = malloc(N * sizeof(char));
    struct Point* points = malloc(N * sizeof(struct Point));
    int** matrix = malloc(ROWS * sizeof(int*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i + 1;
        char_array1[i] = 'A' + (i % 26);
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    memset(char_array2, 0, N);
    
    for (int i = 0; i < ROWS; i++) {
        matrix[i] = malloc(COLS * sizeof(int));
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_post_increment_sum(int_array, N);
            break;
        case 2:
            test2_string_copy(char_array2, char_array1, N);
            result = char_array2[N-1];
            break;
        case 3:
            result = test3_struct_traversal(points, N);
            break;
        case 4:
            result = test4_nested_loops(matrix, ROWS, COLS);
            break;
        case 5:
            result = test5_mixed_forms(int_array, N, 2);
            break;
        case 6:
            result = test6_post_decrement(int_array, N);
            break;
        default:
            /* Run all tests */
            result += test1_post_increment_sum(int_array, N);
            test2_string_copy(char_array2, char_array1, N);
            result += test3_struct_traversal(points, N);
            result += test4_nested_loops(matrix, ROWS, COLS);
            result += test5_mixed_forms(int_array, N, 2);
            result += test6_post_decrement(int_array, N);
            break;
    }
    
    /* Cleanup */
    for (int i = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(points);
    
    printf("Result: %d, Global checksum: %d\n", result, global_checksum);
    return 0;
}
