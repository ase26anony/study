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
    int arr[100];
    long *ptr_arr[50];
    char buffer[256];
    volatile int volatile_var = 42;
    struct misaligned_data packed_data = {0};
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
        if (i < 50) ptr_arr[i] = (long*)&arr[i];
    }
    
    /* Take addresses to force register pressure */
    int *addr1 = &arr[10];
    int *addr2 = &arr[20];
    int *addr3 = &arr[30];
    long *addr4 = (long*)&packed_data;
    
    /* Complex loop with varying assembly constraints */
    for (int iteration = 0; iteration < 10; iteration++) {
        int temp1, temp2, temp3;
        long temp4;
        void *complex_addr;
        
        /* Vary constraints based on iteration */
        int constraint_type = iteration % 4;
        
        switch (constraint_type) {
            case 0:
                /* Complex addressing modes - triggers RELOAD_FOR_INPUT_ADDRESS */
                __asm__ volatile (
                    "mov %[val1], %[idx]\n\t"
                    "lea (%[base], %[idx], 4), %[addr]\n\t"
                    "mov (%[addr]), %[out1]\n\t"
                    "add $0x1234, %[out1]\n\t"
                    "mov %[out1], (%[addr2], %[idx], 2)"
                    : [out1] "=&r" (temp1),
                      [addr] "=&r" (complex_addr)
                    : [base] "r" (arr),
                      [idx] "r" (iteration * 4),
                      [val1] "i" (iteration),
                      [addr2] "r" (buffer)
                    : "memory", "cc"
                );
                break;
                
            case 1:
                /* Multiple operands with early clobber - triggers RELOAD_FOR_INPUT/OUTPUT */
                __asm__ volatile (
                    "imul %[in1], %[in2]\n\t"
                    "add %[in3], %[in2]\n\t"
                    "mov %[in2], %[out1]\n\t"
                    "lea (%[out1], %[in4], 8), %[out2]"
                    : [out1] "=&r" (temp1),
                      [out2] "=&r" (temp2)
                    : [in1] "r" (arr[iteration]),
                      [in2] "0" (arr[iteration + 1]),
                      [in3] "r" (volatile_var),
                      [in4] "r" (iteration)
                    : "memory", "cc"
                );
                break;
                
            case 2:
                /* Nested address computation - triggers RELOAD_FOR_OPERAND_ADDRESS */
                {
                    long *nested_ptr = &arr[iteration];
                    __asm__ volatile (
                        "mov (%[ptr]), %[val]\n\t"
                        "add %%gs:0x0, %[val]\n\t"
                        "mov %[val], (%[ptr2])"
                        : [val] "=&r" (temp4)
                        : [ptr] "r" (nested_ptr),
                          [ptr2] "r" (&buffer[iteration * 8])
                        : "memory", "cc"
                    );
                }
                break;
                
            case 3:
                /* Mixed size operands with memory constraints - triggers various reloads */
                __asm__ volatile (
                    "movzbl (%[char_ptr]), %%eax\n\t"
                    "addl %%eax, (%[int_ptr])\n\t"
                    "movq (%[long_ptr]), %%rbx\n\t"
                    "addq %%rbx, (%[out_ptr])"
                    : 
                    : [char_ptr] "r" (&buffer[iteration]),
                      [int_ptr] "r" (&arr[iteration]),
                      [long_ptr] "r" (ptr_arr[iteration % 50]),
                      [out_ptr] "r" (&global_sum)
                    : "memory", "cc", "rax", "rbx"
                );
                break;
        }
        
        /* Force output address reloads with complex store operations */
        if (iteration % 3 == 0) {
            long complex_index = iteration * 7 + 3;
            __asm__ volatile (
                "mov %[data], (%[base], %[idx], 8)\n\t"
                "incq (%[base], %[idx], 8)"
                : 
                : [data] "r" (iteration * 1000L),
                  [base] "r" (ptr_arr),
                  [idx] "r" (complex_index % 50)
                : "memory", "cc"
            );
        }
        
        /* Address of address computation - triggers RELOAD_FOR_INPADDR_ADDRESS */
        {
            int *addr_of_addr = &addr1;
            __asm__ volatile (
                "mov (%[addr_ptr]), %[reg]\n\t"
                "addl $1, (%[reg])"
                : [reg] "=&r" (temp3)
                : [addr_ptr] "r" (addr_of_addr)
                : "memory", "cc"
            );
        }
        
        /* Force other address reloads with segment registers */
        __asm__ volatile (
            "mov %%fs:0, %[out]\n\t"
            "add %[in], %[out]"
            : [out] "=&r" (temp1)
            : [in] "r" (iteration)
            : "cc"
        );
        
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    checksum += global_sum;
    checksum += global_counter;
    checksum += (long)&checksum; /* Include address for variability */
    
    /* Use checksum to prevent dead code elimination */
    __asm__ volatile (
        "add %[sum], %[total]"
        : [total] "+r" (global_sum)
        : [sum] "r" (checksum)
        : "cc"
    );
    
    printf("Checksum: %ld\n", checksum);
    printf("Global sum: %ld\n", global_sum);
    printf("Counter: %d\n", global_counter);
    
    return (int)(checksum & 0x7FFFFFFF);
}
