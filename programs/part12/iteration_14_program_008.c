/* reload_coverage.c - Complex program to trigger GCC's reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile int scale,
    volatile long offset1, volatile long offset2, volatile char offset3)
{
    /* High register pressure with mixed types */
    int int_arr[128];          /* Integer array - pressure on integer regs */
    double double_arr[64];     /* Double array - pressure on FP regs */
    long long_arr[96];         /* Long array - more integer pressure */
    char *ptr_arr[32];         /* Pointer array - address computations */
    
    /* Many scalar variables with overlapping live ranges */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double ftemp1, ftemp2, ftemp3, ftemp4;
    long ltemp1, ltemp2, ltemp3;
    char *ptr1, *ptr2, *ptr3, **pptr1, **pptr2;
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) int_arr[i] = i * 2;
    for (int i = 0; i < 64; i++) double_arr[i] = i * 1.5;
    for (int i = 0; i < 96; i++) long_arr[i] = i * 3L;
    
    /* Compiler barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* ===== SCENARIO 1: Complex multi-dimensional array access ===== */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < idx1 % 8; i++) {
        for (int j = 0; j < idx2 % 8; j++) {
            /* Complex address calculation with volatile indices */
            int_arr[(i * stride1 + j * stride2) % 128] = 
                double_arr[(i * idx3 + j) % 64] * scale;
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* ===== SCENARIO 2: Multi-level pointer indirection ===== */
    /* This should trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    ptr1 = (char *)int_arr;
    ptr2 = ptr1 + offset1;
    ptr3 = ptr2 + offset2;
    
    /* Chain of pointer dereferences */
    temp1 = *(int *)(ptr3 + offset3);
    temp2 = *(int *)(ptr3 + offset3 * 2);
    temp3 = *(int *)(ptr3 + offset3 * 3);
    
    /* Pointer to pointer operations */
    pptr1 = &ptr1;
    pptr2 = &ptr2;
    **pptr1 = **pptr2 + temp1;
    
    __asm__ volatile("" : : : "memory");
    
    /* ===== SCENARIO 3: Mixed register class pressure ===== */
    /* Integer and floating-point operations interleaved */
    ftemp1 = double_arr[idx1 % 64];
    temp4 = int_arr[idx2 % 128];
    
    /* Convert int to double (requires moving between register classes) */
    ftemp2 = (double)temp4 * ftemp1;
    
    /* Convert double to int (another register class move) */
    temp5 = (int)(ftemp2 * 2.0);
    
    /* More mixed operations */
    ftemp3 = double_arr[idx3 % 64] + (double)long_arr[idx1 % 96];
    ftemp4 = ftemp3 * 3.14159;
    temp6 = (int)ftemp4;
    
    __asm__ volatile("" : : : "memory");
    
    /* ===== SCENARIO 4: Inline assembly with complex constraints ===== */
    /* These should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    
    /* Assembly with memory input and register output */
    __asm__ volatile(
        "movl (%[input]), %[output]\n\t"
        : [output] "=r" (temp7)          /* Output in register */
        : [input] "r" (&int_arr[idx1 % 128])  /* Input address in register */
        : "memory"
    );
    
    /* Assembly with early-clobber output and memory input */
    __asm__ volatile(
        "leaq (%[base], %[index], 4), %[addr]\n\t"
        "movl (%[addr]), %[result]\n\t"
        : [result] "=&r" (temp8), [addr] "=&r" (ltemp1)  /* Early clobber */
        : [base] "r" (int_arr), [index] "r" (idx2)       /* Base and index */
        : "memory"
    );
    
    /* Assembly with multiple memory addresses */
    __asm__ volatile(
        "movq (%[addr1]), %%rax\n\t"
        "addq (%[addr2]), %%rax\n\t"
        "movq %%rax, %[sum]\n\t"
        : [sum] "=r" (ltemp2)
        : [addr1] "r" (&long_arr[idx1 % 96]), 
          [addr2] "r" (&long_arr[idx2 % 96])
        : "rax", "memory"
    );
    
    __asm__ volatile("" : : : "memory");
    
    /* ===== SCENARIO 5: Complex address calculations in loops ===== */
    /* This should trigger RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
    for (volatile int i = 0; i < 4; i++) {
        /* Complex address with multiple components */
        int *addr = &int_arr[
            (i * 7 + idx1 * 3 + idx2 * 5 + idx3 * 11) % 128
        ];
        
        /* Even more complex calculation */
        double *daddr = &double_arr[
            ((*addr) * 13 + i * 17) % 64
        ];
        
        *addr = (int)(*daddr * scale);
        
        /* Pointer arithmetic with different types */
        char *cptr = (char *)addr + offset1 + i * offset2;
        ltemp3 = *(long *)cptr;
    }
    
    /* Final computation using all temporaries */
    volatile int result = 
        temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8 +
        (int)ftemp1 + (int)ftemp2 + (int)ftemp3 + (int)ftemp4 +
        (int)ltemp1 + (int)ltemp2 + (int)ltemp3;
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile int scale = rand() % 10 + 1;
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    volatile char offset3 = rand() % 10;
    
    printf("Testing complex reload scenarios...\n");
    
    int result = trigger_reloads(
        idx1, idx2, idx3, stride1, stride2, scale,
        offset1, offset2, offset3
    );
    
    printf("Result: %d\n", result);
    printf("Reload test completed.\n");
    
    return 0;
}
