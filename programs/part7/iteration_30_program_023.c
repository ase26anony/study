/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed_data[8];
    volatile int vol_var = 0;
    register int reg_var asm ("r12") = 42; /* Try to tie up a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 16];
    for (int i = 0; i < 8; i++) {
        packed_data[i].c = i;
        packed_data[i].i = i * 100;
        packed_data[i].l = i * 1000L;
    }
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 256;
        int scale = 1 + (iter % 4);
        
        switch (iter % 5) {
            case 0: {
                /* Complex addressing mode - RELOAD_FOR_INPUT_ADDRESS */
                long result;
                asm volatile (
                    "mov %[dst], qword ptr [%[base] + %[idx]*8 + %[offset]]\n\t"
                    : [dst] "=r" (result)
                    : [base] "r" (arr), 
                      [idx] "r" (idx),
                      [offset] "i" (16)
                    : "memory"
                );
                checksum += result;
                break;
            }
            
            case 1: {
                /* Multiple operands with early clobber - RELOAD_FOR_INPUT/OUTPUT */
                int in1 = iter, in2 = iter * 2, in3 = iter * 3;
                int out1, out2;
                asm volatile (
                    "lea %[out1], [%[in1] + %[in2]]\n\t"
                    "imul %[out2], %[in3], %[in1]\n\t"
                    : [out1] "=&r" (out1),  /* Early clobber */
                      [out2] "=&r" (out2)   /* Early clobber */
                    : [in1] "r" (in1),
                      [in2] "r" (in2),
                      [in3] "r" (in3)
                );
                checksum += out1 + out2;
                break;
            }
            
            case 2: {
                /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
                long *addr;
                asm volatile (
                    "lea %[addr], [%[base] + %[idx]*%[scale] + %[packed]]\n\t"
                    : [addr] "=r" (addr)
                    : [base] "r" (arr),
                      [idx] "r" (idx),
                      [scale] "i" (sizeof(int)),
                      [packed] "r" (&packed_data[iter % 8])
                    : "memory"
                );
                checksum += (long)addr;
                break;
            }
            
            case 3: {
                /* Output with complex address - RELOAD_FOR_OUTPUT_ADDRESS */
                int value = iter * 100;
                asm volatile (
                    "mov dword ptr [%[base] + %[idx]*4], %[value]\n\t"
                    : 
                    : [base] "r" (arr),
                      [idx] "r" (idx),
                      [value] "r" (value)
                    : "memory"
                );
                checksum += arr[idx];
                break;
            }
            
            case 4: {
                /* Maximum register pressure - RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS */
                int r1, r2, r3, r4, r5, r6, r7, r8;
                asm volatile (
                    "mov %[r1], %[v1]\n\t"
                    "add %[r2], %[v2], %[v3]\n\t"
                    "sub %[r3], %[v4], %[v5]\n\t"
                    "imul %[r4], %[v6], %[v7]\n\t"
                    "lea %[r5], [%[base] + %[idx1]*4]\n\t"
                    "lea %[r6], [%[base] + %[idx2]*4]\n\t"
                    "mov %[r7], dword ptr [%[r5]]\n\t"
                    "mov %[r8], dword ptr [%[r6]]\n\t"
                    : [r1] "=&r" (r1), [r2] "=&r" (r2),
                      [r3] "=&r" (r3), [r4] "=&r" (r4),
                      [r5] "=&r" (r5), [r6] "=&r" (r6),
                      [r7] "=&r" (r7), [r8] "=&r" (r8)
                    : [v1] "r" (iter), [v2] "r" (iter*2),
                      [v3] "r" (iter*3), [v4] "r" (iter*4),
                      [v5] "r" (iter*5), [v6] "r" (iter*6),
                      [v7] "r" (iter*7), [base] "r" (arr),
                      [idx1] "r" (idx), [idx2] "r" ((idx + 1) % 256)
                    : "memory"
                );
                checksum += r1 + r2 + r3 + r4 + r7 + r8;
                break;
            }
        }
        
        /* Force address-of-address reloads */
        if (iter % 3 == 0) {
            long **nested_ptr;
            asm volatile (
                "lea %[ptr], [%[base] + %[idx]*8]\n\t"
                : [ptr] "=r" (nested_ptr)
                : [base] "r" (ptr_arr),
                  [idx] "r" (iter % 16)
            );
            checksum += (long)*nested_ptr;
        }
        
        /* Use volatile to force memory reloads */
        vol_var = iter;
        global_counter = iter;
        checksum += vol_var + global_counter;
    }
    
    /* Final computation to ensure all asm has side effects */
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
