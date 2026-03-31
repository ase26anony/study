/* Test program for hw-doloop.cc uncovered lines 429-436
 * Creates nested loops with partial block overlap to trigger bitmap intersection logic
 */

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
    volatile int L = 75;
    volatile int cond = 0;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int shared = 0;
    dummy1(shared);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Complex if-else structure creates multiple basic blocks */
        if (cond) {
            /* Branch 1 */
            dummy2(1);
            
            /* INNER LOOP A - starts inside if branch */
            volatile int j = 0;
        inner_loop_a_start:
            dummy3(j);
            for (; j < M; ++j) {
                checksum += j * 2;
                dummy1(j);
                
                /* This goto creates partial overlap:
                 * Jumps to a block that's in outer loop but outside the if branch */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* Normal exit from inner loop A */
            dummy4(100);
        } else {
            /* Branch 2 */
            dummy2(2);
            
            /* INNER LOOP B - different from loop A but shares prologue */
            for (volatile int k = 0; k < K; ++k) {
                checksum -= k * 3;
                dummy2(k);
                asm volatile("" : : : "memory");
            }
            
            dummy4(200);
        }
        
        /* This block is part of outer loop and will be jumped to from inner loop A */
    shared_block:
        dummy3(999);
        checksum += i;
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* Memory barrier between loops */
    asm volatile("" : : : "memory");
    
    /* SIBLING LOOP C - shares the common prologue but has different body */
    /* Reset shared variable to reuse the prologue block */
    shared = 1;
    dummy1(shared);
    
    for (volatile int l = 0; l < L; ++l) {
        checksum += l * 4;
        dummy3(l);
        asm volatile("" : : : "memory");
    }
    
    /* Additional loop to create more complex bitmap relationships */
    volatile int extra = 0;
    dummy1(extra);
    
    /* This loop partially overlaps with sibling loop C through dummy1 calls */
    for (volatile int m = 0; m < 25; ++m) {
        if (m % 2) {
            dummy1(m);  /* Shared with prologue of loop C */
            checksum += m;
        } else {
            dummy4(m);  /* Unique block */
            checksum -= m;
        }
        asm volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    dummy1(result);
    
    return result % 256;
}
