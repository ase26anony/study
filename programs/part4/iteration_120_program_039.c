/* Test program for hw-doloop.cc partial overlap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop */

#include <stdio.h>

/* Dummy functions to create unique basic blocks */
void __attribute__((noinline, noclone)) dummy1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop only */
        dummy2(outer);
        
        /* Complex if-else structure creates multiple basic blocks */
        if (condition) {
            /* Branch-specific block - only in true branch */
            volatile int branch_var = outer * 2;
            dummy3(branch_var);
            
            /* INNER LOOP A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                /* Loop body block A1 */
                checksum += inner_a;
                dummy1(inner_a);
                
                /* Use goto to jump to shared block outside the if branch */
                /* This creates partial overlap with outer loop */
                if (inner_a == M/2) {
                    goto shared_block;
                }
                
                /* Loop body block A2 */
                checksum -= inner_a * 2;
                dummy2(inner_a);
            }
            
            /* Block after inner loop A (still in true branch) */
            volatile int post_a = outer + 1;
            dummy4(post_a);
        } else {
            /* INNER LOOP B - in else branch */
            /* Shares common prologue but has different body */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                /* Different body from loop A */
                checksum += inner_b * 3;
                dummy3(inner_b);
                
                /* Different dummy function pattern */
                checksum -= inner_b;
                dummy4(inner_b);
            }
        }
        
        /* Shared block that both inner loops can reach */
        /* This creates the partial overlap condition */
        shared_block:
        shared_counter++;
        dummy1(shared_counter);
        
        /* Another block in outer loop after the shared block */
        checksum += outer;
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING LOOP C - shares prologue with outer loop but has different body */
    /* This creates another partial overlap scenario */
    volatile int reset = shared_counter;
    dummy1(reset);  /* Same dummy call as prologue */
    
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Different body from outer loop */
        checksum += sibling * 4;
        dummy2(sibling + 1000);  /* Different argument pattern */
        
        /* Different control flow pattern */
        if (sibling % 2 == 0) {
            checksum -= sibling;
            dummy3(sibling);
        } else {
            checksum += sibling * 2;
            dummy4(sibling);
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Final calculation to prevent dead code elimination */
    volatile int final = checksum % 1000;
    printf("Result: %d\n", final);
    
    return final;
}
