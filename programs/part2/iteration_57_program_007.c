/* reload_stress.c - Stress test for GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create register pressure */
int global_int1 = 12345;
int global_int2 = 67890;
int global_int3 = 11111;
int global_int4 = 22222;
double global_double1 = 3.14159;
double global_double2 = 2.71828;
char global_char_array[256];
int global_int_array[100];

/* Function prototypes */
int test_function1(void);
int test_function2(void);
int test_function3(void);
int test_function4(void);
int test_function5(void);

/* Helper functions that return values */
int get_value1(void) { return global_int1; }
int get_value2(void) { return global_int2; }
double get_double1(void) { return global_double1; }
int* get_pointer(void) { return global_int_array; }
char* get_char_pointer(void) { return global_char_array; }

/* Complex addressing helper */
int complex_index(int base, int offset) {
    return base + offset * 2 + (base % 7);
}

/* Test 1: Many operands with mixed constraints */
int test_function1(void) {
    int out1, out2, out3, out4;
    int in1 = get_value1();
    int in2 = get_value2();
    double d1 = get_double1();
    int* ptr = get_pointer();
    char* cptr = get_char_pointer();
    
    /* Explicit register variables */
    register int reg_var1 asm ("r12") = in1 * 2;
    register int reg_var2 asm ("r13") = in2 + 100;
    register double reg_double asm ("xmm0") = d1 * 2.0;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[reg1], %[out1]\n\t"
        "mov %[out1], %[out2]\n\t"
        "add $0x7, %[out2]\n\t"
        /* Memory operations with complex addressing */
        "movl (%[ptr],%[in1],4), %[out3]\n\t"
        /* Floating point to integer conversion */
        "cvtsd2si %[dreg], %[out4]\n\t"
        /* Use explicit register variables */
        "add %%r12, %[out4]\n\t"
        "add %%r13, %[out4]"
        
        : [out1] "=r" (out1), 
          [out2] "=m" (out2),  /* Memory constraint forces spills */
          [out3] "=r" (out3),
          [out4] "=r" (out4)
        
        : [in1] "r" (in1),
          [in2] "i" (0x1234),  /* Immediate constraint */
          [reg1] "r" (reg_var1),
          [ptr] "r" (ptr),
          [dreg] "x" (reg_double),
          "r" (reg_var2)       /* Input in fixed register */
        
        : "memory", "cc", "rax", "rbx", "rcx", "rdx", 
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Chain of volatile assembly to create register pressure */
    __asm__ __volatile__ (
        "mov %1, %0\n\t"
        "add %2, %0"
        : "=r" (out1)
        : "r" (out1), "m" (out2)
        : "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
int test_function2(void) {
    int result1, result2;
    double dresult;
    
    /* Function calls directly in asm operands */
    __asm__ __volatile__ (
        "mov %2, %0\n\t"
        "add %3, %0\n\t"
        "cvtsi2sd %0, %1"
        : "=r" (result1), "=x" (dresult)
        : "r" (get_value1()), 
          "r" (complex_index(get_value2(), 5)),  /* Nested call */
          "m" (global_int3)  /* Memory operand */
        : "memory", "cc", "rax", "xmm0", "xmm1"
    );
    
    /* More complex addressing with pointer arithmetic */
    int* ptr = global_int_array + get_value1() % 50;
    __asm__ __volatile__ (
        "mov (%[ptr],%[idx],4), %[out]"
        : [out] "=r" (result2)
        : [ptr] "r" (ptr),
          [idx] "r" (complex_index(get_value2(), 3))  /* Another nested call */
        : "memory"
    );
    
    return result1 + result2 + (int)dresult;
}

/* Test 3: Mixed data types and mode changes */
int test_function3(void) {
    char c1 = 'A';
    short s1 = 1000;
    int i1 = 100000;
    long l1 = 1000000000L;
    float f1 = 1.5f;
    double d1 = 2.5;
    
    int out_int;
    char out_char;
    double out_double;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        /* Convert and mix types */
        "movsx %[c1], %[out_int]\n\t"
        "add %[s1], %[out_int]\n\t"
        "add %[i1], %[out_int]\n\t"
        /* Floating point operations */
        "cvtsi2sd %[out_int], %[out_double]\n\t"
        "addsd %[d1], %[out_double]\n\t"
        /* Convert back to char */
        "cvtsd2si %[out_double], %[out_int]\n\t"
        "mov %b[out_int], %[out_char]"
        
        : [out_int] "=r" (out_int),
          [out_char] "=r" (out_char),
          [out_double] "=x" (out_double)
        
        : [c1] "r" ((int)c1),  /* Cast forces mode change */
          [s1] "r" ((int)s1),
          [i1] "r" (i1),
          [l1] "r" (l1),
          [f1] "x" (f1),
          [d1] "x" (d1)
        
        : "memory", "cc", "xmm0", "xmm1", "xmm2"
    );
    
    /* Force mode changes with explicit casts in operands */
    __asm__ __volatile__ (
        ""
        : 
        : "r" ((short)out_int),  /* Different mode */
          "r" ((char)out_int),   /* Another mode */
          "x" ((float)out_double) /* Float mode */
        : "memory"
    );
    
    return out_int + out_char + (int)out_double;
}

/* Test 4: Secondary reload triggers */
int test_function4(void) {
    int result;
    
    /* Use specific register constraints that may require secondary reloads */
    register int ax_var asm ("ax");
    register int bx_var asm ("bx");
    register int flags_var asm ("flags");
    
    ax_var = get_value1();
    bx_var = get_value2();
    
    /* Assembly that uses specific registers */
    __asm__ __volatile__ (
        /* Force use of AX for specific operation */
        "mov %1, %%ax\n\t"
        "add %2, %%ax\n\t"
        "test %%ax, %%ax\n\t"  /* Sets flags */
        "mov %%ax, %0\n\t"
        /* Try to move flags result - may need secondary reload */
        "lahf\n\t"
        "mov %%ah, %b[flags]"
        
        : "=r" (result), [flags] "=r" (flags_var)
        : "a" (ax_var), "b" (bx_var),  /* Specific register constraints */
          "m" (global_int_array[0])     /* Memory operand */
        : "memory", "cc", "ah"
    );
    
    /* More complex case with mismatched register classes */
    double dval = get_double1();
    int ival;
    
    __asm__ __volatile__ (
        "cvtsd2si %[dval], %[ival]\n\t"
        /* Try to use result in specific register */
        "mov %[ival], %%eax\n\t"
        "imul %%eax, %%eax"
        
        : [ival] "=a" (ival)  /* Must be in EAX */
        : [dval] "x" (dval)   /* XMM register */
        : "memory", "cc", "edx"  /* EDX clobbered by imul */
    );
    
    return result + ival + flags_var;
}

/* Test 5: Maximum register pressure with many live values */
int test_function5(void) {
    /* Declare many local variables to use up registers */
    int v1 = get_value1();
    int v2 = get_value2();
    int v3 = v1 * 2;
    int v4 = v2 + v1;
    int v5 = complex_index(v1, v2);
    int v6 = global_int3;
    int v7 = global_int4;
    double d1 = get_double1();
    double d2 = global_double2;
    int* p1 = get_pointer();
    char* p2 = get_char_pointer();
    
    int out1, out2, out3, out4, out5, out6;
    double dout1, dout2;
    
    /* Massive asm with many operands */
    __asm__ __volatile__ (
        /* Chain of operations using all variables */
        "mov %[v1], %[o1]\n\t"
        "add %[v2], %[o1]\n\t"
        "mov %[o1], %[o2]\n\t"
        "imul %[v3], %[o2]\n\t"
        "mov %[v4], %[o3]\n\t"
        "add %[v5], %[o3]\n\t"
        "mov (%[p1],%[v6],4), %[o4]\n\t"
        "movsx (%[p2],%[v7]), %[o5]\n\t"
        /* Floating point operations */
        "cvtsi2sd %[o1], %[do1]\n\t"
        "addsd %[d1], %[do1]\n\t"
        "cvtsi2sd %[o2], %[do2]\n\t"
        "mulsd %[d2], %[do2]\n\t"
        /* Final integer result */
        "cvtsd2si %[do1], %[o6]\n\t"
        "add %[o3], %[o6]\n\t"
        "add %[o4], %[o6]\n\t"
        "add %[o5], %[o6]"
        
        : [o1] "=r" (out1), [o2] "=r" (out2),
          [o3] "=r" (out3), [o4] "=r" (out4),
          [o5] "=r" (out5), [o6] "=r" (out6),
          [do1] "=x" (dout1), [do2] "=x" (dout2)
        
        : [v1] "r" (v1), [v2] "r" (v2), [v3] "r" (v3),
          [v4] "r" (v4), [v5] "r" (v5), [v6] "r" (v6),
          [v7] "r" (v7), [d1] "x" (d1), [d2] "x" (d2),
          [p1] "r" (p1), [p2] "r" (p2),
          "m" (global_int_array[10]),  /* Extra memory operand */
          "m" (global_char_array[20])  /* Another memory operand */
        
        : "memory", "cc", 
          "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
    );
    
    /* Sequence of volatile asm blocks to prevent optimization */
    for (int i = 0; i < 5; i++) {
        __asm__ __volatile__ (
            "add $1, %0"
            : "+r" (out6)
            :
            : "cc"
        );
    }
    
    return out1 + out2 + out3 + out4 + out5 + out6 + (int)dout1 + (int)dout2;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 3;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_function1();
    checksum += test_function2();
    checksum += test_function3();
    checksum += test_function4();
    checksum += test_function5();
    
    /* Final volatile barrier */
    __asm__ __volatile__ (
        ""
        :
        :
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256;  /* Return deterministic value */
}
