/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload pass by creating inline
 * assembly patterns that force the register allocator to generate many
 * reloads, including secondary reloads. The goal is to trigger the
 * initialization block in push_reload() (lines 1381-1399 of reload.cc)
 * under varied conditions.
 *
 * Compilation recommendations:
 *   gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -c reload_stress.c
 *   gcc -O3 -funroll-loops -fno-optimize-sibling-calls -march=x86-64 -mno-sse -mno-avx -c reload_stress.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_int = 12345;
volatile double global_double = 3.14159;
volatile char global_char = 'A';
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Helper functions to use in assembly operands */
int get_int(void) {
    return global_int * 2;
}

double get_double(void) {
    return global_double * 2.0;
}

int* get_ptr(void) {
    return &global_array[10];
}

/* Test 1: Many operands with mixed types and constraints */
int test_many_operands(void) {
    int out1, out2, out3;
    int in1 = get_int();
    int in2 = global_int;
    double d1 = get_double();
    double d2 = global_double;
    char c1 = global_char;
    short s1 = (short)global_int;
    int* ptr1 = get_ptr();
    int* ptr2 = &global_array[20];
    
    /* Complex inline assembly with many operands of different types
     * and constraints to exhaust registers and force reloads */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[d1], %%xmm0\n\t"   /* Will be forced to integer regs with -mno-sse */
        "mov %[d2], %%xmm1\n\t"
        "movd %%xmm0, %%ebx\n\t"
        "movd %%xmm1, %%ecx\n\t"
        "add %%ebx, %%ecx\n\t"
        "mov %%ecx, %[out2]\n\t"
        "movzbl %[c1], %%edx\n\t"
        "addw %[s1], %%dx\n\t"
        "mov %[ptr1], %%esi\n\t"
        "mov %[ptr2], %%edi\n\t"
        "sub %%esi, %%edi\n\t"
        "add %%edi, %%edx\n\t"
        "mov %%edx, %[out3]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [in1] "r" (in1), [in2] "r" (in2),
          [d1] "r" (*(uint64_t*)&d1), [d2] "r" (*(uint64_t*)&d2),  /* Cast to integer for -mno-sse */
          [c1] "r" ((int)c1), [s1] "r" ((int)s1),
          [ptr1] "r" (ptr1), [ptr2] "r" (ptr2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 2: Nested function calls in operands and complex addressing */
int test_nested_calls(void) {
    int result1, result2;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "mov %[call2], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "mov %[idx1], %%ecx\n\t"
        "mov %[idx2], %%edx\n\t"
        "lea (%[arr], %%ecx, 4), %%esi\n\t"
        "lea (%[arr], %%edx, 4), %%edi\n\t"
        "mov (%%esi), %%ecx\n\t"
        "mov (%%edi), %%edx\n\t"
        "add %%ecx, %%edx\n\t"
        "mov %%edx, %[res2]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [call1] "r" (get_int()), [call2] "r" (get_int() + 1),
          [arr] "r" (global_array), 
          [idx1] "r" (get_int() % 50), [idx2] "r" (get_int() % 30 + 10)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return result1 + result2;
}

/* Test 3: Explicit register variables forcing moves */
int test_explicit_registers(void) {
    register int r1 asm ("r12") = get_int();
    register int r2 asm ("r13") = global_int * 2;
    register double d1 asm ("r14") = *(uint64_t*)&global_double;  /* Integer representation */
    int out1, out2;
    
    /* Using explicit registers that may need to be moved for constraints */
    __asm__ __volatile__ (
        "mov %[reg1], %%eax\n\t"   /* Force move from r12 to eax */
        "add %[reg2], %%eax\n\t"   /* r13 may need secondary reload */
        "mov %%eax, %[out1]\n\t"
        "mov %[reg3], %%ebx\n\t"   /* r14 may need special handling */
        "add $100, %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [reg1] "r" (r1), [reg2] "r" (r2), [reg3] "r" (d1)
        : "eax", "ebx", "memory"
    );
    
    return out1 + out2;
}

/* Test 4: Mixed modes and type conversions */
int test_mixed_modes(void) {
    char c1 = global_char;
    short s1 = (short)global_int;
    int i1 = get_int();
    long long ll1 = (long long)global_int * 1000;
    float f1 = (float)global_double;
    double d1 = get_double();
    int out_int;
    short out_short;
    char out_char;
    
    /* Mixed types requiring different machine modes */
    __asm__ __volatile__ (
        "movsbl %[c_in], %%eax\n\t"      /* char -> int */
        "movswl %[s_in], %%ebx\n\t"      /* short -> int */
        "add %%ebx, %%eax\n\t"
        "add %[i_in], %%eax\n\t"
        "mov %%eax, %[i_out]\n\t"
        "mov %[ll_lo], %%eax\n\t"        /* long long low part */
        "mov %[ll_hi], %%edx\n\t"        /* long long high part */
        "add $1, %%eax\n\t"
        "adc $0, %%edx\n\t"
        "mov %%ax, %[s_out]\n\t"         /* truncate to short */
        "mov %[f_in], %%ecx\n\t"         /* float as integer */
        "mov %[d_in], %%esi\n\t"         /* double as integer (low) */
        "add %%ecx, %%esi\n\t"
        "mov %%sil, %[c_out]\n\t"        /* truncate to char */
        : [i_out] "=r" (out_int), [s_out] "=r" (out_short), [c_out] "=r" (out_char)
        : [c_in] "r" (c1), [s_in] "r" (s1), [i_in] "r" (i1),
          [ll_lo] "r" ((int)(ll1 & 0xFFFFFFFF)), [ll_hi] "r" ((int)(ll1 >> 32)),
          [f_in] "r" (*(int*)&f1), [d_in] "r" (*(int*)&d1)  /* Use low 32 bits */
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    return out_int + out_short + out_char;
}

/* Test 5: Secondary reload triggers with specific constraints */
int test_secondary_reloads(void) {
    int in1 = get_int();
    int in2 = global_int + 100;
    double d1 = global_double;
    int out1, out2;
    
    /* Using "a" constraint (accumulator) which may require secondary reloads
     * when values are in other registers */
    __asm__ __volatile__ (
        "mov %[in2], %%ebx\n\t"
        /* Force in1 into eax via constraint */
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Use memory operand that may need base+index addressing */
        "mov %[d1], %%ecx\n\t"
        "add $0x1000, %%ecx\n\t"
        "mov %%ecx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), "+a" (in1)
        : [in2] "r" (in2), [d1] "r" (*(uint64_t*)&d1)
        : "ebx", "ecx", "memory"
    );
    
    return out1 + out2;
}

/* Test 6: Chain of volatile assembly blocks with interdependencies */
int test_chain_reloads(void) {
    int val1 = get_int();
    int val2 = global_int;
    int val3, val4, val5, val6;
    
    /* Chain 1: val1 -> val3 */
    __asm__ __volatile__ (
        "mov %[v1], %%eax\n\t"
        "add $10, %%eax\n\t"
        "mov %%eax, %[v3]\n\t"
        : [v3] "=r" (val3)
        : [v1] "r" (val1)
        : "eax", "memory"
    );
    
    /* Chain 2: val2 -> val4, using val3 */
    __asm__ __volatile__ (
        "mov %[v2], %%ebx\n\t"
        "add %[v3], %%ebx\n\t"
        "mov %%ebx, %[v4]\n\t"
        : [v4] "=r" (val4)
        : [v2] "r" (val2), [v3] "r" (val3)
        : "ebx", "memory"
    );
    
    /* Chain 3: Mix with memory clobber */
    __asm__ __volatile__ (
        "mov %[v3], %%ecx\n\t"
        "mov %[v4], %%edx\n\t"
        "imul %%edx, %%ecx\n\t"
        "mov %%ecx, %[v5]\n\t"
        : [v5] "=r" (val5)
        : [v3] "r" (val3), [v4] "r" (val4)
        : "ecx", "edx", "memory"
    );
    
    /* Chain 4: Final result */
    __asm__ __volatile__ (
        "mov %[v5], %%esi\n\t"
        "add $1000, %%esi\n\t"
        "mov %%esi, %[v6]\n\t"
        : [v6] "=r" (val6)
        : [v5] "r" (val5)
        : "esi", "memory"
    );
    
    return val3 + val4 + val5 + val6;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 50; i++) {
        global_darray[i] = i * 0.5;
    }
    
    /* Run all tests to stress reload pass */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_mixed_modes();
    checksum += test_secondary_reloads();
    checksum += test_chain_reloads();
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        "add $1, %0\n\t"
        : "+r" (checksum)
        :
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}
