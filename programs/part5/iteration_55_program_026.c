/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register usage patterns */
#define FORCE_REGISTER_PRESSURE __attribute__((noinline, optimize("O0")))

/* Complex data structures to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int *volatile_ptr;
};

struct MultiDim {
    int arr[4][4][4];
    struct NestedData nested;
    volatile long volatile_field;
};

/* Global arrays to force memory operands */
static volatile int global_volatile_array[256];
static struct MultiDim global_structs[8];
static int *global_ptr_array[32];

/* ===== Test Function 1: Complex Array Addressing ===== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
FORCE_REGISTER_PRESSURE
int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4;
    int i5 = e + 5, i6 = f + 6, i7 = g + 7, i8 = h + 8;
    int j1, j2, j3, j4, j5, j6, j7, j8;
    volatile int v1, v2, v3, v4;
    
    /* Multi-dimensional array with complex indexing - forces address reloads */
    int local_arr[4][4][4];
    
    /* Complex address computation requiring multiple registers */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                /* Nested addressing: base + i*64 + j*16 + k*4 */
                /* This often needs RELOAD_FOR_INPUT_ADDRESS */
                local_arr[i][j][k] = 
                    global_structs[i].arr[j][k][0] * i1 +
                    global_structs[j].arr[k][i][1] * i2 +
                    global_structs[k].arr[i][j][2] * i3;
                    
                /* More complex: address of address computation */
                /* May trigger RELOAD_FOR_INPADDR_ADDRESS */
                int *ptr = &local_arr[i][j][k];
                *ptr += *(ptr + (i*j*k) % 4);
            }
        }
    }
    
    /* Force spills with many intermediate values */
    j1 = local_arr[0][0][0] + local_arr[1][1][1];
    j2 = local_arr[0][1][2] + local_arr[1][2][3];
    j3 = local_arr[0][2][3] + local_arr[1][3][0];
    j4 = local_arr[0][3][1] + local_arr[1][0][2];
    j5 = local_arr[2][0][3] + local_arr[3][1][0];
    j6 = local_arr[2][1][0] + local_arr[3][2][1];
    j7 = local_arr[2][2][1] + local_arr[3][3][2];
    j8 = local_arr[2][3][2] + local_arr[3][0][3];
    
    /* Volatile accesses force memory operations */
    v1 = global_volatile_array[i1];
    v2 = global_volatile_array[i2];
    v3 = global_volatile_array[i3];
    v4 = global_volatile_array[i4];
    
    return j1 + j2 + j3 + j4 + j5 + j6 + j7 + j8 + v1 + v2 + v3 + v4;
}

/* ===== Test Function 2: Inline Assembly with Multiple Outputs ===== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
FORCE_REGISTER_PRESSURE
int test_assembly_reloads(int x, int y, int z) {
    int out1, out2, out3, out4, out5, out6;
    volatile int mem1, mem2, mem3;
    int *ptr1 = &mem1;
    int *ptr2 = &mem2;
    int *ptr3 = &mem3;
    
    /* Complex address computations for output operands */
    int *output_addr1 = ptr1 + x * y;
    int *output_addr2 = ptr2 + y * z;
    int *output_addr3 = ptr3 + z * x;
    
    /* Inline asm with multiple outputs and complex addresses */
    /* This forces RELOAD_FOR_OUTPUT_ADDRESS for the memory outputs */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]\n\t"
        "leal (%%eax, %%ebx, 4), %%edx\n\t"
        "movl %%edx, %[out4]\n\t"
        "leal (%%ebx, %%ecx, 2), %%esi\n\t"
        "movl %%esi, %[out5]\n\t"
        "leal (%%ecx, %%eax, 8), %%edi\n\t"
        "movl %%edi, %[out6]"
        : [out1] "=m" (*output_addr1),
          [out2] "=m" (*output_addr2),
          [out3] "=m" (*output_addr3),
          [out4] "=r" (out4),
          [out5] "=r" (out5),
          [out6] "=r" (out6)
        : [in1] "rm" (x),
          [in2] "rm" (y),
          [in3] "rm" (z)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More complex: output address itself needs computation */
    /* May trigger RELOAD_FOR_OUTADDR_ADDRESS */
    int **addr_of_output = &output_addr1;
    **addr_of_output = out4 + out5 + out6;
    
    return *output_addr1 + *output_addr2 + *output_addr3 + out4 + out5 + out6;
}

/* ===== Test Function 3: Pointer Chaining and Structure Access ===== */
/* Targets: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
FORCE_REGISTER_PRESSURE
int test_pointer_chasing(int seed) {
    /* Create many pointer variables to consume registers */
    struct NestedData *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
    int *ip1, *ip2, *ip3, *ip4, *ip5, *ip6, *ip7, *ip8;
    volatile int *vp1, *vp2, *vp3, *vp4;
    
    /* Initialize pointer chain */
    p1 = &global_structs[0].nested;
    p2 = &global_structs[1].nested;
    p3 = &global_structs[2].nested;
    p4 = &global_structs[3].nested;
    
    /* Complex pointer arithmetic - forces operand address reloads */
    ip1 = p1->values + (seed % 8);
    ip2 = p2->values + ((seed + 1) % 8);
    ip3 = p3->values + ((seed + 2) % 8);
    ip4 = p4->values + ((seed + 3) % 8);
    
    /* Multiple levels of indirection */
    vp1 = p1->volatile_ptr;
    vp2 = p2->volatile_ptr;
    vp3 = p3->volatile_ptr;
    vp4 = p4->volatile_ptr;
    
    /* Pointer chasing with computation */
    /* This needs RELOAD_FOR_OPERAND_ADDRESS when dereferencing */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        /* Each of these requires address computation before dereference */
        sum += *(ip1 + i);
        sum += *(ip2 + i * 2);
        sum += *(ip3 + i * 3);
        sum += *(ip4 + i * 4);
        
        /* Volatile pointer dereference - forces reloads */
        if (vp1) sum += *vp1;
        if (vp2) sum += *vp2;
        if (vp3) sum += *vp3;
        if (vp4) sum += *vp4;
        
        /* Address of pointer computation */
        /* May trigger RELOAD_FOR_OPADDR_ADDR */
        int **addr_of_ip = &ip1;
        **addr_of_ip = sum;
    }
    
    /* Complex expression mixing all pointers */
    return *ip1 + *ip2 + *ip3 + *ip4 + 
           (vp1 ? *vp1 : 0) + (vp2 ? *vp2 : 0) +
           (vp3 ? *vp3 : 0) + (vp4 ? *vp4 : 0);
}

/* ===== Test Function 4: Mixed Reload Types with Loop Unrolling ===== */
/* Targets all reload types through unrolled pattern */
FORCE_REGISTER_PRESSURE
int test_mixed_reloads_unrolled(int base) {
    int results[8];
    volatile int temp[8];
    
    /* Manually unrolled loop to maximize register pressure */
    /* Each iteration uses different addressing patterns */
    
    /* Iteration 1: Direct input reloads */
    {
        int a = base + 1, b = base + 2, c = base + 3;
        int d = base + 4, e = base + 5, f = base + 6;
        /* Force input reloads by using values after clobbering registers */
        results[0] = a + b + c + d + e + f;
        /* Use inline asm to clobber registers */
        asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f) : "memory");
        results[0] += a;  /* 'a' needs reload here */
    }
    
    /* Iteration 2: Input address reloads */
    {
        int idx1 = (base * 1) % 8, idx2 = (base * 3) % 8, idx3 = (base * 5) % 8;
        /* Complex array addressing */
        results[1] = global_structs[idx1].arr[idx2][idx3][0] +
                     global_structs[idx2].arr[idx3][idx1][1] +
                     global_structs[idx3].arr[idx1][idx2][2];
    }
    
    /* Iteration 3: Output address reloads */
    {
        int out_val;
        int *output_ptr = &temp[2] + base % 4;
        /* Output to computed address */
        asm volatile (
            "movl %[in], %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %[out]"
            : [out] "=m" (*output_ptr)
            : [in] "r" (base)
            : "eax"
        );
        results[2] = *output_ptr;
    }
    
    /* Iteration 4: Operand address reloads */
    {
        int *ptr1 = &global_volatile_array[base % 256];
        int *ptr2 = ptr1 + 16;
        int *ptr3 = ptr2 + 32;
        /* Chain of pointer dereferences */
        results[3] = *ptr1 + *ptr2 + *ptr3;
    }
    
    /* Iteration 5: Other address reloads */
    {
        struct NestedData *nested_ptr = &global_structs[base % 4].nested;
        int **ptr_to_ptr = &nested_ptr->volatile_ptr;
        if (*ptr_to_ptr) {
            results[4] = **ptr_to_ptr;
        } else {
            results[4] = base;
        }
    }
    
    /* Sum all results */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
    }
    
    return total;
}

/* ===== Main Driver ===== */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_volatile_array[i] = i;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                for (int l = 0; l < 4; l++) {
                    global_structs[i].arr[j][k][l] = i * 1000 + j * 100 + k * 10 + l;
                }
            }
        }
        global_structs[i].nested.volatile_ptr = &global_volatile_array[i * 32];
        for (int j = 0; j < 8; j++) {
            global_structs[i].nested.values[j] = i * 10 + j;
        }
    }
    
    /* Call all test functions with different patterns */
    int result = 0;
    
    /* Test 1: Complex addressing */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Test 2: Assembly with outputs */
    result += test_assembly_reloads(10, 20, 30);
    
    /* Test 3: Pointer chasing */
    result += test_pointer_chasing(42);
    
    /* Test 4: Mixed reloads with unrolling */
    for (int i = 0; i < 4; i++) {
        result += test_mixed_reloads_unrolled(i * 10);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result % 256;
}
