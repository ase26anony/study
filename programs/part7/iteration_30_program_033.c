/* reload_coverage.c - Test program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char extra[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

/* Function to create register pressure */
void create_register_pressure(int iterations) {
    /* Declare many variables to consume registers */
    register int r0 asm("r0") = iterations;
    register int r1 asm("r1") = iterations * 2;
    register int r2 asm("r2") = iterations * 3;
    register int r3 asm("r3") = iterations * 4;
    register int r4 asm("r4") = iterations * 5;
    register int r5 asm("r5") = iterations * 6;
    register int r6 asm("r6") = iterations * 7;
    register int r7 asm("r7") = iterations * 8;
    
    /* Array with complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Packed struct for misaligned access */
    struct misaligned_data packed;
    packed.c = 'A';
    packed.i = 0x12345678;
    packed.l = 0x9876543210ABCDEFLL;
    
    /* Pointers with different alignments */
    char *char_ptr = (char *)&packed;
    int *int_ptr = (int *)(char_ptr + 1);  /* Misaligned */
    long *long_ptr = (long *)(char_ptr + 3); /* Also misaligned */
    
    /* Loop with varying constraints */
    for (int i = 0; i < iterations; i++) {
        int idx = (i * 17) % 256;
        int offset = (i * 13) % 128;
        
        /* Vary constraints based on iteration */
        if (i % 4 == 0) {
            /* Complex addressing mode with multiple components */
            asm volatile (
                "add %[out], %[in1], %[in2], lsl #2\n\t"
                "str %[out], [%[base], %[index], lsl #2]\n\t"
                : [out] "=&r" (array[idx])
                : [in1] "r" (r0), 
                  [in2] "r" (r1),
                  [base] "r" (array),
                  [index] "r" (idx)
                : "memory", "cc"
            );
        } 
        else if (i % 4 == 1) {
            /* Multiple outputs with early clobber */
            int out1, out2;
            asm volatile (
                "mov %[o1], %[a]\n\t"
                "add %[o2], %[b], %[c]\n\t"
                "mul %[o1], %[o1], %[o2]\n\t"
                : [o1] "=&r" (out1),
                  [o2] "=&r" (out2)
                : [a] "r" (r2),
                  [b] "r" (r3),
                  [c] "r" (r4)
                : "cc"
            );
            r0 = out1 + out2;
        }
        else if (i % 4 == 2) {
            /* Nested address computation */
            long *addr_ptr;
            asm volatile (
                "add %[addr], %[base], %[offset], lsl #3\n\t"
                : [addr] "=r" (addr_ptr)
                : [base] "r" (long_ptr),
                  [offset] "r" (offset)
            );
            
            /* Use the computed address */
            asm volatile (
                "ldr %[val], [%[addr]]\n\t"
                "add %[val], %[val], #1\n\t"
                "str %[val], [%[addr]]\n\t"
                : [val] "=&r" (packed.l)
                : [addr] "r" (addr_ptr)
                : "memory"
            );
        }
        else {
            /* Maximum register pressure with memory operands */
            asm volatile (
                "ldr r8, [%[mem1]]\n\t"
                "ldr r9, [%[mem2], %[idx], lsl #2]\n\t"
                "add r10, r8, r9\n\t"
                "str r10, [%[mem3], %[off], lsl #2]\n\t"
                "add %[r0], %[r0], r10\n\t"
                "add %[r1], %[r1], r8\n\t"
                "add %[r2], %[r2], r9\n\t"
                : [r0] "+r" (r0),
                  [r1] "+r" (r1),
                  [r2] "+r" (r2)
                : [mem1] "m" (packed.i),
                  [mem2] "r" (array),
                  [mem3] "r" (array),
                  [idx] "r" (idx),
                  [off] "r" (offset)
                : "r8", "r9", "r10", "memory", "cc"
            );
        }
        
        /* Mix in some C operations to create more register pressure */
        r3 = r0 * r1 + r2;
        r4 = r1 * r2 + r3;
        r5 = r2 * r3 + r4;
        r6 = r3 * r4 + r5;
        r7 = r4 * r5 + r6;
        
        /* Update global volatile to prevent dead code elimination */
        global_counter++;
    }
    
    /* Compute checksum */
    long checksum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    for (int i = 0; i < 256; i++) {
        checksum += array[i];
    }
    checksum += packed.i + packed.l;
    
    global_sum = checksum;
}

/* Function to trigger operand address reloads */
void trigger_operand_address_reloads(void) {
    int data[100];
    int *pointers[10];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Create complex address computations */
    for (int i = 0; i < 10; i++) {
        int offset = i * 7;
        
        /* This should trigger RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "add %[ptr], %[base], %[off], lsl #2\n\t"
            "ldr r11, [%[ptr]]\n\t"
            "add r11, r11, #1\n\t"
            "str r11, [%[ptr]]\n\t"
            : [ptr] "=&r" (pointers[i])
            : [base] "r" (data),
              [off] "r" (offset)
            : "r11", "memory"
        );
    }
    
    /* Nested addressing - address of an address */
    int **ptr_to_ptr = &pointers[5];
    asm volatile (
        "ldr r12, [%[pptr]]\n\t"
        "ldr r13, [r12]\n\t"
        "add r13, r13, #42\n\t"
        "str r13, [r12]\n\t"
        :
        : [pptr] "r" (ptr_to_ptr)
        : "r12", "r13", "memory"
    );
}

/* Function with inline assembly using many constraints */
void many_constraint_asm(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int out1, out2, out3, out4;
    
    /* Many input/output operands to stress register allocation */
    asm volatile (
        "add %[o1], %[a], %[b]\n\t"
        "sub %[o2], %[c], %[d]\n\t"
        "mul %[o3], %[e], %[f]\n\t"
        "and %[o4], %[g], %[h]\n\t"
        "orr %[o1], %[o1], %[o2]\n\t"
        "eor %[o3], %[o3], %[o4]\n\t"
        : [o1] "=&r" (out1),
          [o2] "=&r" (out2),
          [o3] "=&r" (out3),
          [o4] "=&r" (out4)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    global_sum += out1 + out2 + out3 + out4;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Test 1: Create register pressure with complex addressing */
    create_register_pressure(100);
    
    /* Test 2: Trigger operand address reloads */
    trigger_operand_address_reloads();
    
    /* Test 3: Many constraint inline assembly */
    many_constraint_asm();
    
    /* Test 4: Mixed data types and complex memory operands */
    {
        char buffer[128];
        short shorts[64];
        int ints[32];
        long longs[16];
        
        /* Initialize arrays */
        for (int i = 0; i < 128; i++) buffer[i] = i;
        for (int i = 0; i < 64; i++) shorts[i] = i * 2;
        for (int i = 0; i < 32; i++) ints[i] = i * 3;
        for (int i = 0; i < 16; i++) longs[i] = i * 4;
        
        /* Complex asm with mixed types */
        for (int i = 0; i < 8; i++) {
            asm volatile (
                "ldrb r14, [%[buf], %[i]]\n\t"
                "ldrh r15, [%[shorts], %[i], lsl #1]\n\t"
                "add r14, r14, r15\n\t"
                "str r14, [%[ints], %[i], lsl #2]\n\t"
                "ldr r14, [%[ints], %[i], lsl #2]\n\t"
                "str r14, [%[longs], %[i], lsl #3]\n\t"
                :
                : [buf] "r" (buffer),
                  [shorts] "r" (shorts),
                  [ints] "r" (ints),
                  [longs] "r" (longs),
                  [i] "r" (i)
                : "r14", "r15", "memory"
            );
        }
    }
    
    printf("Test completed. Global counter: %d, Global sum: %ld\n", 
           global_counter, global_sum);
    
    return (int)global_sum;
}
