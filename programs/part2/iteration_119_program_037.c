/* reload_stress_test.c
 * Designed to trigger GCC's reload pass initialization of secondary reload fields
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or for more stress: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 12345;
int global_normal = 67890;
static int static_global = 54321;

/* Bitfield structure to generate SUBREG RTL patterns */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global = {0xAAAA, 0x5555, 0x77};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile;
    int output;
    
    /* Force secondary reload: memory -> specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)          /* Output in any register */
        : "m"(input)            /* Input from memory */
        : "%eax", "memory"      /* Clobber eax and memory */
    );
    
    /* Alternative constraints forcing reload decisions */
    int a = output + 1;
    int b = global_normal;
    
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0"
        : "+&r"(a)              /* Early clobber output */
        : "rm"(b), "g"(static_global)  /* Register/memory or general */
        : "cc"
    );
    
    global_normal = a;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int reg1 asm("ebx");
    register int reg2 asm("edi");
    register int reg3 asm("esi");
    
    reg1 = global_volatile;
    reg2 = static_global;
    reg3 = bitfield_global.low16;  /* SUBREG access */
    
    /* Force moves between specific registers */
    asm volatile (
        "xchgl %%ebx, %%edi\n\t"
        "addl %%esi, %%ebx"
        : "+&r"(reg1), "+&r"(reg2)
        : "r"(reg3)
        : "cc"
    );
    
    /* Complex expression requiring multiple reloads */
    int temp = (reg1 << 4) | (reg2 & 0xF);
    
    /* Inline asm with immediate and memory constraints */
    asm volatile (
        "orl $0xFF00, %0\n\t"
        "andl %1, %0"
        : "+r"(temp)
        : "m"(bitfield_global)
        : "cc"
    );
    
    static_global = temp;
}

/* Function to generate STRICT_LOW_PART and SUBREG patterns */
void test_subreg_patterns(void) {
    /* Operations that generate partial register accesses */
    int32_t full_word = global_normal * 2;
    
    /* Explicit truncation - may generate SUBREG */
    int16_t half_word = (int16_t)full_word;
    uint8_t byte_val = (uint8_t)(full_word >> 8);
    
    /* Bitfield operations */
    bitfield_global.low16 = half_word;
    bitfield_global.volatile_field = byte_val;
    
    /* Complex expression with mixed sizes */
    int result = (int)half_word * (int)byte_val;
    
    /* Inline asm that might require secondary reloads for partial regs */
    asm volatile (
        "imull %1, %0\n\t"
        "shrl $4, %0"
        : "+r"(result)
        : "r"((int)bitfield_global.high16)
        : "cc"
    );
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    global_volatile = result;
}

/* Function with loop to create more reload opportunities */
void test_loop_reloads(int iterations) {
    volatile int accum = 0;
    register int counter asm("ecx");
    
    for (counter = 0; counter < iterations; counter++) {
        int temp = global_normal + counter;
        
        /* Mixed constraints in loop body */
        asm volatile (
            "leal (%1, %2, 2), %0\n\t"
            "cmpl $1000, %0\n\t"
            "cmovgl %3, %0"
            : "=&r"(temp)
            : "r"(global_volatile), "r"(counter), "rm"(static_global)
            : "cc"
        );
        
        accum += temp;
        
        /* Access bitfield in loop - generates SUBREG */
        if (counter & 1) {
            bitfield_global.low16 ^= (accum & 0xFFFF);
        }
    }
    
    /* Force spill/reload at end of loop */
    asm volatile ("" : : "r"(accum), "m"(bitfield_global) : "memory");
    
    static_global = accum;
}

/* Function using multiple alternative constraints */
void test_multiple_alternatives(void) {
    int a = global_volatile;
    int b = static_global;
    int c = bitfield_global.low16;
    
    /* Multiple alternative constraints for same operand */
    asm volatile (
        "addl %1, %0\n\t"
        #ifdef __x86_64__
        "movslq %0, %0\n\t"  /* Sign extend for 64-bit */
        #endif
        "andl $0x7FFF, %0"
        : "+r,r,m"(a)        /* Three alternatives for output */
        : "g,r,i"(b), "r,m,0"(c)  /* Multiple alternatives for inputs */
        : "cc"
    );
    
    /* Secondary asm with different register class requirements */
    int d = a;
    asm volatile (
        "bsrl %1, %0"
        : "=c"(d)            /* Fixed ecx output on x86 */
        : "r"(a)
        : "cc"
    );
    
    global_normal = d;
}

/* Main function that orchestrates all tests */
int main(void) {
    int i, total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Run tests multiple times to increase reload opportunities */
    for (i = 0; i < 10; i++) {
        test_restrictive_constraints();
        total += global_normal;
        
        test_register_variables();
        total += static_global;
        
        test_subreg_patterns();
        total += global_volatile;
        
        test_multiple_alternatives();
        total += global_normal;
        
        if (i % 3 == 0) {
            test_loop_reloads(5 + i);
            total += static_global;
        }
        
        /* Create data dependencies between tests */
        global_volatile = (global_volatile * 1103515245 + 12345) & 0x7FFFFFFF;
        static_global ^= global_normal;
    }
    
    /* Final complex expression requiring reloads */
    register int final_result asm("eax");
    final_result = total;
    
    asm volatile (
        "roll $3, %0\n\t"
        "xorl %%ebx, %0"
        : "+a"(final_result)
        : "b"(0xDEADBEEF)    /* Immediate forced into ebx */
        : "cc"
    );
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;  /* Return non-zero to indicate success */
}
