/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_char = 'A';
int global_array[100] = {0};
volatile int volatile_global = 7;

/* Function that returns values forcing register allocation */
int func_return_int(int x) {
    return x * 2 + 1;
}

double func_return_double(double x) {
    return x * 1.5;
}

void* func_return_ptr(void* p) {
    return (char*)p + 1;
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    int result = 0;
    
    /* Complex inline assembly with many input/output operands */
    __asm__ __volatile__ (
        /* Multiple output operands with different constraints */
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[c], %%ebx\n\t"
        "imul %[d], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[e], %%ecx\n\t"
        "sub %[f], %%ecx\n\t"
        "mov %%ecx, %[out3]\n\t"
        "mov %[g], %%edx\n\t"
        "xor %[h], %%edx\n\t"
        "mov %%edx, %[out4]\n\t"
        /* Use many more registers */
        "mov %[i], %%esi\n\t"
        "add %[j], %%esi\n\t"
        "mov %%esi, %[out5]\n\t"
        "mov %[k], %%edi\n\t"
        "sub %[l], %%edi\n\t"
        "mov %%edi, %[out6]\n\t"
        /* Force spill by using all registers */
        "mov %[m], %%r8d\n\t"
        "imul %[n], %%r8d\n\t"
        "mov %%r8d, %[out7]\n\t"
        "mov %[o], %%r9d\n\t"
        "xor %[p], %%r9d\n\t"
        "mov %%r9d, %[out8]\n\t"
        "mov %[q], %%r10d\n\t"
        "add %[r], %%r10d\n\t"
        "mov %%r10d, %[out9]\n\t"
        "mov %[s], %%r11d\n\t"
        "sub %[t], %%r11d\n\t"
        "mov %%r11d, %[out10]\n\t"
        "mov %[u], %%r12d\n\t"
        "imul %[v], %%r12d\n\t"
        "mov %%r12d, %[out11]\n\t"
        "mov %[w], %%r13d\n\t"
        "xor %[x], %%r13d\n\t"
        "mov %%r13d, %[out12]\n\t"
        : [out1] "=r" (a), [out2] "=r" (b), [out3] "=r" (c),
          [out4] "=r" (d), [out5] "=r" (e), [out6] "=r" (f),
          [out7] "=r" (g), [out8] "=r" (h), [out9] "=r" (i),
          [out10] "=r" (j), [out11] "=r" (k), [out12] "=r" (l)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k), [l] "r" (l),
          [m] "r" (m), [n] "r" (n), [o] "r" (o), [p] "r" (p),
          [q] "r" (q), [r] "r" (r), [s] "r" (s), [t] "r" (t),
          [u] "r" (u), [v] "r" (v), [w] "r" (w), [x] "r" (x)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "memory"
    );
    
    result = a + b + c + d + e + f + g + h + i + j + k + l;
    return result;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'a', c2 = 'b';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000;
    long l1 = 10000, l2 = 20000;
    float f1 = 1.5, f2 = 2.5;
    double d1 = 3.14, d2 = 6.28;
    int result = 0;
    
    /* Force mode changes through casts in assembly operands */
    __asm__ __volatile__ (
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "movsx %%al, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[s1], %%ax\n\t"
        "add %[s2], %%ax\n\t"
        "movsx %%ax, %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        /* Mix float and integer operations */
        "movd %[f1], %%xmm0\n\t"
        "movd %[i1], %%xmm1\n\t"
        "cvtdq2ps %%xmm1, %%xmm1\n\t"
        "addps %%xmm0, %%xmm1\n\t"
        "movd %%xmm1, %[out3]\n\t"
        /* Double to integer conversion */
        "movq %[d1], %%xmm0\n\t"
        "cvttsd2si %%xmm0, %%eax\n\t"
        "mov %%eax, %[out4]\n\t"
        : [out1] "=r" (i1), [out2] "=r" (i2), 
          [out3] "=r" (s1), [out4] "=r" (c1)
        : [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [i1] "r" (i1), [f1] "x" (f1),
          [d1] "x" (d1)
        : "rax", "xmm0", "xmm1", "memory"
    );
    
    result = i1 + i2 + s1 + c1;
    return result;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int a, b, c, d, e, f;
    int result = 0;
    
    /* Function calls as operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[call3], %%ebx\n\t"
        "imul %[global], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[call4], %%ecx\n\t"
        "add %[volatile], %%ecx\n\t"
        "mov %%ecx, %[out3]\n\t"
        : [out1] "=r" (a), [out2] "=r" (b), [out3] "=r" (c)
        : [call1] "r" (func_return_int(10)),
          [call2] "r" (func_return_int(20)),
          [call3] "r" (func_return_int(30)),
          [call4] "r" (func_return_int(40)),
          [global] "r" (global_int),
          [volatile] "r" (volatile_global)
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Complex addressing modes with array indexing */
    __asm__ __volatile__ (
        "mov %[idx], %%eax\n\t"
        "lea global_array(,%%eax,4), %%rbx\n\t"
        "mov (%%rbx), %%ecx\n\t"
        "add %[val], %%ecx\n\t"
        "mov %%ecx, (%%rbx)\n\t"
        "mov %%ecx, %[out4]\n\t"
        : [out4] "=r" (d)
        : [idx] "r" (func_return_int(5)),
          [val] "r" (func_return_int(100))
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Pointer arithmetic in operands */
    void* ptr = global_array;
    __asm__ __volatile__ (
        "mov %[ptr], %%rax\n\t"
        "add %[offset], %%rax\n\t"
        "mov (%%rax), %%ebx\n\t"
        "mov %%ebx, %[out5]\n\t"
        : [out5] "=r" (e), [ptr] "+r" (ptr)
        : [offset] "r" (func_return_int(8) * sizeof(int))
        : "rax", "rbx", "memory"
    );
    
    result = a + b + c + d + e;
    return result;
}

/* Test 4: Explicit register variables and secondary reload triggers */
int test_explicit_registers(void) {
    /* Explicit register variables */
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    register int r14_var asm ("r14") = 300;
    register int r15_var asm ("r15") = 400;
    
    int a, b, c, d;
    int result = 0;
    
    /* Force moves between specific registers */
    __asm__ __volatile__ (
        /* Try to force secondary reloads by using specific constraints */
        "mov %[r12], %%eax\n\t"
        "add %[r13], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[r14], %%ebx\n\t"
        "sub %[r15], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        /* Use accumulator constraint */
        "mov %[out1], %%eax\n\t"
        "imul %[out2], %%eax\n\t"
        "mov %%eax, %[out3]\n\t"
        : [out1] "=a" (a), [out2] "=r" (b), [out3] "=a" (c)
        : [r12] "r" (r12_var), [r13] "r" (r13_var),
          [r14] "r" (r14_var), [r15] "r" (r15_var)
        : "rbx", "memory"
    );
    
    /* Memory constraint forcing reloads */
    __asm__ __volatile__ (
        "mov %[mem], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out4]\n\t"
        : [out4] "=r" (d)
        : [mem] "m" (global_array[10])
        : "rax", "memory"
    );
    
    result = a + b + c + d;
    return result;
}

/* Test 5: Complex chains of volatile assembly */
int test_volatile_chains(void) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1, y2, y3, y4;
    int result = 0;
    
    /* Chain 1: Interdependent volatile blocks */
    __asm__ __volatile__ (
        "mov %[x1], %%eax\n\t"
        "add $10, %%eax\n\t"
        "mov %%eax, %[y1]\n\t"
        : [y1] "=r" (y1)
        : [x1] "r" (x1)
        : "rax", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[x2], %%ebx\n\t"
        "add %[y1], %%ebx\n\t"
        "mov %%ebx, %[y2]\n\t"
        : [y2] "=r" (y2)
        : [x2] "r" (x2), [y1] "r" (y1)
        : "rbx", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[x3], %%ecx\n\t"
        "imul %[y2], %%ecx\n\t"
        "mov %%ecx, %[y3]\n\t"
        : [y3] "=r" (y3)
        : [x3] "r" (x3), [y2] "r" (y2)
        : "rcx", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[x4], %%edx\n\t"
        "xor %[y3], %%edx\n\t"
        "mov %%edx, %[y4]\n\t"
        : [y4] "=r" (y4)
        : [x4] "r" (x4), [y3] "r" (y3)
        : "rdx", "memory"
    );
    
    /* Chain 2: With immediate constraints */
    __asm__ __volatile__ (
        "mov %[y4], %%eax\n\t"
        "add $0x100, %%eax\n\t"
        "mov %%eax, %[x1]\n\t"
        : [x1] "=r" (x1)
        : [y4] "r" (y4)
        : "rax", "memory"
    );
    
    result = y1 + y2 + y3 + y4 + x1;
    return result;
}

/* Test 6: Floating point with integer register constraints */
int test_float_int_mix(void) {
    double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    float f1 = 4.4, f2 = 5.5;
    int i1, i2, i3;
    double dout;
    
    /* Force double through integer registers (with -mno-sse) */
    __asm__ __volatile__ (
        /* This should trigger complex reloads when SSE is disabled */
        "movq %[d1], %%rax\n\t"
        "movq %[d2], %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, %[dout]\n\t"
        : [dout] "=r" (dout)
        : [d1] "r" (*(long long*)&d1),
          [d2] "r" (*(long long*)&d2)
        : "rax", "rbx", "memory"
    );
    
    /* Float to int conversion */
    __asm__ __volatile__ (
        "movd %[f1], %%xmm0\n\t"
        "cvttss2si %%xmm0, %%eax\n\t"
        "mov %%eax, %[i1]\n\t"
        : [i1] "=r" (i1)
        : [f1] "x" (f1)
        : "rax", "xmm0", "memory"
    );
    
    /* Mixed constraints */
    __asm__ __volatile__ (
        "mov %[i1], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %[d3], %%xmm0\n\t"
        "cvttsd2si %%xmm0, %%eax\n\t"
        "mov %%eax, %[i2]\n\t"
        : [i2] "=r" (i2)
        : [i1] "r" (i1), [d3] "x" (d3)
        : "rax", "xmm0", "memory"
    );
    
    result = i1 + i2 + (int)dout;
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to trigger various reload patterns */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_volatile_chains();
    checksum += test_float_int_mix();
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
