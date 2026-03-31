/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8][8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to create addressing complexity */
static int global_array[256][256];
static volatile int volatile_global[100];

/* Test 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int x, int y, int z) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8;
    int b1, b2, b3, b4, b5, b6, b7, b8;
    int c1, c2, c3, c4, c5, c6, c7, c8;
    int d1, d2, d3, d4, d5, d6, d7, d8;
    
    /* Force all variables to be live simultaneously */
    a1 = x + 1; a2 = x + 2; a3 = x + 3; a4 = x + 4;
    a5 = x + 5; a6 = x + 6; a7 = x + 7; a8 = x + 8;
    
    b1 = y + 1; b2 = y + 2; b3 = y + 3; b4 = y + 4;
    b5 = y + 5; b6 = y + 6; b7 = y + 7; b8 = y + 8;
    
    c1 = z + 1; c2 = z + 2; c3 = z + 3; c4 = z + 4;
    c5 = z + 5; c6 = z + 6; c7 = z + 7; c8 = z + 8;
    
    /* Complex array addressing - will need address reloads */
    d1 = global_array[a1 + b1][c1] + global_array[a2][b2 + c2];
    d2 = global_array[a3 + b3][c3] + global_array[a4][b4 + c4];
    d3 = global_array[a5 + b5][c5] + global_array[a6][b6 + c6];
    d4 = global_array[a7 + b7][c7] + global_array[a8][b8 + c8];
    
    /* More complex addressing with volatile */
    d5 = volatile_global[a1 * b1] + volatile_global[a2 * b2];
    d6 = volatile_global[a3 * b3] + volatile_global[a4 * b4];
    d7 = volatile_global[a5 * b5] + volatile_global[a6 * b6];
    d8 = volatile_global[a7 * b7] + volatile_global[a8 * b8];
    
    /* Use all variables to prevent dead code elimination */
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 +
           a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 +
           b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 +
           c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
}

/* Test 2: Structure member access with pointer chasing */
static __attribute__((noinline))
int test_structure_access(struct BigStruct *s1, struct BigStruct *s2) {
    int sum = 0;
    
    /* Complex structure member addressing */
    sum += s1->arr[s1->a + s2->b][s1->c * s2->d];
    sum += s2->arr[s2->a + s1->b][s2->c * s1->d];
    
    /* Nested structure access */
    if (s1->next) {
        sum += s1->next->arr[s1->next->a][s1->next->b];
    }
    if (s2->next) {
        sum += s2->next->arr[s2->next->a][s2->next->b];
    }
    
    /* Volatile member access forces memory operations */
    s1->volatile_member = sum;
    s2->volatile_member = sum * 2;
    
    /* Complex expression with many temporaries */
    int t1 = s1->a * s1->b + s1->c * s1->d;
    int t2 = s2->a * s2->b + s2->c * s2->d;
    int t3 = s1->e * s1->f + s1->g * s1->h;
    int t4 = s2->e * s2->f + s2->g * s2->h;
    
    return sum + t1 + t2 + t3 + t4 + 
           s1->volatile_member + s2->volatile_member;
}

/* Test 3: Inline assembly with multiple outputs and complex addressing */
static __attribute__((noinline))
int test_inline_asm(int *ptr, int index) {
    int result1, result2, result3, result4;
    int addr1, addr2, addr3, addr4;
    
    /* Complex address computation for output */
    int *addr_ptr1 = ptr + index * 2;
    int *addr_ptr2 = ptr + index * 3;
    int *addr_ptr3 = ptr + index * 4;
    int *addr_ptr4 = ptr + index * 5;
    
    /* Inline assembly with memory outputs - triggers output address reloads */
    __asm__ volatile (
        "movl %[val1], (%[ptr1])\n\t"
        "movl %[val2], (%[ptr2])\n\t"
        "movl %[val3], (%[ptr3])\n\t"
        "movl %[val4], (%[ptr4])\n\t"
        : 
        : [ptr1] "r" (addr_ptr1), [val1] "r" (index + 1),
          [ptr2] "r" (addr_ptr2), [val2] "r" (index + 2),
          [ptr3] "r" (addr_ptr3), [val3] "r" (index + 3),
          [ptr4] "r" (addr_ptr4), [val4] "r" (index + 4)
        : "memory"
    );
    
    /* More inline assembly with input/output operands */
    __asm__ volatile (
        "imull %[in1], %[out1]\n\t"
        "imull %[in2], %[out2]\n\t"
        "imull %[in3], %[out3]\n\t"
        "imull %[in4], %[out4]\n\t"
        : [out1] "=r" (result1), [out2] "=r" (result2),
          [out3] "=r" (result3), [out4] "=r" (result4)
        : [in1] "r" (index), [in2] "r" (index * 2),
          [in3] "r" (index * 3), [in4] "r" (index * 4)
    );
    
    /* Use computed addresses */
    addr1 = *addr_ptr1;
    addr2 = *addr_ptr2;
    addr3 = *addr_ptr3;
    addr4 = *addr_ptr4;
    
    return result1 + result2 + result3 + result4 + 
           addr1 + addr2 + addr3 + addr4;
}

/* Test 4: Pointer chasing with complex expressions */
static __attribute__((noinline))
int test_pointer_chasing(int **ptr_array, int size) {
    int sum = 0;
    
    /* Manual loop unrolling for register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < size && i < 8; i++) {
        int *ptr = ptr_array[i];
        if (ptr) {
            /* Complex addressing with multiple levels */
            sum += ptr[i] + ptr[i * 2] + ptr[i * 3];
            
            /* More complex expression */
            sum += *(ptr + i) * *(ptr + i + 1);
            sum += *(ptr + i * 2) * *(ptr + i * 2 + 1);
        }
    }
    
    /* Additional complex pointer arithmetic */
    int *base_ptr = ptr_array[0];
    if (base_ptr) {
        for (int j = 0; j < 4; j++) {
            /* This creates operand address reload needs */
            int *temp_ptr = base_ptr + j * 16;
            sum += temp_ptr[0] + temp_ptr[1] + temp_ptr[2] + temp_ptr[3];
        }
    }
    
    return sum;
}

/* Test 5: Mixed operations with builtins */
static __attribute__((noinline))
int test_mixed_operations(int x, int y, int z) {
    /* Many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    
    /* Complex expressions with __builtin_expect to inhibit optimization */
    v1 = __builtin_expect(x * y + z, 1);
    v2 = __builtin_expect(y * z + x, 1);
    v3 = __builtin_expect(z * x + y, 1);
    v4 = __builtin_expect(x * x + y * y, 1);
    v5 = __builtin_expect(y * y + z * z, 1);
    
    /* More variables with complex addressing */
    w1 = global_array[v1][v2] + global_array[v3][v4];
    w2 = global_array[v5][v1] + global_array[v2][v3];
    w3 = volatile_global[v4] + volatile_global[v5];
    w4 = volatile_global[v1] + volatile_global[v2];
    
    /* Nested array access */
    v6 = global_array[global_array[x][y]][global_array[y][z]];
    v7 = global_array[global_array[z][x]][global_array[x][y]];
    
    /* Use all variables */
    v8 = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    v9 = w1 + w2 + w3 + w4;
    v10 = v8 * v9;
    
    return v10 + x + y + z;
}

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_array[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        volatile_global[i] = i * 2;
    }
    
    /* Test 1: Complex addressing */
    result += test_complex_addressing(1, 2, 3);
    result += test_complex_addressing(4, 5, 6);
    
    /* Test 2: Structure access */
    struct BigStruct s1 = {0}, s2 = {0};
    s1.a = 1; s1.b = 2; s1.c = 3; s1.d = 4;
    s1.e = 5; s1.f = 6; s1.g = 7; s1.h = 8;
    s2.a = 9; s2.b = 10; s2.c = 11; s2.d = 12;
    s2.e = 13; s2.f = 14; s2.g = 15; s2.h = 16;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            s1.arr[i][j] = i * j;
            s2.arr[i][j] = i + j;
        }
    }
    
    result += test_structure_access(&s1, &s2);
    
    /* Test 3: Inline assembly */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    result += test_inline_asm(array, 10);
    result += test_inline_asm(array, 20);
    
    /* Test 4: Pointer chasing */
    int *ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &array[i * 8];
    }
    
    result += test_pointer_chasing(ptr_array, 10);
    
    /* Test 5: Mixed operations */
    result += test_mixed_operations(7, 8, 9);
    result += test_mixed_operations(10, 11, 12);
    
    /* Use result to prevent optimization */
    volatile int final_result = result;
    
    return final_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
