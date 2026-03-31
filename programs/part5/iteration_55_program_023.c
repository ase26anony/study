/* Test program to trigger various reload types in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Force register pressure by using many variables */
#define FORCE_REGISTER_PRESSURE \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9; \
    volatile int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9; \
    volatile int y0, y1, y2, y3, y4, y5, y6, y7, y8, y9; \
    volatile int z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;

/* Complex structure to force address computations */
struct Nested {
    int data[4][4];
    struct Nested *next;
    volatile int flags;
};

/* Function 1: Complex array addressing to trigger input address reloads */
static int __attribute__((noinline)) 
test_complex_addressing(int *base, int index1, int index2, int index3) 
{
    FORCE_REGISTER_PRESSURE
    
    /* Multi-dimensional array access forcing address computation */
    int arr[8][8][8];
    
    /* Complex addressing that may need RELOAD_FOR_INPUT_ADDRESS */
    int result = arr[index1][index2][index3] + 
                 arr[index3][index1][index2] + 
                 arr[index2][index3][index1];
    
    /* More complex addressing with pointer arithmetic */
    int *ptr1 = &arr[index1][index2][0];
    int *ptr2 = &arr[index2][index3][0];
    int *ptr3 = &arr[index3][index1][0];
    
    /* Nested addressing requiring RELOAD_FOR_INPADDR_ADDRESS */
    result += *(ptr1 + index3) + *(ptr2 + index1) + *(ptr3 + index2);
    
    /* Force all volatiles to be used */
    v0 = index1; v1 = index2; v2 = index3;
    v3 = result; v4 = (int)ptr1; v5 = (int)ptr2;
    
    return result + v0 + v1 + v2 + v3 + v4 + v5;
}

/* Function 2: Structure access with inline assembly to trigger output address reloads */
static int __attribute__((noinline))
test_struct_asm(struct Nested *s1, struct Nested *s2, struct Nested *s3)
{
    FORCE_REGISTER_PRESSURE
    
    int sum = 0;
    
    /* Complex structure member access */
    sum += s1->data[0][0] + s1->data[1][1] + s1->data[2][2] + s1->data[3][3];
    sum += s2->data[0][3] + s2->data[1][2] + s2->data[2][1] + s2->data[3][0];
    
    /* Inline assembly with memory output - may trigger RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=m" (s3->data[1][2])  /* Memory output */
        : "r" (sum), "r" (s1->flags)
        : "memory"
    );
    
    /* More complex addressing with structure pointers */
    volatile int *addr1 = &s1->data[2][3];
    volatile int *addr2 = &s2->data[3][2];
    volatile int *addr3 = &s3->data[1][1];
    
    /* Pointer chasing - may trigger RELOAD_FOR_OPERAND_ADDRESS */
    sum += *addr1 + *addr2 + *addr3;
    
    /* Use all volatile variables */
    w0 = sum; w1 = (int)s1; w2 = (int)s2; w3 = (int)s3;
    w4 = *addr1; w5 = *addr2; w6 = *addr3;
    
    return sum + w0 + w1 + w2 + w3 + w4 + w5 + w6;
}

/* Function 3: Multiple outputs and complex expressions */
static int __attribute__((noinline))
test_multiple_outputs(int a, int b, int c, int d, int e, int f)
{
    FORCE_REGISTER_PRESSURE
    
    /* Many intermediate computations to use registers */
    int t1 = a * b + c;
    int t2 = b * c + d;
    int t3 = c * d + e;
    int t4 = d * e + f;
    int t5 = e * f + a;
    int t6 = f * a + b;
    
    /* Complex expression with many operands */
    int result = (t1 * t2) + (t3 * t4) - (t5 * t6) +
                 (a << 2) + (b << 3) - (c << 1) +
                 (d >> 2) * (e >> 1) / (f + 1);
    
    /* Inline assembly with multiple outputs */
    int out1, out2, out3;
    asm volatile (
        "movl %3, %0\n\t"
        "imull %4, %0\n\t"
        "movl %5, %1\n\t"
        "addl %6, %1\n\t"
        "movl %0, %2\n\t"
        "addl %1, %2\n\t"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (t1), "r" (t2), "r" (t3), "r" (t4)
        : "cc"
    );
    
    /* Use results */
    result += out1 + out2 + out3;
    
    /* Force volatile usage */
    x0 = a; x1 = b; x2 = c; x3 = d; x4 = e; x5 = f;
    x6 = t1; x7 = t2; x8 = t3; x9 = t4;
    
    return result + x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
}

/* Function 4: Pointer arithmetic and memory indirection */
static int __attribute__((noinline))
test_pointer_chasing(int *base, int size)
{
    FORCE_REGISTER_PRESSURE
    
    volatile int *ptr_array[10];
    int sum = 0;
    
    /* Setup pointer array with complex addressing */
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = base + (i * size) / 4;
    }
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different pointer computations */
    sum += *ptr_array[0] + *(ptr_array[1] + 1) + *(ptr_array[2] + 2);
    sum += *ptr_array[3] + *(ptr_array[4] + 3) + *(ptr_array[5] + 4);
    sum += *ptr_array[6] + *(ptr_array[7] + 5) + *(ptr_array[8] + 6);
    sum += *ptr_array[9] + *(ptr_array[0] + 7) + *(ptr_array[1] + 8);
    
    /* Complex pointer expression - may trigger RELOAD_FOR_OPADDR_ADDR */
    int **pptr = &ptr_array[2];
    sum += **pptr + **(pptr + 1) + **(pptr + 2);
    
    /* More complex indirection */
    volatile int * volatile *vpptr = ptr_array;
    sum += *vpptr[0] + *vpptr[3] + *vpptr[6];
    
    y0 = sum; y1 = (int)base; y2 = size;
    for (int i = 0; i < 10; i++) {
        y3 += (int)ptr_array[i];
    }
    
    return sum + y0 + y1 + y2 + y3;
}

/* Function 5: Mixed operations with inline assembly constraints */
static int __attribute__((noinline))
test_mixed_reloads(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
    FORCE_REGISTER_PRESSURE
    
    /* Use all parameters in complex ways */
    int a = p1 * p2 - p3;
    int b = p4 / (p5 + 1) + p6;
    int c = p7 << (p8 & 3);
    int d = (p1 ^ p2) | (p3 & p4);
    int e = p5 * p6 - p7 * p8;
    int f = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
    
    /* Inline assembly with memory input and output */
    int memory_buffer[16];
    int result;
    
    /* Complex asm with multiple constraints - may trigger RELOAD_OTHER */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        "movl 4(%1), %%ebx\n\t"
        "subl 4(%2), %%ebx\n\t"
        "movl %%ebx, 4(%0)\n\t"
        : 
        : "r" (memory_buffer), "r" (&a), "r" (&b)
        : "%eax", "%ebx", "memory"
    );
    
    /* Another asm with alternative constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "rorl $3, %0\n\t"
        : "=r,r,m" (result)
        : "r,m,r" (c)
        : "cc"
    );
    
    /* Use all the computed values */
    z0 = a; z1 = b; z2 = c; z3 = d; z4 = e; z5 = f;
    z6 = result;
    for (int i = 0; i < 8; i++) {
        z7 += memory_buffer[i];
    }
    
    return a + b + c + d + e + f + result + z0 + z1 + z2 + z3 + z4 + z5 + z6 + z7;
}

/* Main driver function */
int main(int argc, char *argv[])
{
    /* Initialize test data */
    int array_data[100];
    for (int i = 0; i < 100; i++) {
        array_data[i] = i * 3 + 1;
    }
    
    struct Nested s1, s2, s3;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            s1.data[i][j] = i * 4 + j;
            s2.data[i][j] = i * 4 + (3 - j);
            s3.data[i][j] = (i + j) * 2;
        }
    }
    s1.flags = 1; s2.flags = 2; s3.flags = 3;
    
    int total = 0;
    
    /* Call all test functions with different patterns */
    total += test_complex_addressing(array_data, 1, 2, 3);
    total += test_complex_addressing(array_data, 4, 5, 6);
    
    total += test_struct_asm(&s1, &s2, &s3);
    total += test_struct_asm(&s2, &s3, &s1);
    
    total += test_multiple_outputs(1, 2, 3, 4, 5, 6);
    total += test_multiple_outputs(7, 8, 9, 10, 11, 12);
    
    total += test_pointer_chasing(array_data, 16);
    total += test_pointer_chasing(array_data + 20, 8);
    
    total += test_mixed_reloads(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_mixed_reloads(9, 10, 11, 12, 13, 14, 15, 16);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return total % 256;
    }
    return 0;
}
