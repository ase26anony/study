/* reload_stress.c - Stress test for GCC's reload pass */
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int g_volatile = 42;
int g_array[100] = {0};
int g_global = 100;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} g_bitfield = {0};

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers(void) {
    /* Bind variables to specific registers */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    volatile int v = g_volatile;
    
    /* Multiple inline asm with conflicting constraints */
    asm volatile (
        /* Force reload from memory to fixed register */
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %0"
        : "+r"(r1)
        : "m"(v)
        : "cc"
    );
    
    /* Complex constraint with alternatives */
    asm volatile (
        "imull %1, %0"
        : "+r,a"(r2)
        : "rm,i"(g_global)
        : "cc"
    );
    
    /* Multiple output constraints */
    asm volatile (
        "movl %2, %0\n\t"
        "leal (%0,%1,1), %0"
        : "=&a,r"(r1), "=r,m"(r3)
        : "mr,i"(g_array[10]), "1"(r2)
        : "cc"
    );
}

/* Test 2: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    int32_t full_reg = g_global;
    volatile int16_t half_volatile;
    
    /* Generate SUBREG for truncation */
    int16_t truncated = (int16_t)full_reg;
    g_bitfield.low16 = truncated;
    
    /* Access volatile bitfield - forces conservative codegen */
    g_bitfield.volatile_field = truncated & 0xFF;
    
    /* Inline asm with byte register constraints (x86 specific) */
    uint8_t byte_val;
    asm volatile (
        "movb %1, %%al\n\t"
        "addb %%al, %0"
        : "+r,m"(byte_val)
        : "rm,i"(truncated)
        : "al", "cc"
    );
    
    half_volatile = byte_val;
    
    /* Mixed size operations */
    asm volatile (
        "movzwl %1, %0"
        : "=r"(full_reg)
        : "rm"(half_volatile)
    );
}

/* Test 3: Complex addressing modes with multiple alternatives */
void test_complex_addressing(void) {
    register int idx asm("ecx") = 50;
    int result;
    volatile int* volatile_ptr = &g_volatile;
    
    /* Memory operand with index register */
    asm volatile (
        "movl (%1,%2,4), %0"
        : "=r,a"(result)
        : "b"(g_array), "r,c"(idx)
        : "memory"
    );
    
    /* Multiple memory constraints with displacement */
    asm volatile (
        "addl %1, %0"
        : "+r,m"(result)
        : "mi,rm"(g_array[20])
        : "cc"
    );
    
    /* Volatile pointer dereference in asm */
    asm volatile (
        "movl (%1), %0\n\t"
        "orl %0, %0"
        : "=r,r"(result)
        : "r,m"(volatile_ptr)
        : "memory", "cc"
    );
}

/* Test 4: STRICT_LOW_PART and register pressure */
void test_strict_low_part(void) {
    register uint32_t r asm("eax") = 0x12345678;
    register uint16_t low asm("bx");
    volatile uint32_t mask = 0x0000FFFF;
    
    /* Operations that might generate STRICT_LOW_PART */
    asm volatile (
        "andl %1, %0"
        : "+r,r"(r)
        : "rm,i"(mask)
        : "cc"
    );
    
    /* Extract low part with constraint */
    asm volatile (
        "movw %%ax, %0"
        : "=r,r,m"(low)
        : "0,a"(r)
        : "cc"
    );
    
    /* Mix with memory operations */
    g_array[0] = low;
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw %%ax, %0"
        : "+r,m"(g_array[1])
        : "rm,i"(low)
        : "ax", "cc", "memory"
    );
}

/* Test 5: Secondary reloads via restrictive classes */
void test_secondary_reloads(void) {
    double d1 = 3.14, d2 = 2.71;
    volatile double vd = 1.0;
    register double rd asm("xmm0");
    
    /* x87 FPU constraints (if targeting x87) */
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0"
        : "=m,rm"(rd)
        : "m,rm"(vd)
        : "st", "st(1)", "memory"
    );
    
    /* SSE register constraints */
    asm volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0"
        : "=x,xm,xr"(rd)
        : "xm,xr,m"(d1), "xm,xr,m"(d2)
        : "xmm1"
    );
    
    /* Mixed integer/float */
    int int_val;
    asm volatile (
        "cvtsd2si %1, %0"
        : "=r,r,m"(int_val)
        : "x,xm,xr"(rd)
    );
}

/* Test 6: Create register pressure to force spills/reloads */
void test_register_pressure(void) {
    /* Many register variables to create pressure */
    register int r0 asm("eax");
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    register int r4 asm("esi");
    register int r5 asm("edi");
    
    volatile int v = g_volatile;
    
    /* Chain of operations forcing register moves */
    r0 = v;
    asm volatile ("" : "+r"(r0) : : "cc");
    
    r1 = r0 + 1;
    asm volatile ("addl %1, %0" : "+r"(r1) : "rm,i"(g_array[1]) : "cc");
    
    r2 = r1 * 2;
    asm volatile ("imull %1, %0" : "+r,r"(r2) : "rm,i"(r0) : "cc");
    
    r3 = r2 - r1;
    asm volatile ("subl %1, %0" : "+r,m"(r3) : "rm,i"(v) : "cc");
    
    r4 = r3 & 0xFF;
    asm volatile ("andl %1, %0" : "+r,r"(r4) : "rm,i"(0xFF) : "cc");
    
    r5 = r4 | r2;
    asm volatile ("orl %1, %0" : "+r,m"(r5) : "rm,i"(r3) : "cc");
    
    /* Force all to memory */
    g_array[2] = r0;
    g_array[3] = r1;
    g_array[4] = r2;
    g_array[5] = r3;
    g_array[6] = r4;
    g_array[7] = r5;
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    volatile int check = 0;
    
    /* Run each test multiple times to increase reload opportunities */
    for (int i = 0; i < 10; i++) {
        test_fixed_registers();
        check += g_volatile;
        
        test_subreg_patterns();
        check += g_bitfield.low16;
        
        test_complex_addressing();
        check += g_array[i % 10];
        
        test_strict_low_part();
        check += g_array[0];
        
        test_secondary_reloads();
        check += (int)g_global;
        
        test_register_pressure();
        check += g_array[2];
        
        /* Prevent loop elimination */
        asm volatile ("" : : "r"(check) : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    result = check + g_array[0] + g_global + (int)g_bitfield.volatile_field;
    
    /* Final barrier */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result % 256; /* Return non-zero to indicate execution */
}
