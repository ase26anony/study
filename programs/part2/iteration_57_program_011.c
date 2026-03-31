/* reload_stress.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization.
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress.c -o reload_stress
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function that returns values needing computation */
int compute_int(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5;
}

int* compute_ptr(int *p) {
    return p + (global_int % 16);
}

/* Test 1: Many operands exhausting registers */
int test1_many_operands(void) {
    int out1, out2, out3, out4, out5;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    
    /* Register variables with explicit registers */
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    register int r14_var asm ("r14") = 300;
    register int r15_var asm ("r15") = 400;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[r12], %%eax\n\t"
        "sub %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4], %[in5], 2), %%ebx\n\t"
        "add %[r13], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%ecx\n\t"
        "xor %[in7], %%ecx\n\t"
        "or %[r14], %%ecx\n\t"
        "mov %%ecx, %[out3]\n\t"
        "mov %[in8], %%edx\n\t"
        "shl $3, %%edx\n\t"
        "add %[in9], %%edx\n\t"
        "sub %[r15], %%edx\n\t"
        "mov %%edx, %[out4]\n\t"
        "mov %[in10], %%edi\n\t"
        "add %%eax, %%edi\n\t"
        "add %%ebx, %%edi\n\t"
        "mov %%edi, %[out5]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9),
          [in10] "r" (in10), [r12] "r" (r12_var), [r13] "r" (r13_var),
          [r14] "r" (r14_var), [r15] "r" (r15_var)
        : "eax", "ebx", "ecx", "edx", "edi", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test2_nested_calls(void) {
    int out1, out2;
    double dbl_out;
    
    /* Function calls as operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[ptr_calc], %%ebx\n\t"
        "mov (%%ebx), %%ecx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        "fld %[dbl_call]\n\t"
        "fistp %[dbl_out]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [dbl_out] "=m" (dbl_out)
        : [call1] "r" (compute_int(global_int)),
          [call2] "r" (compute_int(global_int * 2)),
          [ptr_calc] "r" (compute_ptr(&global_int)),
          [dbl_call] "m" (compute_double(global_double))
        : "eax", "ebx", "ecx", "memory"
    );
    
    return out1 + out2 + (int)dbl_out;
}

/* Test 3: Mixed data types and mode changes */
int test3_mixed_types(void) {
    char c_out;
    short s_out;
    int i_out;
    long long ll_out;
    float f_temp;
    double d_temp;
    
    char c_in = 'A';
    short s_in = 1000;
    int i_in = 0x12345678;
    long long ll_in = 0x123456789ABCDEF0LL;
    float f_in = 2.71828f;
    double d_in = 1.41421356;
    
    /* Mixed types requiring different machine modes */
    __asm__ __volatile__ (
        "mov %[c_in], %%al\n\t"
        "add $32, %%al\n\t"
        "mov %%al, %[c_out]\n\t"
        "mov %[s_in], %%ax\n\t"
        "add $1000, %%ax\n\t"
        "mov %%ax, %[s_out]\n\t"
        "mov %[i_in], %%eax\n\t"
        "ror $8, %%eax\n\t"
        "mov %%eax, %[i_out]\n\t"
        "mov %[ll_in_low], %%eax\n\t"
        "mov %[ll_in_high], %%edx\n\t"
        "add $1, %%eax\n\t"
        "adc $0, %%edx\n\t"
        "mov %%eax, %[ll_out_low]\n\t"
        "mov %%edx, %[ll_out_high]\n\t"
        "fld %[f_in]\n\t"
        "fadd %[d_in]\n\t"
        "fstp %[f_temp]\n\t"
        "fld %[d_in]\n\t"
        "fmul %[f_in]\n\t"
        "fstp %[d_temp]"
        : [c_out] "=m" (c_out), [s_out] "=m" (s_out), [i_out] "=r" (i_out),
          [ll_out_low] "=r" (((int*)&ll_out)[0]), [ll_out_high] "=r" (((int*)&ll_out)[1]),
          [f_temp] "=m" (f_temp), [d_temp] "=m" (d_temp)
        : [c_in] "r" ((int)c_in), [s_in] "r" ((int)s_in), [i_in] "r" (i_in),
          [ll_in_low] "r" ((int)(ll_in & 0xFFFFFFFF)),
          [ll_in_high] "r" ((int)(ll_in >> 32)),
          [f_in] "m" (f_in), [d_in] "m" (d_in)
        : "eax", "edx", "memory"
    );
    
    return c_out + s_out + i_out + (int)(ll_out & 0xFFFFFFFF) + (int)f_temp;
}

/* Test 4: Complex addressing modes with array indexing */
int test4_complex_addressing(void) {
    int array[100];
    int index1, index2, index3;
    int out1, out2, out3;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    index1 = global_int % 50;
    index2 = compute_int(global_int) % 50;
    index3 = (index1 + index2) % 50;
    
    /* Complex addressing with non-constant offsets */
    __asm__ __volatile__ (
        "mov %[idx1], %%eax\n\t"
        "mov %[arr](,%%eax,4), %%ebx\n\t"
        "mov %%ebx, %[out1]\n\t"
        "mov %[idx2], %%ecx\n\t"
        "lea (%[arr],%%ecx,4), %%edx\n\t"
        "mov (%%edx), %%esi\n\t"
        "add %%ebx, %%esi\n\t"
        "mov %%esi, %[out2]\n\t"
        "mov %[idx3], %%edi\n\t"
        "mov %[arr](,%%edi,4), %%eax\n\t"
        "imul %%esi, %%eax\n\t"
        "mov %%eax, %[out3]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [arr] "r" (array), [idx1] "r" (index1), [idx2] "r" (index2),
          [idx3] "r" (index3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 5: Secondary reload triggers with specific constraints */
int test5_secondary_reloads(void) {
    int out1, out2;
    int in1 = 777, in2 = 888;
    
    /* Using specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "test %%eax, %%eax\n\t"
        "setnz %%al\n\t"
        "movzx %%al, %[out1]\n\t"
        "mov %[in2], %%ebx\n\t"
        "cmp $1000, %%ebx\n\t"
        "setg %%bl\n\t"
        "movzx %%bl, %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2)
        : "eax", "ebx", "memory"
    );
    
    /* Another asm forcing moves between register classes */
    register int ax_var asm ("ax");
    int result;
    
    __asm__ __volatile__ (
        "mov %[in1], %[ax]\n\t"
        "add $111, %[ax]\n\t"
        "mov %[ax], %[res]"
        : [res] "=r" (result), [ax] "=r" (ax_var)
        : "[ax]" (in1)
        : "memory"
    );
    
    return out1 + out2 + result;
}

/* Test 6: Chain of volatile asm blocks with interdependencies */
int test6_chain_reloads(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "mov %%eax, %[t1]"
        : [t1] "=r" (tmp1)
        : [a] "r" (a), [b] "r" (b)
        : "eax", "memory"
    );
    
    /* Chain 2 - depends on Chain 1 */
    __asm__ __volatile__ (
        "mov %[t1], %%ebx\n\t"
        "imul %[c], %%ebx\n\t"
        "mov %%ebx, %[t2]"
        : [t2] "=r" (tmp2)
        : [t1] "r" (tmp1), [c] "r" (c)
        : "ebx", "memory"
    );
    
    /* Chain 3 - depends on Chain 2 */
    __asm__ __volatile__ (
        "mov %[t2], %%ecx\n\t"
        "sub %[d], %%ecx\n\t"
        "mov %%ecx, %[t3]"
        : [t3] "=r" (tmp3)
        : [t2] "r" (tmp2), [d] "r" (d)
        : "ecx", "memory"
    );
    
    /* Chain 4 - depends on Chain 3 */
    __asm__ __volatile__ (
        "mov %[t3], %%edx\n\t"
        "xor %[e], %%edx\n\t"
        "mov %%edx, %[t4]"
        : [t4] "=r" (tmp4)
        : [t3] "r" (tmp3), [e] "r" (e)
        : "edx", "memory"
    );
    
    return tmp1 + tmp2 + tmp3 + tmp4;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test1_many_operands();
    checksum += test2_nested_calls();
    checksum += test3_mixed_types();
    checksum += test4_complex_addressing();
    checksum += test5_secondary_reloads();
    checksum += test6_chain_reloads();
    
    /* Final volatile barrier */
    __asm__ __volatile__ (""
        :
        :
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte as exit code */
}
