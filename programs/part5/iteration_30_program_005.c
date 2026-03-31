#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop designed to generate RTL with:
     * 1. Memory load from address in psrc
     * 2. Memory store to address in pdst  
     * 3. Increment of both pointers
     * This should create candidate patterns for auto-inc-dec pass
     */
    for (int i = 0; i < SIZE; i++) {
        /* Memory access followed by pointer increment */
        *pdst = *psrc;
        
        /* Separate increment statements to keep pattern clear */
        pdst = pdst + 1;
        psrc = psrc + 1;
    }
    
    /* Second loop with different pattern to increase coverage */
    /* Reset pointers for another traversal */
    psrc = src;
    pdst = dst;
    
    /* Alternative: access with offset then increment */
    for (int i = 0; i < SIZE - 1; i++) {
        /* Access current and next element pattern */
        dst[i + 1] = src[i];
        
        /* This creates memory references with constant offset 0 */
        /* which may trigger the reg1_val = 0 initialization */
    }
    
    /* Third pattern: multiple independent access streams */
    int32_t tmp[SIZE];
    int32_t *p1 = src;
    int32_t *p2 = dst;
    int32_t *p3 = tmp;
    
    for (int i = 0; i < SIZE; i++) {
        /* Three memory operations in sequence */
        *p3 = *p1;
        *p2 = *p3;
        
        /* All pointers incremented separately */
        p1 = p1 + 1;
        p2 = p2 + 1;
        p3 = p3 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int32_t sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += dst[i];
    }
    
    /* Also use tmp array to ensure it's not optimized away */
    int32_t sum_tmp = 0;
    for (int i = 0; i < SIZE; i++) {
        sum_tmp += tmp[i];
    }
    
    printf("Checksum dst: %d\n", sum);
    printf("Checksum tmp: %d\n", sum_tmp);
    
    return sum > sum_tmp ? 0 : 1;
}
