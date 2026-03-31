/* Test program to trigger reload.cc lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int b) { return *a + b; }
int helper2(long *a, int b) { return (int)(*a) * b; }
double helper3(double *a, double b) { return *a + b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }
int helper5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }

/* Complex inline assembly with register pressure */
__attribute__((noinline))
int test_reload() {
    /* 20+ register variables to create high register pressure */
    register int r0 asm("r8") = 1;
    register int r1 asm("r9") = 2;
    register int r2 asm("r10") = 3;
    register int r3 asm("r11") = 4;
    register int r4 asm("r12") = 5;
    register int r5 asm("r13") = 6;
    register int r6 asm("r14") = 7;
    register int r7 asm("r15") = 8;
    register long l0 asm("rax") = 100;
    register long l1 asm("rbx") = 200;
    register long l2 asm("rcx") = 300;
    register long l3 asm("rdx") = 400;
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register double d4 asm("xmm4") = 5.5;
    register double d5 asm("xmm5") = 6.6;
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register volatile int v0 asm("ebp") = 42;
    register volatile long v1 asm("esp") = 99;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 + d3 / d4 - d5;
    
    /* Complex inline assembly block 1: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands with mixed constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4], %[in5], 2), %%rbx\n\t"
        "mov %%rbx, %[out2]\n\t"
        "movsd %[in6], %%xmm6\n\t"
        "addsd %[in7], %%xmm6\n\t"
        "movsd %%xmm6, %[out3]\n\t"
        "mov %[in8], %%ecx\n\t"
        "add %[in9], %%ecx\n\t"
        "mov %%ecx, %[out4]"
        : [out1] "=r" (result1),          /* output constraint */
          [out2] "=&r" (result3),         /* early-clobber output */
          [out3] "=x" (result4),          /* xmm register constraint */
          [out4] "+r" (r0)                /* read-write operand */
        : [in1] "r" (r1),                 /* input in register */
          [in2] "m" (arr[2][3]),          /* memory operand */
          [in3] "r" (helper1(&r2, r3)),   /* function call in operand */
          [in4] "r" (l0),                 /* long in register */
          [in5] "r" (l1),                 /* long in register */
          [in6] "x" (d0),                 /* xmm input */
          [in7] "m" (arr[3][4]),          /* mismatched mode: int in memory used as double */
          [in8] "i" (100),                /* immediate */
          [in9] "r" (helper2(&l2, r4))    /* function call with address */
        : "eax", "rbx", "rcx", "xmm6", "memory", "cc"
    );
    
    /* More arithmetic to maintain register pressure */
    r1 = r0 * 2 + result1;
    l1 = l0 + result3;
    d1 = d0 + result4;
    
    /* Complex inline assembly block 2: Array indexing with mismatched modes */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    int idx4 = r3 % 8;
    
    asm volatile (
        /* Multi-dimensional array access with complex addressing */
        "mov %[arrbase], %%rsi\n\t"
        "mov %[idx1], %%eax\n\t"
        "shl $5, %%rax\n\t"              /* i * 8 * 4 */
        "add %[idx2], %%eax\n\t"
        "shl $2, %%rax\n\t"              /* * sizeof(int) */
        "add %%rsi, %%rax\n\t"
        "mov (%%rax), %%ebx\n\t"
        
        "mov %[idx3], %%ecx\n\t"
        "shl $5, %%rcx\n\t"
        "add %[idx4], %%ecx\n\t"
        "shl $2, %%rcx\n\t"
        "add %%rsi, %%rcx\n\t"
        "add (%%rcx), %%ebx\n\t"
        
        "mov %%ebx, %[out1]\n\t"
        
        /* Floating point with mismatched constraints */
        "cvtsi2sd %[in5], %%xmm7\n\t"
        "addsd %[in6], %%xmm7\n\t"
        "movsd %%xmm7, %[out2]"
        : [out1] "=r" (result1),
          [out2] "=x" (result4)
        : [arrbase] "r" (arr),           /* array base pointer */
          [idx1] "r" (idx1),             /* index in register */
          [idx2] "r" (idx2),             /* index in register */
          [idx3] "r" (idx3),             /* index in register */
          [idx4] "r" (idx4),             /* index in register */
          [in5] "r" (helper5(r0, r1, r2, r3, r4)),  /* function call */
          [in6] "x" (d2)                 /* xmm register */
        : "rax", "rbx", "rcx", "rsi", "xmm7", "memory", "cc"
    );
    
    /* Complex inline assembly block 3: Input-output operands with clobbers */
    int io1 = r4;
    long io2 = l2;
    double io3 = d3;
    
    asm volatile (
        /* Mixed size operations forcing reloads */
        "mov %[io1], %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %[io1]\n\t"
        
        "mov %[io2], %%rbx\n\t"
        "shl $2, %%rbx\n\t"
        "mov %%rbx, %[io2]\n\t"
        
        "movsd %[io3], %%xmm8\n\t"
        "mulsd %[in3], %%xmm8\n\t"
        "movsd %%xmm8, %[io3]\n\t"
        
        /* Memory operand with complex addressing */
        "mov %[in4], %%rcx\n\t"
        "mov %[in5], %%edx\n\t"
        "imul %%edx, %%ecx\n\t"
        "mov %%ecx, %[out1]"
        : [io1] "+r" (io1),              /* input-output */
          [io2] "+&r" (io2),             /* early-clobber input-output */
          [io3] "+x" (io3),              /* xmm input-output */
          [out1] "=r" (result2)
        : [in3] "x" (d4),                /* xmm input */
          [in4] "r" (helper1(&io1, io1)), /* function with address */
          [in5] "m" (arr[idx1][idx2])    /* array element in memory */
        : "rax", "rbx", "rcx", "rdx", "xmm8", "memory", "cc"
    );
    
    /* Use all results to prevent dead code elimination */
    int final_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_result += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_result += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_result += result1 + result2 + (int)result3 + (int)result4;
    final_result += io1 + (int)io2 + (int)io3;
    
    /* More array operations with complex addressing */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] += helper1(&final_result, arr[i][j]);
        }
    }
    
    /* Final complex asm with memory clobber */
    asm volatile (
        "mov %[val], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[val]"
        : [val] "+r" (final_result)
        :
        : "eax", "memory"
    );
    
    return final_result;
}

/* Main function to call the test */
int main() {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
