/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;
    unsigned int count : 7;
    unsigned int data : 24;
} global_status;

/* Another struct for pointer-based access */
struct DeviceReg {
    unsigned int enable : 1;
    unsigned int mode : 3;
    unsigned int value : 12;
    unsigned int reserved : 16;
};

/* Function that takes pointer to ensure memory destination */
void set_device_reg(struct DeviceReg *reg, unsigned int val) {
    /* Multiple bitfield assignments with compiler barrier */
    reg->enable = 1;
    asm volatile("" : : : "memory");
    reg->mode = val & 0x7;
    asm volatile("" : : : "memory");
    reg->value = (val >> 3) & 0xFFF;
}

/* Function with inline assembly that may generate STRICT_LOW_PART */
void byte_register_ops(void) {
    /* Using char variables that may be allocated to byte registers */
    register char al_byte asm("al");
    register char ah_byte asm("ah");
    
    /* Inline assembly with byte constraints - may generate STRICT_LOW_PART */
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb $0x23, %1"
        : "=Q" (al_byte), "=Q" (ah_byte)
        :
        : "memory"
    );
    
    /* Store to memory to ensure MEM reference */
    static volatile char mem_byte;
    mem_byte = al_byte;
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    struct {
        unsigned int lock : 1;
        unsigned int counter : 15;
        unsigned int flags : 16;
    } atomic_data;
    
    /* Atomic operations may generate ZERO_EXTRACT with memory */
    __sync_fetch_and_or(&atomic_data.lock, 1);
    __sync_fetch_and_add(&atomic_data.counter, 1);
}

/* Complex function with control flow to prevent optimization */
void complex_bitfield_sequence(int argc, char **argv) {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } local_bf;
    
    /* Volatile read to prevent dead code elimination */
    volatile int seed = argc;
    
    /* Loop with bitfield assignments */
    for (int i = 0; i < 10; i++) {
        /* Conditional assignments based on unpredictable value */
        if (seed & (1 << i)) {
            local_bf.a = i & 0xF;
            local_bf.b = (i + 1) & 0xF;
        } else {
            local_bf.c = (i * 2) & 0xF;
            local_bf.d = (i * 3) & 0xF;
        }
        
        /* Compiler barrier to prevent merging */
        asm volatile("" : : : "memory");
    }
    
    /* Take address and pass to external function to force memory storage */
    set_device_reg((struct DeviceReg*)&local_bf, seed);
}

/* Function that mixes bitfields and inline assembly */
void mixed_operations(void) {
    struct ControlWord {
        unsigned int opcode : 8;
        unsigned int param : 8;
        unsigned int flags : 16;
    } cw;
    
    int external_val = rand();
    
    /* Inline assembly that reads/writes to bitfield memory location */
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (cw.opcode)
        : "r" (external_val)
        : "eax", "memory"
    );
    
    /* Another asm that might use partial register */
    asm volatile(
        "movw $0x1234, %0"
        : "=m" (cw.flags)
        :
        : "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Initialize global bitfield */
    global_status.ready = 1;
    global_status.count = 0;
    global_status.data = 0x123456;
    
    /* Exercise pointer-based bitfield assignment */
    struct DeviceReg *reg = malloc(sizeof(struct DeviceReg));
    if (reg) {
        set_device_reg(reg, argc);
        free(reg);
    }
    
    /* Exercise byte register operations */
    byte_register_ops();
    
    /* Exercise atomic operations */
    atomic_bitfield_ops();
    
    /* Exercise complex control flow */
    complex_bitfield_sequence(argc, argv);
    
    /* Exercise mixed operations */
    mixed_operations();
    
    /* Final volatile store to prevent optimization */
    volatile int result = global_status.ready;
    return result;
}

/* Additional function to increase scheduling complexity */
void scheduler_stress(void) {
    struct {
        unsigned int f1 : 2;
        unsigned int f2 : 2;
        unsigned int f3 : 2;
        unsigned int f4 : 2;
    } bf_array[10];
    
    /* Multiple stores to create scheduling pressure */
    for (int i = 0; i < 10; i++) {
        bf_array[i].f1 = i & 3;
        bf_array[i].f2 = (i + 1) & 3;
        bf_array[i].f3 = (i + 2) & 3;
        bf_array[i].f4 = (i + 3) & 3;
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
}
