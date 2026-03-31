/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with high register pressure and irreducible control flow */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    double db1 = 4.4, db2 = 5.5;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (control & 0xF) {
        case 0:
            a = b + c;
            b = d * e;
            /* Jump to another case */
            goto case5;
            
        case 1:
            f = g - h;
            g = i / (j ? j : 1);
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            break;
            
        case 2:
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            fl1 = fl2 * fl3;
            goto case8;
            
        case 3:
            db1 = db2 * 2.0;
            a = f + g;
            /* Another goto creating irreducible flow */
            goto case10;
            
        case 4:
            h = i * j;
            s3 = s1 - s2;
            break;
            
        case5:  /* Label for goto target */
            c = d + e;
            f = g * h;
            /* Clobber more registers */
            asm volatile("" : : : "esi", "edi");
            break;
            
        case 6:
            ch3 = ch1 + ch2;
            fl3 = fl1 + fl2;
            goto case1;
            
        case 7:
            i = j * a;
            db2 = db1 / 2.0;
            break;
            
        case8:  /* Another goto target */
            s2 = s3 * 2;
            ch2 = ch3 - 1;
            goto case4;
            
        case 9:
            fl2 = fl3 * 2.0f;
            e = f + g;
            /* Complex conditional goto */
            if (a > b) goto case2;
            else goto case7;
            
        case10: /* Final goto target */
            j = a * b;
            s1 = s2 + s3;
            /* Force register pressure */
            asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            break;
            
        case 11:
            a = b * c * d;
            goto case3;
            
        case 12:
            f = g + h + i + j;
            break;
            
        case 13:
            s3 = s1 * s2;
            ch1 = ch2 * ch3;
            goto case6;
            
        case 14:
            fl1 = fl2 / fl3;
            db1 = db2 + 1.0;
            break;
            
        case 15:
            /* Most complex path */
            a = b + c + d + e + f + g + h + i + j;
            s1 = s2 + s3;
            ch3 = ch1 + ch2;
            fl3 = fl1 + fl2;
            db2 = db1 * 2.0;
            /* Clobber all caller-saved registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
            break;
    }
    
    /* More irreducible control flow with goto */
    if (control & 0x10) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    a = b * c;
    if (d > e) goto label3;
    else goto label4;
    
label2:
    f = g * h;
    if (i < j) goto label1;
    else goto label3;
    
label3:
    s1 = s2 - s3;
    ch1 = ch2 + ch3;
    goto label5;
    
label4:
    fl1 = fl2 * fl3;
    db1 = db2 / 2.0;
    goto label6;
    
label5:
    a = a + f + s1;
    goto label7;
    
label6:
    b = b + g + s2;
    goto label8;
    
label7:
    c = c + h + s3;
    goto label9;
    
label8:
    d = d + i + ch1;
    goto label10;
    
label9:
    e = e + j + ch2;
    goto label11;
    
label10:
    fl1 = fl1 + 1.0f;
    goto label12;
    
label11:
    db1 = db1 + 1.0;
    goto label13;
    
label12:
    fl2 = fl2 * 2.0f;
    goto label14;
    
label13:
    db2 = db2 * 2.0;
    goto final_label;
    
label14:
    fl3 = fl3 / 2.0f;
    goto final_label;
    
final_label:
    /* Combine all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j +
             s1 + s2 + s3 + ch1 + ch2 + ch3 +
             (int)fl1 + (int)fl2 + (int)fl3 +
             (int)db1 + (int)db2;
    
    return result;
}

/* Another complex function to increase overall complexity */
__attribute__((noinline))
int secondary_function(volatile int x) {
    int arr[20];
    int sum = 0;
    
    for (int i = 0; i < 20; i++) {
        arr[i] = x * i;
        if (i & 1) {
            arr[i] += x;
            asm volatile("" : : : "eax", "ebx");
        } else {
            arr[i] -= x;
            asm volatile("" : : : "ecx", "edx");
        }
    }
    
    /* Complex loop with goto */
    int j = 0;
loop_start:
    sum += arr[j];
    j++;
    if (j < 10) goto loop_start;
    
    /* Another goto section */
    if (sum > 100) goto large_sum;
    else goto small_sum;
    
large_sum:
    sum *= 2;
    goto done;
    
small_sum:
    sum /= 2;
    goto done;
    
done:
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
    
    /* Use time-based seed for variability */
    srand(time(NULL));
    
    printf("Starting MCF test with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to exercise different paths */
        selector = rand() % 256;
        
        /* Call high-pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_function(selector & 0x7F);
        
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            selector = (selector * 13 + 17) % 256;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional loop with different pattern */
    total = 0;
    for (int i = 0; i < iterations / 10; i++) {
        selector = i * 7;
        total += register_pressure_function(selector);
        
        /* Nested loop for more complexity */
        for (int j = 0; j < 5; j++) {
            selector += secondary_function(j);
        }
    }
    
    printf("Final total: %lld\n", total);
    
    return 0;
}
