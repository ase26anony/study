/* reload_coverage.c - Test program to trigger GCC reload pass coverage */
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

/* Function to create register pressure */
void create_register_pressure(void) {
    /* Declare many variables to use up registers */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    register int r8 asm("r8") = 9;
    register int r9 asm("r9") = 10;
    
    /* Use them in a way that can't be optimized away */
    asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3),
                      "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7),
                      "+r"(r8), "+r"(r9));
}

int main(void) {
    /* Declare variables with different types and storage */
    int array[100];
    long big_array[50];
    char buffer[256];
    volatile int volatile_var = 42;
    struct misaligned_data packed_data;
    int *aliased_ptr = (int*)&packed_data.i;  /* Potentially misaligned */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array[i] = i;
    for (int i = 0; i < 50; i++) big_array[i] = i * 2;
    for (int i = 0; i < 256; i++) buffer[i] = i & 0xFF;
    
    packed_data.c = 'A';
    packed_data.i = 0x12345678;
    packed_data.l = 0x9876543210ABCDEFLL;
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        create_register_pressure();
        
        switch (iteration % 4) {
            case 0:
                /* Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS */
                asm volatile(
                    "mov %[val1], %[idx]\n\t"
                    "add %[val2], %[base], %[idx], lsl #2\n\t"
                    "ldr %[out1], [%[base], %[idx], lsl #2]\n\t"
                    "str %[out2], [%[base], %[idx], lsl #2, #16]\n\t"
                    : [out1] "=r" (array[iteration]),
                      [out2] "=r" (array[iteration + 1])
                    : [base] "r" (array),
                      [idx] "r" (iteration * 4),
                      [val1] "r" (iteration),
                      [val2] "r" (iteration * 2)
                    : "memory", "cc"
                );
                break;
                
            case 1:
                /* Multiple operands with early clobber - RELOAD_FOR_OUTPUT, RELOAD_FOR_INPUT */
                {
                    int temp1, temp2, temp3;
                    asm volatile(
                        "mov %[t1], %[in1]\n\t"
                        "add %[t2], %[in2], %[in3]\n\t"
                        "mul %[t3], %[t1], %[t2]\n\t"
                        : [t1] "=&r" (temp1),  /* Early clobber */
                          [t2] "=&r" (temp2),  /* Early clobber */
                          [t3] "=r" (temp3)
                        : [in1] "r" (iteration),
                          [in2] "r" (iteration * 2),
                          [in3] "r" (iteration * 3),
                          "0" (temp1),  /* Ties to output */
                          "1" (temp2)   /* Ties to output */
                        : "cc"
                    );
                    checksum += temp1 + temp2 + temp3;
                }
                break;
                
            case 2:
                /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
                {
                    long *ptr1, *ptr2;
                    asm volatile(
                        "add %[p1], %[base], %[offset1]\n\t"
                        "add %[p2], %[base], %[offset2]\n\t"
                        "ldr %[val1], [%[p1]]\n\t"
                        "str %[val2], [%[p2]]\n\t"
                        : [p1] "=&r" (ptr1),
                          [p2] "=&r" (ptr2),
                          [val1] "=r" (big_array[iteration]),
                          [val2] "=r" (big_array[iteration + 1])
                        : [base] "r" (big_array),
                          [offset1] "r" (iteration * sizeof(long)),
                          [offset2] "r" ((iteration + 1) * sizeof(long)),
                          "m" (*big_array)
                        : "memory"
                    );
                }
                break;
                
            case 3:
                /* Mixed types and misaligned access - RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
                {
                    char *char_ptr = buffer + iteration;
                    int *int_ptr = (int*)(buffer + iteration);
                    long *long_ptr = (long*)(buffer + iteration);
                    
                    /* Force address of address reloads */
                    asm volatile(
                        "ldrb %[c], [%[cp], #1]\n\t"
                        "ldr %[i], [%[ip], #4]\n\t"
                        "ldr %[l], [%[lp], #8]\n\t"
                        "strb %[c], [%[cp]]\n\t"
                        "str %[i], [%[ip]]\n\t"
                        "str %[l], [%[lp]]\n\t"
                        : [c] "=&r" (buffer[iteration]),
                          [i] "=&r" (*(int_ptr)),
                          [l] "=&r" (*(long_ptr))
                        : [cp] "r" (char_ptr),
                          [ip] "r" (int_ptr),
                          [lp] "r" (long_ptr),
                          "m" (buffer[0]),
                          "m" (*(int_ptr)),
                          "m" (*(long_ptr))
                        : "memory"
                    );
                }
                break;
        }
        
        /* Force output address reloads with complex constraints */
        if (iteration % 2 == 0) {
            int out1, out2;
            asm volatile(
                "mov %[o1], %[idx]\n\t"
                "add %[o2], %[o1], %[const]\n\t"
                : [o1] "=r" (out1),
                  [o2] "=r" (out2)
                : [idx] "r" (iteration),
                  [const] "r" (1000),
                  "0" (out1)  /* Tie constraint */
                : "cc"
            );
            checksum += out1 + out2;
        }
        
        /* Force input address address reloads */
        {
            int *addr_of_addr = &array[iteration];
            asm volatile(
                "ldr %[val], [%[addr]]\n\t"
                "add %[val], %[val], #1\n\t"
                "str %[val], [%[addr]]\n\t"
                : [val] "=&r" (array[iteration])
                : [addr] "r" (addr_of_addr),
                  "m" (*addr_of_addr)
                : "memory", "cc"
            );
        }
        
        /* Update volatile to prevent dead code elimination */
        global_counter++;
        volatile_var = iteration;
    }
    
    /* Compute final checksum to ensure all operations have effect */
    for (int i = 0; i < 100; i++) checksum += array[i];
    for (int i = 0; i < 50; i++) checksum += big_array[i];
    for (int i = 0; i < 256; i++) checksum += buffer[i];
    
    checksum += packed_data.i + (packed_data.l & 0xFFFFFFFF);
    checksum += global_counter + volatile_var;
    
    printf("Checksum: %ld\n", checksum);
    global_sum = checksum;
    
    return (int)(checksum & 0x7FFFFFFF);
}
