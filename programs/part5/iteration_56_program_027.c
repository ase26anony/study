/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int volatile_counter;

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Large immediate constants that aren't directly encodable */
    const long long expensive_const1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long expensive_const2 = 0x5555555555555555LL;
    const unsigned long large_shift = 48;
    
    /* Invariant pointers used in loop */
    int* invariant_ptr1 = &global_array[0];
    int* invariant_ptr2 = &global_array[128];
    double* invariant_ptr3 = &global_doubles[0];
    
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = 0;
    register int r1 asm("ebx") = 0;
    register int r2 asm("ecx") = 0;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Complex loop with invariant usage */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        v1 = invariant_ptr1[i & 0x7F];
        v2 = invariant_ptr2[i & 0x7F];
        v3 = (int)((uintptr_t)invariant_ptr3 & expensive_const1);
        
        /* Expensive constant usage in non-adjacent operations */
        v4 = (int)(expensive_const1 >> (i % 32));
        v5 = (int)(expensive_const2 << (i % 32));
        
        /* Address calculations using invariants */
        v6 = *(int*)((char*)data + (uintptr_t)invariant_ptr1);
        v7 = *(int*)((char*)data + (uintptr_t)invariant_ptr2);
        
        /* More overlapping live ranges */
        v8 = v1 + v2;
        v9 = v3 * v4;
        v10 = v5 / (v6 + 1);
        v11 = v7 ^ v8;
        v12 = v9 | v10;
        v13 = v11 & v12;
        v14 = v13 << (v1 & 0x1F);
        v15 = v14 >> (v2 & 0x1F);
        
        /* Use register variables */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2));
        r0 += v1; r1 += v2; r2 += v3;
        
        v16 = r0 + r1;
        v17 = r2 * v4;
        v18 = v16 ^ v17;
        v19 = v18 | v15;
        v20 = v19 & 0xFFFF;
        
        /* Force spill/reload pressure */
        global_array[i & 0xFF] = v20;
    }
    
    /* Return value using all computed values */
    return r0 + r1 + r2 + v20;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    int result1, result2, result3, result4;
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    /* Multi-output inline assembly forcing hard register references */
    asm volatile (
        "movl %5, %%eax\n\t"
        "movl %6, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl $0x12345678, %%eax\n\t"  /* Large immediate */
        "movl %%eax, %0\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %1\n\t"
        "shrl $8, %%ecx\n\t"
        "movl %%ecx, %2\n\t"
        "movl $0x9ABCDEF0, %%edx\n\t"  /* Another large immediate */
        "xorl %%edx, %%ecx\n\t"
        "movl %%ecx, %3\n\t"
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4)
        : "m"(x), "m"(y)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More register pressure with overlapping lives */
    temp1 = result1 * 0x11111111;
    temp2 = result2 * 0x22222222;
    temp3 = result3 * 0x33333333;
    temp4 = result4 * 0x44444444;
    
    /* Complex dependency chain */
    temp5 = (temp1 + temp2) * (temp3 - temp4);
    temp6 = (temp1 - temp2) * (temp3 + temp4);
    
    /* Use all temporaries in final computation */
    for (int i = 0; i < 8; i++) {
        temp5 = (temp5 << 3) | (temp6 >> 29);
        temp6 = (temp6 << 3) | (temp5 >> 29);
        volatile_counter = i;  /* Prevent optimization */
    }
    
    return temp5 + temp6;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int function_c(int selector, int iterations) {
    /* Labels for computed goto */
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, 
                            &&label4, &&label5, &&label6, &&label7 };
    
    /* Many local variables with register keyword */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    register int reg_d asm("edx");
    
    int array[32];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        array[i] = i * 0x1234567;
    }
    
    /* Complex control flow */
    for (int i = 0; i < iterations; i++) {
        int idx = (selector + i) & 0x7;
        
        /* Computed goto creates unpredictable control flow */
        goto *labels[idx];
        
    label0:
        reg_a = array[0] + array[1];
        reg_b = array[2] * array[3];
        reg_c = reg_a ^ reg_b;
        reg_d = reg_c << 4;
        sum += reg_d;
        continue;
        
    label1:
        reg_a = array[4] - array[5];
        reg_b = array[6] | array[7];
        reg_c = reg_a & reg_b;
        reg_d = reg_c >> 2;
        sum += reg_d;
        continue;
        
    label2:
        reg_a = array[8] ^ array[9];
        reg_b = array[10] + array[11];
        reg_c = reg_a * reg_b;
        reg_d = reg_c & 0xFF;
        sum += reg_d;
        continue;
        
    label3:
        reg_a = array[12] | array[13];
        reg_b = array[14] - array[15];
        reg_c = reg_a / (reg_b + 1);
        reg_d = reg_c | 0x80;
        sum += reg_d;
        continue;
        
    label4:
        reg_a = array[16] & array[17];
        reg_b = array[18] ^ array[19];
        reg_c = reg_a + reg_b;
        reg_d = reg_c * 3;
        sum += reg_d;
        continue;
        
    label5:
        reg_a = array[20] * array[21];
        reg_b = array[22] & array[23];
        reg_c = reg_a - reg_b;
        reg_d = reg_c / 2;
        sum += reg_d;
        continue;
        
    label6:
        reg_a = array[24] + array[25];
        reg_b = array[26] | array[27];
        reg_c = reg_a ^ reg_b;
        reg_d = reg_c << 1;
        sum += reg_d;
        continue;
        
    label7:
        reg_a = array[28] - array[29];
        reg_b = array[30] & array[31];
        reg_c = reg_a * reg_b;
        reg_d = reg_c >> 3;
        sum += reg_d;
        continue;
    }
    
    return sum;
}

/* Function D: Builtin usage for specific hard register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t function_d(int iterations) {
    uint64_t total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Use rdtsc which returns in edx:eax */
        uint64_t tsc1 = __builtin_ia32_rdtsc();
        
        /* Create dependency chain from hard registers */
        uint32_t low = (uint32_t)tsc1;
        uint32_t high = (uint32_t)(tsc1 >> 32);
        
        /* Force register pressure with many operations */
        uint32_t a = low * 0x12345678;
        uint32_t b = high * 0x87654321;
        uint32_t c = a ^ b;
        uint32_t d = c << 16;
        uint32_t e = d | (low & 0xFFFF);
        uint32_t f = e ^ (high >> 16);
        uint32_t g = f * 0x13579BDF;
        uint32_t h = g + 0xFEDCBA98;
        
        /* Another rdtsc to create more hard register references */
        uint64_t tsc2 = __builtin_ia32_rdtsc();
        
        /* Mix with previous results */
        uint32_t i1 = (uint32_t)tsc2;
        uint32_t j = i1 * h;
        uint32_t k = (uint32_t)(tsc2 >> 32) ^ j;
        
        total += ((uint64_t)k << 32) | j;
        
        /* Prevent optimization */
        global_array[i & 0xFF] = k;
    }
    
    return total;
}
#endif

/* Main function to drive all test cases */
int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    int result = 0;
    
    /* Call all test functions to trigger different patterns */
    result += function_a(iterations, global_array);
    result += function_b(iterations, iterations * 2);
    result += function_c(iterations % 8, iterations);
    
    #ifdef __i386__
    result += (int)function_d(iterations / 10);
    #endif
    
    /* Use result to prevent dead code elimination */
    return result & 0xFF;
}
