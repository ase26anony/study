/* test_resource_coverage.c
 * Designed to trigger mark_set_resources lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing critical patterns */
static volatile int external_condition = 1;

/* Global struct with bitfield - ensures memory storage */
struct GlobalStatus {
    unsigned int ready : 1;
    unsigned int count : 4;
    unsigned int error : 2;
    unsigned int padding : 25;
};

struct GlobalStatus global_status;

/* Struct passed by pointer - ensures bitfield is in memory */
struct DeviceReg {
    volatile unsigned int control : 3;
    unsigned int data : 8;
    unsigned int status : 5;
    unsigned int reserved : 16;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_control_field(struct DeviceReg *reg, unsigned int value) {
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* This assignment should generate SET with ZERO_EXTRACT destination
     * referencing memory through the pointer */
    reg->control = value & 0x7;
    
    /* Another barrier to preserve the pattern */
    asm volatile("" : : : "memory");
}

/* Function with multiple bitfield operations in loop */
void update_status_fields(int iterations) {
    struct DeviceReg reg;
    
    /* Initialize */
    reg.control = 0;
    reg.data = 0;
    reg.status = 0;
    
    /* Loop with bitfield assignments - increases chance RTL remains visible */
    for (int i = 0; i < iterations; i++) {
        /* Conditional based on external input to prevent dead code elimination */
        if (external_condition & 0x1) {
            reg.control = (reg.control + 1) & 0x7;  /* ZERO_EXTRACT in memory */
        }
        
        if (external_condition & 0x2) {
            reg.data = (reg.data << 1) | 0x1;
        }
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use the result to prevent optimization */
    global_status.count = reg.control;
}

/* Function using inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    int value = 42;
    char byte_result;
    
    /* Inline assembly with byte constraint - may generate STRICT_LOW_PART
     * when targeting memory */
    asm volatile(
        "movb %[src], %[dst]\n\t"
        : [dst] "=Q" (byte_result)  /* "Q" = byte-addressable register (a, b, c, d) */
        : [src] "r" ((char)value)
        : "memory"
    );
    
    /* Store to global to ensure memory reference */
    global_status.ready = byte_result & 0x1;
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(struct DeviceReg *reg) {
    /* Atomic operation on bitfield - may generate complex RTL with ZERO_EXTRACT */
    int old = __sync_fetch_and_or(&reg->control, 0x1);
    
    /* Another atomic with different operation */
    __sync_fetch_and_and(&reg->control, 0x6);
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_resource_interaction(void) {
    struct DeviceReg reg1, reg2;
    int temp;
    
    /* Initialize */
    reg1.control = 1;
    reg2.control = 2;
    
    /* Multiple inline asm blocks with overlapping resources */
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (reg1.control)   /* Memory destination - bitfield */
        : "r" (external_condition)
        : "eax", "memory"
    );
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
    
    /* Another asm that reads the same memory */
    asm volatile(
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %0\n\t"
        : "+m" (reg2.data)      /* Read-modify-write memory */
        : "r" (temp)
        : "ebx", "memory"
    );
    
    /* Bitfield assignment that might be scheduled around the asm */
    if (external_condition) {
        reg1.status = reg2.data & 0x1F;  /* ZERO_EXTRACT in memory */
    }
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    struct DeviceReg device_reg;
    
    /* Use argc to make control flow unpredictable */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Exercise ZERO_EXTRACT patterns */
    set_control_field(&device_reg, 3);
    update_status_fields(iterations);
    
    /* Exercise potential STRICT_LOW_PART patterns */
    partial_register_ops();
    
    /* Exercise atomic operations on bitfields */
    atomic_bitfield_ops(&device_reg);
    
    /* Exercise complex resource interactions */
    complex_resource_interaction();
    
    /* Use results to prevent dead code elimination */
    printf("Control field: %u\n", device_reg.control);
    printf("Global status: %u\n", global_status.count);
    
    return 0;
}
