/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

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

/* Shared prologue block for partial overlap */
int __attribute__((noinline, noclone)) shared_prologue(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x * 2;
}

int main() {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: Shared block for partial overlap */
    int shared_val = shared_prologue(42);
    
    /* OUTER_LOOP: Loop with complex body */
    for (volatile int i = 0; i < N; ++i) {
        asm volatile("" : : : "memory");  /* Prevent optimization */
        
        /* Complex if-else structure to create multiple basic blocks */
        if (cond) {
            /* TRUE_BRANCH: Contains Inner Loop A */
            
            /* Inner Loop A - starts inside true branch */
            for (volatile int j = 0; j < M1; ++j) {
                dummy1(j + i);
                checksum += j;
                
                /* CRITICAL: Jump to shared block outside true branch */
                if (j == M1/2) {
                    goto shared_block;  /* Creates partial overlap */
                }
                
                dummy2(j * 2);
                checksum -= j;
            }
            
            /* End of true branch path */
            dummy3(i * 3);
        } else {
            /* FALSE_BRANCH: Contains Inner Loop B */
            
            /* Inner Loop B - shares prologue but different body */
            for (volatile int k = 0; k < M2; ++k) {
                dummy2(k + i * 2);
                checksum += k * 2;
                dummy3(k * 3);
            }
            
            dummy4(i * 4);
        }
        
        /* SHARED_BLOCK: Block shared by both paths and inner loops */
        shared_block:
        dummy1(shared_val + i);
        checksum += shared_val;
        
        /* Reset condition for next iteration */
        cond = !cond;
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING_LOOP_C: Shares prologue with outer loop but not contained */
    /* Uses same shared_prologue but different iteration count */
    int shared_val2 = shared_prologue(24);  /* Same prologue block */
    
    for (volatile int l = 0; l < M3; ++l) {
        dummy3(l + shared_val2);
        checksum += l * 3;
        dummy4(l * 4);
        
        /* Different body structure than inner loops */
        if (l % 2) {
            dummy1(l);
        } else {
            dummy2(l * 2);
        }
    }
    
    /* Another loop to ensure multiple candidates */
    volatile int temp = checksum;
    for (volatile int m = 0; m < 50; ++m) {
        dummy4(m + temp);
        checksum += m;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
