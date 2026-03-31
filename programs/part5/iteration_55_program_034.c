/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure to create complex addressing modes */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int *volatile_ptr;
    int matrix[3][3];
};

/* Global arrays to create register pressure through spilling */
volatile int global_array[256];
static struct NestedData nested_array[4][4];

/* ========== TEST FUNCTION 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
__attribute__((noinline, optimize("O0")))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4;
    int i5 = e + 5, i6 = f + 6, i7 = g + 7, i8 = h + 8;
    volatile int v1 = i1, v2 = i2, v3 = i3, v4 = i4;
    int *ptr1 = &i1, *ptr2 = &i2, *ptr3 = &i3, *ptr4 = &i4;
    
    /* Complex multi-level array indexing - forces address reloads */
    int result = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each of these creates different addressing modes */
    result += nested_array[i1 & 3][i2 & 3].values[(i3 + i4) & 7];
    result += nested_array[i2 & 3][i3 & 3].values[(i4 + i5) & 7];
    result += nested_array[i3 & 3][i4 & 3].values[(i5 + i6) & 7];
    result += nested_array[i4 & 3][i5 & 3].values[(i6 + i7) & 7];
    result += nested_array[i5 & 3][i6 & 3].values[(i7 + i8) & 7];
    result += nested_array[i6 & 3][i7 & 3].values[(i8 + i1) & 7];
    
    /* Structure pointer chasing with volatile - forces operand address reloads */
    struct NestedData *current = &nested_array[0][0];
    volatile int *volptr = current->volatile_ptr;
    
    /* Complex address computation that needs reloading */
    result += *(volptr + (i1 * i2 + i3 * i4) / 16);
    result += current->matrix[i1 & 2][i2 & 2];
    
    /* More volatile accesses to force spills */
    v1 = result;
    v2 = result * 2;
    v3 = result * 3;
    v4 = result * 4;
    
    return result + v1 + v2 + v3 + v4;
}

/* ========== TEST FUNCTION 2: Inline Assembly with Multiple Outputs ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
__attribute__((noinline, optimize("O1")))
static int test_asm_reloads(int x, int y, int z) {
    int out1, out2, out3, out4, out5, out6, out7, out8;
    volatile int mem1, mem2, mem3, mem4;
    int *ptr_arr[8];
    
    /* Initialize pointer array with complex addresses */
    ptr_arr[0] = &global_array[x];
    ptr_arr[1] = &global_array[y];
    ptr_arr[2] = &global_array[z];
    ptr_arr[3] = &global_array[x + y];
    ptr_arr[4] = &global_array[y + z];
    ptr_arr[5] = &global_array[z + x];
    ptr_arr[6] = &global_array[x * y];
    ptr_arr[7] = &global_array[y * z];
    
    /* Inline assembly with multiple outputs and memory operands */
    /* This forces output address reloads */
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out2]\n\t"
        "movl %[out2], (%[memptr1])\n\t"  /* Memory output - needs address reload */
        "imull %[in3], %[out3]\n\t"
        "movl %[out3], (%[memptr2])\n\t"  /* Another memory output */
        "leal (%[in1],%[in2],4), %[out4]\n\t"
        "movl %[out4], (%[memptr3])\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), "=m" (mem1), "=m" (mem2), "=m" (mem3)
        : [in1] "r" (x), [in2] "r" (y), [in3] "r" (z),
          [memptr1] "r" (ptr_arr[0]), [memptr2] "r" (ptr_arr[1]),
          [memptr3] "r" (ptr_arr[2])
        : "memory", "cc"
    );
    
    /* More complex asm with alternative constraints */
    int temp1, temp2, temp3;
    asm volatile (
        "movl $0x%0, %1\n\t"
        "addl %%eax, %1\n\t"
        "movl %1, %2\n\t"
        : "=r" (temp1), "=&r" (temp2), "=m" (mem4)
        : "0" (x + y), "a" (z)
        : "cc"
    );
    
    /* Use results to prevent dead code elimination */
    return out1 + out2 + out3 + out4 + mem1 + mem2 + mem3 + mem4 + temp1 + temp2;
}

/* ========== TEST FUNCTION 3: Pointer Chasing and Operand Address Reloads ========== */
/* Targets: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
__attribute__((noinline))
static int test_pointer_chasing(int seed) {
    /* Create many local pointers to consume registers */
    int *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
    int **pp1, **pp2, **pp3;
    volatile int *vp1, *vp2;
    
    int arr1[16], arr2[16], arr3[16], arr4[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr1[i] = seed + i;
        arr2[i] = seed * i;
        arr3[i] = seed - i;
        arr4[i] = seed ^ i;
    }
    
    /* Complex pointer assignments */
    p1 = &arr1[(seed + 1) & 15];
    p2 = &arr2[(seed + 2) & 15];
    p3 = &arr3[(seed + 3) & 15];
    p4 = &arr4[(seed + 4) & 15];
    p5 = p1 + (seed & 7);
    p6 = p2 + ((seed >> 1) & 7);
    p7 = p3 + ((seed >> 2) & 7);
    p8 = p4 + ((seed >> 3) & 7);
    
    pp1 = &p1;
    pp2 = &p2;
    pp3 = &p3;
    
    vp1 = (volatile int *)p5;
    vp2 = (volatile int *)p6;
    
    /* Pointer chasing with volatile - forces operand address reloads */
    int sum = 0;
    
    /* Unrolled loop with complex addressing */
    sum += **pp1;  /* Needs operand address reload */
    sum += **pp2;
    sum += **pp3;
    
    sum += *(*pp1 + 1);  /* More complex address computation */
    sum += *(*pp2 + 2);
    sum += *(*pp3 + 3);
    
    /* Volatile accesses with address computation */
    sum += *(vp1 + (*p1 & 3));  /* Address depends on register value */
    sum += *(vp2 + (*p2 & 3));
    
    /* Complex expression mixing pointers and arrays */
    sum += arr1[*(p1 + (*p2 & 3)) & 15];
    sum += arr2[*(p2 + (*p3 & 3)) & 15];
    sum += arr3[*(p3 + (*p4 & 3)) & 15];
    
    /* Force spills with many live values */
    volatile int spill1 = *p1, spill2 = *p2, spill3 = *p3, spill4 = *p4;
    volatile int spill5 = *p5, spill6 = *p6, spill7 = *p7, spill8 = *p8;
    
    return sum + spill1 + spill2 + spill3 + spill4 + spill5 + spill6 + spill7 + spill8;
}

/* ========== TEST FUNCTION 4: Mixed Reload Types with Builtins ========== */
__attribute__((noinline, optimize("O2")))
static int test_mixed_reloads(int a, int b, int c, int d) {
    /* Use __builtin_expect to create data dependencies */
    int x = __builtin_expect(a, 1) + __builtin_expect(b, 0);
    int y = __builtin_expect(c, 1) * __builtin_expect(d, 0);
    int z = __builtin_expect(a * b, c * d);
    
    /* Small 2D array for complex indexing */
    int matrix[4][4] = {
        {a, b, c, d},
        {b, c, d, a},
        {c, d, a, b},
        {d, a, b, c}
    };
    
    /* Complex addressing with multiple dimensions */
    int sum = 0;
    
    /* Manual unrolling with varying index computations */
    sum += matrix[x & 3][y & 3];
    sum += matrix[y & 3][z & 3];
    sum += matrix[z & 3][x & 3];
    sum += matrix[(x+y) & 3][(y+z) & 3];
    sum += matrix[(y+z) & 3][(z+x) & 3];
    sum += matrix[(z+x) & 3][(x+y) & 3];
    
    /* Address of element used in computation */
    int *elem_ptr = &matrix[(x*y) & 3][(y*z) & 3];
    sum += *elem_ptr;
    sum += *(elem_ptr + ((x+z) & 3));
    
    /* Force address computation into register */
    int (*mat_ptr)[4] = &matrix[(a+b) & 3];
    sum += (*mat_ptr)[(c+d) & 3];
    
    /* Volatile store to force memory traffic */
    volatile int vs1 = sum;
    volatile int vs2 = sum * 2;
    volatile int vs3 = sum * 3;
    
    /* Complex expression that uses all variables */
    return vs1 + vs2 + vs3 + x + y + z + a + b + c + d;
}

/* ========== MAIN DRIVER FUNCTION ========== */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                nested_array[i][j].values[k] = i * 100 + j * 10 + k;
            }
            nested_array[i][j].next = &nested_array[(i+1)&3][(j+1)&3];
            nested_array[i][j].volatile_ptr = &global_array[i * 16 + j * 4];
            for (int m = 0; m < 3; m++) {
                for (int n = 0; n < 3; n++) {
                    nested_array[i][j].matrix[m][n] = i * m + j * n;
                }
            }
        }
    }
    
    int result = 0;
    
    /* Call test functions with different arguments to trigger various reloads */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_complex_addressing(8, 7, 6, 5, 4, 3, 2, 1);
    
    result += test_asm_reloads(10, 20, 30);
    result += test_asm_reloads(40, 50, 60);
    
    result += test_pointer_chasing(100);
    result += test_pointer_chasing(200);
    result += test_pointer_chasing(300);
    
    result += test_mixed_reloads(5, 6, 7, 8);
    result += test_mixed_reloads(9, 10, 11, 12);
    result += test_mixed_reloads(13, 14, 15, 16);
    
    /* Use result to prevent dead code elimination */
    return result & 0xFF;
}

#pragma GCC pop_options
