/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct {
        int x, y, z;
    } nested;
    volatile int volatile_member;
};

/* Global arrays to force address computations */
static int global_array1[256];
static int global_array2[256];
static struct BigStruct global_structs[16];

/* Test 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    volatile int v1 = a, v2 = b, v3 = c, v4 = d;
    int i1 = e, i2 = f, i3 = g, i4 = h;
    int j1, j2, j3, j4, j5, j6, j7, j8;
    int k1, k2, k3, k4, k5, k6, k7, k8;
    
    /* Complex array indexing - forces RELOAD_FOR_INPUT_ADDRESS */
    j1 = global_array1[(a + b) & 0xFF];
    j2 = global_array2[(c * d) & 0xFF];
    
    /* Multi-level array indexing with pointer arithmetic */
    int *ptr1 = &global_array1[(a + c) & 0xFF];
    int *ptr2 = &global_array2[(b + d) & 0xFF];
    
    /* Nested addressing - forces RELOAD_FOR_INPADDR_ADDRESS */
    j3 = *(ptr1 + (e * f) / 16);
    j4 = *(ptr2 + (g * h) / 16);
    
    /* Structure member access with offset computation */
    struct BigStruct *sptr = &global_structs[(a + b + c) & 0xF];
    j5 = sptr->arr[(d + e) & 0x7];
    j6 = sptr->nested.x + sptr->nested.y;
    
    /* Volatile access forces memory operations */
    sptr->volatile_member = j1 + j2;
    j7 = sptr->volatile_member;
    
    /* Complex expression with many operands */
    j8 = ((j1 * j2) + (j3 * j4) - (j5 * j6)) / (j7 + 1);
    
    /* More local variables to increase pressure */
    k1 = a * b * c;
    k2 = d * e * f;
    k3 = g * h * a;
    k4 = b * c * d;
    k5 = e * f * g;
    k6 = h * a * b;
    k7 = c * d * e;
    k8 = f * g * h;
    
    /* Use all variables to prevent dead code elimination */
    return j1 + j2 + j3 + j4 + j5 + j6 + j7 + j8 +
           k1 + k2 + k3 + k4 + k5 + k6 + k7 + k8;
}

/* Test 2: Inline assembly with multiple outputs and clobbers */
static __attribute__((noinline))
int test_asm_reloads(int a, int b, int c, int d) {
    int out1, out2, out3, out4;
    int addr1, addr2;
    volatile int mem1, mem2;
    
    /* Inline asm with memory output - forces RELOAD_FOR_OUTPUT_ADDRESS */
    __asm__ volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[mem1]\n\t"
        : [out1] "=r" (out1), [mem1] "=m" (mem1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    /* More complex asm with multiple constraints */
    __asm__ volatile (
        "imull %[in3], %[in4]\n\t"
        "movl %%eax, %[out2]\n\t"
        "leal (%[out2], %[in1], 4), %[out3]\n\t"
        : [out2] "=r" (out2), [out3] "=r" (out3)
        : [in3] "r" (c), [in4] "r" (d), [in1] "r" (a)
        : "eax", "cc"
    );
    
    /* Asm with address computation - forces RELOAD_FOR_OPERAND_ADDRESS */
    int *ptr = &global_array1[a & 0xFF];
    __asm__ volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[ptr])\n\t"
        : 
        : [ptr] "r" (ptr)
        : "eax", "memory", "cc"
    );
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(out1 > out2, 0)) {
        addr1 = (int)(&global_array2[b & 0xFF]);
        addr2 = (int)(&global_structs[c & 0xF]);
        out4 = addr1 + addr2;
    } else {
        out4 = out1 * out2 * out3;
    }
    
    return out1 + out2 + out3 + out4 + mem1;
}

/* Test 3: Pointer chasing and complex memory operands */
static __attribute__((noinline))
int test_pointer_chasing(int seed) {
    /* Create many pointer variables */
    int *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
    int **pp1, **pp2, **pp3;
    volatile int *vp1, *vp2;
    
    /* Initialize pointers with complex expressions */
    p1 = &global_array1[(seed + 1) & 0xFF];
    p2 = &global_array2[(seed * 2) & 0xFF];
    p3 = p1 + (seed & 0xF);
    p4 = p2 + ((seed >> 4) & 0xF);
    
    /* Pointer to pointer - forces RELOAD_FOR_OPADDR_ADDR */
    pp1 = &p1;
    pp2 = &p2;
    
    /* Volatile pointers force reloads */
    vp1 = (volatile int *)p3;
    vp2 = (volatile int *)p4;
    
    /* Complex pointer arithmetic */
    p5 = *pp1 + (**pp2);
    p6 = *pp2 + (**pp1);
    
    /* Memory access through multiple levels */
    pp3 = &p5;
    p7 = *pp3 + (seed & 0x7);
    p8 = p7 - (seed & 0x3);
    
    /* Access through all pointers */
    int sum = *p1 + *p2 + *p3 + *p4 + *p5 + *p6 + *p7 + *p8;
    *vp1 = sum;
    *vp2 = sum >> 1;
    
    /* More complex addressing modes */
    sum += *(p1 + *p2 / 1024);
    sum += *(p2 + *p1 / 1024);
    sum += *(p3 + *p4 / 1024);
    sum += *(p4 + *p3 / 1024);
    
    return sum;
}

/* Test 4: Mixed types and manual loop unrolling */
static __attribute__((noinline))
int test_mixed_operands(int iterations) {
    int results[8];
    volatile int temp[8];
    struct BigStruct local_struct;
    
    /* Initialize local struct */
    local_struct.a = iterations;
    local_struct.b = iterations * 2;
    local_struct.c = iterations * 3;
    local_struct.d = iterations * 4;
    for (int i = 0; i < 8; i++) {
        local_struct.arr[i] = iterations + i;
    }
    
    /* Manual unrolling to increase register pressure */
    /* Unrolled loop iteration 1 */
    {
        int idx1 = (iterations + 0) & 0xFF;
        int idx2 = (iterations * 2) & 0xFF;
        int *addr1 = &global_array1[idx1];
        int *addr2 = &global_array2[idx2];
        results[0] = *addr1 + *addr2 + local_struct.arr[0];
        temp[0] = results[0];
    }
    
    /* Unrolled loop iteration 2 */
    {
        int idx1 = (iterations + 1) & 0xFF;
        int idx2 = (iterations * 3) & 0xFF;
        struct BigStruct *sptr = &global_structs[idx1 & 0xF];
        results[1] = sptr->a + sptr->b + local_struct.arr[1];
        temp[1] = results[1];
    }
    
    /* Continue with more unrolled iterations... */
    for (int i = 2; i < 8; i++) {
        int idx = (iterations + i) & 0xFF;
        results[i] = global_array1[idx] + global_array2[idx] + local_struct.arr[i];
        temp[i] = results[i];
    }
    
    /* Complex expression using all results */
    int final = 0;
    for (int i = 0; i < 8; i++) {
        final += results[i] * (i + 1);
        final -= temp[i] / (i + 2);
    }
    
    return final;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array1[i] = i;
        global_array2[i] = 255 - i;
    }
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        global_structs[i].nested.x = i;
        global_structs[i].nested.y = i * 2;
        global_structs[i].nested.z = i * 3;
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i + j;
        }
    }
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_asm_reloads(9, 10, 11, 12);
    total += test_pointer_chasing(13);
    total += test_mixed_operands(14);
    
    /* Call multiple times with different values */
    for (int i = 0; i < 4; i++) {
        total += test_complex_addressing(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        total += test_asm_reloads(i+8, i+9, i+10, i+11);
        total += test_pointer_chasing(i+12);
        total += test_mixed_operands(i+13);
    }
    
    /* Use the result to prevent optimization */
    volatile int sink = total;
    return sink > 0 ? 0 : 1;
}

#pragma GCC pop_options
