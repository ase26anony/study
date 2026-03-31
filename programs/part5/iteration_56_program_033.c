/* test_early_remat.c - Target specific patterns to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdio.h>

/* Global data for address calculations - ensures non-encodable addresses */
static int global_array[256] = {0};
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};
static volatile int volatile_sink;

/* Prevent optimizations from eliminating our carefully crafted patterns */
#define KEEP_ALIVE(x) do { volatile_sink = (x); } while(0)

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int loop_with_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations * 2;
    register int r1 asm("ebx") = iterations + 0x7FFFFFFF; /* Large immediate */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Invariant values used in loop - will be rematerialized */
    const long invariant1 = (long)&global_array[128]; /* Non-encodable address */
    const long invariant2 = 0x123456789ABCDEF0LL;     /* 64-bit constant on 32-bit */
    const int invariant3 = 0x55555555;
    
    /* Complex loop with many live values */
    for (int idx = 0; idx < iterations; idx++) {
        /* Use invariants in multiple non-adjacent calculations */
        a = (idx * invariant3) & 0xFF;
        b = (a + (int)(invariant1 >> 32)) | invariant3;
        
        /* Address calculation using invariant */
        int *ptr = (int*)((char*)data + (invariant2 & 0xFFFFFFFF));
        c = *ptr + idx;
        
        /* More operations keeping many values live */
        d = a * b - c;
        e = d ^ invariant3;
        f = e + (int)invariant1;
        g = f * g / (h + 1);
        h = g ^ a ^ b ^ c;
        i = h + (invariant2 >> 32);
        j = i * j - k;
        k = j | l;
        l = k & m;
        m = l ^ n;
        n = m + o;
        o = n * p;
        p = o - idx;
        
        /* Use register variables */
        r0 = r0 + a + b;
        r1 = r1 ^ c ^ d;
    }
    
    /* Force all values to be used to extend live ranges */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + r0 + r1;
    KEEP_ALIVE(result);
    return result;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int asm_clobber_test(int x, int y) {
    int a, b, c, d, e, f, g, h;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[a]\n\t"
        "movl %%ebx, %[b]\n\t"
        : [a] "=&r" (a), [b] "=&r" (b)  /* Early clobber outputs */
        : [x] "rm" (x), [y] "rm" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* More operations creating register pressure */
    c = a * 0x12345678;  /* Large immediate */
    d = b + 0x9ABCDEF0;  /* Another large immediate */
    
    /* Second asm with different clobbers */
    asm volatile (
        "rdtsc\n\t"  /* Uses eax, edx - specific hard registers */
        "movl %%eax, %[e]\n\t"
        "movl %%edx, %[f]\n\t"
        : [e] "=r" (e), [f] "=r" (f)
        :
        : "eax", "edx"
    );
    
    /* Mix results keeping them all live */
    g = (c & e) | (d & f);
    h = g * a * b * c * d * e * f;
    
    /* Complex expression with many temporaries */
    int result = (a << 3) | (b << 2) | (c << 1) | d;
    result = result ^ e ^ f ^ g ^ h;
    result = result + 0x55555555;  /* Another large immediate */
    
    KEEP_ALIVE(result);
    return result;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int complex_control_flow(int selector, int iterations) {
    /* Many local variables with register keyword */
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    register int reg3;
    
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 0x10001;  /* Non-simple constant */
    }
    
    reg1 = selector;
    reg2 = iterations;
    reg3 = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int temp1 = reg1 * 0x12345678;
        int temp2 = reg2 + 0x9ABCDEF0;
        int temp3 = temp1 ^ temp2;
        int temp4 = temp3 & 0x55555555;
        int temp5 = temp4 | 0xAAAAAAAA;
        
        /* Switch creates complex control flow */
        switch (i % 5) {
            case 0:
                reg1 = temp1 + arr[0];
                reg2 = temp2 - arr[1];
                break;
            case 1:
                reg1 = temp3 * arr[2];
                reg2 = temp4 / (arr[3] + 1);
                break;
            case 2:
                reg1 = temp5 ^ arr[4];
                reg2 = ~temp1 & arr[5];
                break;
            case 3:
                /* Computed goto */
                goto *labels[i % 5];
            case 4:
                reg1 = (reg1 << 3) | (reg2 << 1);
                reg2 = reg1 ^ reg2;
                break;
        }
        
        /* Keep many values live across the switch */
        int temp6 = reg1 + temp1;
        int temp7 = reg2 + temp2;
        int temp8 = temp6 * temp7;
        int temp9 = temp8 ^ temp3;
        int temp10 = temp9 & temp4;
        
        reg3 += temp6 + temp7 + temp8 + temp9 + temp10;
        
        label0:
        label1:
        label2:
        label3:
        label4:
        /* Empty labels for computed goto targets */
    }
    
    KEEP_ALIVE(reg3);
    return reg3;
}

/* Function D: Nested loops with address calculations */
__attribute__((noinline, noclone))
int nested_loops_address_calc(int size) {
    /* Use global array with non-encodable base address */
    int *base = &global_array[64];  /* Middle of array - non-simple address */
    int sum = 0;
    
    /* Many local variables creating register pressure */
    int v1 = 0x11111111, v2 = 0x22222222, v3 = 0x33333333;
    int v4 = 0x44444444, v5 = 0x55555555, v6 = 0x66666666;
    int v7 = 0x77777777, v8 = 0x88888888, v9 = 0x99999999;
    
    /* Nested loops with invariant address calculations */
    for (int i = 0; i < size; i++) {
        /* Invariant derived from base pointer */
        int *row_start = base + i * 16;
        
        for (int j = 0; j < 8; j++) {
            /* Complex address calculation with invariant */
            int *elem = row_start + j * 2;
            
            /* Use many variables in computation */
            v1 = *elem + v1;
            v2 = v1 ^ v2;
            v3 = v2 * v3;
            v4 = v3 - v4;
            v5 = v4 | v5;
            v6 = v5 & v6;
            v7 = v6 + v7;
            v8 = v7 ^ v8;
            v9 = v8 * v9;
            
            /* Use large immediate */
            v1 += 0x12345678;
            v2 ^= 0x9ABCDEF0;
        }
        
        /* Cross-iteration dependencies */
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    }
    
    /* Force all values to be live at end */
    int result = sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    KEEP_ALIVE(result);
    return result;
}

/* Main function that calls all test patterns */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0x10001;
    }
    
    int result = 0;
    
    /* Call each test function with arguments that create register pressure */
    result += loop_with_invariants(100, global_array);
    result += asm_clobber_test(0x12345678, 0x9ABCDEF0);
    result += complex_control_flow(argc, 50);
    result += nested_loops_address_calc(10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    return result & 0xFF;
}
