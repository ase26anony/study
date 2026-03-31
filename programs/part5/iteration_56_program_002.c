/* test_early_remat.c - Program to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global arrays for address calculations */
static int global_array[1024];
static double global_double_array[1024];
static char global_char_array[2048];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that are expensive to materialize */
    const long long expensive_const1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long expensive_const2 = 0x8000000000000000LL;
    const unsigned long large_mask = 0xFFFFFFFF00000000UL;
    
    /* Loop invariants that will be used in address calculations */
    int *array_ptr = global_array;
    double *double_ptr = global_double_array;
    char *char_ptr = global_char_array;
    
    int sum = 0;
    int i, j;
    
    /* Complex loop with multiple invariants used in different places */
    for (i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        int idx1 = (i * 7) & 0x3FF;
        int idx2 = (i * 13) & 0x3FF;
        int idx3 = (i * 29) & 0x7FF;
        
        /* Multiple uses of invariants with expensive constants */
        int val1 = array_ptr[idx1] + (int)(expensive_const1 >> 32);
        double val2 = double_ptr[idx2] * (double)(expensive_const2 >> 48);
        char val3 = char_ptr[idx3] ^ (char)(large_mask >> 24);
        
        /* Complex condition using invariants */
        if ((uintptr_t)array_ptr & large_mask) {
            val1 += (int)((uintptr_t)double_ptr >> 16);
        }
        
        if ((uintptr_t)char_ptr & 0xFF000000) {
            val3 += (char)((uintptr_t)array_ptr >> 8);
        }
        
        /* Nested loop to extend live ranges */
        for (j = 0; j < 4; j++) {
            /* More uses of invariants and constants */
            int temp = (array_ptr[(idx1 + j) & 0x3FF] * 
                       (int)(expensive_const1 >> (j * 8))) +
                      (int)((uintptr_t)double_ptr >> (j * 4));
            
            sum += temp + val1 + (int)val2 + val3;
            
            /* Use data pointer (another invariant) */
            if (data) {
                sum += data[(i * 4 + j) % 256];
            }
        }
        
        /* Modify invariants slightly to prevent optimization */
        array_ptr = &global_array[(i + 1) & 0x3FF];
        double_ptr = &global_double_array[(i * 2) & 0x3FF];
    }
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
long long func_asm_clobber(int a, int b, int c) {
    /* Register variables to force specific allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register long long result asm("edx");
    
    /* Complex inline assembly with multiple outputs and clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "movl %3, %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %%edx\n\t"
        "shll $16, %%edx\n\t"
        "xorl %%eax, %%edx\n\t"
        "rdtsc\n\t"  /* Uses eax, edx - clobbers our values */
        "addl %%edx, %0\n\t"
        : "=r" (result)
        : "r" (r1), "r" (r2), "r" (r3)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More operations to extend live ranges */
    for (int i = 0; i < 16; i++) {
        asm volatile (
            "addl $1, %0\n\t"
            "rorl $4, %0\n\t"
            : "+r" (r1)
            :
            : "cc"
        );
        
        result += r1 + r2 + r3;
        
        /* Use computed goto to create complex control flow */
        void *labels[] = { &&label1, &&label2, &&label3 };
        goto *labels[i % 3];
        
    label1:
        r2 += (i * 7);
        continue;
    label2:
        r3 ^= (i * 13);
        continue;
    label3:
        r1 |= (i * 29);
        continue;
    }
    
    return result;
}

/* Function C: Complex control flow with many temporaries */
__attribute__((noinline, noclone))
int func_complex_flow(int seed, int *output) {
    /* Many local variables with overlapping live ranges */
    int a = seed * 3;
    int b = seed * 7;
    int c = seed * 13;
    int d = seed * 29;
    int e = seed * 53;
    int f = seed * 97;
    int g = seed * 131;
    int h = seed * 193;
    int i = seed * 257;
    int j = seed * 311;
    
    int result = 0;
    int counter;
    
    /* Nested loops with switch inside */
    for (counter = 0; counter < 100; counter++) {
        int mod = counter % 10;
        
        switch (mod) {
            case 0:
                a = b + c;
                d = e * f;
                result += a * d;
                break;
            case 1:
                g = h ^ i;
                j = a | b;
                result += g - j;
                break;
            case 2:
                c = d << 2;
                f = e >> 1;
                result += c & f;
                break;
            case 3:
                h = i * 3;
                a = b / 2;
                result += h % a;
                break;
            case 4:
                e = f + g;
                j = h - i;
                result += e | j;
                break;
            case 5:
                b = c ^ d;
                g = h * i;
                result += b & g;
                break;
            case 6:
                d = e << 3;
                i = j >> 2;
                result += d | i;
                break;
            case 7:
                f = g * 5;
                a = b + c;
                result += f % a;
                break;
            case 8:
                h = i ^ j;
                c = d & e;
                result += h - c;
                break;
            case 9:
                j = a * 7;
                g = h / 3;
                result += j + g;
                break;
        }
        
        /* Conditional with many live variables */
        if (counter & 1) {
            int temp1 = a * b + c * d;
            int temp2 = e * f + g * h;
            int temp3 = i * j + a * c;
            int temp4 = b * d + e * g;
            int temp5 = f * h + i * j;
            
            result += temp1 - temp2 + temp3 - temp4 + temp5;
            
            /* Use all variables in a complex expression */
            output[counter] = (a & b) | (c ^ d) + (e & f) - (g | h) * (i ^ j);
        } else {
            /* Different complex expression using all variables */
            output[counter] = (a | b) & (c ^ d) - (e & f) + (g | h) ^ (i & j);
        }
        
        /* Rotate values to extend live ranges */
        int tmp = a;
        a = b; b = c; c = d; d = e; e = f;
        f = g; g = h; h = i; i = j; j = tmp;
    }
    
    return result;
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
        global_double_array[i] = i * 1.5;
    }
    for (int i = 0; i < 2048; i++) {
        global_char_array[i] = i & 0xFF;
    }
    
    int local_data[256];
    for (int i = 0; i < 256; i++) {
        local_data[i] = i * 7;
    }
    
    int output[100];
    
    /* Call all test functions with arguments that create register pressure */
    int result1 = func_loop_invariants(100, local_data);
    long long result2 = func_asm_clobber(argc, result1, 12345);
    int result3 = func_complex_flow(result1 + argc, output);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + (int)result2 + result3;
    
    /* Use output to prevent optimization */
    for (int i = 0; i < 100; i++) {
        final_result += output[i];
    }
    
    return final_result & 0xFF;  /* Return non-zero result */
}
