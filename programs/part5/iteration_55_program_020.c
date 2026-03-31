/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8][8];
    struct {
        int x, y, z;
    } nested;
    volatile int volatile_member;
};

/* Global arrays to prevent constant propagation */
volatile int global_volatile_array[256];
int global_array[256][256];

/* Test 1: Complex array addressing with multiple index calculations */
static __attribute__((noinline)) 
int test_complex_addressing(int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    int d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Complex array indexing - forces address reloads */
    a1 = global_array[idx1 * 2][idx2 * 3] + 
         global_array[idx2 * 3][idx3 * 4] +
         global_array[idx3 * 4][idx1 * 2];
    
    /* Multi-level array access with volatile */
    a2 = global_volatile_array[idx1] + 
         global_volatile_array[idx2] +
         global_volatile_array[idx3];
    
    /* Address computation that needs its own register */
    a3 = *(int*)((char*)global_array + idx1 * 1024 + idx2 * 512 + idx3 * 256);
    
    /* More computations to create register pressure */
    b1 = a1 * a2 + a3;
    b2 = a1 * a3 - a2;
    b3 = a2 * a3 + a1;
    
    /* Nested addressing modes */
    c1 = global_array[global_volatile_array[idx1]][global_volatile_array[idx2]];
    c2 = global_array[global_volatile_array[idx2]][global_volatile_array[idx3]];
    
    /* Force spills with many intermediate values */
    d1 = b1 + b2 + b3 + c1 + c2;
    d2 = b1 * b2 * b3 * c1 * c2;
    d3 = (b1 << 2) | (b2 << 4) | (b3 << 6);
    
    /* Complex expression with many operands */
    return d1 * d2 + d3 * a1 - a2 * a3 + b1 * b2 - b3 * c1 + c2;
}

/* Test 2: Structure member accesses with pointer arithmetic */
static __attribute__((noinline))
int test_structure_access(struct BigStruct *s1, struct BigStruct *s2, 
                          struct BigStruct *s3) {
    int sum = 0;
    
    /* Access structure members with complex addressing */
    sum += s1->a + s1->b + s1->c + s1->d;
    sum += s2->e + s2->f + s2->g + s2->h;
    sum += s3->nested.x + s3->nested.y + s3->nested.z;
    
    /* Array within structure with computed indices */
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            /* Manual unrolling to increase register pressure */
            sum += s1->arr[i][j] + s2->arr[j][i] + s3->arr[i][i];
            sum += s1->arr[j][i] + s2->arr[i][j] + s3->arr[j][j];
        }
    }
    
    /* Pointer arithmetic with multiple bases */
    int *p1 = &s1->a;
    int *p2 = &s2->b;
    int *p3 = &s3->c;
    
    /* Complex address computations */
    sum += *(p1 + 1) + *(p2 + 2) + *(p3 + 3);
    sum += *(p1 - 1) + *(p2 - 2) + *(p3 - 3);
    
    /* Volatile member access forces memory operations */
    s1->volatile_member = sum;
    s2->volatile_member = sum * 2;
    s3->volatile_member = sum * 3;
    
    return sum + s1->volatile_member + s2->volatile_member + s3->volatile_member;
}

/* Test 3: Inline assembly with multiple outputs and clobbers */
static __attribute__((noinline))
int test_inline_asm(int x, int y, int z) {
    int out1, out2, out3, out4, out5, out6;
    int addr1, addr2, addr3;
    
    /* Complex address computation for output */
    int *addr_ptr1 = &global_array[x][y];
    int *addr_ptr2 = &global_array[y][z];
    int *addr_ptr3 = &global_array[z][x];
    
    /* Inline asm with memory output - forces output address reloads */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%eax\n\t"
        "imull %%ebx, %%ecx\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=m" (*addr_ptr1),
          [out2] "=m" (*addr_ptr2),
          [out3] "=m" (*addr_ptr3)
        : [in1] "rm" (x),
          [in2] "rm" (y),
          [in3] "rm" (z)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Another asm with input address reloads */
    asm volatile (
        "leal (%[base], %[index], 4), %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (out4)
        : [base] "r" (global_array),
          [index] "r" (x * 256 + y)
        : "eax", "ebx", "memory"
    );
    
    /* Asm with operand address reload */
    int * volatile volatile_ptr = &global_volatile_array[0];
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[out5]\n\t"
        : [out5] "=r" (out5)
        : [ptr] "r" (volatile_ptr)
        : "eax", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Test 4: Pointer chasing and complex expressions */
static __attribute__((noinline))
int test_pointer_chasing(int iterations) {
    int **ptr1, **ptr2, **ptr3;
    int *arr1[16], *arr2[16], *arr3[16];
    int values[64];
    int i, j, sum = 0;
    
    /* Initialize pointer arrays */
    for (i = 0; i < 16; i++) {
        arr1[i] = &values[(i * 3) % 64];
        arr2[i] = &values[(i * 5) % 64];
        arr3[i] = &values[(i * 7) % 64];
        values[i] = i * 2;
    }
    
    /* Pointer chasing loop - creates operand address reloads */
    ptr1 = arr1;
    ptr2 = arr2;
    ptr3 = arr3;
    
    for (i = 0; i < iterations; i++) {
        /* Complex pointer dereferencing */
        sum += **ptr1 + **ptr2 + **ptr3;
        
        /* Address computations on pointers */
        ptr1 = &arr1[(**ptr1) % 16];
        ptr2 = &arr2[(**ptr2) % 16];
        ptr3 = &arr3[(**ptr3) % 16];
        
        /* More computations to use registers */
        sum += *(*ptr1 + 1) + *(*ptr2 + 2) + *(*ptr3 + 3);
    }
    
    return sum;
}

/* Test 5: Mixed operations with many intermediate values */
static __attribute__((noinline))
int test_mixed_operations(int a, int b, int c, int d, int e, int f) {
    /* Declare many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5, y6, y7, y8, y9, y10;
    int z1, z2, z3, z4, z5, z6, z7, z8, z9, z10;
    
    /* Complex expressions with many operands */
    v1 = a + b * c - d / (e + 1) + f;
    v2 = b + c * d - e / (f + 1) + a;
    v3 = c + d * e - f / (a + 1) + b;
    v4 = d + e * f - a / (b + 1) + c;
    v5 = e + f * a - b / (c + 1) + d;
    v6 = f + a * b - c / (d + 1) + e;
    
    /* More computations creating data dependencies */
    w1 = v1 * v2 + v3 * v4 - v5 * v6;
    w2 = v2 * v3 + v4 * v5 - v6 * v1;
    w3 = v3 * v4 + v5 * v6 - v1 * v2;
    w4 = v4 * v5 + v6 * v1 - v2 * v3;
    w5 = v5 * v6 + v1 * v2 - v3 * v4;
    w6 = v6 * v1 + v2 * v3 - v4 * v5;
    
    /* Use builtins to prevent optimization */
    x1 = __builtin_expect(w1 > 0, 1) ? w1 : w2;
    x2 = __builtin_expect(w2 > 0, 1) ? w2 : w3;
    x3 = __builtin_expect(w3 > 0, 1) ? w3 : w4;
    x4 = __builtin_expect(w4 > 0, 1) ? w4 : w5;
    x5 = __builtin_expect(w5 > 0, 1) ? w5 : w6;
    x6 = __builtin_expect(w6 > 0, 1) ? w6 : w1;
    
    /* Final complex expression */
    return x1 + x2 * x3 - x4 / (x5 + 1) + x6 +
           v1 - v2 + v3 * v4 - v5 / (v6 + 1) +
           w1 * w2 + w3 * w4 - w5 * w6;
}

/* Main driver function */
int main(void) {
    int result = 0;
    struct BigStruct s1, s2, s3;
    
    /* Initialize structures */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            s1.arr[i][j] = i * j;
            s2.arr[i][j] = i + j;
            s3.arr[i][j] = i - j;
        }
    }
    s1.nested.x = 1; s1.nested.y = 2; s1.nested.z = 3;
    s2.nested.x = 4; s2.nested.y = 5; s2.nested.z = 6;
    s3.nested.x = 7; s3.nested.y = 8; s3.nested.z = 9;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_volatile_array[i] = i;
        for (int j = 0; j < 256; j++) {
            global_array[i][j] = i * 256 + j;
        }
    }
    
    /* Run all tests to trigger different reload types */
    result += test_complex_addressing(1, 2, 3);
    result += test_complex_addressing(4, 5, 6);
    result += test_complex_addressing(7, 8, 9);
    
    result += test_structure_access(&s1, &s2, &s3);
    result += test_structure_access(&s2, &s3, &s1);
    result += test_structure_access(&s3, &s1, &s2);
    
    result += test_inline_asm(10, 20, 30);
    result += test_inline_asm(40, 50, 60);
    result += test_inline_asm(70, 80, 90);
    
    result += test_pointer_chasing(5);
    result += test_pointer_chasing(10);
    result += test_pointer_chasing(15);
    
    result += test_mixed_operations(1, 2, 3, 4, 5, 6);
    result += test_mixed_operations(7, 8, 9, 10, 11, 12);
    result += test_mixed_operations(13, 14, 15, 16, 17, 18);
    
    /* Use result to prevent dead code elimination */
    return result > 0 ? 0 : 1;
}

#pragma GCC pop_options
