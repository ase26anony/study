/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force register pressure by using many variables */
#define FORCE_REGISTER_PRESSURE \
    volatile int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9; \
    volatile int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19; \
    volatile int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;

/* Complex addressing patterns to trigger address reloads */
typedef struct {
    int data[8];
    int* ptr;
    struct {
        int x;
        int y;
        int z;
    } coord;
} ComplexStruct;

/* Test function 1: Complex array addressing with nested indices */
__attribute__((noinline))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    FORCE_REGISTER_PRESSURE
    
    /* Multi-dimensional array with volatile to force memory accesses */
    volatile int arr3d[4][4][4];
    volatile int arr2d[8][8];
    
    /* Many index variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4;
    int i5 = e + 5, i6 = f + 6, i7 = a * b, i8 = c * d;
    int i9 = e * f, i10 = a + b + c, i11 = d + e + f;
    
    /* Complex addressing patterns that need address reloads */
    int result = 0;
    
    /* RELOAD_FOR_INPUT_ADDRESS: address computation for input */
    result += arr3d[i1][i2][i3];
    result += arr3d[i4][i5][i6];
    
    /* More complex addressing with multiple computations */
    result += arr2d[i7 + i8][i9 + i10];
    result += arr2d[i1 * i2][i3 * i4];
    
    /* Nested addressing requiring temporary address registers */
    result += arr3d[arr2d[i1][i2] & 3][arr2d[i3][i4] & 3][arr2d[i5][i6] & 3];
    
    return result;
}

/* Test function 2: Structure access with pointer chasing */
__attribute__((noinline))
static int test_structure_access(int seed) {
    FORCE_REGISTER_PRESSURE
    
    ComplexStruct cs1, cs2, cs3, cs4;
    ComplexStruct* ptrs[4] = {&cs1, &cs2, &cs3, &cs4};
    
    /* Initialize structures */
    for (int i = 0; i < 8; i++) {
        cs1.data[i] = seed + i;
        cs2.data[i] = seed * i;
        cs3.data[i] = seed - i;
        cs4.data[i] = seed ^ i;
    }
    
    cs1.coord.x = seed;
    cs1.coord.y = seed * 2;
    cs1.coord.z = seed * 3;
    
    cs2.coord.x = seed + 1;
    cs2.coord.y = seed + 2;
    cs2.coord.z = seed + 3;
    
    /* Complex structure member access patterns */
    int sum = 0;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: pointer needs reloading before dereference */
    sum += ptrs[0]->data[ptrs[1]->coord.x & 7];
    sum += ptrs[1]->data[ptrs[2]->coord.y & 7];
    sum += ptrs[2]->data[ptrs[3]->coord.z & 7];
    
    /* More complex: pointer arithmetic with structure offsets */
    sum += ((int*)ptrs[0])[ptrs[1]->coord.x] + ((int*)ptrs[1])[ptrs[2]->coord.y];
    
    /* RELOAD_FOR_INPADDR_ADDRESS: address of address computation */
    int* temp_ptr = &ptrs[3]->data[ptrs[0]->coord.x & 7];
    sum += *temp_ptr;
    
    return sum;
}

/* Test function 3: Inline assembly with multiple outputs */
__attribute__((noinline))
static int test_inline_asm(int a, int b, int c, int d) {
    FORCE_REGISTER_PRESSURE
    
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3, mem4;
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: complex address for output operand */
    mem1 = a;
    mem2 = b;
    mem3 = c;
    mem4 = d;
    
    /* Inline asm with multiple outputs and clobbers to increase register pressure */
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %%eax, %%ebx\n\t"
        "movl %%ebx, %[out3]\n\t"
        "leal (%%eax, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %[out4]"
        : [out1] "=m" (out1), [out2] "=m" (out2), 
          [out3] "=m" (out3), [out4] "=m" (out4)
        : [in1] "rm" (mem1), [in2] "rm" (mem2),
          [in3] "rm" (mem3), [in4] "rm" (mem4)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: output address needs reload */
    volatile int* output_array[4] = {&out1, &out2, &out3, &out4};
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += *output_array[i];
    }
    
    return result;
}

/* Test function 4: Mixed operations with unrolled loops */
__attribute__((noinline))
static int test_mixed_operations(int iterations) {
    FORCE_REGISTER_PRESSURE
    
    volatile int array[16];
    volatile int indices[8];
    int accumulators[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        array[i] = i * iterations;
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = (i * 3) & 0xF;
        accumulators[i] = 0;
    }
    
    /* Manually unrolled loop to increase register pressure */
    /* Each iteration uses different addressing patterns */
    
    /* Iteration 1: Simple array access */
    accumulators[0] += array[indices[0]];
    accumulators[1] += array[indices[1]];
    
    /* Iteration 2: More complex addressing */
    accumulators[2] += array[(indices[2] + indices[3]) & 0xF];
    accumulators[3] += array[(indices[4] * indices[5]) & 0xF];
    
    /* Iteration 3: Nested addressing */
    accumulators[4] += array[array[indices[6]] & 0xF];
    accumulators[5] += array[array[indices[7]] & 0xF];
    
    /* Iteration 4: Mixed operations */
    accumulators[6] = (accumulators[0] + accumulators[2]) * 
                      (accumulators[1] - accumulators[3]);
    accumulators[7] = (accumulators[4] | accumulators[5]) ^ 
                      (accumulators[6] & 0xFF);
    
    /* RELOAD_FOR_OPADDR_ADDR: operand address needs reload */
    int* addr_array[8];
    for (int i = 0; i < 8; i++) {
        addr_array[i] = &accumulators[i];
    }
    
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        final_result += *addr_array[i];
    }
    
    return final_result;
}

/* Test function 5: Pointer arithmetic and complex expressions */
__attribute__((noinline))
static int test_pointer_arithmetic(int base) {
    FORCE_REGISTER_PRESSURE
    
    volatile int data_block[64];
    volatile int* ptrs[8];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        data_block[i] = base + i;
    }
    
    /* Create pointer array with complex relationships */
    ptrs[0] = &data_block[0];
    ptrs[1] = &data_block[8];
    ptrs[2] = &data_block[16];
    ptrs[3] = &data_block[24];
    ptrs[4] = &data_block[32];
    ptrs[5] = &data_block[40];
    ptrs[6] = &data_block[48];
    ptrs[7] = &data_block[56];
    
    int sum = 0;
    
    /* Complex pointer arithmetic that needs multiple reloads */
    sum += *(ptrs[0] + (ptrs[1] - ptrs[0]));
    sum += *(ptrs[2] + ((ptrs[3] - ptrs[2]) >> 1));
    
    /* RELOAD_FOR_OTHER_ADDRESS: other address computations */
    int offset = (ptrs[4] - ptrs[0]) / sizeof(int);
    sum += *(ptrs[0] + offset);
    
    /* More complex: pointer to pointer dereference */
    volatile int** pptr = &ptrs[5];
    sum += **pptr;
    
    /* Chain of pointer accesses */
    sum += *(*(&ptrs[6]) + 2);
    
    return sum;
}

/* Main driver function */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Call all test functions with different arguments to ensure
       they're not optimized away and to trigger different paths */
    
    /* Test 1: Complex array addressing */
    result += test_complex_addressing(argc, 1, 2, 3, 4, 5);
    
    /* Test 2: Structure access */
    result += test_structure_access(argc + 10);
    
    /* Test 3: Inline assembly */
    result += test_inline_asm(argc, argc * 2, argc * 3, argc * 4);
    
    /* Test 4: Mixed operations */
    result += test_mixed_operations(argc + 20);
    
    /* Test 5: Pointer arithmetic */
    result += test_pointer_arithmetic(argc + 30);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    return sink & 0xFF;
}
