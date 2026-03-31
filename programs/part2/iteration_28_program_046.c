/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining and no inter-procedural analysis */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15;
    short s1 = 16, s2 = 17, s3 = 18, s4 = 19;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c', ch4 = 'd';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5; /* Create cross-block jumps */
        
        case 1:
            c = d - e;
            d = e / (f ? f : 1);
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 2:
            e = f ^ g;
            f = g | h;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_8;
        
        case 3:
            g = h & i;
            h = i << 2;
            asm volatile("" : : : "r11", "r12", "r13");
            break;
        
        case 4:
            i = j >> 1;
            j = k + l;
            asm volatile("" : : : "r14", "r15");
            /* Fall through */
        
        case 5:
        case_5:
            k = l * m;
            l = m - n;
            asm volatile("" : : : "xmm0", "xmm1");
            break;
        
        case 6:
            m = n / (o ? o : 1);
            n = o ^ a;
            asm volatile("" : : : "xmm2", "xmm3");
            goto case_12;
        
        case 7:
            o = a | b;
            s1 = s2 + s3;
            asm volatile("" : : : "xmm4", "xmm5");
            break;
        
        case 8:
        case_8:
            s2 = s3 * s4;
            s3 = s4 - ch1;
            asm volatile("" : : : "xmm6", "xmm7");
            break;
        
        case 9:
            s4 = ch1 ^ ch2;
            ch1 = ch2 | ch3;
            asm volatile("" : : : "xmm8", "xmm9");
            goto case_15;
        
        case 10:
            ch2 = ch3 & ch4;
            ch3 = ch4 << 1;
            asm volatile("" : : : "xmm10", "xmm11");
            break;
        
        case 11:
            ch4 = fl1 > fl2 ? 'x' : 'y';
            fl1 = fl2 * fl3;
            asm volatile("" : : : "xmm12", "xmm13");
            break;
        
        case 12:
        case_12:
            fl2 = fl3 + 1.0f;
            fl3 = fl1 - 0.5f;
            asm volatile("" : : : "xmm14", "xmm15");
            break;
        
        case 13:
            a = s1 + ch1;
            b = s2 * ch2;
            asm volatile("" : : : "eax", "ebx", "ecx");
            goto case_0_again;
        
        case 14:
            c = s3 - ch3;
            d = s4 / (ch4 ? ch4 : 1);
            asm volatile("" : : : "edx", "esi", "edi");
            break;
        
        case 15:
        case_15:
            e = fl1 > 0 ? 1 : 0;
            f = fl2 < 10 ? 2 : 3;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 16:
            g = (int)fl1 + (int)fl2;
            h = (int)fl3 * 2;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case_negative;
        
        case 17:
            i = j << (k % 4);
            j = k >> (l % 4);
            asm volatile("" : : : "r14", "r15");
            break;
        
        case 18:
            k = l | m;
            l = m & n;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 19:
            m = n ^ o;
            n = o + a;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
        
        default:
            a = b = c = 0;
            break;
    }
    
    /* Another goto label for irreducible flow */
    case_0_again:
    a = a * 2;
    
    /* Negative case label */
    case_negative:
    if (control < 0) {
        b = b * 3;
        goto final_calc;
    }
    
    /* More arithmetic to use all variables */
    c = c + d;
    d = d - e;
    e = e * f;
    f = f / (g ? g : 1);
    g = g ^ h;
    h = h | i;
    i = i & j;
    j = j << 1;
    k = k >> 1;
    l = l + m;
    m = m - n;
    n = n * o;
    o = o / (s1 ? s1 : 1);
    s1 = s1 ^ s2;
    s2 = s2 | s3;
    s3 = s3 & s4;
    s4 = s4 + ch1;
    ch1 = ch1 - ch2;
    ch2 = ch2 * ch3;
    ch3 = ch3 / (ch4 ? ch4 : 1);
    ch4 = ch4 ^ (char)a;
    fl1 = fl1 + fl2;
    fl2 = fl2 - fl3;
    fl3 = fl3 * 2.0f;
    
final_calc:
    /* Combine all variables to prevent elimination */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o +
                 s1 + s2 + s3 + s4 + ch1 + ch2 + ch3 + ch4 + (int)fl1 + 
                 (int)fl2 + (int)fl3;
    
    /* Final register clobber */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi",
                               "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                               "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                               "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                               "xmm12", "xmm13", "xmm14", "xmm15");
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int base_selector = rand();
    long long total = 0;
    
    /* Hot loop calling the register pressure function */
    for (int count = 0; count < iterations; count++) {
        /* Vary selector to hit different switch cases */
        volatile int selector = base_selector + count;
        int result = register_pressure_function(selector);
        total += result;
        
        /* Occasionally change base to hit negative case */
        if (count % 1000 == 0) {
            base_selector = -rand() % 100;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}
