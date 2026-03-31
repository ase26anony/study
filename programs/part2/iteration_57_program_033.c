/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_char_array[256];
int global_int_array[100];

/* Helper functions for nested calls in asm operands */
int func_return_int(int x) {
    return x * 2 + 1;
}

double func_return_double(double x) {
    return x * 1.5;
}

void* func_return_ptr(void* p) {
    return (char*)p + 1;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int result = 0;
    
    /* Declare many register variables to consume registers */
    register int r0 asm ("eax") = 1;
    register int r1 asm ("ebx") = 2;
    register int r2 asm ("ecx") = 3;
    register int r3 asm ("edx") = 4;
    register int r4 asm ("esi") = 5;
    register int r5 asm ("edi") = 6;
    
    int out1, out2, out3, out4, out5, out6;
    int in1 = 10, in2 = 20, in3 = 30, in4 = 40, in5 = 50, in6 = 60;
    double d1 = 1.1, d2 = 2.2;
    char c1 = 'A', c2 = 'B';
    short s1 = 100, s2 = 200;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out2]\n\t"
        "imull %[r0], %[out3]\n\t"
        "movl %[r1], %[out4]\n\t"
        "leal (%[r2],%[r3],2), %[out5]\n\t"
        "movl %[r4], %[out6]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5), [out6] "=r" (out6)
        
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "i" (30),
          [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2),
          [r3] "r" (r3), [r4] "r" (r4),
          "m" (global_int_array), "m" (global_char_array)
        
        : "memory", "cc"
    );
    
    result = out1 + out2 + out3 + out4 + out5 + out6;
    return result;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result = 0;
    int out1, out2, out3;
    double dout;
    
    /* Using function calls directly in asm operands */
    __asm__ __volatile__ (
        "movl %%eax, %[out1]\n\t"
        "addl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        
        : "a" (func_return_int(global_int)), 
          "b" (func_return_int(global_int + 1)),
          "c" (global_int_array[func_return_int(2)]),
          "m" (global_int), "m" (global_int_array)
        
        : "memory"
    );
    
    /* Mixed types with pointer arithmetic */
    char* ptr = global_char_array;
    __asm__ __volatile__ (
        "movb (%[ptr]), %%al\n\t"
        "movsbl %%al, %[out1]"
        
        : [out1] "=r" (out1)
        
        : [ptr] "r" (ptr + func_return_int(global_int) % 128),
          "m" (global_char_array)
        
        : "al", "memory"
    );
    
    result = out1 + out2 + out3;
    return result;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    char c1 = 'X', c2 = 'Y';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000;
    double d1 = 123.456;
    float f1 = 789.012f;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Force mode changes by using different sized operands */
    __asm__ __volatile__ (
        /* char to int extension */
        "movsbl %[c1], %%eax\n\t"
        "addl %[i1], %%eax\n\t"
        "movl %%eax, %[out_int]\n\t"
        
        /* short operations */
        "movswl %[s1], %%ebx\n\t"
        "addw %[s2], %%bx\n\t"
        "movw %%bx, %[out_short]\n\t"
        
        /* char operations */
        "movb %[c2], %%cl\n\t"
        "addb $1, %%cl\n\t"
        "movb %%cl, %[out_char]"
        
        : [out_int] "=r" (out_int), [out_short] "=r" (out_short),
          [out_char] "=r" (out_char)
        
        : [c1] "r" (c1), [c2] "r" (c2), [s1] "r" (s1), [s2] "r" (s2),
          [i1] "r" (i1), "m" (global_int)
        
        : "eax", "ebx", "ecx", "memory", "cc"
    );
    
    /* Double through integer registers (no SSE) */
    long long d_as_int;
    memcpy(&d_as_int, &d1, sizeof(double));
    
    __asm__ __volatile__ (
        "movq %[dval], %%rax\n\t"
        "addq $0x1000, %%rax\n\t"
        "movq %%rax, %[out]"
        
        : [out] "=r" (d_as_int)
        
        : [dval] "r" (d_as_int)
        
        : "rax", "memory"
    );
    
    memcpy(&out_double, &d_as_int, sizeof(double));
    
    result = out_int + out_short + out_char + (int)out_double;
    return result;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Use specific register constraints that may require secondary reloads */
    register int must_be_eax asm ("eax") = 0x1234;
    register int must_be_ebx asm ("ebx") = 0x5678;
    register int must_be_ecx asm ("ecx") = 0x9ABC;
    
    int out1, out2, out3;
    
    /* Complex asm with specific register constraints and memory operands */
    __asm__ __volatile__ (
        /* Force moves between specific registers */
        "xchgl %%eax, %%ebx\n\t"
        "xchgl %%ebx, %%ecx\n\t"
        "xchgl %%ecx, %%eax\n\t"
        
        /* Memory operations that might need reloads */
        "movl %[mem1], %%edx\n\t"
        "addl %%eax, %%edx\n\t"
        "movl %%edx, %[out1]\n\t"
        
        "movl %[mem2], %%esi\n\t"
        "imull %%ebx, %%esi\n\t"
        "movl %%esi, %[out2]\n\t"
        
        "movl %[mem3], %%edi\n\t"
        "orl %%ecx, %%edi\n\t"
        "movl %%edi, %[out3]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        
        : "a" (must_be_eax), "b" (must_be_ebx), "c" (must_be_ecx),
          [mem1] "m" (global_int),
          [mem2] "m" (global_int_array[10]),
          [mem3] "m" (global_int_array[20]),
          "m" (global_int_array)
        
        : "edx", "esi", "edi", "memory", "cc"
    );
    
    result = out1 + out2 + out3;
    
    /* Another test with immediate constraints */
    __asm__ __volatile__ (
        "movl $0xDEADBEEF, %[out1]"
        
        : [out1] "=r" (out1)
        
        : "i" (0x1234), "i" (0x5678)
        
        : "memory"
    );
    
    result += out1;
    return result;
}

/* Test 5: Complex addressing modes */
int test_complex_addressing(void) {
    int result = 0;
    int index = 16;
    int scale = 2;
    int offset = 8;
    
    int out1, out2, out3;
    
    /* Complex addressing modes that might need reloads */
    __asm__ __volatile__ (
        /* Base + index * scale + displacement */
        "movl %[base](,%[idx],%[scale]), %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        /* More complex with function call in address computation */
        "leal (%[base],%[idx],4), %%ebx\n\t"
        "movl (%%ebx), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        
        /* Even more complex */
        "movl %[offset](%[base],%[idx],%[scale]), %%edx\n\t"
        "movl %%edx, %[out3]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        
        : [base] "r" (global_int_array), 
          [idx] "r" (func_return_int(index) % 50),
          [scale] "i" (sizeof(int)),
          [offset] "i" (offset),
          "m" (global_int_array)
        
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    result = out1 + out2 + out3;
    return result;
}

/* Test 6: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int result = 0;
    int val1 = 1, val2 = 2, val3 = 3, val4 = 4;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Chain of volatile asm blocks creating register pressure */
    __asm__ __volatile__ (
        "addl $1, %[v1]\n\t"
        "movl %[v1], %[t1]"
        
        : [t1] "=r" (tmp1)
        : [v1] "0" (val1)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "imull %[t1], %[v2]\n\t"
        "movl %[v2], %[t2]"
        
        : [t2] "=r" (tmp2)
        : [v2] "0" (val2), [t1] "r" (tmp1)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "xorl %[t1], %[v3]\n\t"
        "orl %[t2], %[v3]\n\t"
        "movl %[v3], %[t3]"
        
        : [t3] "=r" (tmp3)
        : [v3] "0" (val3), [t1] "r" (tmp1), [t2] "r" (tmp2)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "leal (%[t1],%[t2],2), %[v4]\n\t"
        "addl %[t3], %[v4]\n\t"
        "movl %[v4], %[t4]"
        
        : [t4] "=r" (tmp4)
        : [v4] "0" (val4), [t1] "r" (tmp1), [t2] "r" (tmp2), [t3] "r" (tmp3)
        : "memory", "cc"
    );
    
    result = tmp1 + tmp2 + tmp3 + tmp4;
    return result;
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
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_complex_addressing();
    checksum += test_volatile_chains();
    
    /* Final asm to ensure all values are used */
    __asm__ __volatile__ (
        "addl $1, %0"
        : "+r" (checksum)
        :
        : "memory", "cc"
    );
    
    printf("Final checksum: %d\n", checksum);
    return checksum;
}
