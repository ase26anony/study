/* Test program for hw-doloop.cc partial overlap bitmap analysis */
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
    /* Volatile variables to prevent optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Outer Loop - will contain partially overlapping inner loops */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common prologue block - shared by inner loops */
        volatile int common = outer * 2;
        dummy1(common);
        
        /* Branch creates separate basic blocks */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            volatile int inner_a;
            
            /* Label for goto to create partial overlap */
            shared_block:
            
            for (inner_a = 0; inner_a < M1; ++inner_a) {
                /* Unique body for loop A */
                dummy2(inner_a + common);
                checksum += inner_a;
                
                /* Memory barrier to prevent optimization */
                asm volatile("" : : : "memory");
                
                /* This goto creates partial overlap by jumping
                   to a block that's in outer loop but outside
                   the true branch */
                if (inner_a == M1/2) {
                    goto overlap_point;
                }
            }
            
            /* Continuation after loop A in true branch */
            dummy3(common + 1000);
        } else {
            /* Inner Loop B - different body, same iteration count */
            for (volatile int inner_b = 0; inner_b < M1; ++inner_b) {
                dummy3(inner_b - common);
                checksum -= inner_b;
                asm volatile("" : : : "memory");
            }
        }
        
        /* This block is part of outer loop but outside the if-else */
        overlap_point:
        dummy4(outer);
        
        /* Another goto back to create more complex CFG */
        if (outer % 2 == 0) {
            goto shared_block;
        }
    }
    
    /* Sibling Loop C - shares common prologue pattern but different body */
    {
        /* Reuse similar prologue structure */
        volatile int common = 42;
        dummy1(common);
        
        /* Different loop structure */
        for (volatile int sibling = 0; sibling < M3; ++sibling) {
            /* Unique body that shares some blocks with outer loop
               through dummy1 call but has different operations */
            if (sibling % 3 == 0) {
                dummy2(sibling);
            } else {
                dummy3(sibling * 2);
            }
            checksum += sibling;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Additional complexity: Loop D that partially overlaps with sibling */
    {
        volatile int setup = 50;
        dummy1(setup);  /* Shared block with sibling loop */
        
        for (volatile int loop_d = 0; loop_d < M2; ++loop_d) {
            /* Partially shared, partially unique blocks */
            if (loop_d < M2/2) {
                dummy2(loop_d);  /* Shared with sibling's true branch */
            } else {
                dummy4(loop_d);  /* Unique to this loop */
            }
            
            /* Jump to create overlap with outer loop's structure */
            if (loop_d == M2-1) {
                volatile int temp = loop_d;
                dummy1(temp);  /* Another shared block */
            }
            
            checksum += loop_d * 2;
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
