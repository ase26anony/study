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

/* Shared prologue block - will be part of multiple loops */
int __attribute__((noinline)) shared_prologue(int base) {
    volatile int x = base * 2;
    asm volatile("" : : : "memory");
    return x;
}

int main() {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int selector = 1;
    
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common block that will be shared with inner loops */
        int shared = shared_prologue(outer);
        
        /* This creates partial overlap scenario */
        if (selector > 0) {
            /* Inner Loop A - starts inside true branch but extends beyond it */
            /* This loop will have blocks both inside and outside the if branch */
            volatile int inner_a = 0;
            
            /* Label for goto to create partial overlap */
            loop_a_start:
            
            for (inner_a = 0; inner_a < M1; ++inner_a) {
                dummy1(inner_a + shared);
                checksum += inner_a;
                
                /* Critical: Jump to block outside the if branch but still in outer loop */
                if (inner_a == M1/2) {
                    /* This goto creates the partial overlap */
                    goto shared_continuation;
                }
            }
            
            /* Block only in Inner Loop A's true branch path */
            dummy2(shared * 2);
            checksum += 1;
            
            shared_continuation:
            /* This block is in both Outer Loop and Inner Loop A */
            dummy3(shared * 3);
            checksum += 2;
            
        } else {
            /* Inner Loop B - shares the shared_prologue but different body */
            for (volatile int inner_b = 0; inner_b < M2; ++inner_b) {
                dummy2(inner_b - shared);
                checksum -= inner_b;
                asm volatile("" : : : "memory");
            }
            
            /* Different block pattern for Inner Loop B */
            dummy4(shared * 4);
            checksum += 3;
        }
        
        /* Outer loop continuation block */
        asm volatile("" : : : "memory");
        checksum += outer;
    }
    
    /* Sibling Loop C - shares some blocks with outer loop but not all */
    /* Uses same shared_prologue but different iteration count and body */
    {
        volatile int sibling_base = 42;
        int shared_c = shared_prologue(sibling_base);
        
        /* Loop C shares the prologue but has completely different body */
        for (volatile int sibling = 0; sibling < M3; ++sibling) {
            /* Different dummy function calls create different basic blocks */
            dummy3(sibling + shared_c);
            dummy4(sibling - shared_c);
            checksum += sibling * 2;
            
            /* Insert memory barrier to prevent optimization */
            asm volatile("" : : : "memory");
        }
        
        /* Different epilogue for sibling loop */
        dummy1(shared_c * 5);
        checksum += 4;
    }
    
    /* Another partial overlap scenario: two sequential loops sharing setup */
    {
        volatile int setup = 0;
        asm volatile("" : "=r"(setup) : : "memory");
        
        /* Loop D and E will be siblings with partial overlap */
        for (volatile int d = 0; d < 50; ++d) {
            /* Shared setup block */
            int temp = shared_prologue(d + setup);
            dummy1(temp);
            checksum += d;
            
            /* Loop D specific */
            dummy2(d * 2);
        }
        
        /* Different iteration count creates different bitmap */
        for (volatile int e = 0; e < 75; ++e) {
            /* Reuse the same shared_prologue pattern */
            int temp = shared_prologue(e + setup);
            dummy1(temp);  /* Same as Loop D */
            checksum -= e;
            
            /* Loop E specific - different from Loop D */
            dummy3(e * 3);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
