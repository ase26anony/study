/* auto-inc-dec-test.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c auto-inc-dec-test.c
 * Or: gcc -O3 -funroll-loops -fno-inline -c auto-inc-dec-test.c
 */

#include <stddef.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use all register variables to create pressure */
        r1 += r2; r3 += r4; r5 += r6; r7 += r8; r9 += r10;
        r2 += r1; r4 += r3; r6 += r5; r8 += r7; r10 += r9;
    }
    
    /* Use variables to prevent optimization */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use register variables */
        c1++; c2++; c3++; c4++;
        cnt1 += c1; cnt2 += c2; cnt3 += c3; cnt4 += c4;
        
        i++;
    } while (i < len);
    
    /* Make pointers appear used */
    use_ptr(dst);
    use_ptr((void*)src);
    
    /* Use local variables */
    asm volatile("" : : "r"(cnt1), "r"(cnt2));
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    int i = 0;
    
    /* Heavy register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    
    for (i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        total += p->x + p->y;
        
        /* Post-increment after access */
        p++;
        
        /* Complex calculations with register variables */
        a1 = a2 * a3 + a4;
        a2 = a3 * a4 + a5;
        a3 = a4 * a5 + a1;
        a4 = a5 * a1 + a2;
        a5 = a1 * a2 + a3;
        
        b1 += b2; b2 += b3; b3 += b4; b4 += b5; b5 += b1;
    }
    
    /* Use all variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int matrix[10][10], int rows, int cols) {
    int sum = 0;
    int i, j;
    
    /* Register pressure */
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    int u1 = 1, u2 = 2, u3 = 3, u4 = 4, u5 = 5;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        j = 0;
        
        while (j < cols) {
            /* Array access with post-increment on index */
            sum += row[j++];
            
            /* More register pressure */
            t1 = u1 + t2;
            t2 = u2 + t3;
            t3 = u3 + t4;
            t4 = u4 + t5;
            t5 = u5 + t1;
            
            u1++; u2++; u3++; u4++; u5++;
        }
    }
    
    /* Use variables */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
int test_mixed_pointers(char* data, int size) {
    int sum = 0;
    char* p = data;
    int* ip = (int*)data;
    int count = size / sizeof(int);
    
    /* Register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    char c1 = 'x', c2 = 'y', c3 = 'z';
    
    /* Process as integers with post-increment */
    for (int i = 0; i < count; i++) {
        sum += *ip++;
        
        /* Also process as chars */
        c1 = *p++;
        c2 = *p++;
        c3 = *p++;
        
        v1 += c1; v2 += c2; v3 += c3; v4 += v1;
    }
    
    /* Process remaining bytes */
    while (p < data + size) {
        sum += *p++;
        v1++; v2++; v3++; v4++;
    }
    
    /* Use everything */
    use_int(v1 + v2 + v3 + v4);
    use_char(c1);
    
    return sum;
}

/* Test 6: Pointer arithmetic with stride */
int test_pointer_stride(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* High register pressure */
    int r[10];
    for (int i = 0; i < 10; i++) r[i] = i;
    
    /* Use p += stride which may decompose to base + offset */
    for (; p < end; p += stride) {
        sum += *p;
        
        /* Heavy computation with all registers */
        for (int i = 0; i < 9; i++) {
            r[i] += r[i+1];
        }
        r[9] += r[0];
    }
    
    /* Use register array */
    int reg_sum = 0;
    for (int i = 0; i < 10; i++) reg_sum += r[i];
    asm volatile("" : : "r"(reg_sum));
    
    return sum;
}

/* Main function with command-line arguments to prevent constant folding */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_array[100];
    char char_array[200];
    struct Point points[50];
    int matrix[10][10];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = (i * 3) % 7;
    }
    
    for (int i = 0; i < 200; i++) {
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command-line arguments to select tests */
    int test_mask = 0;
    if (argc > 1) {
        test_mask = argv[1][0] - '0';
    }
    
    int result = 0;
    
    /* Run selected tests */
    if (test_mask & 1) {
        result += test_int_array_sum(int_array, 100);
    }
    
    if (test_mask & 2) {
        char dest[200];
        test_string_copy(dest, char_array, 200);
        result += dest[0];
    }
    
    if (test_mask & 4) {
        result += test_struct_array(points, 50);
    }
    
    if (test_mask & 8) {
        result += test_nested_loops(matrix, 10, 10);
    }
    
    if (test_mask & 16) {
        result += test_mixed_pointers(char_array, 200);
    }
    
    if (test_mask & 32) {
        result += test_pointer_stride(int_array, 100, 2);
    }
    
    /* Store result in global volatile */
    global_sum = result;
    
    return result != 0 ? 0 : 1;
}

/* Dummy implementations of external functions */
void __attribute__((noinline)) use_int(int x) {
    asm volatile("" : : "r"(x));
}

void __attribute__((noinline)) use_ptr(void* p) {
    asm volatile("" : : "r"(p));
}

void __attribute__((noinline)) use_char(char c) {
    asm volatile("" : : "r"(c));
}
