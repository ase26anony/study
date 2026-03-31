/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* ========== Test Function 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static int __attribute__((noinline)) 
test_array_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = g & h;
    volatile int v5 = a | c;
    volatile int v6 = b ^ d;
    volatile int v7 = e + g;
    volatile int v8 = f * h;
    
    /* Multi-dimensional array with complex indexing */
    int arr3d[4][4][4];
    int arr2d[8][8];
    
    /* Initialize arrays to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                arr3d[i][j][k] = i * 16 + j * 4 + k;
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr2d[i][j] = i * 8 + j;
        }
    }
    
    /* Complex addressing expressions that need address reloads */
    int result = 0;
    
    /* Nested addressing requiring input address reloads */
    result += arr3d[v1 & 3][v2 & 3][v3 & 3];
    result += arr3d[v4 & 3][v5 & 3][v6 & 3];
    
    /* More complex addressing with computations */
    result += arr2d[(v1 + v2) & 7][(v3 + v4) & 7];
    result += arr2d[(v5 + v6) & 7][(v7 + v8) & 7];
    
    /* Address of address computation (inpaddr address) */
    int *ptr1 = &arr3d[v1 & 3][v2 & 3][v3 & 3];
    int *ptr2 = &arr2d[(v4 + v5) & 7][(v6 + v7) & 7];
    
    result += *ptr1 + *ptr2;
    
    return result;
}

/* ========== Test Function 2: Structure and Inline Assembly ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
struct ComplexStruct {
    int data[8];
    struct {
        int x, y, z;
    } coords[4];
    volatile int flags;
};

static int __attribute__((noinline))
test_struct_asm(struct ComplexStruct *s1, struct ComplexStruct *s2, 
                int idx1, int idx2, int idx3, int idx4) {
    volatile int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Many structure accesses with complex addressing */
    temp1 = s1->data[idx1] + s2->data[idx2];
    temp2 = s1->coords[idx3].x * s2->coords[idx4].y;
    temp3 = s1->coords[idx2].z - s2->coords[idx1].x;
    
    /* Inline assembly with multiple outputs and memory operands */
    /* This should trigger output address reloads */
    int out1, out2;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]"
        : [out1] "=m" (s1->data[idx1]),  /* Memory output - needs address reload */
          [out2] "=m" (s2->coords[idx3].x) /* Another memory output */
        : [in1] "r" (temp1),
          [in2] "r" (temp2),
          [in3] "r" (temp3)
        : "%eax", "memory", "cc"
    );
    
    /* More inline asm with complex address computations */
    int * volatile ptr = &s1->data[idx4];
    asm volatile (
        "movl (%[ptr]), %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, (%[ptr])"
        : 
        : [ptr] "r" (ptr)
        : "%ebx", "memory"
    );
    
    /* Operand address reloads */
    int (* volatile funcptr)(int) = (int (*)(int))&test_array_addressing;
    volatile int indirect_result = funcptr(idx1);
    
    return s1->data[idx1] + s2->coords[idx3].x + indirect_result;
}

/* ========== Test Function 3: Pointer Chasing and Volatile ========== */
/* Targets: RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
static int __attribute__((noinline))
test_pointer_chasing(int seed, int iterations) {
    /* Create many pointer variables */
    volatile int *ptr_array[16];
    int data_blocks[16][8];
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            data_blocks[i][j] = (i * 8 + j) ^ seed;
        }
        ptr_array[i] = &data_blocks[i][0];
    }
    
    /* Pointer chasing with many intermediate values */
    volatile int *current = ptr_array[0];
    int sum = 0;
    
    /* Unrolled loop to increase register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations && i < 16; i++) {
        volatile int *next1 = ptr_array[(i + 1) & 15];
        volatile int *next2 = ptr_array[(i + 2) & 15];
        volatile int *next3 = ptr_array[(i + 3) & 15];
        
        /* Complex pointer arithmetic */
        int offset1 = (i * 3) & 7;
        int offset2 = (i * 5) & 7;
        int offset3 = (i * 7) & 7;
        
        /* Multiple memory accesses with address computations */
        sum += current[offset1];
        sum += next1[offset2];
        sum += next2[offset3];
        
        /* Address of pointer (opaddr addr) */
        volatile int **ptr_to_ptr = &current;
        *ptr_to_ptr = next3;
        
        /* Mix with other operations */
        sum += offset1 * offset2 - offset3;
    }
    
    /* Force other address reloads with obscure pattern */
    int (* volatile array_of_funcs[4])(int, int, int, int, int, int, int, int) = {
        test_array_addressing,
        test_array_addressing,
        test_array_addressing,
        test_array_addressing
    };
    
    /* Call through function pointer array with many args */
    sum += array_of_funcs[sum & 3](sum, sum>>1, sum>>2, sum>>3, 
                                   sum>>4, sum>>5, sum>>6, sum>>7);
    
    return sum;
}

/* ========== Test Function 4: Mixed Everything ========== */
/* Attempts to trigger any remaining reload types */
static int __attribute__((noinline))
test_mixed_reloads(double d1, double d2, float f1, float f2) {
    /* Mix float and int operations to use different register sets */
    volatile float farr[8];
    volatile double darr[4];
    volatile int iarr[12];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) farr[i] = f1 * i + f2;
    for (int i = 0; i < 4; i++) darr[i] = d1 * i - d2;
    for (int i = 0; i < 12; i++) iarr[i] = i * 3;
    
    /* Complex expression mixing everything */
    int result = 0;
    
    /* Force spills and reloads with many live values */
    float ftemp1 = farr[0] + farr[1];
    float ftemp2 = farr[2] * farr[3];
    double dtemp1 = darr[0] / darr[1];
    double dtemp2 = darr[2] - darr[3];
    int itemp1 = iarr[0] ^ iarr[1];
    int itemp2 = iarr[2] & iarr[3];
    int itemp3 = iarr[4] | iarr[5];
    int itemp4 = iarr[6] + iarr[7];
    int itemp5 = iarr[8] - iarr[9];
    int itemp6 = iarr[10] * iarr[11];
    
    /* Use all temporaries in complex address computation */
    volatile int * volatile ptr1 = &iarr[(int)ftemp1 & 11];
    volatile int * volatile ptr2 = &iarr[(int)ftemp2 & 11];
    volatile int * volatile ptr3 = &iarr[((itemp1 + itemp2) * itemp3) & 11];
    
    /* Multiple indirections */
    result += *ptr1 + *ptr2 + *ptr3;
    result += (int)(dtemp1 * 100) + (int)(dtemp2 * 100);
    result += itemp4 * itemp5 / (itemp6 ? itemp6 : 1);
    
    /* Inline asm with many clobbers to force register spills */
    asm volatile (
        "movl %[val1], %%eax\n\t"
        "movl %[val2], %%ebx\n\t"
        "movl %[val3], %%ecx\n\t"
        "movl %[val4], %%edx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (iarr[0])
        : [val1] "r" (itemp1),
          [val2] "r" (itemp2),
          [val3] "r" (itemp3),
          [val4] "r" (itemp4)
        : "%eax", "%ebx", "%ecx", "%edx", "memory", "cc"
    );
    
    return result + iarr[0];
}

/* ========== Main Driver ========== */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize test data */
    struct ComplexStruct s1, s2;
    for (int i = 0; i < 8; i++) {
        s1.data[i] = i * 2;
        s2.data[i] = i * 3;
    }
    for (int i = 0; i < 4; i++) {
        s1.coords[i].x = i;
        s1.coords[i].y = i * 2;
        s1.coords[i].z = i * 3;
        s2.coords[i].x = i * 4;
        s2.coords[i].y = i * 5;
        s2.coords[i].z = i * 6;
    }
    
    /* Call all test functions with different arguments */
    result += test_array_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_array_addressing(9, 10, 11, 12, 13, 14, 15, 16);
    
    result += test_struct_asm(&s1, &s2, 1, 2, 3, 0);
    result += test_struct_asm(&s2, &s1, 0, 3, 2, 1);
    
    result += test_pointer_chasing(result, 8);
    result += test_pointer_chasing(result ^ 0x55AA, 6);
    
    result += test_mixed_reloads(1.5, 2.5, 3.0f, 4.0f);
    result += test_mixed_reloads(5.5, 6.5, 7.0f, 8.0f);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result % 256;
}

#pragma GCC pop_options
