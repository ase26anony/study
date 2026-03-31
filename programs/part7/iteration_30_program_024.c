/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload pass */
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

int main(void) {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 12345;  /* Force specific register */
    auto long auto_array[32] = {0};
    volatile short volatile_shorts[16];
    static double static_doubles[8];
    struct misaligned_data packed_struct;
    
    /* Initialize variables */
    for (int i = 0; i < 32; i++) {
        auto_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 16; i++) {
        volatile_shorts[i] = i * 2;
    }
    for (int i = 0; i < 8; i++) {
        static_doubles[i] = i * 1.5;
    }
    
    packed_struct.c = 'A';
    packed_struct.i = 0xDEADBEEF;
    packed_struct.l = 0xCAFEBABE12345678ULL;
    packed_struct.tail = 'Z';
    
    long *misaligned_ptr = (long*)((char*)&packed_struct + 1);
    char *char_ptr = (char*)auto_array;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 100; iteration++) {
        int idx = iteration % 32;
        int offset = iteration * 4;
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple memory operands
           Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            /* Complex addressing mode for input */
            "add %[out1], %[in1], %[in2], lsl #2\n\t"
            /* Even more complex for another input */
            "add %[out2], %[in3], %[in4], lsl #1\n\t"
            : [out1] "=&r" (reg_var), 
              [out2] "=r" (auto_array[idx])
            : [in1] "r" (auto_array[idx]), 
              [in2] "r" (idx),
              [in3] "m" (*(struct misaligned_data*)(char_ptr + offset)),
              [in4] "r" (global_index),
              "m" (auto_array[0]), "m" (auto_array[31])  /* Extra memory constraints */
            : "cc", "memory"
        );
        
        /* VARYING CONSTRAINT 2: Many operands to cause register pressure
           Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        asm volatile (
            "mov %[o1], %[i1]\n\t"
            "add %[o2], %[i2], %[i3]\n\t"
            "sub %[o3], %[i4], %[i5]\n\t"
            "mul %[o4], %[i6], %[i7]\n\t"
            : [o1] "=&r" (auto_array[0]),
              [o2] "=&r" (auto_array[1]),
              [o3] "=&r" (auto_array[2]),
              [o4] "=&r" (auto_array[3])
            : [i1] "r" (reg_var),
              [i2] "r" (auto_array[4]),
              [i3] "r" (auto_array[5]),
              [i4] "r" (auto_array[6]),
              [i5] "r" (auto_array[7]),
              [i6] "r" (auto_array[8]),
              [i7] "r" (auto_array[9]),
              "m" (auto_array[10]), "m" (auto_array[11]),  /* Extra memory pressure */
              "m" (auto_array[12]), "m" (auto_array[13])
            : "cc", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
        );
        
        /* VARYING CONSTRAINT 3: Nested address computation
           Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        {
            long *ptr1 = &auto_array[idx];
            long *ptr2 = &auto_array[(idx + 1) % 32];
            
            asm volatile (
                /* Take address of memory operand that itself needs reloading */
                "ldr x0, [%[addr1], %[offset1], lsl #3]\n\t"
                "str x0, [%[addr2], %[offset2], lsl #3]\n\t"
                : 
                : [addr1] "r" (auto_array),
                  [offset1] "r" (idx),
                  [addr2] "r" (auto_array),
                  [offset2] "r" ((idx + 2) % 32),
                  "m" (*ptr1), "m" (*ptr2)  /* These may need address reloads */
                : "x0", "memory"
            );
        }
        
        /* VARYING CONSTRAINT 4: Output address reloads
           Likely triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            /* Complex addressing for output */
            "str %[val], [%[base], %[index], lsl #3]\n\t"
            /* Another with different scale */
            "strh %w[shortval], [%[base2], %[index2], lsl #1]\n\t"
            : 
            : [val] "r" (reg_var),
              [base] "r" (auto_array),
              [index] "r" (idx),
              [shortval] "r" (volatile_shorts[idx % 16]),
              [base2] "r" (volatile_shorts),
              [index2] "r" ((idx * 3) % 16),
              "m" (auto_array[0])  /* Memory clobber */
            : "memory"
        );
        
        /* VARYING CONSTRAINT 5: Mixed data types and sizes
           Likely triggers: RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "ldrb w1, [%[charptr], %[idx1]]\n\t"
            "ldr x2, [%[longptr], %[idx2], lsl #3]\n\t"
            "add w1, w1, w2\n\t"
            "strb w1, [%[charptr], %[idx3]]\n\t"
            : 
            : [charptr] "r" (char_ptr),
              [idx1] "r" (iteration % 31),
              [longptr] "r" (auto_array),
              [idx2] "r" ((iteration + 1) % 32),
              [idx3] "r" ((iteration + 2) % 31),
              "m" (packed_struct),  /* Misaligned access */
              "m" (*misaligned_ptr) /* Definitely misaligned */
            : "w1", "x2", "memory"
        );
        
        /* Update globals to prevent dead code elimination */
        global_counter += reg_var + auto_array[idx];
        global_index = iteration;
        
        /* Change constraints every few iterations */
        if (iteration % 25 == 0) {
            /* Force different register allocation */
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
    
    /* Compute checksum to ensure all operations had effect */
    unsigned long long checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += auto_array[i];
    }
    checksum += reg_var;
    checksum += packed_struct.i;
    checksum += packed_struct.l;
    checksum += global_counter;
    
    printf("Checksum: %llu\n", checksum);
    printf("Final reg_var: %d\n", reg_var);
    printf("Global counter: %ld\n", global_counter);
    
    return (checksum > 0) ? 0 : 1;
}
