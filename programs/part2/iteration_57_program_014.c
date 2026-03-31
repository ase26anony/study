/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload pass by creating inline
 * assembly patterns that force the register allocator to generate numerous
 * reloads, including secondary reloads. The goal is to trigger the
 * initialization block in reload.cc (lines 1381-1399) for many reload entries.
 *
 * Compilation recommendations:
 *   gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -funroll-loops -fno-optimize-sibling-calls reload_stress.c -o reload_stress
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile char global_char = 'A';
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Function that returns a value, forcing evaluation before assembly */
static inline int get_index(void) {
    return global_int & 0xF;
}

static inline double compute_value(int x) {
    return x * 0.5;
}

/* Test functions with different reload-triggering patterns */

/* Test 1: Many operands with mixed types and constraints */
static int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = global_int;
    int in2 = global_int + 1;
    double d1 = global_double;
    double d2 = global_double * 2.0;
    char c1 = global_char;
    short s1 = (short)global_int;
    
    /* Use explicit register variables to force specific register allocation */
    register int reg_var1 asm ("r12") = in1 * 2;
    register int reg_var2 asm ("r13") = in2 * 3;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        "mov %[d1], %%rax\n\t"           /* Force double to integer reg */
        "add %[in1], %[out1]\n\t"
        "imul %[reg1], %[out2]\n\t"
        "mov %[c1], %%cl\n\t"
        "add %%cl, %[out3]\n\t"
        "mov %[s1], %%dx\n\t"
        "add %%dx, %[out4]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (in1), [d1] "r" ((int)d1),  /* Cast double to int for mode change */
          [reg1] "r" (reg_var1), [c1] "r" ((int)c1),
          [s1] "r" ((int)s1), [reg2] "r" (reg_var2)
        : "rax", "rcx", "rdx", "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in assembly operands */
static int test_nested_calls(void) {
    int result1, result2, result3;
    int index1, index2;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "lea (%[idx1], %[idx2], 4), %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "imul %[idx1], %[res2]\n\t"
        "add %[idx2], %[res3]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2), [res3] "=r" (result3)
        : [idx1] "r" (get_index()),           /* Function call as operand */
          [idx2] "r" (get_index() + 2)        /* Another function call */
        : "rax", "memory", "cc"
    );
    
    /* Chain of volatile assembly blocks with interdependent operands */
    __asm__ __volatile__ (
        "add $1, %0\n\t"
        : "+r" (result1)
        :
        : "cc"
    );
    
    __asm__ __volatile__ (
        "add %1, %0\n\t"
        : "+r" (result2)
        : "r" (result1)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
static int test_mixed_modes(void) {
    char c_out;
    short s_out;
    int i_out;
    long long ll_out;
    float f_temp;
    double d_temp;
    
    int i_in = global_int;
    double d_in = global_double;
    float f_in = (float)global_double;
    
    /* Force mode changes by using different sized operands */
    __asm__ __volatile__ (
        "mov %[i_in], %%eax\n\t"
        "mov %%al, %[c_out]\n\t"          /* int -> char truncation */
        "mov %%ax, %[s_out]\n\t"          /* int -> short truncation */
        "cvtsi2sd %[i_in], %%xmm0\n\t"    /* int -> double (would use xmm if SSE enabled) */
        "movq %%xmm0, %[d_temp]\n\t"
        "cvtsd2ss %%xmm0, %%xmm1\n\t"     /* double -> float */
        "movd %%xmm1, %[f_temp]\n\t"
        : [c_out] "=r" (c_out), [s_out] "=r" (s_out),
          [d_temp] "=r" (ll_out), [f_temp] "=r" (i_out)  /* Using int to hold float bits */
        : [i_in] "r" (i_in)
        : "rax", "xmm0", "xmm1", "memory"
    );
    
    /* Another assembly with explicit register constraints */
    register double d_reg asm ("xmm0") = d_in;  /* Try to force xmm register */
    int int_from_double;
    
    __asm__ __volatile__ (
        "movq %[dreg], %%rax\n\t"         /* Move double to integer register */
        "shr $32, %%rax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (int_from_double)
        : [dreg] "x" (d_reg)              /* 'x' constraint for SSE register */
        : "rax", "memory"
    );
    
    return (int)c_out + s_out + i_out + int_from_double;
}

/* Test 4: Complex addressing modes with array accesses */
static int test_complex_addressing(void) {
    int results[4] = {0};
    int index = get_index();
    int offset = global_int & 0x7;
    
    /* Pointer arithmetic in operands */
    __asm__ __volatile__ (
        "mov (%[arr], %[idx], 4), %%eax\n\t"
        "mov %%eax, %[res0]\n\t"
        "mov 4(%[arr], %[idx], 4), %%ebx\n\t"
        "add %%ebx, %[res1]\n\t"
        "lea (%[arr], %[off], 2), %%rcx\n\t"
        "mov (%%rcx), %%edx\n\t"
        "add %%edx, %[res2]\n\t"
        : [res0] "=r" (results[0]), [res1] "=r" (results[1]), [res2] "=r" (results[2])
        : [arr] "r" (global_array), [idx] "r" (index), [off] "r" (offset)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Memory operand with complex expression */
    __asm__ __volatile__ (
        "mov %[idx], %%eax\n\t"
        "add $10, %%eax\n\t"
        "cltq\n\t"
        "mov global_array(%%rax), %[res3]\n\t"
        : [res3] "=r" (results[3])
        : [idx] "r" (index)
        : "rax", "memory"
    );
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Test 5: Secondary reload triggers with specific register constraints */
static int test_secondary_reloads(void) {
    int result = 0;
    int input1 = global_int;
    int input2 = global_int * 2;
    
    /* Try to force moves between register classes */
    register int forced_reg asm ("rax") = input1;
    
    /* Assembly with specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        "push %%rbx\n\t"
        "mov %[in2], %%rbx\n\t"
        "add %%rbx, %%rax\n\t"
        "pop %%rbx\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (result)
        : [in2] "r" (input2), "[out]" (forced_reg)
        : "rbx", "cc"
    );
    
    /* Another attempt with flag-generating instruction */
    int flag_result;
    __asm__ __volatile__ (
        "test %[in1], %[in1]\n\t"
        "setg %%al\n\t"
        "movzx %%al, %[out]\n\t"
        : [out] "=r" (flag_result)
        : [in1] "r" (input1)
        : "rax", "cc"
    );
    
    return result + flag_result;
}

/* Test 6: Many clobbers to increase register pressure */
static int test_many_clobbers(void) {
    int out1, out2, out3, out4, out5;
    int in1 = global_int;
    int in2 = global_int + 1;
    int in3 = global_int + 2;
    int in4 = global_int + 3;
    int in5 = global_int + 4;
    
    /* Clobber many registers to force spills */
    __asm__ __volatile__ (
        "mov %[in1], %%r10\n\t"
        "mov %[in2], %%r11\n\t"
        "add %%r10, %%r11\n\t"
        "mov %%r11, %[out1]\n\t"
        "mov %[in3], %%r12\n\t"
        "mov %[in4], %%r13\n\t"
        "imul %%r12, %%r13\n\t"
        "mov %%r13, %[out2]\n\t"
        "mov %[in5], %%r14\n\t"
        "lea (%%r14, %%r14, 2), %%r15\n\t"
        "mov %%r15, %[out3]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5)
        : "r10", "r11", "r12", "r13", "r14", "r15", "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array with values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run all tests multiple times to increase reload opportunities */
    for (int i = 0; i < 10; i++) {
        checksum += test_many_operands();
        checksum += test_nested_calls();
        checksum += test_mixed_modes();
        checksum += test_complex_addressing();
        checksum += test_secondary_reloads();
        checksum += test_many_clobbers();
        
        /* Modify globals to create different scenarios */
        global_int += 7;
        global_double *= 1.1;
        global_char += 1;
    }
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (checksum)
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
