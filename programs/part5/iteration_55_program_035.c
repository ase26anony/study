/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations that reduce register pressure */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to provide base addresses */
static int global_array[256];
static struct BigStruct global_structs[16];
static volatile int volatile_global = 0;

/* Function 1: Complex array addressing with multiple index calculations */
__attribute__((noinline))
static int test_complex_addressing(int seed) {
    /* Declare many local variables to consume registers */
    register int r0 asm("eax") = seed;
    register int r1 asm("ebx") = seed * 2;
    register int r2 asm("ecx") = seed * 3;
    int i1 = seed + 1, i2 = seed + 2, i3 = seed + 3, i4 = seed + 4;
    int i5 = seed + 5, i6 = seed + 6, i7 = seed + 7, i8 = seed + 8;
    int i9 = seed + 9, i10 = seed + 10, i11 = seed + 11, i12 = seed + 12;
    volatile int vi1 = seed, vi2 = seed * 2;
    
    /* Multi-dimensional array access pattern - forces address reloads */
    int *ptr_array[8];
    for (int j = 0; j < 8; j++) {
        ptr_array[j] = &global_array[j * 16];
    }
    
    /* Complex addressing: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different addressing modes */
    
    /* Pattern 1: Base + index*scale + offset */
    sum += global_array[i1 + i2 * 4 + 8];
    sum += global_array[i3 + i4 * 2 + 16];
    
    /* Pattern 2: Pointer chasing with address computation */
    int *ptr1 = &global_array[i5];
    int *ptr2 = &global_array[i6];
    sum += *(ptr1 + i7) + *(ptr2 + i8);
    
    /* Pattern 3: Nested array of pointers */
    sum += *(ptr_array[i9 & 7] + (i10 & 15));
    
    /* Volatile accesses force spills */
    vi1 = sum;
    sum += vi2;
    
    /* Use all register variables in computation */
    asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2));
    sum += r0 + r1 + r2;
    
    return sum;
}

/* Function 2: Structure member accesses with inline assembly */
__attribute__((noinline))
static int test_struct_reloads(int idx) {
    struct BigStruct local_struct;
    struct BigStruct *struct_ptr = &global_structs[idx & 15];
    volatile struct BigStruct *volatile_struct_ptr = &global_structs[(idx + 1) & 15];
    
    /* Initialize with many different values */
    local_struct.a = idx; local_struct.b = idx * 2;
    local_struct.c = idx * 3; local_struct.d = idx * 4;
    local_struct.e = idx * 5; local_struct.f = idx * 6;
    local_struct.g = idx * 7; local_struct.h = idx * 8;
    
    /* Complex structure member addressing */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    /* by using structure pointers in complex expressions */
    
    /* Inline assembly with multiple outputs and memory constraints */
    asm volatile (
        "movl %[ptr_a], %[t1]\n\t"
        "movl %[ptr_b], %[t2]\n\t"
        "addl %[t2], %[t1]\n\t"
        "movl %[t1], %[t3]\n\t"
        : [t1]"=&r"(temp1), [t2]"=&r"(temp2), [t3]"=r"(temp3)
        : [ptr_a]"m"(struct_ptr->a), [ptr_b]"m"(struct_ptr->b)
        : "cc"
    );
    
    /* More complex addressing with volatile */
    temp4 = volatile_struct_ptr->arr[idx & 7];
    temp5 = volatile_struct_ptr->arr[(idx + 1) & 7];
    
    /* Pointer arithmetic on structure pointer */
    struct BigStruct *ptr2 = struct_ptr + (idx & 3);
    temp6 = ptr2->c + ptr2->d;
    
    /* Chain of structure accesses */
    temp7 = struct_ptr->arr[struct_ptr->a & 7];
    
    /* Use all temporaries */
    return temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + local_struct.a;
}

/* Function 3: Inline assembly with multiple output operands to memory */
__attribute__((noinline))
static int test_asm_reloads(int base) {
    int outputs[8];
    int inputs[8];
    
    /* Initialize inputs */
    for (int i = 0; i < 8; i++) {
        inputs[i] = base + i * 17;
    }
    
    /* Complex inline assembly with multiple memory outputs */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    
    /* Use many register clobbers to increase pressure */
    asm volatile (
        "movl %[in0], %%eax\n\t"
        "movl %[in1], %%ebx\n\t"
        "movl %[in2], %%ecx\n\t"
        "movl %[in3], %%edx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %%eax, %[out0]\n\t"
        "imull %%ebx, %%ecx\n\t"
        "movl %%ecx, %[out1]\n\t"
        "movl %[in4], %%esi\n\t"
        "movl %[in5], %%edi\n\t"
        "leal (%%esi,%%edi,2), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        : [out0]"=m"(outputs[0]), [out1]"=m"(outputs[1]), [out2]"=m"(outputs[2])
        : [in0]"m"(inputs[0]), [in1]"m"(inputs[1]), [in2]"m"(inputs[2]),
          [in3]"m"(inputs[3]), [in4]"m"(inputs[4]), [in5]"m"(inputs[5])
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* More assembly with different constraints */
    int temp1, temp2;
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movl $0x9ABCDEF0, %1\n\t"
        : "=r"(temp1), "=r"(temp2)
        :
        : "cc"
    );
    
    outputs[3] = temp1;
    outputs[4] = temp2;
    
    /* Force use of all outputs */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += outputs[i];
    }
    
    return sum;
}

/* Function 4: Mixed addressing modes and pointer chains */
__attribute__((noinline))
static int test_mixed_reloads(int start) {
    /* Create many pointer variables */
    int *p1 = &global_array[start];
    int *p2 = &global_array[start + 16];
    int *p3 = &global_array[start + 32];
    int *p4 = &global_array[start + 48];
    volatile int *vp1 = &global_array[start + 64];
    volatile int *vp2 = &global_array[start + 80];
    
    /* Intermediate computation variables */
    int a = start, b = start * 2, c = start * 3, d = start * 4;
    int e = start * 5, f = start * 6, g = start * 7, h = start * 8;
    int i = start * 9, j = start * 10, k = start * 11, l = start * 12;
    
    /* Complex expressions mixing everything */
    /* This should trigger RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    
    /* Chain of dependent computations */
    int val1 = *p1 + a;
    int val2 = *(p2 + b) + val1;
    int val3 = *(p3 + c) + val2;
    int val4 = *(p4 + d) + val3;
    
    /* Volatile accesses interspersed */
    int v1 = *vp1;
    *vp2 = val4;
    
    /* More complex addressing */
    int val5 = *(p1 + (e & 15)) + *(p2 + (f & 15));
    int val6 = *(p3 + (g & 15)) + *(p4 + (h & 15));
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect((val5 > val6), 0)) {
        val5 = val6 + *vp1;
    }
    
    /* Pointer to pointer */
    int **pp1 = &p1;
    int **pp2 = &p2;
    int val7 = **pp1 + **pp2;
    
    /* Final computation using all variables */
    return val1 + val2 + val3 + val4 + val5 + val6 + val7 + v1 + i + j + k + l;
}

/* Function 5: Loop with unrolled complex addressing */
#pragma GCC unroll 4
__attribute__((noinline))
static int test_loop_reloads(int iterations) {
    int sum = 0;
    int array[32];
    
    /* Initialize local array */
    for (int i = 0; i < 32; i++) {
        array[i] = i * iterations;
    }
    
    /* Unrolled loop with complex addressing */
    for (int i = 0; i < iterations && i < 8; i++) {
        /* Each iteration uses different addressing patterns */
        int idx1 = i * 2;
        int idx2 = i * 3;
        int idx3 = i * 4;
        
        /* Direct array access */
        sum += array[idx1 & 31];
        
        /* Pointer-based access */
        int *ptr = &array[idx2 & 31];
        sum += *ptr + *(ptr + 1);
        
        /* Complex computation in address */
        sum += array[(idx1 + idx2 + idx3) & 31];
        
        /* Volatile access to force spill */
        volatile_global = sum;
        sum += volatile_global;
        
        /* Structure-like access pattern */
        struct { int x; int y; } point = {i, i*2};
        sum += point.x + point.y;
    }
    
    return sum;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i * 10 + j;
        }
    }
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(argc);
    result += test_struct_reloads(argc + 1);
    result += test_asm_reloads(argc + 2);
    result += test_mixed_reloads(argc + 3);
    result += test_loop_reloads(argc + 4);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result & 0xFF;
}

#pragma GCC pop_options
