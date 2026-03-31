/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready: 1;
    unsigned int count: 3;
    unsigned int flags: 4;
    unsigned int padding: 24;
};

struct GlobalStatus global_status = {0, 0, 0, 0};

/* Struct passed by pointer to force memory access */
struct BitfieldContainer {
    unsigned int field1: 5;
    unsigned int field2: 7;
    unsigned int field3: 10;
    unsigned int field4: 10;
};

/* Function that takes pointer to ensure bitfield is in memory */
void set_bitfield_in_memory(struct BitfieldContainer *container, 
                           unsigned int val1, unsigned int val2) {
    /* Multiple bitfield assignments to increase RTL visibility */
    container->field1 = val1 & 0x1F;
    COMPILER_BARRIER();
    container->field2 = val2 & 0x7F;
    COMPILER_BARRIER();
    
    /* Conditional assignment to prevent optimization */
    if (val1 > val2) {
        container->field3 = (val1 + val2) & 0x3FF;
    } else {
        container->field4 = (val2 - val1) & 0x3FF;
    }
}

/* Function with inline assembly that may generate STRICT_LOW_PART */
void partial_register_operations(int *memory_loc) {
    /* Use byte-addressable register constraints */
    register uint8_t byte_reg asm("al");
    
    /* Inline assembly with "=Q" constraint for byte-addressable register */
    asm volatile (
        "movb $0x42, %0\n\t"
        "movb %0, (%1)\n\t"
        : "=Q" (byte_reg)
        : "r" (memory_loc)
        : "memory"
    );
    
    /* Additional operations to create scheduling complexity */
    asm volatile (
        "lock addb $1, (%0)\n\t"
        :
        : "r" (memory_loc)
        : "memory", "cc"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(struct GlobalStatus *status) {
    /* Atomic operation on bitfield - may generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&status->flags, 0x3);
    
    /* Mixed operations to create resource tracking needs */
    status->count = (status->count + 1) & 0x7;
    COMPILER_BARRIER();
    
    /* Volatile ensures memory access */
    status->ready = 1;
}

/* Complex function with multiple resource interactions */
void complex_resource_interaction(struct BitfieldContainer *cont1,
                                 struct BitfieldContainer *cont2,
                                 int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Bitfield assignments in loop - harder to optimize away */
        cont1->field1 = (cont1->field1 + i) & 0x1F;
        cont2->field2 = (cont2->field2 ^ i) & 0x7F;
        
        /* Inline assembly that reads/writes memory */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "andl $0x1F, %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            :
            : "r" (&cont1->field1), "r" (&cont2->field2)
            : "eax", "memory"
        );
        
        /* Compiler barrier prevents loop optimization */
        COMPILER_BARRIER();
    }
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    struct BitfieldContainer container1 = {0, 0, 0, 0};
    struct BitfieldContainer container2 = {0, 0, 0, 0};
    int memory_buffer[4] = {0};
    
    /* Use argc for unpredictable control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Multiple operations to trigger different RTL patterns */
    
    /* 1. Bitfield assignment through pointer (ZERO_EXTRACT with MEM) */
    set_bitfield_in_memory(&container1, argc, iterations);
    
    /* 2. Partial register operations (potential STRICT_LOW_PART) */
    partial_register_operations(memory_buffer);
    
    /* 3. Atomic operations on global bitfield */
    atomic_bitfield_ops(&global_status);
    
    /* 4. Complex loop with mixed operations */
    complex_resource_interaction(&container1, &container2, iterations);
    
    /* Additional volatile bitfield assignment */
    volatile struct {
        unsigned int status_bits: 8;
        unsigned int control_bits: 8;
    } volatile_reg = {0, 0};
    
    volatile_reg.status_bits = argc & 0xFF;
    volatile_reg.control_bits = iterations & 0xFF;
    
    /* Mix with function calls to prevent optimization */
    if (container1.field1 > container2.field2) {
        set_bitfield_in_memory(&container2, 
                              container1.field1, 
                              container2.field2);
    }
    
    /* Final atomic operation */
    __sync_synchronize();
    
    /* Return something based on results to prevent dead code elimination */
    return (container1.field1 + container2.field2 + 
            global_status.count + volatile_reg.status_bits) & 0xFF;
}

/* Additional global to ensure external linkage */
struct BitfieldContainer *external_ref;

void init_external(void) {
    external_ref = malloc(sizeof(struct BitfieldContainer));
    if (external_ref) {
        external_ref->field1 = 1;
        external_ref->field2 = 2;
        external_ref->field3 = 3;
        external_ref->field4 = 4;
    }
}
