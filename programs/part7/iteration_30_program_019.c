/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char tail;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main(void) {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed;
    volatile int vol_var = 0;
    register int reg_var asm("ebx") = 42; /* Hint register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (long*)&arr[i * 4];
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    packed.tail = 'Z';
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 256;
        long *base_ptr = &arr[0];
        long scale = iter + 1;
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple index registers
           Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[val1], %[out1]\n\t"
            "add %[idx], %[out1]\n\t"
            : [out1] "=r" (arr[idx])
            : [val1] "m" (arr[idx + 128]), 
              [idx] "r" (idx),
              "m" (arr[idx + 64])  /* Extra memory operand */
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 2: Early-clobber with overlapping operands
           Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        {
            int temp1, temp2, temp3;
            asm volatile (
                "mov %[in1], %[out1]\n\t"
                "imul %[in2], %[out1]\n\t"
                "add %[out1], %[out2]\n\t"
                "sub %[in3], %[out3]\n\t"
                : [out1] "=&r" (temp1),  /* Early clobber */
                  [out2] "=&r" (temp2),  /* Early clobber */
                  [out3] "=r" (temp3)
                : [in1] "r" (iter),
                  [in2] "rm" (scale),    /* Register or memory */
                  [in3] "rm" (idx),
                  "m" (arr[iter]),       /* Extra memory constraint */
                  "m" (arr[iter + 1])
                : "cc", "memory"
            );
            vol_var += temp1 + temp2 + temp3;
        }
        
        /* VARYING CONSTRAINT 3: Nested address computation
           Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        {
            long *addr1, *addr2;
            asm volatile (
                "lea (%[base], %[idx], 4), %[addr1]\n\t"
                "lea (%[base], %[idx], 8), %[addr2]\n\t"
                : [addr1] "=r" (addr1),
                  [addr2] "=r" (addr2)
                : [base] "r" (base_ptr),
                  [idx] "r" (idx),
                  "m" (*base_ptr)        /* Force memory operand */
                : "cc"
            );
            ptr_arr[iter % 16] = addr1;
            global_sum += (long)addr2;
        }
        
        /* VARYING CONSTRAINT 4: Output address reloads
           Likely triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        {
            long result1, result2;
            asm volatile (
                "mov (%[src1]), %[dst1]\n\t"
                "mov (%[src2]), %[dst2]\n\t"
                "add %[dst1], %[dst2]\n\t"
                : [dst1] "=rm" (result1),  /* Register or memory */
                  [dst2] "=rm" (result2)
                : [src1] "r" (&arr[idx]),
                  [src2] "r" (&arr[idx + 64]),
                  "m" (arr[idx]),          /* Input memory */
                  "m" (arr[idx + 64])
                : "cc", "memory"
            );
            global_sum += result1 + result2;
        }
        
        /* VARYING CONSTRAINT 5: Mixed data types with alignment issues
           Likely triggers: RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
        {
            char *cptr = (char*)&packed;
            int *iptr = (int*)(cptr + 1);  /* Misaligned! */
            long *lptr = (long*)(cptr + 5); /* Potentially misaligned */
            
            asm volatile (
                "movzbl (%[c]), %%eax\n\t"
                "addl (%[i]), %%eax\n\t"
                "addq (%[l]), %%rax\n\t"
                "movl %%eax, %[out]\n\t"
                : [out] "=rm" (vol_var)
                : [c] "r" (cptr),
                  [i] "r" (iptr),
                  [l] "r" (lptr),
                  "m" (*cptr),
                  "m" (*iptr),
                  "m" (*lptr)
                : "rax", "cc", "memory"
            );
        }
        
        /* VARYING CONSTRAINT 6: Many operands to cause register pressure */
        if (iter % 3 == 0) {
            int r1, r2, r3, r4, r5, r6, r7, r8;
            asm volatile (
                "mov %[a1], %[o1]\n\t"
                "add %[a2], %[o2]\n\t"
                "sub %[a3], %[o3]\n\t"
                "imul %[a4], %[o4]\n\t"
                "or %[a5], %[o5]\n\t"
                "and %[a6], %[o6]\n\t"
                "xor %[a7], %[o7]\n\t"
                "mov %[a8], %[o8]\n\t"
                : [o1] "=&r" (r1), [o2] "=&r" (r2),
                  [o3] "=&r" (r3), [o4] "=&r" (r4),
                  [o5] "=&r" (r5), [o6] "=&r" (r6),
                  [o7] "=&r" (r7), [o8] "=&r" (r8)
                : [a1] "rm" (iter),     [a2] "rm" (idx),
                  [a3] "rm" (scale),    [a4] "rm" (arr[iter]),
                  [a5] "rm" (arr[idx]), [a6] "rm" (vol_var),
                  [a7] "rm" (reg_var),  [a8] "rm" (global_counter),
                  "m" (arr[iter]), "m" (arr[idx]), "m" (packed)
                : "cc", "memory"
            );
            vol_var += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
        }
        
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    checksum += global_sum + vol_var + packed.i + (long)packed.l;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Volatile var: %d\n", vol_var);
    
    return (checksum != 0) ? 0 : 1;
}
