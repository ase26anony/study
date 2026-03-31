/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload1.cc */
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
volatile long global_counter = 0;
volatile int global_index = 0;

int main() {
    /* Declare variables with different storage classes and types */
    register int reg_var asm("ebx") = 12345;  /* Try to bind to specific reg */
    volatile int vol_var = 67890;
    int auto_var = 11111;
    long long_var = 22222;
    char char_array[256];
    int int_array[100];
    long long_array[50];
    
    /* Packed/misaligned data */
    struct misaligned_data packed_data;
    packed_data.c = 'A';
    packed_data.i = 0xDEADBEEF;
    packed_data.l = 0xCAFEBABE12345678ULL;
    packed_data.tail = 'Z';
    
    /* Pointers with different properties */
    int *restrict restr_ptr = int_array;
    volatile int *vol_ptr = &vol_var;
    char *char_ptr = char_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_array[i] = i;
    for (int i = 0; i < 100; i++) int_array[i] = i * 3;
    for (int i = 0; i < 50; i++) long_array[i] = i * 7;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary constraints based on iteration */
        int use_memory = iteration & 1;
        int use_complex_addr = iteration & 2;
        int many_operands = iteration & 4;
        
        if (many_operands) {
            /* Many operands to cause register pressure - RELOAD_FOR_INPUT/OUTPUT/OTHER */
            asm volatile (
                /* Complex addressing with multiple memory operands */
                "mov %[val1], %[out1]\n\t"
                "add %[val2], %[out1]\n\t"
                "lea (%[base], %[index], 4), %[out2]\n\t"
                "imul %[val3], %[out2]\n\t"
                "mov %[out2], (%[dest], %[idx2], 2)\n\t"
                : [out1] "=&r" (auto_var),      /* Early clobber output */
                  [out2] "=&r" (long_var)       /* Another early clobber */
                : [val1] "rm" (reg_var),        /* Register or memory */
                  [val2] "rm" (vol_var),        /* Register or memory */
                  [val3] "r" (iteration),       /* Register only */
                  [base] "r" (int_array),       /* Base register */
                  [index] "r" (global_index),   /* Index register */
                  [dest] "r" (long_array),      /* Destination base */
                  [idx2] "r" (auto_var & 0xF)   /* Scaled index */
                : "memory", "cc"
            );
        }
        
        if (use_complex_addr) {
            /* Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS, etc. */
            long complex_addr_result;
            
            /* Nested address computation */
            asm volatile (
                /* Take address of memory operand with complex addressing */
                "lea (%[base], %[idx], %[scale]), %[tmp]\n\t"
                "mov (%[tmp], %[disp]), %[result]\n\t"
                "add %[addend], %[result]\n\t"
                : [result] "=r" (complex_addr_result),
                  [tmp] "=&r" (reg_var)  /* Temporary for address */
                : [base] "r" (char_array),
                  [idx] "r" (global_index * 2),
                  [scale] "i" (sizeof(int)),
                  [disp] "r" (iteration * 4),
                  [addend] "rm" (packed_data.i)
                : "memory"
            );
            
            /* Output address reload */
            int output_addr_var;
            asm volatile (
                "mov %[in], %%eax\n\t"
                "mov %%eax, %[out]\n\t"
                : [out] "=m" (*(int*)((char*)int_array + iteration))  /* Complex output address */
                : [in] "r" (complex_addr_result)
                : "%eax", "memory"
            );
        }
        
        if (use_memory) {
            /* Memory-to-memory operations forcing reloads */
            struct misaligned_data temp;
            
            /* Operand address reloads */
            asm volatile (
                /* Multiple memory accesses with addressing */
                "mov %[src1], %%eax\n\t"
                "add %[src2], %%eax\n\t"
                "mov %%eax, %[dst1]\n\t"
                "mov %[src3], %%ebx\n\t"
                "lea (%[src4], %%ebx, 2), %%ecx\n\t"
                "mov %%ecx, %[dst2]\n\t"
                : [dst1] "=m" (temp.i),
                  [dst2] "=m" (temp.l)
                : [src1] "m" (packed_data.i),
                  [src2] "m" (auto_var),
                  [src3] "m" (global_counter),
                  [src4] "r" (char_ptr)
                : "%eax", "%ebx", "%ecx", "memory"
            );
        }
        
        /* Mixed data types and alignment - RELOAD_FOR_OPERAND_ADDRESS */
        {
            char *misaligned_ptr = (char*)&packed_data.i + 1;  /* Misaligned pointer */
            int misaligned_load;
            
            asm volatile (
                "mov (%[ptr]), %[out]\n\t"
                : [out] "=r" (misaligned_load)
                : [ptr] "r" (misaligned_ptr)
                : "memory"
            );
            
            /* Use the result */
            vol_var ^= misaligned_load;
        }
        
        /* Force address of address reloads */
        {
            int **ptr_to_ptr = &restr_ptr;
            int indirect_result;
            
            asm volatile (
                "mov (%[ptrptr]), %[tmp]\n\t"
                "mov (%[tmp], %[idx], 4), %[result]\n\t"
                : [result] "=r" (indirect_result),
                  [tmp] "=&r" (long_var)
                : [ptrptr] "r" (ptr_to_ptr),
                  [idx] "r" (iteration)
                : "memory"
            );
            
            auto_var += indirect_result;
        }
        
        /* Update globals to prevent loop elimination */
        global_counter += iteration;
        global_index = (global_index + 1) & 0xFF;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    unsigned long checksum = 0;
    checksum += reg_var;
    checksum += vol_var;
    checksum += auto_var;
    checksum += long_var;
    
    for (int i = 0; i < 256; i++) checksum += char_array[i];
    for (int i = 0; i < 100; i++) checksum += int_array[i];
    for (int i = 0; i < 50; i++) checksum += long_array[i];
    
    checksum += packed_data.i;
    checksum += packed_data.l;
    
    printf("Checksum: %lu\n", checksum);
    printf("Global counter: %ld\n", (long)global_counter);
    
    return 0;
}
