/* test_auto_inc_dec.c - Program to trigger auto-increment/decrement RTL patterns */

#include <stddef.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern int get_seed(void) __attribute__((noinline));

/* Prevent inlining and optimization */
#define NOOPT __attribute__((noinline, noipa))

/* Test 1: Integer array summation with post-increment pointer */
NOOPT int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = get_seed(), r2 = get_seed(), r3 = get_seed();
    int r4 = get_seed(), r5 = get_seed(), r6 = get_seed();
    
    /* Loop with post-increment */
    while (p < end) {
        sum += *p++;  /* Post-increment - target pattern */
        
        /* Use register pressure variables to prevent spilling */
        r1 ^= sum;
        r2 += r1;
        r3 -= r2;
        r4 *= r3 + 1;
        r5 ^= r4;
        r6 += r5;
    }
    
    /* Make all variables appear used */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6));
    
    return sum;
}

/* Test 2: String copy with post-increment on both pointers */
NOOPT void test_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int i1 = 0, i2 = 0, i3 = 0;
    
    /* Copy loop with post-increment */
    for (int i = 0; i < n; i++) {
        *d++ = *s++;  /* Dual post-increment */
        
        /* Use variables to create pressure */
        c1 = *s;
        c2 = c1 + 1;
        c3 = c2 - 1;
        c4 = c3 ^ c1;
        i1 += c1;
        i2 += c2;
        i3 += c3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4));
    }
    
    /* Opaque use of pointers */
    use_ptr(dst);
    use_ptr((void*)src);
}

/* Simple struct for testing */
struct Point {
    int x;
    int y;
    int z;
};

/* Test 3: Struct array traversal with post-increment */
NOOPT int test_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    
    /* High register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6, a7 = 7, a8 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->x + p->y;
        p++;  /* Post-increment after access */
        
        /* Complex use of pressure variables */
        a1 += p->x;
        a2 -= p->y;
        a3 *= a1 + 1;
        a4 ^= a2;
        a5 += a3;
        a6 -= a4;
        a7 *= a5;
        a8 ^= a6;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                             "r"(a5), "r"(a6), "r"(a7), "r"(a8));
    }
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
NOOPT int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple local variables for register pressure */
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with post-increment */
        while (p < end) {
            sum += *p++;  /* Post-increment in loop */
            
            /* Update pressure variables */
            t1 += sum;
            t2 ^= t1;
            t3 = t2 - t1;
            t4 = t3 * 2;
            t5 = t4 ^ t2;
            t6 += t5;
        }
        
        /* Prevent optimization across iterations */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6));
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
NOOPT int test_mixed_arithmetic(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    
    /* Extreme register pressure */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 0;
    register int r2 asm("r2") = 0;
    register int r3 asm("r3") = 0;
    register int r4 asm("r4") = 0;
    
    for (int i = 0; i < n; i++) {
        /* Combined form that may decompose to base+offset */
        sum += *(p += stride);  /* May create (plus (reg) (const_int 0)) pattern */
        
        /* Use all register variables */
        r0 += sum;
        r1 ^= r0;
        r2 = r1 - r0;
        r3 = r2 * 3;
        r4 += r3;
        
        /* Force register usage */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
    }
    
    return sum + r0 + r1 + r2 + r3 + r4;
}

/* Test 6: Do-while loop with post-decrement */
NOOPT int test_do_while_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    
    do {
        sum += *p--;  /* Post-decrement */
        
        /* Use pressure variables */
        v1 += sum;
        v2 -= v1;
        v3 *= v2 + 1;
        v4 ^= v3;
        v5 += v4;
        
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    } while (p >= arr);
    
    return sum;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int N = 100;
    int int_array[N];
    char char_array[N];
    struct Point points[N];
    int* matrix[10];
    int matrix_data[10][10];
    
    /* Seed with command-line argument to prevent constant folding */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_array[i] = (i * seed) % 100;
        char_array[i] = 'A' + (i % 26);
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        matrix[i] = matrix_data[i];
        for (int j = 0; j < 10; j++) {
            matrix_data[i][j] = (i * 10 + j) * seed;
        }
    }
    
    /* Run all tests */
    int result = 0;
    
    result += test_int_array_sum(int_array, N);
    global_checksum = result;
    
    test_string_copy(char_array, char_array + N/2, N/2);
    result += char_array[0];
    
    result += test_struct_array(points, N);
    result += test_nested_loops(matrix, 10, 10);
    result += test_mixed_arithmetic(int_array, N, 2);
    result += test_do_while_decrement(int_array, N);
    
    /* Use result to prevent elimination */
    global_checksum = result;
    
    /* Opaque use of result */
    use_int(result);
    
    return result != 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_checksum ^= x;
}

void use_ptr(void* p) {
    global_checksum += (int)(intptr_t)p;
}

void use_char(char c) {
    global_checksum += c;
}

int get_seed(void) {
    return global_checksum + 1;
}
