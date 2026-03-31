/* test_early_remat.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0x0FEDCBA987654321,
    0xDEADBEEFCAFEBABE, 0xBAADF00D12345678
};

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    const long* invariant_ptr = large_constants;
    const int invariant_limit = 1000;
    
    /* Large immediate that needs rematerialization */
    const long expensive_const = 0x7FFFFFFFFFFFFFFF;
    
    /* Complex loop with many live values */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        v0 = (invariant_ptr[0] & expensive_const) ? 1 : 0;
        v1 = (invariant_ptr[1] | expensive_const) ? 2 : 0;
        v2 = (invariant_ptr[2] ^ expensive_const) ? 3 : 0;
        
        /* Address calculation using invariants */
        v3 = data[(i + invariant_limit) % 256];
        v4 = data[(i * invariant_limit) % 256];
        
        /* Overlapping live ranges */
        v5 = v0 + v1 + v2 + v3 + v4;
        v6 = v5 * r0;
        v7 = v6 / (r1 + 1);
        v8 = v7 ^ r2;
        v9 = v8 << 2;
        
        /* More operations to extend live ranges */
        v10 = v9 + i;
        v11 = v10 - invariant_limit;
        v12 = v11 * (int)(expensive_const & 0xFFFFFFFF);
        v13 = v12 | (int)(expensive_const >> 32);
        v14 = v13 ^ v0;
        v15 = v14 + v1;
        v16 = v15 - v2;
        v17 = v16 * v3;
        v18 = v17 / v4;
        v19 = v18 % (r0 + 1);
        
        /* Use all variables to keep them live */
        r0 = (r0 + v19) & 0xFF;
        r1 = (r1 ^ v18) & 0xFF;
        r2 = (r2 | v17) & 0xFF;
        
        /* Force spill/reload pressure */
        data[i % 256] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                       v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    }
    
    return r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    register int reg_var1 asm("esi") = a;
    register int reg_var2 asm("edi") = b;
    register int reg_var3 asm("ebp") = c;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "movl %3, %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result1), "=&r" (result2), "=&r" (result3)
        : "r" (reg_var1), "r" (reg_var2), "r" (reg_var3)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More assembly with different clobbers */
    asm volatile (
        "rdtsc\n\t"
        "movl %%eax, %0\n\t"
        "movl %%edx, %1\n\t"
        : "=r" (result2), "=r" (result3)
        :
        : "eax", "edx", "cc"
    );
    
    /* Use results in complex expressions */
    int x = result1 * 0x12345678;
    int y = result2 / 0x9ABCDEF;
    int z = result3 % 0x13579BDF;
    
    /* Chain of operations keeping values live */
    for (int i = 0; i < 100; i++) {
        x = (x << 3) | (y >> 5);
        y = (y ^ z) + (reg_var1 * i);
        z = (z & x) - (reg_var2 / (i + 1));
        reg_var3 = (reg_var3 + x + y + z) & 0xFFFF;
    }
    
    return x + y + z + reg_var1 + reg_var2 + reg_var3;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int* data) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    register int counter asm("ebx") = seed;
    int temp[20];
    
    /* Initialize many temporaries */
    for (int i = 0; i < 20; i++) {
        temp[i] = data[i] * 0x98765432 + i;
    }
    
    int result = 0;
    int state = seed % 5;
    
    /* Computed goto creating complex control flow */
    goto *labels[state];
    
label0:
    for (int i = 0; i < 100; i++) {
        switch (i % 4) {
            case 0:
                temp[0] = temp[1] + temp[2] * counter;
                temp[3] = temp[4] ^ temp[5];
                break;
            case 1:
                temp[6] = temp[7] | temp[8];
                temp[9] = temp[10] - temp[11];
                counter += temp[6];
                break;
            case 2:
                temp[12] = temp[13] & temp[14];
                temp[15] = temp[16] << 2;
                counter ^= temp[12];
                break;
            case 3:
                temp[17] = temp[18] >> 1;
                temp[19] = temp[0] % (counter + 1);
                counter = (counter * temp[17]) & 0xFF;
                break;
        }
        result += temp[i % 20];
    }
    goto end;

label1:
    /* Different path with overlapping live ranges */
    for (int i = 0; i < 50; i++) {
        int t0 = temp[0] + large_constants[0];
        int t1 = temp[1] - large_constants[1];
        int t2 = temp[2] * (int)large_constants[2];
        int t3 = temp[3] / (int)large_constants[3];
        
        temp[4] = t0 ^ t1;
        temp[5] = t2 | t3;
        temp[6] = (t0 + t1) * (t2 - t3);
        counter = (counter + temp[4] + temp[5] + temp[6]) % 1000;
    }
    result = counter;
    goto end;

label2:
label3:
label4:
    /* More complex paths */
    for (int i = 0; i < 75; i++) {
        for (int j = 0; j < 10; j++) {
            temp[j] = (temp[j] * 0x1234567 + i * j) & 0xFFFF;
            temp[j + 10] = (temp[j + 10] ^ 0x89ABCDEF) + counter;
        }
        counter = (counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    result = temp[0] + temp[10];

end:
    return result + counter;
}

/* Main function that calls all test patterns */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    int result = 0;
    
    /* Call function A with loop invariants */
    result += func_loop_invariants(1000, global_array);
    
    /* Call function B with inline assembly */
    result += func_asm_clobber(0x11111111, 0x22222222, 0x33333333);
    
    /* Call function C with complex control flow */
    result += func_complex_control(argc > 1 ? atoi(argv[1]) : 42, global_array);
    
    /* Use result to prevent dead code elimination */
    return result & 0xFF;
}
