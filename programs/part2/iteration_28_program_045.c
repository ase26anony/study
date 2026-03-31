/* test_mcf.c - Program to trigger MCF algorithm special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (control % 12) {
        case 0:
            a = b + c;
            b = d * e;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            f = g - h;
            s1 = s2 + s3;
            fl1 = fl2 * fl3;
            goto label3;
        
        case 2:
        label1:
            i = j * a;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            goto label2;
        
        case 3:
            d = e / 2;
            fl2 = fl1 + 1.0f;
            /* Fall through */
        
        case 4:
            s2 = s1 * 2;
            ch2 = ch3 - 1;
            goto label4;
        
        case 5:
        label2:
            h = i + j;
            fl3 = fl1 / fl2;
            asm volatile("" : : : "r8", "r9", "r10");
            /* Complex conditional jump */
            if (a > b) goto label5;
            else goto label6;
        
        case 6:
            c = d - e;
            s3 = s1 + s2;
            goto label7;
        
        case 7:
        label3:
            j = a * b;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            goto label8;
        
        case 8:
        label4:
            e = f + g;
            fl1 = fl2 - fl3;
            /* Nested conditional */
            if (ch1 == 'a') {
                if (s1 > 10) goto label3;
            }
            break;
        
        case 9:
        label5:
            a = c * d;
            s1 = s3 - s2;
            goto label9;
        
        case 10:
        label6:
            b = e / f;
            ch1 = ch2 * 2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            /* Another goto to create irreducible flow */
            goto label10;
        
        case 11:
        label7:
            g = h * i;
            fl2 = fl3 + 1.5f;
            /* Jump back to earlier case */
            goto label4;
        
        default:
        label8:
            d = j - a;
            s2 = s1 / 2;
            break;
    }
    
    /* More labels and gotos for irreducible control flow */
    label9:
    result = a + b;
    asm volatile("" : : : "xmm3", "xmm4");
    goto label11;
    
    label10:
    result = c - d;
    ch2 = ch3 + 1;
    goto label12;
    
    label11:
    result += e + f;
    fl3 = fl1 * fl2;
    /* Conditional jump forward */
    if (result > 100) goto label13;
    
    label12:
    result += g + h;
    s3 = s1 + s2;
    asm volatile("" : : : "xmm5", "xmm6", "xmm7");
    
    label13:
    result += i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 + fl2 + fl3);
    
    /* Final register clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall complexity */
NOINLINE int secondary_function(volatile int x) {
    int arr[20];
    int sum = 0;
    
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : : : "eax");
    }
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += arr[i];
            goto loop_mid;
        } else if (i % 3 == 1) {
            sum -= arr[i];
            goto loop_end;
        } else {
            sum *= arr[i];
        }
        
        loop_mid:
        asm volatile("" : : : "ebx", "ecx");
        
        loop_end:
        if (i % 4 == 0) {
            asm volatile("" : : : "edx");
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling complex functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = rand() % 100;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_function(selector % 20);
        
        /* Mix results to prevent optimization */
        total += res1 + res2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
            selector = i;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional complexity with nested loops */
    {
        int matrix[10][10];
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                matrix[i][j] = i * j + selector;
                asm volatile("" : : : "eax");
            }
        }
        
        /* Process matrix with gotos */
        int mat_sum = 0;
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (matrix[i][j] % 2 == 0) {
                    goto even_handler;
                } else {
                    goto odd_handler;
                }
                
                even_handler:
                mat_sum += matrix[i][j];
                continue;
                
                odd_handler:
                mat_sum -= matrix[i][j];
                asm volatile("" : : : "ebx");
            }
        }
        
        total += mat_sum;
    }
    
    printf("Final total: %lld\n", total);
    return 0;
}
