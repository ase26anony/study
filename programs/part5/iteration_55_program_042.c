/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct ComplexStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct ComplexStruct *next;
    volatile int volatile_member;
};

/* Global arrays to create addressing complexity */
static int global_array_1[256];
static int global_array_2[256];
static int global_array_3[256];
static struct ComplexStruct struct_array[32];

/* Function 1: Focus on RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
static __attribute__((noinline)) 
int test_input_reloads(int param1, int param2, int param3, int param4,
                       int param5, int param6, int param7, int param8) {
    /* Many local variables to exhaust registers */
    int local1 = param1 * 2;
    int local2 = param2 + param3;
    int local3 = param4 ^ param5;
    int local4 = param6 | param7;
    int local5 = param8 << 2;
    int local6 = local1 + local2;
    int local7 = local3 - local4;
    int local8 = local5 * local6;
    int local9 = local7 / (local8 ? local8 : 1);
    int local10 = local9 & 0xFF;
    
    /* Complex array addressing - forces address reloads */
    volatile int result = 0;
    
    /* Multi-level array indexing requiring address computation */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* This creates RELOAD_FOR_INPUT_ADDRESS */
            result += global_array_1[i * 8 + j * 2 + local1];
            /* More complex addressing with multiple components */
            result += global_array_2[(i << 3) + (j << 1) + local2];
        }
    }
    
    /* Structure member accesses with offsets */
    for (int i = 0; i < 4; i++) {
        struct ComplexStruct *ptr = &struct_array[i];
        /* Chain of accesses requiring multiple reloads */
        result += ptr->a + ptr->b + ptr->c;
        ptr->volatile_member = result;  /* Volatile write forces spill */
    }
    
    return result + local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* Function 2: Focus on RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
static __attribute__((noinline))
void test_output_address_reloads(int *output1, int *output2, 
                                 int *output3, int *output4,
                                 int idx1, int idx2, int idx3) {
    /* Complex output addressing with inline assembly */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Multiple outputs with complex address computation */
    asm volatile (
        /* Output to memory with complex addressing */
        "movl %[val1], (%[out1], %[idx1], 4)\n\t"
        "movl %[val2], 4(%[out2], %[idx2], 2)\n\t"
        "movl %[val3], 8(%[out3], %[idx3], 1)\n\t"
        : /* No outputs (memory side effects only) */
        : [val1] "r" (idx1), [val2] "r" (idx2), [val3] "r" (idx3),
          [out1] "r" (output1), [out2] "r" (output2), [out3] "r" (output3),
          [idx1] "r" (idx1), [idx2] "r" (idx2), [idx3] "r" (idx3)
        : "memory"
    );
    
    /* More local variables to increase pressure */
    volatile int v1 = *output1;
    volatile int v2 = *output2;
    volatile int v3 = *output3;
    volatile int v4 = *output4;
    
    /* Complex pointer arithmetic */
    int *ptr1 = output1 + idx1 * 2;
    int *ptr2 = output2 + idx2 * 3;
    int *ptr3 = output3 + idx3 * 4;
    int *ptr4 = output4 + (idx1 + idx2) * 5;
    
    /* Force address computations to be materialized */
    asm volatile (
        "movl %%eax, (%0)\n\t"
        "movl %%ebx, (%1)\n\t"
        "movl %%ecx, (%2)\n\t"
        "movl %%edx, (%3)\n\t"
        : 
        : "r" (ptr1), "r" (ptr2), "r" (ptr3), "r" (ptr4)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

/* Function 3: Focus on RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
static __attribute__((noinline))
int test_operand_address_reloads(int base) {
    /* Array of pointers - each needs its address reloaded */
    int *ptr_array[16];
    int values[16];
    
    /* Initialize pointer array with complex addresses */
    for (int i = 0; i < 16; i++) {
        values[i] = base + i * 7;
        ptr_array[i] = &values[i] + (i & 3);  /* Complex pointer arithmetic */
    }
    
    int sum = 0;
    volatile int *volatile_ptr;
    
    /* Pointer chasing with volatile */
    for (int i = 0; i < 8; i++) {
        volatile_ptr = ptr_array[i];
        sum += *volatile_ptr;  /* Forces operand address reload */
        
        /* More complex: pointer to pointer */
        int **pptr = &ptr_array[i];
        sum += **pptr;
        
        /* Even more complex: pointer with offset */
        sum += *(volatile_ptr + (i & 1));
    }
    
    /* Inline asm with memory operand constraints */
    int result;
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=r" (result)
        : [ptr] "m" (*ptr_array)  /* Memory constraint forces address reload */
        : "eax", "memory"
    );
    
    return sum + result;
}

/* Function 4: Focus on RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
static __attribute__((noinline))
int test_other_reload_types(void) {
    /* Use many different data types and operations */
    short s1, s2, s3, s4, s5, s6, s7, s8;
    char c1, c2, c3, c4, c5, c6, c7, c8;
    long long ll1, ll2, ll3, ll4;
    float f1, f2, f3, f4;
    
    /* Initialize all variables to prevent optimization */
    s1 = 1; s2 = 2; s3 = 3; s4 = 4; s5 = 5; s6 = 6; s7 = 7; s8 = 8;
    c1 = 'a'; c2 = 'b'; c3 = 'c'; c4 = 'd'; c5 = 'e'; c6 = 'f'; c7 = 'g'; c8 = 'h';
    ll1 = 100LL; ll2 = 200LL; ll3 = 300LL; ll4 = 400LL;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f;
    
    /* Mixed-type expressions force unusual reloads */
    int result = 0;
    result += (int)s1 + (int)s2 + (int)s3 + (int)s4;
    result += (int)c1 + (int)c2 + (int)c3 + (int)c4;
    result += (int)(ll1 >> 32) + (int)(ll2 >> 32);
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    /* Complex inline asm with multiple alternative constraints */
    int a = 10, b = 20, c = 30;
    asm volatile (
        "addl %[b], %[a]\n\t"
        "subl %[c], %[a]\n\t"
        : [a] "+&r,m" (a)  /* Alternative constraints */
        : [b] "ri,m" (b), [c] "ri,m" (c)
        : "cc"
    );
    
    return result + a;
}

/* Function 5: Unrolled loops to multiply register pressure */
static __attribute__((noinline))
int test_unrolled_reloads(int iterations) {
    int sum = 0;
    
    /* Manually unrolled loop */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations; i++) {
        /* Many temporaries in loop body */
        int t1 = i * 2;
        int t2 = i * 3;
        int t3 = i * 4;
        int t4 = i * 5;
        int t5 = i * 6;
        int t6 = i * 7;
        int t7 = i * 8;
        int t8 = i * 9;
        int t9 = i * 10;
        int t10 = i * 11;
        
        /* Complex addressing in loop */
        sum += global_array_1[t1 & 0xFF] +
               global_array_2[t2 & 0xFF] +
               global_array_3[t3 & 0xFF];
        
        /* Volatile accesses force spills */
        volatile int vs1 = t4;
        volatile int vs2 = t5;
        volatile int vs3 = t6;
        
        sum += vs1 + vs2 + vs3 + t7 + t8 + t9 + t10;
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array_1[i] = i;
        global_array_2[i] = i * 2;
        global_array_3[i] = i * 3;
    }
    
    for (int i = 0; i < 32; i++) {
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = i * 3;
        struct_array[i].volatile_member = i * 4;
    }
    
    /* Call all test functions to trigger different reload types */
    total += test_input_reloads(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_input_reloads(9, 10, 11, 12, 13, 14, 15, 16);
    
    int out1, out2, out3, out4;
    test_output_address_reloads(&out1, &out2, &out3, &out4, 1, 2, 3);
    total += out1 + out2 + out3 + out4;
    
    total += test_operand_address_reloads(100);
    total += test_operand_address_reloads(200);
    
    total += test_other_reload_types();
    total += test_other_reload_types();
    
    total += test_unrolled_reloads(8);
    total += test_unrolled_reloads(16);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    
    return final_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
