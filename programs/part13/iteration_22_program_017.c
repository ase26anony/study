/* test_reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Complex function to force temporary evaluation */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Global arrays for memory operand testing */
int global_array[1024];
float float_array[1024];
double double_array[1024];

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        int input = i * 3 + 7;
        int output1, output2, output3;
        
        /* Force reload by requiring specific hard register for output */
        /* while input is complex expression */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output1)          /* Must be in eax */
            : "mr"(input + global_array[compute_index(i) % 1024])
            : /* No clobbers */
        );
        
        /* Early-clobber constraint forcing reload */
        int temp = input * 2;
        asm volatile (
            "addl %2, %0\n\t"
            "movl %0, %1\n\t"
            : "=&r"(output2), "=r"(output3)  /* & = early clobber */
            : "r"(temp), "0"(output1)
            : "cc"
        );
        
        /* Mixed size operands requiring mode changes */
        char char_val = (char)(input & 0xFF);
        long long ll_output;
        asm volatile (
            "movsbl %1, %k0\n\t"     /* Sign extend byte to dword in low 32 bits */
            "salq $32, %0\n\t"       /* Shift to high 32 bits */
            : "=r"(ll_output)
            : "r"(char_val)
            : "cc"
        );
        
        checksum += output1 + output2 + output3 + (int)(ll_output >> 32);
    }
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        /* Builtin with function call as argument - forces temporary */
        int popcnt = __builtin_popcount(compute_index(i));
        
        /* Builtin with memory access and computation */
        int ctz = __builtin_ctz(global_array[compute_index(i) % 1024] | 1);
        
        /* Atomic builtin with complex address */
        int index = compute_index(i) % 1024;
        int old_val = __atomic_fetch_add(&global_array[index], popcnt, __ATOMIC_RELAXED);
        
        /* Math builtin with float computation */
        float fval = float_array[index] * 2.0f + 1.0f;
        float sqrt_val = __builtin_sqrtf(fval);
        
        checksum += popcnt + ctz + old_val + (int)sqrt_val;
    }
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(int iterations) {
    int i;
    
    /* Declare register variables (GCC extension) */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    for (i = 0; i < iterations; i++) {
        r1 = i * 2;
        r2 = i * 3;
        r3 = i * 5;
        
        /* Force conflict: use register variable in asm requiring different reg */
        int result;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "subl %3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(result)
            : "r"(r1), "r"(r2), "r"(r3)
            : "eax", "cc"
        );
        
        /* Try to take address (will generate warning but test reload) */
        int *ptr;
        asm volatile (
            "leal %1, %0\n\t"
            : "=r"(ptr)
            : "r"(r1)
        );
        
        checksum += result + (int)(intptr_t)ptr;
    }
}

/* ===== Test 4: Architecture-Specific Secondary Reload Tests ===== */
void test_secondary_reload_trigger(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Test designed to trigger secondary reloads on various architectures */
        
        #if defined(__arm__) || defined(__aarch64__)
        /* ARM: Access to system registers often needs secondary reload */
        uint32_t control_reg;
        asm volatile (
            "mrs %0, cpsr\n\t"
            : "=r"(control_reg)
        );
        
        /* NEON to ARM register transfer might need secondary reload */
        float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
        float arm_float;
        asm volatile (
            "vmov.f32 %0, %1[0]\n\t"
            : "=r"(arm_float)
            : "w"(neon_vec)
        );
        
        checksum += control_reg + (int)arm_float;
        
        #elif defined(__x86_64__) || defined(__i386__)
        /* x86: Control register access needs secondary reload */
        uint32_t cr0_val;
        asm volatile (
            "mov %%cr0, %0\n\t"
            : "=r"(cr0_val)
        );
        
        /* x87 FPU stack manipulation */
        double x = 3.14159 * i;
        double y;
        asm volatile (
            "fldl %1\n\t"
            "fsqrt\n\t"
            "fstpl %0\n\t"
            : "=m"(y)
            : "m"(x)
        );
        
        /* MMX/SSE register constraints */
        __m128i vec = _mm_set_epi32(i, i+1, i+2, i+3);
        int elem;
        asm volatile (
            "pextrd $0, %1, %0\n\t"
            : "=r"(elem)
            : "x"(vec)
        );
        
        checksum += cr0_val + (int)y + elem;
        
        #elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
        /* PowerPC: SPR access needs secondary reload */
        uint32_t spr;
        asm volatile (
            "mfspr %0, 0x10\n\t"  /* VRSAVE on some PPC */
            : "=r"(spr)
        );
        checksum += spr;
        
        #else
        /* Generic: Try to force memory operand reload with register pressure */
        double d1 = double_array[i % 1024];
        double d2 = double_array[(i + 1) % 1024];
        double d3;
        
        /* Create register pressure */
        int r1 = i, r2 = i*2, r3 = i*3, r4 = i*4, r5 = i*5;
        int r6 = i*6, r7 = i*7, r8 = i*8, r9 = i*9, r10 = i*10;
        
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "faddp\n\t"
            "fstpl %0\n\t"
            : "=m"(d3)
            : "m"(d1), "m"(d2)
        );
        
        checksum += (int)d3 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
        #endif
    }
}

/* ===== Test 5: Mixed Mode and Complex Addressing ===== */
void test_mixed_mode_addressing(int iterations) {
    struct complex_addr {
        int data[16];
        int offset;
    } ca;
    
    int i;
    for (i = 0; i < iterations; i++) {
        ca.offset = i % 8;
        
        /* Complex addressing mode that might not be directly supported */
        int value;
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"
            : "=r"(value)
            : "r"(&ca.data), "r"(ca.offset)
        );
        
        /* Different sized accesses to same location */
        short short_val;
        asm volatile (
            "movw (%1, %2, 2), %w0\n\t"
            : "=r"(short_val)
            : "r"(&ca.data), "r"(ca.offset * 2)
        );
        
        /* Force reload of address computation */
        long long ll_addr = (long long)&ca.data[ca.offset];
        int indirect;
        asm volatile (
            "movl (%1), %0\n\t"
            : "=r"(indirect)
            : "r"(ll_addr)
        );
        
        checksum += value + short_val + indirect;
    }
}

/* ===== Main Test Driver ===== */
int main(int argc, char **argv) {
    int iterations = 1000;
    int i;
    
    /* Initialize global arrays */
    for (i = 0; i < 1024; i++) {
        global_array[i] = compute_index(i);
        float_array[i] = (float)compute_index(i) / 1000.0f;
        double_array[i] = (double)compute_index(i) / 1000.0;
    }
    
    printf("Starting reload coverage tests...\n");
    
    /* Run all tests multiple times to increase coverage probability */
    for (i = 0; i < 10; i++) {
        test_asm_constraint_conflict(iterations / 10);
        test_builtin_complex_operand(iterations / 10);
        test_register_variable_abuse(iterations / 10);
        test_secondary_reload_trigger(iterations / 10);
        test_mixed_mode_addressing(iterations / 10);
        
        global_counter++;
    }
    
    printf("Tests completed. Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}
