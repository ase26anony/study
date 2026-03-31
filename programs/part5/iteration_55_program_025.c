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
    struct {
        int x, y, z;
    } nested;
    volatile int vol;
};

/* Global arrays to prevent constant propagation */
volatile int global_array[256];
volatile struct BigStruct global_structs[4];

/* Function 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    int i1 = a + 1;
    int i2 = b + 2;
    int i3 = c + 3;
    int i4 = d + 4;
    int i5 = e + 5;
    int i6 = f + 6;
    int i7 = a * b;
    int i8 = c * d;
    int i9 = e * f;
    int i10 = a + b + c;
    int i11 = d + e + f;
    int i12 = a * c * e;
    int i13 = b * d * f;
    int i14 = i1 + i2 + i3;
    int i15 = i4 + i5 + i6;
    
    /* Multi-dimensional array with complex indexing - triggers address reloads */
    int arr3d[4][4][4];
    
    /* Complex address computations that need multiple reloads */
    arr3d[i1 & 3][i2 & 3][i3 & 3] = 
        global_array[i1 * i2 + i3] + 
        global_array[i4 * i5 + i6];
    
    /* Nested addressing requiring RELOAD_FOR_INPUT_ADDRESS */
    arr3d[(i7 + i8) & 3][(i9 + i10) & 3][(i11 + i12) & 3] = 
        global_array[global_array[i13] + i14] +
        global_array[global_array[i15] + i1];
    
    /* Pointer arithmetic with multiple bases */
    int *ptr1 = &arr3d[0][0][0];
    int *ptr2 = &arr3d[1][0][0];
    int *ptr3 = &arr3d[2][0][0];
    
    /* Complex addressing modes */
    ptr1[i1 * 4 + i2] = ptr2[i3 * 4 + i4] + ptr3[i5 * 4 + i6];
    
    /* Force spills and reloads */
    volatile int v1 = i1;
    volatile int v2 = i2;
    volatile int v3 = i3;
    
    return arr3d[0][0][0] + arr3d[1][1][1] + arr3d[2][2][2];
}

/* Function 2: Inline assembly with multiple outputs and clobbers */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int a, b, c, d, e, f, g, h;
    
    /* Inline asm with multiple outputs - forces output address reloads */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "movl %[z], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[a]\n\t"
        "movl %%ebx, %[b]\n\t"
        "movl %%ecx, %[c]"
        : [a] "=m" (a), [b] "=m" (b), [c] "=m" (c)
        : [x] "mr" (x), [y] "mr" (y), [z] "mr" (z)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* More variables to increase pressure */
    int i1 = x + y;
    int i2 = y + z;
    int i3 = z + x;
    int i4 = x * y;
    int i5 = y * z;
    int i6 = z * x;
    int i7 = i1 + i2;
    int i8 = i3 + i4;
    int i9 = i5 + i6;
    
    /* Complex expression with many intermediate values */
    d = ((i1 * i2) + (i3 * i4)) / (i5 + 1);
    e = ((i6 * i7) + (i8 * i9)) / (i1 + 1);
    f = ((i2 * i3) + (i4 * i5)) / (i6 + 1);
    g = ((i7 * i8) + (i9 * i1)) / (i2 + 1);
    h = ((i3 * i4) + (i5 * i6)) / (i7 + 1);
    
    /* Memory operands with complex addresses */
    global_array[a + b] = c + d;
    global_array[b + c] = d + e;
    global_array[c + d] = e + f;
    
    /* Force address computations */
    int *ptr = &global_array[0];
    ptr[a * b % 256] = ptr[b * c % 256] + ptr[c * d % 256];
    
    return a + b + c + d + e + f + g + h;
}

/* Function 3: Structure member accesses with offsets */
static __attribute__((noinline))
int test_struct_addressing(int idx) {
    struct BigStruct locals[3];
    int result = 0;
    
    /* Initialize with complex expressions */
    for (int i = 0; i < 3; i++) {
        locals[i].a = idx + i * 10;
        locals[i].b = idx + i * 20;
        locals[i].c = idx + i * 30;
        locals[i].d = idx + i * 40;
        locals[i].e = idx + i * 50;
        locals[i].f = idx + i * 60;
        locals[i].g = idx + i * 70;
        locals[i].h = idx + i * 80;
        
        /* Nested structure access */
        locals[i].nested.x = idx * i;
        locals[i].nested.y = idx + i;
        locals[i].nested.z = idx - i;
        
        /* Array within struct */
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                locals[i].arr[j][k] = idx + i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex structure member addressing - triggers operand address reloads */
    result += locals[0].arr[locals[1].a & 7][locals[2].b & 7];
    result += locals[1].arr[locals[2].c & 7][locals[0].d & 7];
    result += locals[2].arr[locals[0].e & 7][locals[1].f & 7];
    
    /* Pointer to member with offset computation */
    int *ptr = &locals[0].a;
    ptr += (locals[1].b & 3);  /* Complex offset */
    result += *ptr;
    
    /* Address of array element within struct */
    int *arrptr = &locals[1].arr[2][3];
    arrptr += (locals[2].c & 3);
    result += *arrptr;
    
    /* Volatile access forces memory traffic */
    locals[0].vol = result;
    locals[1].vol = result * 2;
    locals[2].vol = result * 3;
    
    return result;
}

/* Function 4: Pointer chasing and complex indirection */
static __attribute__((noinline))
int test_pointer_chasing(int seed) {
    int buffer[64];
    int *ptr1, *ptr2, *ptr3, *ptr4;
    
    /* Initialize buffer with values */
    for (int i = 0; i < 64; i++) {
        buffer[i] = seed + i * 3;
    }
    
    /* Multiple pointers with complex relationships */
    ptr1 = &buffer[0];
    ptr2 = &buffer[16];
    ptr3 = &buffer[32];
    ptr4 = &buffer[48];
    
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses many intermediate values */
    
    /* Iteration 1 */
    int idx1 = (*ptr1) & 15;
    int idx2 = (*ptr2) & 15;
    int idx3 = (*ptr3) & 15;
    int idx4 = (*ptr4) & 15;
    
    sum += ptr1[idx1] + ptr2[idx2] + ptr3[idx3] + ptr4[idx4];
    
    /* Complex pointer arithmetic */
    ptr1 += (idx1 + idx2) & 7;
    ptr2 += (idx2 + idx3) & 7;
    ptr3 += (idx3 + idx4) & 7;
    ptr4 += (idx4 + idx1) & 7;
    
    /* Iteration 2 */
    idx1 = (*ptr1) & 7;
    idx2 = (*ptr2) & 7;
    idx3 = (*ptr3) & 7;
    idx4 = (*ptr4) & 7;
    
    sum += ptr1[idx1 * 2] + ptr2[idx2 * 2] + ptr3[idx3 * 2] + ptr4[idx4 * 2];
    
    /* More pointer manipulation */
    int *ptr5 = ptr1 + (idx1 * idx2) % 8;
    int *ptr6 = ptr2 + (idx2 * idx3) % 8;
    int *ptr7 = ptr3 + (idx3 * idx4) % 8;
    int *ptr8 = ptr4 + (idx4 * idx1) % 8;
    
    sum += *ptr5 + *ptr6 + *ptr7 + *ptr8;
    
    /* Address of pointer (double indirection) */
    int **pptr1 = &ptr1;
    int **pptr2 = &ptr2;
    
    sum += **pptr1 + **pptr2;
    
    return sum;
}

/* Function 5: Mixed operations with inline asm constraints */
static __attribute__((noinline))
int test_mixed_constraints(int p1, int p2, int p3, int p4) {
    /* Many local variables */
    int v1 = p1, v2 = p2, v3 = p3, v4 = p4;
    int v5 = p1 + p2, v6 = p2 + p3, v7 = p3 + p4, v8 = p4 + p1;
    int v9 = p1 * p2, v10 = p2 * p3, v11 = p3 * p4, v12 = p4 * p1;
    int v13 = v1 + v5, v14 = v2 + v6, v15 = v3 + v7, v16 = v4 + v8;
    int v17 = v9 - v10, v18 = v10 - v11, v19 = v11 - v12, v20 = v12 - v9;
    
    /* Inline asm with memory and register constraints */
    int out1, out2, out3;
    
    /* This asm should trigger various reload types due to constraints */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "addl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "leal (%[in5], %[in6], 2), %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=m" (out1), [out2] "=m" (out2), [out3] "=m" (out3)
        : [in1] "mr" (v1), [in2] "mr" (v5), [in3] "mr" (v9), 
          [in4] "mr" (v13), [in5] "r" (v17), [in6] "r" (v18)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Use all variables in complex expression */
    int result = out1 + out2 + out3;
    result += v2 + v6 + v10 + v14 + v18;
    result += v3 + v7 + v11 + v15 + v19;
    result += v4 + v8 + v12 + v16 + v20;
    
    /* Complex array access with the result */
    int temp_arr[8];
    for (int i = 0; i < 8; i++) {
        temp_arr[i] = result + i * 100;
    }
    
    /* Multi-level indexing */
    result += temp_arr[(v1 + v2) & 7] * temp_arr[(v3 + v4) & 7];
    result += temp_arr[(v5 + v6) & 7] / (temp_arr[(v7 + v8) & 7] + 1);
    
    return result;
}

/* Main driver that calls all test functions */
int main(void) {
    int total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call each test function multiple times with different args */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6);
    total += test_complex_addressing(7, 8, 9, 10, 11, 12);
    
    total += test_asm_reloads(13, 14, 15);
    total += test_asm_reloads(16, 17, 18);
    
    total += test_struct_addressing(19);
    total += test_struct_addressing(20);
    
    total += test_pointer_chasing(21);
    total += test_pointer_chasing(22);
    
    total += test_mixed_constraints(23, 24, 25, 26);
    total += test_mixed_constraints(27, 28, 29, 30);
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000000) {
        return 1;
    }
    
    return 0;
}

#pragma GCC pop_options
