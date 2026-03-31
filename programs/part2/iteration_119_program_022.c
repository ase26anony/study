/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to prevent optimization */
volatile int global_volatile_int = 42;
volatile int16_t global_volatile_short = 100;
int global_int = 1234;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int high_bits : 8;
    unsigned int pad : 16;
} volatile global_bitfield = {1, 2, 0};

/* Test 1: Force secondary reloads via fixed register constraints */
void test_fixed_registers(void) {
    /* Bind variables to specific registers */
    register int ebx_var asm("ebx") = global_volatile_int;
    register int eax_var asm("eax") = global_int;
    register int ecx_var asm("ecx");
    
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        /* Move from memory to fixed register - may need secondary reload */
        "movl %1, %0\n\t"
        /* Arithmetic with mixed constraints */
        "addl %2, %0\n\t"
        "subl %3, %0"
        : "=a"(eax_var), "+&b"(ebx_var)
        : "m"(global_volatile_int), "r"(global_int)
        : "cc", "memory"
    );
    
    /* Force use of the results */
    global_int = eax_var + ebx_var;
    
    /* Another test with immediate to fixed register */
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "roll $8, %0"
        : "=c"(ecx_var)
        :: "cc"
    );
    
    /* Use all register-bound variables to prevent dead code elimination */
    asm volatile ("" :: "b"(ebx_var), "a"(eax_var), "c"(ecx_var));
}

/* Test 2: Complex addressing modes with restrictive constraints */
void test_complex_addressing(void) {
    int array[100] = {0};
    volatile int* volatile_ptr = &global_volatile_int;
    int result;
    
    /* Force memory operand with displacement that might need secondary reload */
    asm volatile (
        "movl 16(%1), %0\n\t"
        "imull %2, %0"
        : "=r"(result)
        : "r"(array), "rm"(global_int)
        : "cc", "memory"
    );
    
    /* Multiple alternative constraints */
    int temp = result;
    asm volatile (
        "addl %1, %0"
        : "+r,m"(temp)
        : "r,m,i"(global_volatile_int)
        : "cc"
    );
    
    /* Pointer chasing that might generate complex addressing */
    asm volatile (
        "movl (%1), %0\n\t"
        "addl (%2), %0"
        : "=&r"(result)
        : "r"(volatile_ptr), "r"(&global_int)
        : "memory"
    );
    
    global_int = result + temp;
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield accesses generate SUBREG */
    struct bitfield_struct local_bf;
    local_bf.low_bits = global_volatile_short & 0xFF;
    local_bf.high_bits = (global_volatile_short >> 8) & 0xFF;
    
    /* Force partial register access */
    int32_t full_word = 0xDEADBEEF;
    int16_t half_word;
    
    /* This should generate a SUBREG */
    asm volatile (
        "movw %w1, %0"
        : "=r"(half_word)
        : "r"(full_word)
    );
    
    /* STRICT_LOW_PART pattern via inline assembly */
    int32_t target;
    asm volatile (
        "movl %1, %0\n\t"
        "andl $0xFFFF, %0"
        : "=r"(target)
        : "rm"(full_word)
        : "cc"
    );
    
    /* Mix with volatile to prevent optimization */
    global_bitfield.low_bits = half_word & 0xFF;
    global_bitfield.high_bits = (target >> 8) & 0xFF;
}

/* Test 4: Secondary reloads via immediate constraints */
void test_immediate_reloads(void) {
    register int esi_var asm("esi");
    register int edi_var asm("edi");
    int output;
    
    /* Large immediate might need secondary reload on some arches */
    asm volatile (
        "movl $0x12345678, %1\n\t"
        "leal 100(%1, %2, 4), %0"
        : "=r"(output), "=&r"(esi_var)
        : "r"(global_int)
        : "cc"
    );
    
    /* Immediate to memory with register pressure */
    asm volatile (
        "movl %1, (%0)\n\t"
        "lock addl $1, (%0)"
        :: "r"(&global_volatile_int), "i"(42)
        : "memory", "cc"
    );
    
    /* Multiple constraints forcing reload decisions */
    asm volatile (
        "cpuid"
        : "=a"(esi_var), "=b"(edi_var)
        : "a"(0)
        : "ecx", "edx", "cc"
    );
    
    global_int = output + esi_var + edi_var;
}

/* Test 5: Register class restrictions with target attributes */
#ifdef __x86_64__
/* Force use of specific register classes */
__attribute__((target("arch=core2")))
void test_register_classes(void) {
    uint64_t rax_var, rbx_var;
    
    /* Force use of legacy registers that might need secondary reloads */
    asm volatile (
        "movq %%rax, %0\n\t"
        "movq %%rbx, %1"
        : "=r"(rax_var), "=r"(rbx_var)
        :: "memory"
    );
    
    /* Complex 64-bit operation with mixed constraints */
    uint64_t large_imm = 0xFFFFFFFF00000000ULL;
    asm volatile (
        "addq %1, %0"
        : "+r"(rax_var)
        : "rmi"(large_imm)
        : "cc"
    );
    
    /* Use results */
    asm volatile ("" :: "a"(rax_var), "b"(rbx_var));
}
#endif

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test_fixed_registers();
        test_complex_addressing();
        test_subreg_patterns();
        test_immediate_reloads();
        
        #ifdef __x86_64__
        test_register_classes();
        #endif
        
        /* Mix in some arithmetic to create value flow */
        result += global_int + global_volatile_int;
        global_volatile_int ^= result;
        
        /* Memory barrier to force reloads */
        asm volatile ("" ::: "memory");
    }
    
    printf("Final result: %d\n", result);
    
    /* Ensure all variables are used */
    asm volatile ("" 
        : 
        : "m"(global_bitfield), "m"(global_volatile_short)
        : "memory"
    );
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
