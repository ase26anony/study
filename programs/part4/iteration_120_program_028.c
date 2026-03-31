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
    volatile int M = 100;
    volatile int K = 50;
    volatile int L = 200;
    volatile int counter = 0;
    volatile int flag = 1;
    
    /* COMMON PROLOGUE BLOCK - shared by multiple loops */
    volatile int shared_init = 0;
    dummy1(shared_init);
    
    /* OUTER LOOP - will partially overlap with inner loops */
    for (volatile int i = 0; i < N; ++i) {
        asm volatile("" : : : "memory");
        
        /* Branch creates multiple basic blocks in outer loop */
        if (flag) {
            /* INNER LOOP A - starts inside if branch */
            volatile int j = 0;
            
            /* Loop header block inside if branch */
            dummy2(j);
            
shared_label:
            /* This block is SHARED between inner loop A and outer loop */
            /* Inner loop A jumps here from its body */
            dummy3(counter);
            
            /* Inner loop A body continues here */
            for (; j < M; ++j) {
                asm volatile("" : : : "memory");
                counter += i * j;
                dummy2(j);
                
                /* Jump to shared block that's also in outer loop */
                if (j % 2 == 0) {
                    goto shared_label;  /* Creates partial overlap */
                }
                
                /* Another basic block in inner loop A */
                dummy3(j * 2);
            }
            
            /* End of if branch */
            dummy4(i);
        } else {
            /* INNER LOOP B - different body, shares prologue */
            for (volatile int k = 0; k < K; ++k) {
                asm volatile("" : : : "memory");
                counter -= i * k;
                dummy3(k);
            }
        }
        
        /* Outer loop continues with another basic block */
        dummy1(i * 2);
    }
    
    /* SIBLING LOOP C - shares prologue but has different body */
    /* Re-initialize shared prologue */
    shared_init = 1;
    dummy1(shared_init);
    
    for (volatile int l = 0; l < L; ++l) {
        asm volatile("" : : : "memory");
        counter += l * l;
        dummy4(l);
        
        /* Different control flow pattern */
        if (l % 3 == 0) {
            dummy2(l);
        } else {
            dummy3(l);
        }
    }
    
    /* Additional complexity to ensure loops aren't optimized away */
    volatile int checksum = 0;
    for (volatile int i = 0; i < 10; ++i) {
        checksum += counter;
        dummy1(checksum);
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}
