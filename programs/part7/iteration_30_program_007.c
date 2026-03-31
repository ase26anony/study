/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
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
    volatile int vol_var = 42;
    register int reg_var asm("ebx") = 100;  /* Hint for register allocation */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 8; i++) {
        packed_data[i].c = i;
        packed_data[i].i = i * 100;
        packed_data[i].l = i * 1000L;
    }
    
    /* Complex addressing mode variables */
    int *base_ptr = arr;
    long index_reg = 16;
    int scale = 4;
    long displacement = 8;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int temp1, temp2, temp3;
        long temp_long;
        void *addr_temp;
        
        /* VARYING CONSTRAINT PATTERNS EACH ITERATION */
        switch (iter % 4) {
            case 0:
                /* Complex addressing with multiple memory operands */
                /* Triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    "movl %[base], %%eax\n\t"
                    "movl %[index], %%ebx\n\t"
                    "movl %[scale], %%ecx\n\t"
                    "imull %%ecx, %%ebx\n\t"
                    "addl %[disp], %%ebx\n\t"
                    "movl (%%eax, %%ebx), %[out1]\n\t"
                    "leal (%%eax, %%ebx, %[scale2]), %[out2]\n\t"
                    : [out1] "=r" (temp1), [out2] "=r" (temp2)
                    : [base] "m" (base_ptr), [index] "r" (index_reg),
                      [scale] "r" (scale), [disp] "r" (displacement),
                      [scale2] "i" (2)
                    : "eax", "ebx", "ecx", "memory"
                );
                global_sum += temp1 + temp2;
                break;
                
            case 1:
                /* Many operands causing register pressure */
                /* Triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                asm volatile (
                    "mov %[in1], %[out1]\n\t"
                    "add %[in2], %[out1]\n\t"
                    "mov %[in3], %[out2]\n\t"
                    "sub %[in4], %[out2]\n\t"
                    "imul %[in5], %[out3]\n\t"
                    : [out1] "=&r" (temp1),  /* Early clobber */
                      [out2] "=&r" (temp2),  /* Early clobber */
                      [out3] "=r" (temp3)
                    : [in1] "r" (arr[iter]), 
                      [in2] "r" (arr[iter + 1]),
                      [in3] "r" (arr[iter + 2]),
                      [in4] "r" (arr[iter + 3]),
                      [in5] "r" (arr[iter + 4]),
                      "0" (temp1)  /* Tied operand */
                    : "cc"
                );
                global_sum += temp1 * temp2 - temp3;
                break;
                
            case 2:
                /* Nested address computation */
                /* Triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
                {
                    int *addr_of_mem = &arr[iter * 16];
                    asm volatile (
                        "movl %[addr], %%eax\n\t"
                        "movl (%%eax), %%ebx\n\t"
                        "leal 4(%%eax), %[out]\n\t"
                        "movl (%[out]), %%ecx\n\t"
                        : [out] "=r" (addr_temp)
                        : [addr] "m" (addr_of_mem)
                        : "eax", "ebx", "ecx", "memory"
                    );
                    temp1 = *(int*)addr_temp;
                    global_sum += temp1;
                }
                break;
                
            case 3:
                /* Mixed data types with alignment issues */
                /* Triggers: RELOAD_FOR_OTHER_ADDRESS, various others */
                {
                    char *char_ptr = (char*)&packed_data[iter % 4];
                    int *int_ptr = (int*)(char_ptr + 1);  /* Misaligned! */
                    
                    asm volatile (
                        "movzbl (%[char]), %%eax\n\t"
                        "movl (%[int]), %%ebx\n\t"
                        "addl %%eax, %%ebx\n\t"
                        "movl %%ebx, %[out]\n\t"
                        "movq (%[long]), %%rcx\n\t"
                        "addq %%rcx, %[sum]\n\t"
                        : [out] "=m" (packed_data[iter % 4].i),
                          [sum] "+m" (global_sum)
                        : [char] "r" (char_ptr),
                          [int] "r" (int_ptr),
                          [long] "r" (&packed_data[iter % 4].l)
                        : "rax", "rbx", "rcx", "memory"
                    );
                }
                break;
        }
        
        /* Modify addressing parameters to vary patterns */
        index_reg = (index_reg * 13 + 7) % 128;
        displacement = (displacement + iter) % 64;
        scale = (scale % 4) + 1;
        
        /* Force output address reloads */
        if (iter % 3 == 0) {
            long *output_ptr = &global_sum;
            asm volatile (
                "movq %[val], %%rax\n\t"
                "addq %%rax, (%[out])\n\t"
                : 
                : [val] "r" (iter * 1000L),
                  [out] "r" (output_ptr)
                : "rax", "memory"
            );
        }
        
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations had effect */
    uint64_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += packed_data[i].i + packed_data[i].l;
    }
    checksum += global_sum + global_counter + vol_var;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Global sum: %ld\n", global_sum);
    printf("Counter: %d\n", global_counter);
    
    return (checksum > 1000000) ? 0 : 1;
}
