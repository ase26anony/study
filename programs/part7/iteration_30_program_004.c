/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all */
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
volatile long global_counter = 0;
volatile int global_index = 0;

int main() {
    /* Declare variables with different types and storage */
    register int reg_var asm("r12") = 1234;  /* Force specific register */
    volatile int vol_var = 5678;
    int auto_var = 9012;
    long long_var = 34567;
    char char_array[256];
    int int_array[128];
    long long_array[64];
    
    /* Packed/misaligned struct */
    struct misaligned_data packed;
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE;
    
    /* Pointers with different properties */
    int *restrict ptr1 = &auto_var;
    volatile int *ptr2 = &vol_var;
    char *ptr3 = char_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_array[i] = i;
    for (int i = 0; i < 128; i++) int_array[i] = i * 2;
    for (int i = 0; i < 64; i++) long_array[i] = i * 3;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary base/index registers each iteration */
        int base_idx = iter * 7 % 64;
        int index_idx = iter * 13 % 64;
        int scale = 1 + (iter % 4);
        int displacement = iter * 8;
        
        /* Complex addressing mode 1: [base + index*scale + displacement] */
        /* Forces RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[out1], [%[base] + %[index]*%[scale] + %[disp]]\n\t"
            : [out1] "=r" (auto_var)
            : [base] "r" (&long_array[base_idx]),
              [index] "r" (index_idx),
              [scale] "i" (scale),
              [disp] "i" (displacement)
            : "memory"
        );
        
        /* Complex addressing mode 2 with early-clobber */
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        long temp_out;
        asm volatile (
            "lea %[out], [%[base] + %[index]*4 + %[offset]]\n\t"
            "mov [%[dest]], %[out]\n\t"
            : [out] "=&r" (temp_out),
              [dest] "=m" (long_array[iter % 8])
            : [base] "r" (int_array),
              [index] "r" (iter),
              [offset] "i" (16)
            : "memory"
        );
        
        /* Multiple operands exceeding available registers */
        /* Forces RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        long r1, r2, r3, r4, r5, r6, r7, r8;
        asm volatile (
            "mov %0, %1\n\t"
            "add %0, %2\n\t"
            "sub %3, %4\n\t"
            "imul %5, %6\n\t"
            "or %7, %8\n\t"
            : "=&r" (r1), "=&r" (r2), "=&r" (r3),
              "=&r" (r4), "=&r" (r5), "=&r" (r6),
              "=r" (r7), "=r" (r8)
            : "r" (auto_var + iter), "r" (vol_var - iter),
              "r" (reg_var * iter), "r" (long_var / (iter + 1)),
              "r" (packed.i), "r" (packed.l),
              "r" (ptr1), "r" (ptr2), "r" (ptr3 + iter)
            : "cc", "memory"
        );
        
        /* Nested address computation */
        /* Forces RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        void *addr1, *addr2, *addr3;
        asm volatile (
            "lea %0, [%1 + %2*2]\n\t"
            "lea %3, [%4 + %5]\n\t"
            "mov %6, %7\n\t"
            : "=&r" (addr1), "=&r" (addr2), "=r" (addr3)
            : "r" (&char_array[displacement]),
              "r" (&int_array[index_idx]),
              "r" (base_idx * sizeof(int)),
              "m" (packed),
              "r" (&packed + iter)
            : "memory"
        );
        
        /* Address of address computation */
        /* Forces RELOAD_FOR_OTHER_ADDRESS */
        long **nested_ptr = (long**)&ptr1;
        asm volatile (
            "mov %[val], [%[ptrptr]]\n\t"
            "mov [%[out]], %[val]\n\t"
            : [out] "=m" (long_array[iter % 16])
            : [ptrptr] "r" (nested_ptr),
              [val] "r" (*nested_ptr)
            : "memory"
        );
        
        /* Mixed size operands with alignment issues */
        char *misaligned_ptr = (char*)&packed.i;
        asm volatile (
            "movzx %0, byte ptr [%1]\n\t"
            "mov %2, dword ptr [%3]\n\t"
            "mov %4, qword ptr [%5]\n\t"
            : "=r" (auto_var), "=r" (vol_var), "=r" (reg_var)
            : "r" (misaligned_ptr),
              "r" (misaligned_ptr + 1),
              "r" (misaligned_ptr + 4)
            : "memory"
        );
        
        /* Update variables to create dependencies between iterations */
        auto_var += iter;
        vol_var ^= iter;
        reg_var *= (iter + 1);
        long_var -= iter;
        global_counter += iter;
        
        /* Force memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    /* Compute checksum to ensure all operations have side effects */
    unsigned long checksum = 0;
    checksum += auto_var;
    checksum += vol_var;
    checksum += reg_var;
    checksum += long_var;
    checksum += global_counter;
    
    for (int i = 0; i < 256; i++) checksum += char_array[i];
    for (int i = 0; i < 128; i++) checksum += int_array[i];
    for (int i = 0; i < 64; i++) checksum += long_array[i];
    
    checksum += packed.c + packed.i + packed.l;
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
