/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC */
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
    char buffer[128];
    volatile int vol_var = 42;
    struct misaligned_data packed;
    register int reg_var asm ("r12") = 100; /* Try to tie up a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 8];
    for (int i = 0; i < 128; i++) buffer[i] = (char)(i + 1);
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7;
        long temp1, temp2, temp3;
        void *addr1, *addr2;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Multiple output operands with early-clobber */
        asm volatile (
            "mov %[out1], %[in1]\n\t"
            "add %[out1], %[in2]\n\t"
            "mov %[out2], %[in3]\n\t"
            "imul %[out2], %[in4]\n\t"
            : [out1] "=&r" (temp1), [out2] "=&r" (temp2)
            : [in1] "r" (arr[idx]), [in2] "r" (iter),
              [in3] "r" (arr[idx + 1]), [in4] "r" (iter + 1)
            : "cc"
        );
        
        /* Complex addressing mode - RELOAD_FOR_INPUT_ADDRESS */
        /* Base + index*scale + displacement */
        asm volatile (
            "mov %0, [%1 + %2*4 + %3]\n\t"
            : "=r" (temp3)
            : "r" (arr), "r" (idx), "i" (16), "m" (arr[0])
            : "memory"
        );
        
        /* Take address of complex memory operand - RELOAD_FOR_OPERAND_ADDRESS */
        addr1 = &arr[idx * 2 + iter];
        asm volatile (
            "lea %0, [%1 + %2*2]\n\t"
            : "=r" (addr2)
            : "r" (arr), "r" (idx), "m" (arr[0])
        );
        
        /* Nested address computation - RELOAD_FOR_INPADDR_ADDRESS */
        /* Address of a memory operand that itself needs reload */
        long **nested_ptr = &ptr_arr[iter % 8];
        asm volatile (
            "mov %0, [%1]\n\t"
            "add %0, %2\n\t"
            : "=r" (temp1)
            : "r" (nested_ptr), "r" (iter * 8), "m" (*nested_ptr)
            : "memory"
        );
        
        /* Output with complex address - RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "mov [%1 + %2*8], %0\n\t"
            : "=m" (arr[0])
            : "r" (arr), "r" (iter), "r" (temp1)
            : "memory"
        );
        
        /* Mixed data types and misaligned access */
        char *char_ptr = (char*)&packed.i;
        asm volatile (
            "movzx %0, byte ptr [%1]\n\t"
            "add %0, %2\n\t"
            : "=r" (temp2)
            : "r" (char_ptr + 1), "r" (iter)  /* Misaligned int access */
            : "memory"
        );
        
        /* Many operands to cause register pressure - RELOAD_OTHER */
        asm volatile (
            "mov %0, %1\n\t"
            "add %0, %2\n\t"
            "add %0, %3\n\t"
            "add %0, %4\n\t"
            "add %0, %5\n\t"
            : "=&r" (vol_var)
            : "r" (arr[iter]), "r" (arr[iter + 1]),
              "r" (arr[iter + 2]), "r" (arr[iter + 3]),
              "r" (temp1), "m" (buffer[iter])
            : "cc", "memory"
        );
        
        /* RELOAD_FOR_OTHER_ADDRESS through volatile and clobbers */
        asm volatile (
            ""
            : 
            : "r" (&arr[idx]), "r" (&buffer[iter]), 
              "r" (&packed), "r" (&vol_var)
            : "memory"
        );
        
        /* Update checksum */
        global_sum += temp1 + temp2 + temp3 + vol_var + (long)addr2;
        global_counter++;
    }
    
    /* Additional test: inline asm with output address reload */
    {
        long output_arr[4];
        long *out_ptr = &output_arr[2];  /* Not aligned to 16 bytes */
        
        /* RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "mov [%1 + %2], %0\n\t"
            : "=m" (output_arr[0])
            : "r" (out_ptr), "r" (iter * 4), "r" (global_sum)
            : "memory"
        );
        
        global_sum += output_arr[0];
    }
    
    /* Compute final checksum to ensure all asm executed */
    long final_sum = global_sum;
    for (int i = 0; i < 256; i++) final_sum += arr[i];
    for (int i = 0; i < 128; i++) final_sum += buffer[i];
    
    printf("Checksum: %ld\n", final_sum);
    printf("Iterations: %d\n", global_counter);
    
    return (final_sum > 0) ? 0 : 1;
}
