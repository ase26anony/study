/* reload_stress.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization.
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress.c -o reload_stress
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
char global_buffer[256];

/* Function that returns values needing computation */
int compute_index(int i) {
    return (i * 7 + 3) % 100;
}

double compute_double(int i) {
    return (double)i * 0.5 + 1.0;
}

/* Test 1: Many operands with mixed types to exhaust registers */
int test1_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    int out1, out2, out3, out4;
    double dout1, dout2;
    char cout;
    short sout;
    
    /* Complex inline asm with many operands of different types */
    __asm__ __volatile__ (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "movq %[din1], %%xmm0\n\t"
        "cvttsd2si %%xmm0, %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %[out3]\n\t"
        "movb %[cin], %%cl\n\t"
        "movb %%cl, %[cout]\n\t"
        "movw %[sin], %%dx\n\t"
        "movw %%dx, %[sout]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [cout] "=r" (cout), [sout] "=r" (sout),
          [dout1] "=x" (dout1), [dout2] "=x" (dout2)
        : [in1] "r" (r0), [in2] "r" (r1), [in3] "r" (r2),
          [din1] "x" (global_double), [cin] "r" ((char)65), [sin] "r" ((short)1000),
          "m" (global_array[0]), "m" (global_buffer[0])
        : "rax", "rbx", "rcx", "rdx", "xmm0", "memory"
    );
    
    return out1 + out2 + out3 + cout + sout;
}

/* Test 2: Nested function calls in asm operands */
int test2_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "movl %[idx1], %%eax\n\t"
        "addl %[idx2], %%eax\n\t"
        "movl %%eax, %[res1]\n\t"
        "cvtsi2sd %[dval], %%xmm0\n\t"
        "movsd %%xmm0, %[dres]"
        : [res1] "=r" (result1), [dres] "=x" (dresult)
        : [idx1] "r" (compute_index(global_int)),
          [idx2] "r" (compute_index(global_int + 1)),
          [dval] "r" ((int)compute_double(global_int))
        : "rax", "xmm0", "memory"
    );
    
    /* Another asm with pointer arithmetic */
    int *ptr = &global_array[compute_index(10)];
    __asm__ __volatile__ (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[res2]"
        : [res2] "=r" (result2)
        : [ptr] "r" (ptr + compute_index(5))
        : "rax", "memory"
    );
    
    return result1 + result2 + (int)dresult;
}

/* Test 3: Explicit register variables with constraints */
int test3_register_vars(void) {
    register int a asm ("rax") = 100;
    register int b asm ("rbx") = 200;
    register int c asm ("rcx") = 300;
    int out_a, out_b, out_c;
    
    /* Force moves between specific registers */
    __asm__ __volatile__ (
        "movl %%eax, %%esi\n\t"
        "addl %%ebx, %%esi\n\t"
        "movl %%esi, %[outa]\n\t"
        "movl %%ecx, %%edi\n\t"
        "subl %%eax, %%edi\n\t"
        "movl %%edi, %[outb]\n\t"
        "imull %%ebx, %%ecx\n\t"
        "movl %%ecx, %[outc]"
        : [outa] "=r" (out_a), [outb] "=" (out_b), [outc] "=r" (out_c)
        : "a" (a), "b" (b), "c" (c)
        : "esi", "edi", "memory"
    );
    
    /* Mixed size accesses */
    char char_out;
    __asm__ __volatile__ (
        "movb %%al, %%dl\n\t"
        "movb %%dl, %[cout]"
        : [cout] "=r" (char_out)
        : "a" (a)
        : "dl"
    );
    
    return out_a + out_b + out_c + char_out;
}

/* Test 4: Memory clobber and volatile chains */
int test4_memory_clobber(void) {
    int x = 10, y = 20, z = 30;
    int r1, r2, r3, r4;
    
    /* Chain of volatile asm with memory clobber */
    __asm__ __volatile__ (
        "movl %[x], %%eax\n\t"
        "addl $5, %%eax\n\t"
        "movl %%eax, %[r1]"
        : [r1] "=r" (r1)
        : [x] "m" (x)
        : "rax", "memory"
    );
    
    __asm__ __volatile__ (
        "movl %[y], %%ebx\n\t"
        "addl %[r1], %%ebx\n\t"
        "movl %%ebx, %[r2]"
        : [r2] "=r" (r2)
        : [y] "m" (y), [r1] "r" (r1)
        : "rbx", "memory"
    );
    
    __asm__ __volatile__ (
        "movl %[z], %%ecx\n\t"
        "imull %[r2], %%ecx\n\t"
        "movl %%ecx, %[r3]\n\t"
        "leal (%[r1],%[r2],2), %%edx\n\t"
        "movl %%edx, %[r4]"
        : [r3] "=r" (r3), [r4] "=r" (r4)
        : [z] "m" (z), [r1] "r" (r1), [r2] "r" (r2)
        : "rcx", "rdx", "memory"
    );
    
    return r1 + r2 + r3 + r4;
}

/* Test 5: Mixed modes and secondary reload triggers */
int test5_mixed_modes(void) {
    int i = 255;
    short s = -100;
    char c = 'A';
    double d = 2.71828;
    float f = 1.414f;
    long long ll = 0x123456789ABCDEF0LL;
    
    int out_i;
    short out_s;
    char out_c;
    double out_d;
    float out_f;
    long long out_ll;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        "movl %[in_i], %%eax\n\t"
        "movl %%eax, %[out_i]\n\t"
        "movswl %[in_s], %%ebx\n\t"
        "movl %%ebx, %%ecx\n\t"
        "movb %[in_c], %%dl\n\t"
        "movb %%dl, %[out_c]\n\t"
        "movq %[in_d], %%xmm0\n\t"
        "movq %%xmm0, %[out_d]\n\t"
        "movd %[in_f], %%xmm1\n\t"
        "movd %%xmm1, %[out_f]\n\t"
        "movq %[in_ll], %%r8\n\t"
        "movq %%r8, %[out_ll]"
        : [out_i] "=r" (out_i), [out_c] "=r" (out_c),
          [out_d] "=x" (out_d), [out_f] "=x" (out_f),
          [out_ll] "=r" (out_ll)
        : [in_i] "r" (i), [in_s] "r" (s), [in_c] "r" (c),
          [in_d] "x" (d), [in_f] "x" (f), [in_ll] "r" (ll)
        : "rax", "rbx", "rcx", "rdx", "r8", "xmm0", "xmm1", "memory"
    );
    
    /* Force mode change with cast in operand */
    __asm__ __volatile__ (
        "cvttsd2si %[dbl], %%eax\n\t"
        "movl %%eax, %[out_s]"
        : [out_s] "=r" (out_s)
        : [dbl] "x" ((double)(int)d)  /* Cast forces mode change */
        : "rax", "memory"
    );
    
    return out_i + out_s + out_c + (int)out_d + (int)out_f + (int)out_ll;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    int checksum = 0;
    
    checksum += test1_many_operands();
    checksum += test2_nested_calls();
    checksum += test3_register_vars();
    checksum += test4_memory_clobber();
    checksum += test5_mixed_modes();
    
    /* Final volatile barrier */
    __asm__ __volatile__ ("" : : : "memory");
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}
