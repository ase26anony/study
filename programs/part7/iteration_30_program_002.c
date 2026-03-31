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

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long index_reg = 50;
    int scale_factor = 4;
    int displacement = 100;
    volatile int *volatile_ptr = &global_counter;
    char char_arr[256];
    double double_arr[50];
    
    /* Packed/misaligned struct */
    struct misaligned_data packed_data;
    packed_data.c = 'A';
    packed_data.i = 0x12345678;
    packed_data.l = 0x9876543210ABCDEFLL;
    
    /* Address-taken variables */
    int addr_taken1, addr_taken2, addr_taken3;
    int *ptr1 = &addr_taken1;
    int *ptr2 = &addr_taken2;
    int *ptr3 = &addr_taken3;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary scale and displacement each iteration */
        int scale = 1 << (iteration % 4);  /* 1, 2, 4, 8 */
        int disp = iteration * 16;
        
        /* Complex addressing mode: [base + index*scale + displacement]
           This can trigger RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[idx]\n\t"
            "add %[val2], %[idx], %[scale]\n\t"
            : [idx] "=r" (index_reg)
            : [val1] "r" (arr[iteration]), 
              [val2] "r" (disp),
              [scale] "r" (scale)
            : "cc"
        );
        
        /* Multiple operands with conflicting constraints
           Triggers RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        int temp1, temp2, temp3;
        asm volatile (
            "mov %[out1], %[in1]\n\t"
            "add %[out2], %[in2], %[in3]\n\t"
            "imul %[out3], %[in4], %[in5]\n\t"
            : [out1] "=&r" (temp1),  /* Early clobber */
              [out2] "=&r" (temp2),  /* Early clobber */
              [out3] "=r" (temp3)
            : [in1] "r" (arr[iteration]),
              [in2] "r" (scale),
              [in3] "r" (disp),
              [in4] "r" (iteration),
              [in5] "r" (global_counter)
            : "cc"
        );
        
        /* Nested address computation 
           Triggers RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        long complex_addr;
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            "mov (%[addr]), %[val]\n\t"
            : [addr] "=r" (complex_addr),
              [val] "=r" (temp1)
            : [base] "r" (arr),
              [index] "r" (index_reg),
              [scale] "i" (sizeof(int))
            : "memory"
        );
        
        /* Memory operand with complex addressing that itself needs reload
           Triggers RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[src], %%rax\n\t"
            "mov %%rax, %[dest]\n\t"
            : [dest] "=m" (*(int*)((char*)arr + index_reg * scale + disp))
            : [src] "r" (iteration)
            : "rax", "memory"
        );
        
        /* Output with complex address
           Triggers RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int output_val;
        asm volatile (
            "mov %[in], %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %[out]\n\t"
            : [out] "=m" (*(int*)((uintptr_t)&packed_data.i + iteration))
            : [in] "r" (global_counter)
            : "eax", "memory"
        );
        
        /* Mixed data types with memory clobber
           Triggers RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
        asm volatile (
            "movzbl %[char_in], %%eax\n\t"
            "addl %[int_in], %%eax\n\t"
            "movq %[long_in], %%rbx\n\t"
            "addq %%rax, %%rbx\n\t"
            "mov %%rbx, %[long_out]\n\t"
            : [long_out] "=m" (packed_data.l)
            : [char_in] "m" (packed_data.c),
              [int_in] "m" (packed_data.i),
              [long_in] "m" (packed_data.l)
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Force spill/reload with many operands */
        int r1, r2, r3, r4, r5, r6, r7, r8;
        asm volatile (
            "mov %[a1], %[o1]\n\t"
            "mov %[a2], %[o2]\n\t"
            "mov %[a3], %[o3]\n\t"
            "mov %[a4], %[o4]\n\t"
            "mov %[a5], %[o5]\n\t"
            "mov %[a6], %[o6]\n\t"
            "mov %[a7], %[o7]\n\t"
            "mov %[a8], %[o8]\n\t"
            : [o1] "=&r" (r1), [o2] "=&r" (r2),
              [o3] "=&r" (r3), [o4] "=&r" (r4),
              [o5] "=&r" (r5), [o6] "=&r" (r6),
              [o7] "=&r" (r7), [o8] "=&r" (r8)
            : [a1] "r" (arr[iteration]),
              [a2] "r" (arr[iteration + 1]),
              [a3] "r" (arr[iteration + 2]),
              [a4] "r" (arr[iteration + 3]),
              [a5] "r" (arr[iteration + 4]),
              [a6] "r" (arr[iteration + 5]),
              [a7] "r" (arr[iteration + 6]),
              [a8] "r" (arr[iteration + 7])
            : "cc"
        );
        
        /* Update global counter to prevent dead code elimination */
        global_counter += iteration + temp1 + temp2 + temp3;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    uint64_t checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    checksum += index_reg;
    checksum += packed_data.i;
    checksum += (packed_data.l & 0xFFFFFFFF) + ((packed_data.l >> 32) & 0xFFFFFFFF);
    checksum += global_counter;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
