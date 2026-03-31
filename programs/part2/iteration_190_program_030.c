/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move an instruction that is currently marked as BB_END of its basic block,
 * specifically covering lines 905-913 in caller-save.cc.
 *
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -march=x86-64 -fdump-rtl-caller_save caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function to create register pressure */
int __attribute__((noinline)) 
pressure_function(int a, int b, int c, int d, int e, int f) {
    /* Use multiple call-clobbered registers in calculations */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    
    /* Function call that clobbers registers */
    int r = rand();
    
    /* More register-heavy computations after call */
    asm volatile ("" 
                  : /* outputs */ 
                  : /* inputs */ 
                  : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc");
    
    return v1 + v2 + v3 + r;
}

/* Main test function with complex control flow */
int __attribute__((noinline))
test_caller_save(int iterations) {
    volatile int cond = 1;  /* Prevent optimization */
    int sum = 0;
    int i, j;
    
    /* Outer loop to increase caller-save pass activity */
    for (i = 0; i < iterations; i++) {
        volatile int loop_cond = i % 3;
        
        /* Inner loop with multiple basic blocks */
        for (j = 0; j < 10; j++) {
            /* Critical: Create a basic block that ends with a conditional */
            if (loop_cond > 0) {
                /* This block will end with a jump to merge point */
                
                /* Use multiple temporaries to occupy call-clobbered registers */
                int t1 = i * j;
                int t2 = i + j;
                int t3 = i ^ j;
                int t4 = i - j;
                int t5 = i | j;
                int t6 = i & j;
                
                /* Clobber registers before call */
                asm volatile ("" 
                              : /* outputs */ 
                              : /* inputs */ 
                              : "rax", "rcx", "rdx", "rsi", "rdi", "cc");
                
                /* Function call - creates need for caller-save */
                int result = pressure_function(t1, t2, t3, t4, t5, t6);
                
                /* Clobber registers after call */
                asm volatile ("" 
                              : /* outputs */ 
                              : /* inputs */ 
                              : "rax", "rcx", "rdx", "rsi", "rdi", "cc");
                
                /* More computations to keep values live across calls */
                t1 = result * t1;
                t2 = result + t2;
                t3 = result ^ t3;
                
                sum += t1 + t2 + t3;
                
                /* This instruction could become BB_END */
                if (cond) {
                    /* Another level of nesting to create block boundaries */
                    volatile int inner_cond = result % 2;
                    if (inner_cond) {
                        int temp = rand();  /* Another call */
                        sum += temp;
                    }
                }
            } else {
                /* Alternative path to maintain control flow complexity */
                sum += rand() % 100;
            }
            
            /* Merge point - critical for BB_END logic */
            volatile int merge_var = sum & 1;
            if (merge_var) {
                sum += j;
            }
        }
        
        /* Additional control flow to split blocks */
        switch (i % 4) {
            case 0:
                sum += pressure_function(i, i+1, i+2, i+3, i+4, i+5);
                break;
            case 1:
                sum -= rand();
                break;
            default:
                sum ^= rand();
                break;
        }
    }
    
    return sum;
}

/* Secondary test with different pattern */
int __attribute__((noinline))
test_caller_save_alt(int iterations) {
    volatile int toggle = 0;
    int total = 0;
    
    while (iterations-- > 0) {
        /* Create a small basic block ending with conditional */
        if (toggle) {
            /* Use many registers */
            int a = iterations * 3;
            int b = iterations + 5;
            int c = iterations ^ 0xFF;
            int d = iterations << 2;
            int e = iterations >> 1;
            int f = iterations | 0xAA;
            
            /* Force register spilling */
            asm volatile ("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "cc");
            
            /* Call that requires caller-save */
            int res = pressure_function(a, b, c, d, e, f);
            
            asm volatile ("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "cc");
            
            total += res;
            
            /* This compare/jump could be BB_END */
            if (total > 1000) {
                total -= 500;
            }
        } else {
            total += rand() % 50;
        }
        
        toggle = !toggle;
        
        /* Force another basic block boundary */
        volatile int check = total % 7;
        if (check == 0) {
            total += pressure_function(total, total+1, total+2, total+3, total+4, total+5);
        }
    }
    
    return total;
}

int main() {
    srand(time(NULL));
    
    printf("Testing caller-save BB_END update scenario...\n");
    
    /* Run both test patterns to increase coverage chances */
    int result1 = test_caller_save(100);
    int result2 = test_caller_save_alt(50);
    
    int final_result = result1 + result2;
    
    /* Use result to prevent optimization */
    volatile int output = final_result;
    printf("Result: %d (hex: 0x%x)\n", output, output);
    
    /* Additional test with array to create more register pressure */
    volatile int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        if (arr[i] % 2) {
            /* Call in conditional block */
            sum += pressure_function(arr[i], arr[9-i], i, sum, output, final_result);
        }
    }
    
    printf("Array test sum: %d\n", sum);
    
    return 0;
}
