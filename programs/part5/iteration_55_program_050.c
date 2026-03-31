/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Disable inlining for specific functions to maintain register pressure */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    int matrix[3][3];
};

/* Global arrays to prevent optimization */
int global_array[256];
struct NestedData global_structs[16];
VOLATILE_VAR int volatile_global = 0;

/* Test 1: Complex array addressing with multiple index computations */
NOINLINE static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to create register pressure */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4, i5 = e + 5, i6 = f + 6;
    int j1 = a * 2, j2 = b * 3, j3 = c * 4, j4 = d * 5, j5 = e * 6, j6 = f * 7;
    int k1 = a ^ b, k2 = c ^ d, k3 = e ^ f;
    int l1 = a | b, l2 = c | d, l3 = e | f;
    
    /* Multi-dimensional array with complex indexing */
    int arr3d[4][4][4];
    
    /* Force many address computations - should trigger RELOAD_FOR_INPUT_ADDRESS */
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    sum += arr3d[i1][j1][k1] + arr3d[i2][j2][k2] + arr3d[i3][j3][k3];
    sum += arr3d[i4][j4][l1] + arr3d[i5][j5][l2] + arr3d[i6][j6][l3];
    
    /* More complex addressing with pointer arithmetic */
    int *ptr1 = &arr3d[0][0][0] + i1 * 16 + j1 * 4 + k1;
    int *ptr2 = &arr3d[0][0][0] + i2 * 16 + j2 * 4 + k2;
    int *ptr3 = &arr3d[0][0][0] + i3 * 16 + j3 * 4 + k3;
    
    /* Nested pointer dereferencing - may trigger RELOAD_FOR_OPERAND_ADDRESS */
    sum += *ptr1 + *ptr2 + *ptr3;
    
    /* Complex expression with many intermediate values */
    int temp1 = (*ptr1 * i1 + *ptr2 * i2) / (j1 + 1);
    int temp2 = (*ptr2 * i3 + *ptr3 * i4) / (j2 + 1);
    int temp3 = (*ptr3 * i5 + *ptr1 * i6) / (j3 + 1);
    
    return sum + temp1 + temp2 + temp3;
}

/* Test 2: Structure access with complex addressing modes */
NOINLINE static int test_structure_addressing(struct NestedData *data, int idx) {
    /* Many local variables */
    int a = idx, b = idx * 2, c = idx * 3, d = idx * 4;
    int e = idx * 5, f = idx * 6, g = idx * 7, h = idx * 8;
    
    /* Complex structure member access - may trigger RELOAD_FOR_INPADDR_ADDRESS */
    int val1 = data->values[a] + data->values[b];
    int val2 = data->matrix[c % 3][d % 3] + data->matrix[e % 3][f % 3];
    
    /* Pointer chasing with address computation */
    struct NestedData *current = data;
    int sum = 0;
    
    /* Manual unrolling */
    sum += current->values[g % 8];
    current = current->next;
    if (current) {
        sum += current->values[h % 8];
        /* More complex address computation */
        sum += current->matrix[a % 3][b % 3] * current->matrix[c % 3][d % 3];
    }
    
    /* Volatile accesses to force memory operations */
    VOLATILE_VAR int vol1 = val1;
    VOLATILE_VAR int vol2 = val2;
    
    return sum + vol1 + vol2;
}

/* Test 3: Inline assembly with multiple outputs and complex constraints */
NOINLINE static int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3;
    int addr1, addr2, addr3;
    
    /* Complex address computations for output */
    int *out_addr1 = &global_array[x * 4 + y];
    int *out_addr2 = &global_array[y * 4 + z];
    int *out_addr3 = &global_array[z * 4 + x];
    
    /* Inline asm with multiple outputs and memory constraints
       May trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=m" (*out_addr1),  /* Memory output - needs address reload */
          [out2] "=m" (*out_addr2),
          [out3] "=m" (*out_addr3)
        : [in1] "rm" (x),            /* Register or memory input */
          [in2] "rm" (y),
          [in3] "rm" (z)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* More asm with input address reloads */
    int array[8] = {x, y, z, x+y, y+z, z+x, x*y, y*z};
    
    __asm__ volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl 4(%[ptr]), %%eax\n\t"
        "addl 8(%[ptr]), %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=r" (result1)
        : [ptr] "r" (&array[0])      /* Register holding address - may need reload */
        : "eax", "memory"
    );
    
    /* Asm with operand address reload */
    __asm__ volatile (
        "leal (%[a], %[b], 4), %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl %[c], %%ebx\n\t"
        "movl %%ebx, %[res]\n\t"
        : [res] "=r" (result2)
        : [a] "r" (&global_array[0]),
          [b] "r" (x),
          [c] "r" (y)
        : "eax", "ebx", "memory"
    );
    
    return result1 + result2 + *out_addr1;
}

/* Test 4: Mixed operations with volatile and complex expressions */
NOINLINE static int test_mixed_operations(int base) {
    /* Create many local variables to exhaust registers */
    int v1 = base + 1, v2 = base + 2, v3 = base + 3, v4 = base + 4;
    int v5 = base + 5, v6 = base + 6, v7 = base + 7, v8 = base + 8;
    int v9 = base + 9, v10 = base + 10, v11 = base + 11, v12 = base + 12;
    int v13 = base + 13, v14 = base + 14, v15 = base + 15, v16 = base + 16;
    
    /* Complex expressions with many intermediate values */
    int t1 = (v1 * v2) + (v3 * v4) - (v5 / (v6 + 1));
    int t2 = (v7 * v8) + (v9 * v10) - (v11 / (v12 + 1));
    int t3 = (v13 * v14) + (v15 * v16) - (v1 / (v2 + 1));
    
    /* Volatile variables to force memory accesses */
    VOLATILE_VAR int vol_t1 = t1;
    VOLATILE_VAR int vol_t2 = t2;
    VOLATILE_VAR int vol_t3 = t3;
    
    /* Complex array addressing with multiple bases */
    int arr1[8], arr2[8], arr3[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        arr1[i] = v1 + i;
        arr2[i] = v2 + i * 2;
        arr3[i] = v3 + i * 3;
    }
    
    /* Complex memory access pattern - may trigger various reload types */
    int sum = 0;
    
    /* Manual unrolling with complex addressing */
    sum += arr1[v1 % 8] + arr2[v2 % 8] + arr3[v3 % 8];
    sum += arr1[v4 % 8] * arr2[v5 % 8] - arr3[v6 % 8];
    sum += arr1[v7 % 8] / (arr2[v8 % 8] + 1) + arr3[v9 % 8];
    
    /* Pointer arithmetic with multiple bases */
    int *p1 = &arr1[0] + (v10 % 8);
    int *p2 = &arr2[0] + (v11 % 8);
    int *p3 = &arr3[0] + (v12 % 8);
    
    /* Chain of computations */
    sum += *p1 + *p2 + *p3;
    sum += *(p1 + (v13 % 4)) - *(p2 + (v14 % 4));
    sum += *(p3 + (v15 % 4)) * *(p1 + (v16 % 4));
    
    return sum + vol_t1 + vol_t2 + vol_t3;
}

/* Test 5: Function with parameter passing causing reloads */
NOINLINE static int test_parameter_reloads(int a, int b, int c, int d, int e, int f,
                                          int g, int h, int i, int j, int k, int l) {
    /* All parameters should be in memory at -O0, causing reloads when used */
    
    /* Complex expression using all parameters */
    int result = (a * b) + (c * d) - (e * f) + (g / (h + 1)) 
                 + (i ^ j) | (k & l) + (a << 2) - (b >> 1);
    
    /* More computations to increase register pressure */
    int t1 = a + b + c + d;
    int t2 = e + f + g + h;
    int t3 = i + j + k + l;
    
    /* Array access using parameters as indices */
    int local_arr[16];
    for (int idx = 0; idx < 16; idx++) {
        local_arr[idx] = idx * idx;
    }
    
    /* Complex addressing */
    result += local_arr[a % 16] * local_arr[b % 16];
    result += local_arr[c % 16] / (local_arr[d % 16] + 1);
    result -= local_arr[e % 16] | local_arr[f % 16];
    
    return result + t1 + t2 + t3;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].values[j] = i * 10 + j;
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                global_structs[i].matrix[j][k] = i * 3 + j * 10 + k;
            }
        }
        global_structs[i].next = (i < 15) ? &global_structs[i + 1] : NULL;
    }
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6);
    total += test_complex_addressing(7, 8, 9, 10, 11, 12);
    
    total += test_structure_addressing(&global_structs[0], 1);
    total += test_structure_addressing(&global_structs[5], 2);
    total += test_structure_addressing(&global_structs[10], 3);
    
    total += test_asm_reloads(1, 2, 3);
    total += test_asm_reloads(4, 5, 6);
    
    total += test_mixed_operations(100);
    total += test_mixed_operations(200);
    
    /* Test with many parameters to force stack passing */
    total += test_parameter_reloads(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    total += test_parameter_reloads(13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
    
    /* Use the result to prevent dead code elimination */
    volatile_global = total;
    
    return total > 0 ? 0 : 1;
}
