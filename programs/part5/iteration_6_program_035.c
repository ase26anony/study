/* test_caller_save.c - Target GCC's caller-save pass insertion logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to force calls */
static __attribute__((noinline)) int external_func(int x) {
    return x ^ 0x1234;
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

/* Volatile globals to prevent optimizations */
static volatile int volatile_counter = 0;
static volatile v4si volatile_vec = {0};

/* Main test function with caller-save pressure */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Declare many variables in call-clobbered registers */
    register int r1 asm("rax") = seed + 1;
    register int r2 asm("rbx") = seed + 2;
    register int r3 asm("rcx") = seed + 3;
    register double f1 asm("xmm0") = seed * 1.5;
    register double f2 asm("xmm1") = seed * 2.5;
    v4si vec1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si vec2 = {seed + 4, seed + 5, seed + 6, seed + 7};
    mmx_t mmx1 = {seed * 2};
    mmx_t mmx2 = {seed * 3};
    
    /* Volatile locals to extend liveness */
    volatile int v1 = r1;
    volatile double vf1 = f1;
    volatile v4si vvec1 = vec1;
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ====== BLOCK 1: Integer register pressure ====== */
        /* Compute with integer registers */
        r1 = r1 * 1103515245 + 12345;
        r2 = r2 * 1103515245 + 54321;
        r3 = r3 * 1103515245 + 67890;
        
        /* Clobber integer registers - simulates function call */
        asm volatile(
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0xDEADBEEF, %%eax\n\t"
            "mov $0xCAFEBABE, %%ebx\n\t"
            "mov $0xBAADF00D, %%ecx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Use original values after clobber - forces save/restore */
        sum += r1 ^ r2 ^ r3;
        v1 = r1;  /* Extend liveness */
        
        /* ====== BLOCK 2: SSE register pressure ====== */
        /* Compute with SSE registers */
        f1 = f1 * 1.6180339887 + i;
        f2 = f2 * 2.7182818284 - i;
        
        /* Clobber SSE registers */
        asm volatile(
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            :
            :
            : "xmm0", "xmm1", "memory"
        );
        
        /* Use original values */
        sum += (int)(f1 + f2);
        vf1 = f1;
        
        /* ====== BLOCK 3: Vector register pressure ====== */
        /* Vector computations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * 2;
        
        /* Clobber xmm2-xmm5 (call-clobbered on x86-64 SysV) */
        asm volatile(
            "# CLOBBER VECTOR REGS\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            :
            :
            : "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vectors after clobber */
        sum += vec1[0] + vec1[1] + vec1[2] + vec1[3];
        vvec1 = vec1;
        
        /* ====== BLOCK 4: MMX register pressure ====== */
        /* MMX computations */
        mmx1 = mmx1 + mmx2;
        mmx2 = mmx2 * 3;
        
        /* Clobber MMX registers */
        asm volatile(
            "# CLOBBER MMX REGS\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            :
            :
            : "mm0", "mm1", "memory"
        );
        
        /* Use MMX values */
        sum += (int)mmx1[0];
        
        /* ====== CRITICAL: Function call at end of basic block ====== */
        /* Make a real function call that will end a basic block */
        int call_result = external_func(sum);
        
        /* Conditional jump immediately after call to create block boundary */
        if (call_result & 1) {
            /* This creates a basic block ending with the call above */
            sum += 1000;
        } else {
            sum += 2000;
        }
        
        /* Another clobber after the branch to force insertion */
        asm volatile(
            "# FINAL CLOBBER\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            :
            :
            : "r10", "r11", "memory"
        );
        
        /* Use values again */
        sum += r2 + (int)f2;
        
        /* Update volatile to prevent dead code elimination */
        volatile_counter = i;
    }
    
    return sum;
}

/* Second test function with different pattern */
static __attribute__((noinline))
int test_caller_save2(int iterations, int seed) {
    register double d1 asm("xmm6") = seed * 3.14159;
    register double d2 asm("xmm7") = seed * 2.71828;
    int acc = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy FP computation */
        d1 = d1 * d2 + i;
        d2 = d2 * d1 - i;
        
        /* Clobber call-clobbered registers */
        asm volatile(
            "# CLOBBER XMM6-XMM7\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            :
            :
            : "xmm6", "xmm7", "memory"
        );
        
        /* Use after clobber */
        acc += (int)(d1 + d2);
        
        /* Function call that should end a basic block */
        int tmp = external_func(acc);
        
        /* Unconditional jump-like structure */
        switch (tmp & 3) {
            case 0: acc += 1; break;
            case 1: acc += 2; break;
            case 2: acc += 3; break;
            default: acc += 4; break;
        }
        
        /* Another clobber */
        asm volatile(
            "# CLOBBER RAX\n\t"
            "mov $0, %%rax\n\t"
            :
            :
            : "rax", "memory"
        );
        
        acc += (int)d1;
    }
    
    return acc;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int seed = 42;
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Testing caller-save insertion logic...\n");
    
    /* Call test functions multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        int result1 = test_caller_save(iterations, seed + i * 100);
        int result2 = test_caller_save2(iterations / 2, seed + i * 50);
        total += result1 + result2;
        printf("Iteration %d: result1=%d, result2=%d\n", 
               i, result1, result2);
    }
    
    printf("Total checksum: %d\n", total);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return total != 0 ? 0 : 1;
}
