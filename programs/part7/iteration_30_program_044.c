/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    char buffer[128];
    double doubles[32];
    struct misaligned_data packed_data[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 4];
    for (int i = 0; i < 128; i++) buffer[i] = (char)(i ^ 0x55);
    for (int i = 0; i < 32; i++) doubles[i] = i * 1.5;
    for (int i = 0; i < 8; i++) {
        packed_data[i].c = i;
        packed_data[i].i = i * 100;
        packed_data[i].l = i * 1000L;
    }
    
    /* Variables for register pressure */
    register int r1 asm("r8") = 1;
    register int r2 asm("r9") = 2;
    register int r3 asm("r10") = 3;
    register int r4 asm("r11") = 4;
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 100; iter++) {
        int idx = iter & 0xFF;
        int scale = (iter % 4) + 1;
        
        /* Vary constraints based on iteration */
        if (iter % 10 == 0) {
            /* Complex addressing mode - triggers RELOAD_FOR_INPUT_ADDRESS */
            asm volatile (
                "add %[out], %[in1], %[in2], lsl #2\n\t"
                : [out] "=r" (r1)
                : [in1] "r" ((uintptr_t)&arr[idx]),
                  [in2] "r" (scale)
                : "cc"
            );
        } else if (iter % 10 == 1) {
            /* Multiple outputs with early clobber - triggers RELOAD_FOR_OUTPUT */
            asm volatile (
                "mov %[out1], %[in1]\n\t"
                "add %[out2], %[in1], %[in2]\n\t"
                : [out1] "=&r" (r1), [out2] "=&r" (r2)
                : [in1] "r" (iter), [in2] "r" (idx)
                : "cc"
            );
        } else if (iter % 10 == 2) {
            /* Memory operand with complex addressing - triggers RELOAD_FOR_INPADDR_ADDRESS */
            long *addr = &ptr_arr[iter % 8][scale];
            asm volatile (
                "ldr %[out], [%[addr], %[offset], lsl #3]\n\t"
                : [out] "=r" (r3)
                : [addr] "r" (addr),
                  [offset] "r" (iter & 7)
                : "memory"
            );
        } else if (iter % 10 == 3) {
            /* Output with address computation - triggers RELOAD_FOR_OUTPUT_ADDRESS */
            long result;
            asm volatile (
                "str %[in], [%[base], %[index], lsl #3]\n\t"
                : "=m" (ptr_arr[iter % 8][scale])
                : [in] "r" ((long)iter * 100),
                  [base] "r" (ptr_arr[iter % 8]),
                  [index] "r" (scale)
                : "memory"
            );
        } else if (iter % 10 == 4) {
            /* Nested address computation - triggers RELOAD_FOR_OPERAND_ADDRESS */
            struct misaligned_data *p = &packed_data[iter % 4];
            int offset = (iter % 3) * 8;
            asm volatile (
                "add %[out], %[base], %[offset]\n\t"
                "ldr %[val], [%[out]]\n\t"
                : [out] "=&r" (r4), [val] "=r" (r1)
                : [base] "r" (p), [offset] "r" (offset)
                : "memory"
            );
        } else if (iter % 10 == 5) {
            /* Mixed size operands with memory clobber - triggers RELOAD_OTHER */
            char c1, c2;
            int i1, i2;
            long l1, l2;
            
            asm volatile (
                "ldrb %[c1], [%[buf1]]\n\t"
                "ldr %[i1], [%[arr1], %[idx1], lsl #2]\n\t"
                "ldr %[l1], [%[ptr1]]\n\t"
                "add %[i2], %[i1], %[c1]\n\t"
                "str %[i2], [%[arr2], %[idx2], lsl #2]\n\t"
                : [c1] "=&r" (c1), [i1] "=&r" (i1), [l1] "=&r" (l1),
                  [i2] "=&r" (i2)
                : [buf1] "r" (&buffer[iter % 64]),
                  [arr1] "r" (arr), [idx1] "r" ((iter * 7) & 0xFF),
                  [ptr1] "r" (&ptr_arr[iter % 8][0]),
                  [arr2] "r" (arr), [idx2] "r" ((iter * 11) & 0xFF)
                : "memory", "cc"
            );
        } else if (iter % 10 == 6) {
            /* Address of address computation - triggers RELOAD_FOR_OPADDR_ADDR */
            long **addr_of_addr = &ptr_arr[iter % 8];
            asm volatile (
                "mov %[out], %[addr]\n\t"
                : [out] "=r" (r2)
                : [addr] "r" (addr_of_addr)
                :
            );
        } else if (iter % 10 == 7) {
            /* Multiple memory constraints - triggers RELOAD_FOR_OTHER_ADDRESS */
            double d1, d2;
            asm volatile (
                "fldd d0, [%[dbl1]]\n\t"
                "fldd d1, [%[dbl2]]\n\t"
                "fmuld d2, d0, d1\n\t"
                "fstd d2, [%[dbl3]]\n\t"
                : "=m" (doubles[iter % 16])
                : [dbl1] "r" (&doubles[(iter + 1) % 16]),
                  [dbl2] "r" (&doubles[(iter + 2) % 16]),
                  [dbl3] "r" (&doubles[iter % 16])
                : "d0", "d1", "d2", "memory"
            );
        } else if (iter % 10 == 8) {
            /* Maximum register pressure - triggers various reloads */
            int t1, t2, t3, t4, t5, t6, t7, t8;
            asm volatile (
                "mov %[t1], %[v1]\n\t"
                "add %[t2], %[t1], %[v2]\n\t"
                "mul %[t3], %[t2], %[v3]\n\t"
                "sub %[t4], %[t3], %[v4]\n\t"
                "and %[t5], %[t4], %[v5]\n\t"
                "orr %[t6], %[t5], %[v6]\n\t"
                "eor %[t7], %[t6], %[v7]\n\t"
                "mov %[t8], %[t7]\n\t"
                : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3),
                  [t4] "=&r" (t4), [t5] "=&r" (t5), [t6] "=&r" (t6),
                  [t7] "=&r" (t7), [t8] "=&r" (t8)
                : [v1] "r" (iter), [v2] "r" (idx), [v3] "r" (scale),
                  [v4] "r" (r1), [v5] "r" (r2), [v6] "r" (r3),
                  [v7] "r" (r4)
                : "cc"
            );
            r1 = t8;  /* Use result to prevent elimination */
        } else {
            /* Mixed constraints with volatile - triggers RELOAD_FOR_INPUT */
            volatile int *volatile_ptr = &arr[iter % 128];
            asm volatile (
                "ldr %[out], [%[ptr]]\n\t"
                "add %[out], %[out], #1\n\t"
                "str %[out], [%[ptr]]\n\t"
                : [out] "=&r" (r1), [ptr] "+r" (volatile_ptr)
                :
                : "memory", "cc"
            );
        }
        
        /* Update checksum to ensure side effects */
        checksum += r1 + r2 + r3 + r4;
        checksum += arr[iter & 0xFF];
        checksum += (long)buffer[iter & 0x7F];
        global_counter++;
    }
    
    /* Final computation to use all variables */
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += packed_data[i].i + packed_data[i].l;
    }
    
    global_sum = checksum;
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (int)(checksum & 0x7FFFFFFF);
}
