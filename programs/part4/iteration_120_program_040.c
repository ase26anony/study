/* Test program for hw-doloop.cc uncovered lines 429-436
 * Creates nested loops with partial block overlap to trigger bitmap intersection logic
 */

/* Prevent optimization of dummy functions */
#define NOOPT __attribute__((noinline, noclone))

/* Memory barrier to prevent loop fusion */
#define BARRIER() asm volatile("" : : : "memory")

/* Dummy functions to create unique basic blocks */
NOOPT void dummy1(int x) { BARRIER(); }
NOOPT void dummy2(int x) { BARRIER(); }
NOOPT void dummy3(int x) { BARRIER(); }
NOOPT void dummy4(int x) { BARRIER(); }
NOOPT void dummy5(int x) { BARRIER(); }

int main(void) {
    /* Volatile variables to prevent constant propagation */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        BARRIER();
        
        /* This if-else creates multiple basic blocks within outer loop */
        if (cond) {
            /* Branch 1: Contains INNER LOOP A */
            
            /* Prologue block that will be shared with inner loop A */
            volatile int inner_a_setup = i * 2;
            dummy2(inner_a_setup);
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int j = 0; j < M1; ++j) {
                dummy3(j + i);
                checksum += j;
                
                /* This goto creates partial overlap:
                 * Jumps to a block that's in outer loop but outside the if branch */
                if (j == M1/2) {
                    goto shared_block;
                }
            }
            
            /* Continuation after inner loop A (only if goto not taken) */
            dummy4(i * 3);
        } else {
            /* Branch 2: Contains INNER LOOP B */
            
            /* Same prologue as inner loop A - creates shared basic block */
            volatile int inner_b_setup = i * 2;
            dummy2(inner_b_setup);
            
            /* INNER LOOP B - distinct from A but shares prologue */
            for (volatile int k = 0; k < M2; ++k) {
                dummy5(k - i);
                checksum -= k;
            }
            
            dummy4(i * 4);
        }
        
        /* Shared block that both inner loops can reach
         * This creates partial overlap: inner loop A's blocks are partially
         * inside outer loop's if branch, partially in this shared block */
        shared_block:
        dummy1(i + 100);
        checksum += i;
        
        /* Change condition to ensure both branches are taken */
        cond = !cond;
        BARRIER();
    }
    
    /* Reset shared prologue */
    shared_counter = 1;
    dummy1(shared_counter);
    
    /* SIBLING LOOP C - shares prologue with inner loops but has different body */
    for (volatile int l = 0; l < M3; ++l) {
        /* Same prologue as inner loops A and B */
        volatile int sibling_setup = l * 2;
        dummy2(sibling_setup);
        
        /* Different body to create distinct basic blocks */
        dummy3(l * 3);
        checksum += l * 2;
        BARRIER();
    }
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    
    /* Simple output to prevent optimization */
    if (result > 1000000) {
        dummy1(result);
    }
    
    return 0;
}
