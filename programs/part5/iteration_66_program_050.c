/* Test program to trigger reload.cc lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int b) { return *a + b; }
long helper2(long *a, long b, long c) { return *a * b + c; }
double helper3(double *a, double b) { return *a / b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }
int* helper5(int *a, int idx) { return &a[idx]; }

/* Complex inline assembly with register pressure */
__attribute__((noinline))
int test_reload() {
    /* Create high register pressure with many register variables */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    register int r8 asm("r8") = 9;
    register int r9 asm("r9") = 10;
    register long l0 asm("r10") = 100;
    register long l1 asm("r11") = 200;
    register long l2 asm("r12") = 300;
    register long l3 asm("r13") = 400;
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register int *p0 asm("r14") = &r0;
    register int *p1 asm("r15") = &r1;
    register int *p2 asm("rbx") = &r2;
    register int *p3 asm("rbp") = &r3;
    register int *p4 asm("rsi") = &r4;
    register int *p5 asm("rdi") = &r5;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r6 ^ r7 | r8 & r9;
    l0 = l1 * l2 + l3 - (r0 * 100);
    d0 = d1 * d2 - d3 / (d0 + 1.0);
    
    /* Complex index calculations */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = (r2 + r3) % 8;
    int idx4 = (r4 * r5) % 8;
    
    /* First inline asm: Many operands with mixed constraints */
    int out1, out2, out3;
    long out4;
    double out5;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (out1)           /* General reg output */
        "=&r" (out2)          /* Early-clobber output */
        "=m" (arr[idx1][idx2]) /* Memory output */
        "=r" (out4)           /* Long output (DImode vs SImode mismatch) */
        "=f" (out5)           /* FP reg output */
        
        /* Input operands */
        : "r" (r0)            /* General reg input */
        "m" (arr[idx3][idx4]) /* Memory input with complex addressing */
        "r" (helper1(&r1, r2)) /* Function call in operand */
        "i" (255)             /* Immediate */
        "r" (l0)              /* Long input */
        "f" (d0)              /* FP input */
        
        /* Input-output operands */
        "+r" (r6)             /* Read-write operand */
        "+m" (arr[2][3])      /* Read-write memory */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use results to prevent dead code elimination */
    r7 = out1 + out2;
    l1 = out4 + helper2(&l2, l3, r7);
    d1 = out5 * helper3(&d2, d3);
    
    /* Second inline asm: Mismatched modes and array operands */
    int arr2[4][4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                arr2[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex 3D array indexing with function calls */
    int idx5 = helper1(&r0, r1) % 4;
    int idx6 = helper1(&r2, r3) % 4;
    int idx7 = helper1(&r4, r5) % 4;
    
    /* Vector-like operation with mismatched mode (V4SImode vs SImode) */
    int vec_in[4] = {r0, r1, r2, r3};
    int vec_out[4];
    
    asm volatile (
        /* Mixed constraints causing potential reloads */
        "=r" (vec_out[0])
        "=r" (vec_out[1])
        "=m" (vec_out[2])
        "=r" (vec_out[3])
        "=m" (arr2[idx5][idx6][idx7])
        
        : "m" (vec_in[0])
        "r" (vec_in[1])
        "m" (vec_in[2])
        "r" (vec_in[3])
        "r" (helper5(arr2[idx5][idx6], idx7))
        "i" (4096)
        
        /* Input-output with complex addressing */
        "+r" (p0)
        "+m" (arr2[1][2][3])
        
        : "memory", "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    /* Third inline asm: Maximum operand count with nested function calls */
    int final_result;
    int temp1, temp2, temp3, temp4, temp5;
    
    asm volatile (
        /* 10 operands total */
        "=r" (final_result)
        "=&r" (temp1)
        "=r" (temp2)
        "=m" (temp3)
        "=r" (temp4)
        
        : "r" (helper1(&vec_out[0], vec_out[1]))
        "m" (arr[helper1(&idx1, idx2)][helper1(&idx3, idx4)])
        "r" (helper2(&l0, l1, l2))
        "i" (1000)
        "r" (helper3(&d0, d1))
        "m" (arr2[0][helper1(&idx5, idx6)][idx7])
        
        /* Input-output with address-taken arguments */
        "+r" (p1)
        "+m" (*helper5(&arr[0][0], final_result))
        
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Final computation using all results */
    int sum = final_result;
    for (int i = 0; i < 4; i++) {
        sum += vec_out[i];
    }
    sum += out1 + out2 + out3;
    sum += arr[idx1][idx2] + arr2[idx5][idx6][idx7];
    sum += temp1 + temp2 + temp3 + temp4 + temp5;
    
    /* More register pressure operations */
    r8 = sum * r9;
    l2 = l3 + r8;
    d2 = d3 * sum;
    
    /* Final array computation */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum;
}

/* Main function to call the test */
int main() {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
