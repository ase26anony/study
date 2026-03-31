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
    struct BigStruct *next;
    volatile int sync;
};

/* Global arrays to force address computations */
static int global_array_1[256];
static int global_array_2[256];
static int global_array_3[256];
static struct BigStruct global_structs[16];

/* Function 1: Complex array indexing with multiple address computations */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static __attribute__((noinline)) 
int test_complex_addressing(int idx1, int idx2, int idx3, int idx4, int idx5) {
    volatile int sink;
    int i, j, k;
    
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    int d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initialize locals with complex expressions */
    a1 = idx1 * 2 + idx2;
    a2 = idx2 * 3 + idx3;
    a3 = idx3 * 4 + idx4;
    a4 = idx4 * 5 + idx5;
    a5 = idx5 * 6 + idx1;
    
    /* Multi-level array indexing - forces address reloads */
    for (i = 0; i < 4; i++) {
        /* Manual loop unrolling to increase pressure */
        /* Each iteration uses different addressing modes */
        
        /* RELOAD_FOR_INPUT_ADDRESS: base + index*scale */
        b1 = global_array_1[a1 + i * 8];
        b2 = global_array_1[a2 + i * 8 + 1];
        b3 = global_array_1[a3 + i * 8 + 2];
        b4 = global_array_1[a4 + i * 8 + 3];
        
        /* RELOAD_FOR_INPADDR_ADDRESS: address of address computation */
        c1 = global_array_2[global_array_1[a1 + i] + a2];
        c2 = global_array_2[global_array_1[a2 + i] + a3];
        c3 = global_array_2[global_array_1[a3 + i] + a4];
        c4 = global_array_2[global_array_1[a4 + i] + a5];
        
        /* Complex expression with many intermediate values */
        d1 = (b1 * c1) + (b2 * c2) + (b3 * c3) + (b4 * c4);
        d2 = (b1 + c1) * (b2 + c2) - (b3 + c3) * (b4 + c4);
        
        /* Force spills with volatile */
        sink = d1;
        sink = d2;
        
        /* Structure member access with offset */
        global_structs[i].arr[a1 & 7] = d1;
        global_structs[i].arr[a2 & 7] = d2;
    }
    
    /* Pointer chasing with complex addressing */
    struct BigStruct *ptr = &global_structs[0];
    for (j = 0; j < 8; j++) {
        /* RELOAD_FOR_OPERAND_ADDRESS: pointer needs reloading */
        ptr->a = ptr->b + ptr->c;
        ptr->b = ptr->arr[ptr->d & 7];
        
        /* Complex address computation for next pointer */
        int offset = (ptr->e + ptr->f) & 15;
        ptr = &global_structs[offset];
        
        /* Force memory barrier */
        sink = ptr->sync;
    }
    
    return a1 + a2 + a3 + a4 + a5;
}

/* Function 2: Inline assembly with multiple outputs and clobbers */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3;
    volatile int memory[16];
    int *ptr1, *ptr2, *ptr3;
    
    /* Many local variables to consume registers */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int u1, u2, u3, u4, u5, u6, u7, u8, u9, u10;
    
    /* Complex pointer arithmetic */
    ptr1 = memory + x;
    ptr2 = memory + y;
    ptr3 = memory + z;
    
    /* Inline assembly with multiple outputs and memory constraints */
    /* This forces output address reloads */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %%ebx, %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "leal (%%eax,%%ebx,2), %%edx\n\t"
        "movl %%edx, %[out3]"
        : [out1] "=m" (*ptr1),  /* RELOAD_FOR_OUTPUT_ADDRESS */
          [out2] "=m" (*(ptr2 + x)),  /* More complex address */
          [out3] "=m" (*(ptr3 + y * 2))  /* RELOAD_FOR_OUTADDR_ADDRESS */
        : [in1] "rm" (x),  /* Register or memory */
          [in2] "rm" (y),
          [in3] "rm" (z)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Another asm with alternative constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "rorl $8, %0"
        : "=r,r,m" (result1)  /* Alternative constraints */
        : "r,m,i" (x + y)     /* RELOAD_OTHER patterns */
        : "cc"
    );
    
    /* Complex expressions using results */
    t1 = *ptr1 + result1;
    t2 = *(ptr2 + x) - result1;
    t3 = *(ptr3 + y * 2) * result1;
    
    /* Chain computations to force more reloads */
    for (int i = 0; i < 4; i++) {
        u1 = t1 + i;
        u2 = t2 + i * 2;
        u3 = t3 + i * 3;
        
        /* Force address computations */
        memory[(u1 + u2) & 15] = u3;
        memory[(u2 + u3) & 15] = u1;
        memory[(u3 + u1) & 15] = u2;
    }
    
    return t1 + t2 + t3;
}

/* Function 3: Mixed operand types with function calls */
/* Targets: RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
static __attribute__((noinline))
int test_mixed_operands(int base) {
    /* Local arrays to force spilling */
    int arr1[8], arr2[8], arr3[8];
    int *ptr_arr[4];
    volatile int barrier;
    
    /* Initialize arrays with complex patterns */
    for (int i = 0; i < 8; i++) {
        arr1[i] = base + i * 3;
        arr2[i] = base + i * 5;
        arr3[i] = base + i * 7;
    }
    
    /* Set up pointer array with complex addresses */
    ptr_arr[0] = &arr1[(base + 1) & 7];
    ptr_arr[1] = &arr2[(base + 2) & 7];
    ptr_arr[2] = &arr3[(base + 3) & 7];
    ptr_arr[3] = &global_array_1[base & 255];
    
    /* Complex pointer dereferencing chain */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* RELOAD_FOR_OPADDR_ADDR: address of pointer needs reload */
        int *ptr = ptr_arr[i & 3];
        
        /* Complex addressing mode */
        int idx = (i * 7 + base) & 7;
        int val1 = ptr[idx];
        int val2 = ptr[(idx + 1) & 7];
        int val3 = ptr[(idx + 2) & 7];
        
        /* Use all values in complex expression */
        sum += val1 * val2 - val3;
        
        /* Modify pointer through complex expression */
        ptr_arr[i & 3] = &arr1[(val1 + val2) & 7];
        
        /* Memory barrier */
        barrier = sum;
    }
    
    /* Nested address computation */
    int ***nested_ptr = (int***)&ptr_arr;
    int **deref1 = *nested_ptr;
    int *deref2 = *deref1;
    int final_val = *deref2;
    
    return sum + final_val;
}

/* Function 4: Extreme register pressure with unrolled loops */
#pragma GCC unroll 4
static __attribute__((noinline))
int test_register_pressure(int seed) {
    /* Maximum local variables */
    int v01, v02, v03, v04, v05, v06, v07, v08, v09, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    volatile int mem[32];
    
    /* Initialize all variables with unique expressions */
    v01 = seed + 1;   v02 = seed * 2;   v03 = seed / 3;   v04 = seed - 4;
    v05 = seed ^ 5;   v06 = seed | 6;   v07 = seed & 7;   v08 = seed << 1;
    v09 = seed >> 2;  v10 = ~seed;      v11 = seed + 11;  v12 = seed * 12;
    v13 = seed / 13;  v14 = seed - 14;  v15 = seed ^ 15;  v16 = seed | 16;
    v17 = seed & 17;  v18 = seed << 3;  v19 = seed >> 4;  v20 = seed + 20;
    v21 = seed * 21;  v22 = seed / 22;  v23 = seed - 23;  v24 = seed ^ 24;
    v25 = seed | 25;  v26 = seed & 26;  v27 = seed << 5;  v28 = seed >> 6;
    v29 = seed + 29;  v30 = seed * 30;
    
    /* Unrolled computation using all variables */
    /* Each line carefully uses different variables to prevent optimization */
    v01 = v01 + v02 - v03 * v04 / (v05 + 1);
    v06 = v06 ^ v07 | v08 & v09 >> (v10 & 31);
    v11 = v11 * v12 + v13 - v14 % (v15 + 1);
    v16 = v16 | v17 ^ v18 << (v19 & 31);
    v20 = v20 - v21 * v22 / (v23 + 1);
    v24 = v24 ^ v25 | v26 & v27 >> (v28 & 31);
    v29 = v29 * v30 + v01 - v02 % (v03 + 1);
    
    /* Force memory stores with complex addresses */
    for (int i = 0; i < 32; i++) {
        int idx = (v01 + i * v02) & 31;
        mem[idx] = v03 + i * v04 - v05;
    }
    
    /* Complex reduction */
    int result = 0;
    for (int i = 0; i < 32; i++) {
        int idx1 = (v06 + i) & 31;
        int idx2 = (v07 + i * 2) & 31;
        int idx3 = (v08 + i * 3) & 31;
        
        result += mem[idx1] * mem[idx2] - mem[idx3];
    }
    
    return result + v01 + v06 + v11 + v16 + v20 + v24 + v29;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array_1[i] = i * 3;
        global_array_2[i] = i * 5;
        global_array_3[i] = i * 7;
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
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i * 10 + j;
        }
        global_structs[i].next = &global_structs[(i + 1) & 15];
        global_structs[i].sync = 0;
    }
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5);
    total += test_complex_addressing(6, 7, 8, 9, 10);
    
    total += test_asm_reloads(10, 20, 30);
    total += test_asm_reloads(40, 50, 60);
    
    total += test_mixed_operands(100);
    total += test_mixed_operands(200);
    
    total += test_register_pressure(1234);
    total += test_register_pressure(5678);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    
    return final_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
