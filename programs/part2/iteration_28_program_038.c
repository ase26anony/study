/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Complex function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short f = 6, g = 7, h = 8, i = 9, j = 10;
    char k = 11, l = 12, m = 13, n = 14, o = 15;
    float p = 16.0f, q = 17.0f, r = 18.0f;
    double s = 19.0, t = 20.0;
    unsigned int u = 21, v = 22, w = 23;
    
    /* Use inline assembly to clobber registers (x86-64 example) */
    asm volatile("" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Complex switch with irreducible control flow via goto */
    switch (selector % 12) {
        case 0:
            a = b + c;
            d = e * f;
            goto label1;
        
        case 1:
            g = h - i;
            j = k * 2;
            if (a > 10) goto label3;
            else goto label2;
        
        case 2:
        label1:
            p = q + r;
            s = t * 2.0;
            if (u < v) goto case4;
            break;
        
        case 3:
            m = n | o;
            u = v ^ w;
            goto label4;
        
        case 4:
        case4:
            a = (b << 2) + c;
            d = e ^ f;
            goto label5;
        
        case 5:
        label2:
            g = h / (i + 1);
            j = k % 5;
            if (l > 0) goto label6;
            break;
        
        case 6:
            p = p * q - r;
            s = s / (t + 1.0);
            goto label7;
        
        case 7:
        label3:
            u = v * w + a;
            b = c << d;
            goto label8;
        
        case 8:
        label5:
            m = n & o;
            f = g | h;
            if (i > j) goto label9;
            break;
        
        case 9:
        label6:
            k = l + m - n;
            o = p > q ? 1 : 0;
            goto label10;
        
        case 10:
        label7:
            r = s < t ? p : q;
            u = v + w;
            if (a < b) goto label11;
            break;
        
        case 11:
        label8:
            c = d * e + f;
            g = h & i;
            /* fall through */
        
        default:
        label9:
            j = k | l;
            m = n ^ o;
        label10:
            p = q * r;
            s = t + 1.0;
        label11:
            u = v - w;
            a = b + c + d;
            break;
    }
    
    /* More arithmetic to increase register pressure */
    a = a + b - c * d / (e + 1);
    f = (g << 2) | (h >> 1);
    k = l * m - n + o;
    p = q * 2.0f - r / 3.0f;
    s = t * 1.5 + s / 2.0;
    u = (v * w) % 100;
    
    /* Another inline assembly to clobber more registers */
    asm volatile("" : : : 
        "rax", "rbx", "rcx", "rdx",
        "xmm0", "xmm1", "xmm2", "xmm3");
    
    /* Complex computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j + 
                 k + l + m + n + o + (int)p + (int)q + 
                 (int)r + (int)s + (int)t + u + v + w;
    
    /* Irreducible loop with goto */
    int counter = 3;
loop_start:
    if (counter-- > 0) {
        result = (result * 31 + 17) % 1000;
        if (result % 2 == 0) goto loop_start;
    }
    
    return result;
}

/* Another complex function to create more opportunities */
NOINLINE static int secondary_pressure_function(volatile int x) {
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = i * x;
    }
    
    /* Nested loops with complex indexing */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (i != j) {
                sum += vars[i] * vars[j];
                if (sum > 1000) goto early_exit;
            }
        }
        /* Another goto creating irreducible flow */
        if (i % 3 == 0) goto skip_point;
        sum -= x;
    skip_point:
        asm volatile("" : : : "rax", "rbx");
    }
    
early_exit:
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int selector = rand() % 100;
    
    int total = 0;
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call both pressure functions */
        total += register_pressure_function(selector);
        if (i % 3 == 0) {
            total += secondary_pressure_function(selector % 20);
        }
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %d\n", total);
    return 0;
}
