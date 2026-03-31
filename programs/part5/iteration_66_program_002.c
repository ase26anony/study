/* Complex inline assembly test to trigger reload.cc logic */
#include <stdio.h>
#include <stdint.h>

/* Helper functions that take addresses */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b + 1.0;
}

void helper4(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2 + i;
    }
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long l0 asm("r8") = 100;
    register long l1 asm("r9") = 200;
    register long l2 asm("r10") = 300;
    register long l3 asm("r11") = 400;
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register int r14 = 15, r15 = 16, r16 = 17, r17 = 18;
    
    /* Phase 2: Multi-dimensional array */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Phase 3: Complex arithmetic to create live ranges */
    for (int i = 0; i < 100; i++) {
        r0 = r1 + r2 * r3 - r4 / (r5 + 1);
        r1 = r2 ^ r3 | r4 & r5;
        l0 = l1 * l2 + l3 - (r0 * r1);
        l1 = l2 / (l3 + 1) * l0;
        d0 = d1 * d2 + d3 / d0;
        d1 = d2 - d3 * d0 + d1;
        
        /* Use pointers */
        *p0 = *p1 + r0;
        *p2 = *p2 + l0;
        *p3 = *p3 + d0;
        
        /* Array operations */
        arr[r0 & 7][r1 & 7] = arr[r2 & 7][r3 & 7] + arr[r4 & 7][r5 & 7];
    }
    
    /* Phase 4: First complex inline asm with many operands */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands mixing constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[in4], %%ebx\n\t"
        "sub %[in5], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%rcx\n\t"
        "add %[in7], %%rcx\n\t"
        "mov %%rcx, %[out3]\n\t"
        "movsd %[in8], %%xmm4\n\t"
        "addsd %[in9], %%xmm4\n\t"
        "movsd %%xmm4, %[out4]\n\t"
        : [out1] "=&r" (result1),      /* early clobber */
          [out2] "=r" (result2),
          [out3] "=&r" (result3),      /* early clobber */
          [out4] "=x" (result4),
          [inout1] "+r" (r6)           /* input-output */
        : [in1] "r" (r0),
          [in2] "r" (r1),
          [in3] "m" (arr[r2 & 7][r3 & 7]),  /* memory operand */
          [in4] "r" (r2),
          [in5] "r" (r3),
          [in6] "r" (l0),
          [in7] "r" (l1),
          [in8] "x" (d0),              /* xmm register constraint */
          [in9] "x" (d1),
          [mem] "m" (arr[0][0])        /* memory constraint */
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc", "memory"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + r0;
    d0 = result4 + r0;
    
    /* Phase 5: Second asm with mismatched modes and function calls */
    int idx1 = r0 & 7;
    int idx2 = r1 & 7;
    int idx3 = r2 & 7;
    int idx4 = r3 & 7;
    
    /* Call helper functions within asm operands */
    int complex_val1 = helper1(&arr[idx1][idx2], &arr[idx3][idx4]);
    long complex_val2 = helper2(&l0, &l1, &l2);
    double complex_val3 = helper3(&d0, &d1);
    
    /* Modify array through helper */
    helper4(&arr[0][0], 64);
    
    long long wide_result;
    int narrow_result;
    
    asm volatile (
        /* Mismatched modes: DImode value in SImode constraint */
        "mov %[wide_in], %%rax\n\t"
        "shr $32, %%rax\n\t"
        "mov %%eax, %[narrow_out]\n\t"
        /* Array element with complex addressing */
        "mov %[arr_elem1], %%rbx\n\t"
        "add %[arr_elem2], %%rbx\n\t"
        "mov %%rbx, %[wide_out]\n\t"
        /* FP operation with integer constraint mismatch */
        "cvtsi2sd %[int_in], %%xmm6\n\t"
        "addsd %[fp_in], %%xmm6\n\t"
        "movsd %%xmm6, %[fp_out]\n\t"
        : [narrow_out] "=r" (narrow_result),   /* SImode output */
          [wide_out] "=&r" (wide_result),      /* DImode output, early clobber */
          [fp_out] "=x" (d2)                   /* DFmode output */
        : [wide_in] "r" ((long long)l0 * l1),  /* DImode input */
          [arr_elem1] "m" (arr[helper1(&idx1, &idx2) & 7][helper1(&idx3, &idx4) & 7]), /* complex addressing */
          [arr_elem2] "m" (arr[complex_val1 & 7][complex_val2 & 7]),
          [int_in] "r" (complex_val1),         /* integer in FP operation */
          [fp_in] "x" (complex_val3)           /* float in FP operation */
        : "rax", "rbx", "rcx", "rdx",
          "xmm6", "xmm7", "xmm8", "xmm9",
          "cc", "memory"
    );
    
    /* Phase 6: Third asm with input-output operands and more pressure */
    int io1 = r4, io2 = r5, io3 = r6;
    long io4 = l3;
    double io5 = d3;
    
    asm volatile (
        /* Multiple input-output operands */
        "add %[inc1], %[io1]\n\t"
        "sub %[inc2], %[io2]\n\t"
        "imul %[inc3], %[io3]\n\t"
        "add %[inc4], %[io4]\n\t"
        "addsd %[inc5], %[io5]\n\t"
        /* Force spill/reload with many clobbers */
        "mov $0x12345678, %%eax\n\t"
        "mov $0x9ABCDEF0, %%ebx\n\t"
        "mov $0x11111111, %%ecx\n\t"
        "mov $0x22222222, %%edx\n\t"
        "mov $0x33333333, %%esi\n\t"
        "mov $0x44444444, %%edi\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        : [io1] "+r" (io1),
          [io2] "+r" (io2),
          [io3] "+r" (io3),
          [io4] "+r" (io4),
          [io5] "+x" (io5)
        : [inc1] "r" (r7),
          [inc2] "r" (r8),
          [inc3] "r" (r9),
          [inc4] "r" (r10),
          [inc5] "x" (d0)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "cc", "memory"
    );
    
    /* Phase 7: Final computation using all results */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17;
    final_sum += result1 + result2 + narrow_result;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += io1 + io2 + io3 + (int)io4 + (int)io5;
    final_sum += (int)wide_result;
    
    /* Use array to prevent elimination */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
