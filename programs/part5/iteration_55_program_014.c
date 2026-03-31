/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

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

/* Global arrays to prevent optimization */
volatile int global_volatile_array[256];
int global_array[256][256];
struct BigStruct global_structs[16];

/* Function 1: Complex array addressing with multiple index computations */
static inline __attribute__((always_inline)) 
int test_complex_addressing(int i, int j, int k, int l, int m) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    volatile int v1, v2, v3, v4, v5;
    
    /* Complex array indexing - triggers RELOAD_FOR_INPUT_ADDRESS */
    a1 = global_array[i][j] + global_array[j][k];
    a2 = global_array[k][l] * global_array[l][m];
    
    /* Multi-level array access with computed indices */
    a3 = global_array[i + j][k + l] - global_array[j + k][l + m];
    
    /* Volatile accesses force memory operations */
    v1 = global_volatile_array[i];
    v2 = global_volatile_array[j];
    v3 = global_volatile_array[k];
    
    /* Complex expression with many intermediate values */
    b1 = (i * j) + (k * l) + (m * i);
    b2 = (j * k) + (l * m) + (i * j);
    b3 = b1 * b2 - a1 * a2;
    b4 = b3 + a3 * v1;
    
    /* Manual loop unrolling for register pressure */
    #pragma GCC unroll 4
    for (int x = 0; x < 4; x++) {
        /* Each iteration uses different addressing */
        c1 = global_array[i + x][j];
        c2 = global_array[j + x][k];
        c3 = c1 * c2 + b4;
        b4 = c3 - global_volatile_array[x];
    }
    
    return a1 + a2 + a3 + b1 + b2 + b3 + b4 + v1 + v2 + v3;
}

/* Function 2: Structure member accesses with pointer chasing */
static __attribute__((noinline))
int test_structure_access(int idx) {
    struct BigStruct local_structs[4];
    struct BigStruct *ptr1, *ptr2, *ptr3, *ptr4;
    int sum = 0;
    
    /* Initialize pointers with complex expressions */
    ptr1 = &local_structs[idx % 4];
    ptr2 = &global_structs[(idx + 1) % 16];
    ptr3 = ptr1->next;
    ptr4 = ptr2->next;
    
    /* Multiple structure member accesses - may trigger RELOAD_FOR_OPERAND_ADDRESS */
    int t1 = ptr1->a + ptr1->b;
    int t2 = ptr2->c * ptr2->d;
    int t3 = ptr3->e - ptr3->f;
    int t4 = ptr4->g / (ptr4->h + 1);
    
    /* Complex array access within structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Nested array indexing - needs address reloads */
            t1 += ptr1->arr[i][j];
            t2 += ptr2->arr[j][i];
            
            /* Volatile member access */
            ptr1->volatile_member = i * j;
            t3 += ptr1->volatile_member;
        }
    }
    
    /* Pointer arithmetic with multiple bases */
    sum = t1 + t2 + t3 + t4;
    
    /* Inline assembly with multiple outputs to clobber registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (sum)
        : "r" (sum), "r" (idx)
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
    
    return sum;
}

/* Function 3: Inline assembly with complex constraints */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3, result4;
    int *ptr1, *ptr2;
    volatile int *volatile_ptr;
    
    /* Complex address computations for output */
    ptr1 = &global_array[x][y];
    ptr2 = &global_array[y][z];
    volatile_ptr = &global_volatile_array[x];
    
    /* Inline asm with memory output - may trigger RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        : 
        : "r" (ptr1), "r" (ptr2), "r" (volatile_ptr)
        : "%eax", "memory"
    );
    
    /* Multiple output operands */
    asm volatile (
        "movl $1, %0\n\t"
        "movl $2, %1\n\t"
        "movl $3, %2\n\t"
        "movl $4, %3\n\t"
        : "=r" (result1), "=r" (result2), "=r" (result3), "=r" (result4)
        :
        : "%eax", "%ebx", "%ecx", "%edx"
    );
    
    /* Asm with input/output operand */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+r" (result1)
        : "r" (result2)
        : "%eax", "%ebx"
    );
    
    return result1 + result2 + result3 + result4 + *ptr1 + *volatile_ptr;
}

/* Function 4: Mixed addressing modes and builtins */
static __attribute__((noinline))
int test_mixed_addressing(int base) {
    int arr1[16], arr2[16], arr3[16];
    int *ptr_arr[8];
    int sum = 0;
    
    /* Initialize arrays and pointers */
    for (int i = 0; i < 16; i++) {
        arr1[i] = base + i;
        arr2[i] = base * i;
        arr3[i] = base - i;
    }
    
    for (int i = 0; i < 8; i++) {
        ptr_arr[i] = &arr1[i * 2];
    }
    
    /* Complex pointer expressions with multiple dereferences */
    for (int i = 0; i < 8; i++) {
        /* Chain of dereferences - may trigger RELOAD_FOR_OPADDR_ADDR */
        int val = *(ptr_arr[i] + arr2[i] - arr3[i]);
        
        /* Use __builtin_expect to inhibit optimization */
        if (__builtin_expect(val > 0, 1)) {
            sum += val;
        } else {
            sum -= val;
        }
        
        /* Complex address computation */
        int *complex_ptr = &arr1[arr2[i] % 16] + (arr3[i] % 8);
        sum += *complex_ptr;
        
        /* Volatile pointer access */
        volatile int *vptr = (volatile int *)&arr2[i];
        sum += *vptr;
    }
    
    /* Nested function calls with many arguments */
    sum += test_complex_addressing(sum % 8, (sum + 1) % 8, 
                                   (sum + 2) % 8, (sum + 3) % 8,
                                   (sum + 4) % 8);
    
    return sum;
}

/* Function 5: Extreme register pressure with all patterns combined */
static __attribute__((noinline))
int test_extreme_pressure(int seed) {
    /* Maximum local variables */
    int v01, v02, v03, v04, v05, v06, v07, v08, v09, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    volatile int vv1, vv2, vv3, vv4, vv5;
    
    int matrix[4][4];
    int *ptr1, *ptr2, *ptr3;
    struct BigStruct local_struct;
    
    /* Initialize everything */
    v01 = seed; v02 = seed * 2; v03 = seed * 3; v04 = seed * 4; v05 = seed * 5;
    v06 = seed * 6; v07 = seed * 7; v08 = seed * 8; v09 = seed * 9; v10 = seed * 10;
    v11 = seed * 11; v12 = seed * 12; v13 = seed * 13; v14 = seed * 14; v15 = seed * 15;
    v16 = seed * 16; v17 = seed * 17; v18 = seed * 18; v19 = seed * 19; v20 = seed * 20;
    v21 = seed * 21; v22 = seed * 22; v23 = seed * 23; v24 = seed * 24; v25 = seed * 25;
    v26 = seed * 26; v27 = seed * 27; v28 = seed * 28; v29 = seed * 29; v30 = seed * 30;
    
    /* Complex expressions using all variables */
    v01 = v01 + v02 - v03 * v04 / (v05 + 1);
    v06 = v06 * v07 + v08 - v09 * v10;
    v11 = v11 & v12 | v13 ^ v14;
    v16 = (v15 << 2) | (v16 >> 3);
    
    /* Matrix operations */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = v01 * i + v02 * j + v03;
        }
    }
    
    /* Pointer chasing with complex addresses */
    ptr1 = &matrix[0][0];
    ptr2 = &matrix[1][1];
    ptr3 = &matrix[2][2];
    
    /* Multiple memory accesses with different addressing */
    v17 = *ptr1 + *(ptr1 + 1) + *(ptr1 + 2);
    v18 = ptr2[0] + ptr2[1] + ptr2[2];
    v19 = *(ptr3 - 1) + *ptr3 + *(ptr3 + 1);
    
    /* Structure operations */
    local_struct.a = v20; local_struct.b = v21; local_struct.c = v22;
    local_struct.d = v23; local_struct.e = v24; local_struct.f = v25;
    local_struct.g = v26; local_struct.h = v27;
    
    /* Volatile operations */
    vv1 = v28; vv2 = v29; vv3 = v30;
    vv4 = vv1 + vv2;
    vv5 = vv3 * vv4;
    
    /* Call other test functions */
    v30 += test_complex_addressing(v01, v02, v03, v04, v05);
    v30 += test_structure_access(v06);
    v30 += test_asm_reloads(v07, v08, v09);
    v30 += test_mixed_addressing(v10);
    
    /* Final complex computation */
    int result = v01 + v06 + v11 + v16 + v17 + v18 + v19 + v30 + vv5;
    result += local_struct.a + local_struct.b + local_struct.c;
    
    return result;
}

/* Main driver function */
int main(int argc, char **argv) {
    int sum = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_volatile_array[i] = i;
        for (int j = 0; j < 256; j++) {
            global_array[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        global_structs[i].d = i * 4;
        global_structs[i].e = i * 5;
        global_structs[i].f = i * 6;
        global_structs[i].g = i * 7;
        global_structs[i].h = i * 8;
        global_structs[i].next = &global_structs[(i + 1) % 16];
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                global_structs[i].arr[x][y] = i + x + y;
            }
        }
    }
    
    /* Run all test functions with different parameters */
    sum += test_complex_addressing(1, 2, 3, 4, 5);
    sum += test_complex_addressing(6, 7, 8, 9, 10);
    sum += test_complex_addressing(11, 12, 13, 14, 15);
    
    sum += test_structure_access(1);
    sum += test_structure_access(2);
    sum += test_structure_access(3);
    
    sum += test_asm_reloads(4, 5, 6);
    sum += test_asm_reloads(7, 8, 9);
    sum += test_asm_reloads(10, 11, 12);
    
    sum += test_mixed_addressing(13);
    sum += test_mixed_addressing(14);
    sum += test_mixed_addressing(15);
    
    sum += test_extreme_pressure(16);
    sum += test_extreme_pressure(17);
    sum += test_extreme_pressure(18);
    
    /* Use the result to prevent dead code elimination */
    if (sum > 0) {
        return 0;
    } else {
        return 1;
    }
}

#pragma GCC pop_options
