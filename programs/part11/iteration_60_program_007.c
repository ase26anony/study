/* Test case for early rematerialization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types for mode conversions */
typedef struct {
    int a : 4;
    int b : 8;
    int c : 12;
    int d : 8;
} packed_struct;

/* Function with high register pressure and complex dataflow */
int complex_calculation(int x1, int x2, int x3, int x4, int x5,
                        int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4 + v3;
    int d = c - x5 * v4;
    int e = d | x6 & v5;
    int f = e + x7 / (v6 ? v6 : 1);
    int g = f * x8 - v7;
    int h = g ^ x9 + v8;
    int i = h - x10 * v9;
    int j = i | x1 & v10;
    
    /* More intermediate values with different lifetimes */
    int k = j + x2 * v1;
    int l = k - x3 ^ v2;
    int m = l * x4 + v3;
    int n = m | x5 - v4;
    int o = n ^ x6 * v5;
    int p = o + x7 & v6;
    int q = p - x8 | v7;
    int r = q * x9 + v8;
    int s = r ^ x10 - v9;
    int t = s | x1 * v10;
    
    /* Even more values to ensure high pressure */
    int u = t + x2 - v1;
    int v = u * x3 ^ v2;
    int w = v | x4 + v3;
    int x = w - x5 & v4;
    int y = x ^ x6 * v5;
    int z = y + x7 - v6;
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)z;
    short s2 = (short)(z >> 8);
    int i1 = (int)s1 * (int)s2;  /* Mode conversion here */
    
    char c1 = (char)(i1 & 0xFF);
    char c2 = (char)((i1 >> 8) & 0xFF);
    int i2 = (int)c1 + (int)c2;  /* Another mode conversion */
    
    /* Use packed structure to create sub-register accesses */
    packed_struct ps;
    ps.a = c1;
    ps.b = c2;
    ps.c = s1;
    ps.d = s2;
    
    int struct_val = ps.a + ps.b * 2 + ps.c * 3 + ps.d * 4;
    
    /* Complex loop with many live values */
    int sum = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Use __builtin_expect to create conditional basic blocks */
        if (__builtin_expect((iter & 1) == 0, 0)) {
            /* Even iteration: use one set of values */
            sum += a + c + e + g + i + k + m + o + q + s + u + w + y;
        } else {
            /* Odd iteration: use different set */
            sum += b + d + f + h + j + l + n + p + r + t + v + x + z;
        }
        
        /* Switch statement creating complex control flow */
        switch (iter % 8) {
            case 0:
                sum += i1 * struct_val;
                break;
            case 1:
                sum += i2 ^ ps.c;
                break;
            case 2:
                sum += s1 - ps.d;
                break;
            case 3:
                sum += c1 | ps.a;
                break;
            case 4:
                sum += c2 & ps.b;
                break;
            case 5:
                sum += (i1 << 2) + i2;
                break;
            case 6:
                sum += (struct_val >> 1) ^ i1;
                break;
            case 7:
                sum += ps.a * ps.b + ps.c - ps.d;
                break;
        }
        
        /* Inline assembly to create hard register constraints */
        int asm_input = sum;
        int asm_output;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (asm_output)
            : "r" (asm_input)
            : "%eax"
        );
        sum = asm_output;
        
        /* Address calculations that are rematerialization candidates */
        int* base_ptr = &sum;
        int val1 = *(base_ptr);
        int val2 = *(base_ptr);
        int val3 = *(base_ptr);
        
        sum = val1 + val2 + val3;
        
        /* More arithmetic to keep values live */
        a += iter;
        b -= iter;
        c ^= iter;
        d |= iter;
        e &= iter;
    }
    
    /* Final aggregation */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t +
                 u + v + w + x + y + z + i1 + i2 + struct_val + sum;
    
    return result;
}

/* Another function to create more compilation context */
void additional_pressure(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex expression with many temporaries */
        int t1 = arr[i] * 2;
        int t2 = t1 + 1;
        int t3 = t2 ^ 0x55;
        int t4 = t3 << 2;
        int t5 = t4 | 0xAA;
        int t6 = t5 - 1;
        int t7 = t6 * 3;
        int t8 = t7 / 2;
        int t9 = t8 ^ t1;
        int t10 = t9 | t2;
        int t11 = t10 & t3;
        int t12 = t11 + t4;
        int t13 = t12 - t5;
        int t14 = t13 * t6;
        int t15 = t14 / t7;
        int t16 = t15 ^ t8;
        int t17 = t16 | t9;
        int t18 = t17 & t10;
        int t19 = t18 + t11;
        int t20 = t19 - t12;
        
        arr[i] = t20;
    }
}

int main() {
    /* Initialize with many different values */
    int inputs[20];
    for (int i = 0; i < 20; i++) {
        inputs[i] = i * 3 + 1;
    }
    
    /* Call the complex function multiple times */
    int total = 0;
    for (int i = 0; i < 1000; i++) {
        total += complex_calculation(
            inputs[0] + i, inputs[1] - i, inputs[2] ^ i,
            inputs[3] | i, inputs[4] & i, inputs[5] + i * 2,
            inputs[6] - i * 3, inputs[7] ^ i * 4,
            inputs[8] | i * 5, inputs[9] & i * 6
        );
        
        /* Modify inputs slightly each iteration */
        inputs[i % 20] ^= total & 0xFF;
    }
    
    /* Create more register pressure */
    additional_pressure(inputs, 20);
    
    /* Final computation */
    int final_result = total;
    for (int i = 0; i < 20; i++) {
        final_result += inputs[i];
    }
    
    printf("Result: %d\n", final_result);
    return 0;
}
