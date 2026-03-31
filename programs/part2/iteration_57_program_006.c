/* reload_test.c - Stress GCC's reload mechanism to hit uncovered lines 1381-1399 in reload.cc */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100] = {0};
char global_buffer[256] = {0};

/* Function prototypes */
int test_many_operands(void);
int test_nested_calls(void);
int test_mixed_types(void);
int test_secondary_reloads(void);
int test_register_variables(void);

/* Helper functions for nested calls */
int func_return_int(int x) { return x * 2 + 1; }
double func_return_double(double x) { return x * 1.5; }
int* func_return_ptr(int *p) { return p + global_int; }

int main(void) {
    int checksum = 0;
    
    /* Initialize global array with values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run all test patterns to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_register_variables();
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4, out5, out6, out7, out8;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7, in8 = 8;
    int in9 = 9, in10 = 10, in11 = 11, in12 = 12;
    double din1 = 1.1, din2 = 2.2, dout1, dout2;
    
    /* Complex inline assembly with many operands of different types */
    __asm__ __volatile__(
        /* Multiple outputs with different constraints */
        "mov %[i1], %[o1]\n\t"
        "add %[i2], %[o1]\n\t"
        "imul %[i3], %[o1]\n\t"
        "mov %[i4], %[o2]\n\t"
        "lea (%[i5],%[i6],2), %[o3]\n\t"
        "mov %[i7], %[o4]\n\t"
        /* Mix in some floating point via integer registers */
        "movq %[d1], %%xmm0\n\t"
        "movq %%xmm0, %[do1]\n\t"
        : [o1] "=r"(out1), [o2] "=r"(out2), [o3] "=r"(out3),
          [o4] "=r"(out4), [o5] "=r"(out5), [do1] "=m"(dout1)
        : [i1] "r"(in1), [i2] "r"(in2), [i3] "r"(in3), [i4] "r"(in4),
          [i5] "r"(in5), [i6] "r"(in6), [i7] "r"(in7), [i8] "r"(in8),
          [d1] "x"(din1), "m"(global_array)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
    );
    
    /* Second assembly block with overlapping operands */
    __asm__ __volatile__(
        "addl $1, %0\n\t"
        "subl $2, %1\n\t"
        "imull $3, %2\n\t"
        : "+r"(out1), "+r"(out2), "+r"(out3)
        : 
        : "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls within assembly operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    int *ptr_result;
    
    /* Function calls as input operands - forces evaluation into registers */
    __asm__ __volatile__(
        "mov %[f1], %[r1]\n\t"
        "add %[f2], %[r1]\n\t"
        "mov %[f3], %[r2]\n\t"
        : [r1] "=r"(result1), [r2] "=r"(result2), [r3] "=r"(result3)
        : [f1] "r"(func_return_int(global_int)),
          [f2] "r"(func_return_int(global_int + 1)),
          [f3] "r"(global_int + func_return_int(10)),
          "m"(global_buffer)
        : "memory"
    );
    
    /* Complex addressing modes with function calls */
    int index = global_int;
    __asm__ __volatile__(
        "mov (%[base],%[idx],4), %[out]\n\t"
        : [out] "=r"(result3)
        : [base] "r"(global_array),
          [idx] "r"(func_return_int(index) % 50)
        : "memory"
    );
    
    /* Mixed pointer arithmetic */
    __asm__ __volatile__(
        "lea (%[ptr],%[off],1), %[res]\n\t"
        : [res] "=r"(ptr_result)
        : [ptr] "r"(global_array),
          [off] "r"(func_return_int(global_int) * sizeof(int))
        : "cc"
    );
    
    return result1 + result2 + result3 + (ptr_result - global_array);
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B', cout;
    short s1 = 1000, s2 = 2000, sout;
    int i1 = 1000000, i2 = 2000000, iout;
    long l1 = 999999999L, l2 = 888888888L, lout;
    float f1 = 1.234f, f2 = 5.678f, fout;
    double d1 = 9.87654321, d2 = 1.23456789, dout;
    
    /* Assembly forcing mode conversions */
    __asm__ __volatile__(
        /* Char to int extension */
        "movsbl %[c1], %[io]\n\t"
        /* Short to int */
        "movswl %[s1], %%eax\n\t"
        "addl %%eax, %[io]\n\t"
        /* Float via integer register */
        "movd %[f1], %%xmm0\n\t"
        "movd %%xmm0, %%eax\n\t"
        "addl %%eax, %[io]\n\t"
        : [io] "+r"(iout), [co] "=r"(cout), [so] "=r"(sout)
        : [c1] "r"(c1), [s1] "r"(s1), [f1] "x"(f1),
          [d1] "x"(d1), [l1] "r"(l1)
        : "eax", "xmm0", "xmm1", "cc"
    );
    
    /* Different sized outputs from same input */
    __asm__ __volatile__(
        "mov %[in], %%eax\n\t"
        "mov %%al, %[c]\n\t"
        "mov %%ax, %[s]\n\t"
        "mov %%eax, %[i]\n\t"
        : [c] "=m"(cout), [s] "=m"(sout), [i] "=m"(iout)
        : [in] "r"(i1)
        : "eax", "memory"
    );
    
    /* Double to int truncation */
    __asm__ __volatile__(
        "cvttsd2si %[din], %[out]\n\t"
        : [out] "=r"(iout)
        : [din] "x"(d1)
        : 
    );
    
    return iout + cout + sout;
}

/* Test 4: Trigger secondary reloads */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Use explicit register variables */
    register int r12_var asm ("r12") = 0x12345678;
    register int r13_var asm ("r13") = 0x87654321;
    register int r14_var asm ("r14") = global_int;
    
    /* Force moves between specific registers */
    __asm__ __volatile__(
        "mov %[r12], %%eax\n\t"
        "add %[r13], %%eax\n\t"
        "mov %%eax, %[r14]\n\t"
        "mov %[r14], %0\n\t"
        : "=r"(result)
        : [r12] "r"(r12_var), [r13] "r"(r13_var), [r14] "r"(r14_var)
        : "eax", "r12", "r13", "r14", "cc"
    );
    
    /* Immediate constraints that may need secondary reloads */
    __asm__ __volatile__(
        "test %[imm], %%eax\n\t"
        "setz %%al\n\t"
        "movzx %%al, %[out]\n\t"
        : [out] "=r"(result)
        : [imm] "i"(0xFFFF), "a"(result)
        : "cc"
    );
    
    /* Memory constraint with complex addressing */
    __asm__ __volatile__(
        "lock xaddl %[val], (%[ptr])\n\t"
        : [val] "+r"(result)
        : [ptr] "r"(&global_int)
        : "memory", "cc"
    );
    
    return result;
}

/* Test 5: Explicit register variables and clobbers */
int test_register_variables(void) {
    /* Declare multiple register variables */
    register int rbx_var asm ("rbx");
    register int rcx_var asm ("rcx");
    register int rdx_var asm ("rdx");
    register int rsi_var asm ("rsi");
    register int rdi_var asm ("rdi");
    register int r8_var asm ("r8");
    register int r9_var asm ("r9");
    register int r10_var asm ("r10");
    register int r11_var asm ("r11");
    
    /* Initialize them with complex expressions */
    rbx_var = global_int + 1;
    rcx_var = func_return_int(rbx_var);
    rdx_var = global_array[rbx_var % 100];
    rsi_var = (int)(global_double * 100.0);
    rdi_var = rbx_var * rcx_var;
    r8_var = rdx_var ^ rsi_var;
    r9_var = rdi_var | r8_var;
    r10_var = ~r9_var;
    r11_var = r10_var + global_int;
    
    int final_result;
    
    /* Massive assembly using all register variables */
    __asm__ __volatile__(
        /* Chain computations through all registers */
        "mov %[rbx], %%eax\n\t"
        "add %[rcx], %%eax\n\t"
        "imul %[rdx], %%eax\n\t"
        "add %[rsi], %%eax\n\t"
        "sub %[rdi], %%eax\n\t"
        "xor %[r8], %%eax\n\t"
        "or %[r9], %%eax\n\t"
        "and %[r10], %%eax\n\t"
        "add %[r11], %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        
        /* Clobber many registers to force spills */
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        : [out] "=r"(final_result)
        : [rbx] "r"(rbx_var), [rcx] "r"(rcx_var), [rdx] "r"(rdx_var),
          [rsi] "r"(rsi_var), [rdi] "r"(rdi_var), [r8] "r"(r8_var),
          [r9] "r"(r9_var), [r10] "r"(r10_var), [r11] "r"(r11_var)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* Sequence of volatile blocks with dependencies */
    int temp = final_result;
    for (int i = 0; i < 10; i++) {
        __asm__ __volatile__(
            "addl $1, %0\n\t"
            "imull $3, %0\n\t"
            : "+r"(temp)
            :
            : "cc"
        );
    }
    
    return final_result + temp;
}
