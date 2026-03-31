/* test-early-remat.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdio.h>

/* Global data for address calculations - creates non-encodable immediates */
static int global_array[256] = {0};
static long global_matrix[16][16] = {0};
static volatile int global_volatile = 42;

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
unsigned long func_loop_invariants(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = global_volatile;
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    unsigned long sum = 0;
    
    /* Large immediate constants that need rematerialization */
    const long LARGE_CONST_1 = 0x7FFFFFFF;
    const long LARGE_CONST_2 = 0x80000000;
    const long LARGE_CONST_3 = 0x12345678;
    const long LARGE_CONST_4 = 0x9ABCDEF0;
    
    /* Loop invariants used in multiple places */
    int* invariant_ptr = &global_array[128];
    long* matrix_ptr = &global_matrix[8][8];
    int invariant_offset = 64;
    
    /* Complex loop with many live values */
    for (a = 0; a < iterations; a++) {
        /* Use invariants in address calculations */
        b = invariant_ptr[a % 128];
        c = matrix_ptr[(a * 3) % 16];
        
        /* Use large constants in non-adjacent operations */
        d = (a * LARGE_CONST_1) >> 3;
        e = (b * LARGE_CONST_2) >> 4;
        f = (c * LARGE_CONST_3) >> 5;
        g = (d * LARGE_CONST_4) >> 6;
        
        /* More operations creating register pressure */
        h = (e + f) * invariant_offset;
        i = (g - h) / (invariant_offset + 1);
        j = (h * i) % (LARGE_CONST_1 & 0xFFFF);
        k = (i + j) | (LARGE_CONST_2 & 0xFFFF);
        l = (j * k) ^ (LARGE_CONST_3 & 0xFFFF);
        m = (k - l) & (LARGE_CONST_4 & 0xFFFF);
        n = (l + m) * (a + 1);
        o = (m - n) / (a + 2);
        p = (n * o) % 256;
        
        /* Use register variables */
        asm volatile("" : "+r"(r0), "+r"(r1) : : "memory");
        r0 = (r0 + p) & 0xFF;
        r1 = (r1 + o) & 0xFF;
        
        /* Complex condition with invariants */
        if ((a % invariant_offset) == 0) {
            sum += b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        } else {
            sum += r0 + r1 + (long)invariant_ptr;
        }
        
        /* Force spill/reload by using all variables again */
        data[a % 256] = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + r0 + r1;
    }
    
    return sum + (long)invariant_ptr + (long)matrix_ptr;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
long func_asm_clobber(int x, int y, int z) {
    long result1, result2, result3;
    register long r10 asm("r10") = x;
    register long r11 asm("r11") = y;
    register long r12 asm("r12") = z;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "movl %[z], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2), 
          [out3] "=&r" (result3)
        : [x] "r" (r10), [y] "r" (r11), [z] "r" (r12)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More inline asm with different clobbers */
    asm volatile (
        "cpuid"
        : "=a"(result1), "=b"(result2), "=c"(result3), "=d"(result1)
        : "a"(0)
        : "memory"
    );
    
    /* Chain hard register references */
    register long rax asm("rax") = result1;
    register long rbx asm("rbx") = result2;
    register long rcx asm("rcx") = result3;
    
    asm volatile("" : "+r"(rax), "+r"(rbx), "+r"(rcx));
    
    return rax + rbx + rcx + r10 + r11 + r12;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int* output) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    register int counter asm("esi") = seed;
    register int temp1 asm("edi") = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int total = 0;
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Use many temporaries with overlapping lives */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            f = g + h;
            g = h + i;
            h = i + j;
            i = j + k;
            j = k + l;
            k = l + m;
            l = m + n;
            m = n + o;
            n = o + p;
            o = p + a;
            p = a + b;
            
            /* Switch with computed goto */
            int idx = (counter++) % 5;
            goto *labels[idx];
            
        label0:
            temp1 = a * b;
            output[0] += temp1;
            continue;
            
        label1:
            temp1 = c * d;
            output[1] += temp1;
            continue;
            
        label2:
            temp1 = e * f;
            output[2] += temp1;
            continue;
            
        label3:
            temp1 = g * h;
            output[3] += temp1;
            continue;
            
        label4:
            temp1 = i * j;
            output[4] += temp1;
            continue;
        }
        
        /* Force all variables live across loop iteration */
        total += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + temp1;
        
        /* Use builtins that return in specific registers */
        unsigned long long tsc = __builtin_ia32_rdtsc();
        total += (tsc >> 32) + (tsc & 0xFFFFFFFF);
    }
    
    return total + counter;
}

/* Main function that combines everything */
int main(int argc, char** argv) {
    int data_buffer[256] = {0};
    int output_buffer[5] = {0};
    unsigned long total = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            global_matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Call functions with different patterns */
    total += func_loop_invariants(1000, data_buffer);
    total += func_asm_clobber(argc, argc * 2, argc * 3);
    total += func_complex_control(argc, output_buffer);
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 256; i++) {
        total += data_buffer[i];
    }
    
    for (int i = 0; i < 5; i++) {
        total += output_buffer[i];
    }
    
    /* Return result to ensure code isn't dead */
    return (int)(total % 0x7FFFFFFF);
}
