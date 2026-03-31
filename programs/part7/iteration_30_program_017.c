/* reload_coverage.c - Program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main(void) {
    /* Declare variables with different types and storage */
    register int reg_var asm("r12") = 1234;  /* Force specific register */
    volatile int vol_var = 5678;
    int auto_var = 9012;
    long *ptr_var = (long*)malloc(64 * sizeof(long));
    struct misaligned_data packed_data = {0};
    
    /* Initialize arrays with different alignments */
    char char_array[256] __attribute__((aligned(1)));
    int int_array[128] __attribute__((aligned(4)));
    long long_array[64] __attribute__((aligned(16)));
    
    /* Initialize pointers */
    for (int i = 0; i < 64; i++) {
        if (ptr_var) ptr_var[i] = i * 100;
    }
    for (int i = 0; i < 256; i++) char_array[i] = i;
    for (int i = 0; i < 128; i++) int_array[i] = i * 2;
    for (int i = 0; i < 64; i++) long_array[i] = i * 3;
    
    /* Loop to create varying constraints */
    for (int iteration = 0; iteration < 10; iteration++) {
        int offset = iteration * 7;
        
        /* Complex addressing mode 1: RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[out1], %[in1]\n\t"
            "add %[out1], %[idx1]\n\t"
            "mov %[out2], %[in2]\n\t"
            : [out1] "=&r" (auto_var), [out2] "=m" (long_array[offset % 32])
            : [in1] "m" (*(int*)(char_array + offset * 4)),  /* Misaligned access */
              [in2] "r" (reg_var),
              [idx1] "r" (offset)
            : "memory", "cc"
        );
        
        /* Complex addressing mode 2: RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], 4), %[temp]\n\t"
            "mov (%[temp], %[disp]), %[out]\n\t"
            : [out] "=r" (reg_var), [temp] "=&r" (auto_var)
            : [base] "r" (long_array), 
              [index] "r" (offset % 16),
              [disp] "r" (iteration * 8)
            : "memory"
        );
        
        /* Multiple operands with conflicting constraints: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT */
        asm volatile (
            "imul %[a], %[b]\n\t"
            "add %[b], %[c]\n\t"
            "sub %[c], %[d]\n\t"
            : [a] "+&r" (auto_var), 
              [b] "=&r" (vol_var),
              [c] "=m" (int_array[offset % 64]),
              [d] "+r" (reg_var)
            : [e] "m" (packed_data.i),
              [f] "r" (global_counter),
              [g] "i" (123)
            : "memory", "cc", "rax", "rdx"  /* Explicit clobbers for imul */
        );
        
        /* Nested address computation: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        long *addr_ptr = &long_array[offset % 32];
        asm volatile (
            "mov (%[addr]), %[val]\n\t"
            "add %[inc], %[val]\n\t"
            "mov %[val], (%[addr])\n\t"
            : [val] "=&r" (auto_var)
            : [addr] "r" (addr_ptr),
              [inc] "r" (iteration)
            : "memory"
        );
        
        /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Use address of memory operand that itself needs reload */
        asm volatile (
            "mov %[in], %%rax\n\t"
            "add (%%rax), %[out]\n\t"
            : [out] "+r" (reg_var)
            : [in] "r" (&int_array[offset % 32]),
              "m" (int_array[offset % 32])  /* Memory input */
            : "memory", "rax"
        );
        
        /* RELOAD_FOR_OTHER_ADDRESS - create spill/reload scenario */
        /* Many operands to cause register pressure */
        asm volatile (
            "mov %[a1], %[t1]\n\t"
            "mov %[a2], %[t2]\n\t"
            "mov %[a3], %[t3]\n\t"
            "mov %[a4], %[t4]\n\t"
            "add %[t1], %[t2]\n\t"
            "add %[t3], %[t4]\n\t"
            "imul %[t2], %[t4]\n\t"
            "mov %[t4], %[out]\n\t"
            : [out] "=m" (global_array[iteration % 32]),
              [t1] "=&r" (auto_var),
              [t2] "=&r" (vol_var),
              [t3] "=&r" (reg_var),
              [t4] "=&r" (global_counter)
            : [a1] "m" (*(long*)((char*)long_array + offset)),  /* Complex address */
              [a2] "m" (*(int*)((char*)int_array + offset * 2)),
              [a3] "r" (ptr_var ? ptr_var[offset % 32] : 0),
              [a4] "i" (456)
            : "memory", "cc", "rax", "rdx"
        );
        
        /* RELOAD_OTHER - early clobber with overlapping */
        asm volatile (
            "mov %[src], %[dst]\n\t"
            "shl $2, %[dst]\n\t"
            : [dst] "=&r" (auto_var)
            : [src] "0" (auto_var)  /* Same as output */
            : "cc"
        );
        
        /* Change constraints each iteration */
        if (iteration % 3 == 0) {
            /* Force different register allocation */
            asm volatile ("# Force spill" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        }
        
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += reg_var;
    checksum += vol_var;
    checksum += auto_var;
    checksum += global_counter;
    
    for (int i = 0; i < 64 && ptr_var; i++) {
        checksum += ptr_var[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    
    checksum += packed_data.i;
    
    printf("Checksum: %lu\n", checksum);
    
    if (ptr_var) free(ptr_var);
    return 0;
}
