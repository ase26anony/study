/* Test case to trigger min-cost flow fixup graph debug dumping in GCC IRA.
   Compile with a debug-built GCC (configured with --enable-checking) using:
     gcc-debug -O2 -march=armv7-a -c test.c -o test.o
   The coverage occurs during compilation, not at runtime. */

/* Force use of priority-based IRA algorithm for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create high register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Clobber many ARM registers to increase perceived pressure */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Complex control flow with many simultaneously live variables */
    /* First basic block: use all variables in computations */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 ^ v12;
    v13 = v14 | v15;
    v16 = v1 + v4;
    
    /* Branch point creating multiple live ranges across blocks */
    if (v1 > v4) {
        /* Block A: different subset of variables used */
        v2 = v3 + v7;
        v5 = v6 * v10;
        v8 = v9 - v13;
        v11 = v12 ^ v16;
        v14 = v15 | v1;
        
        /* More computations to extend liveness */
        v3 = v2 + v5;
        v6 = v8 * v11;
        v9 = v14 - v16;
        v12 = v1 ^ v4;
        v15 = v7 | v10;
        
        /* Clobber registers again */
        asm volatile("" : : : "memory");
    } else {
        /* Block B: another subset of variables used */
        v3 = v4 + v8;
        v6 = v7 * v11;
        v9 = v10 - v14;
        v12 = v13 ^ v1;
        v15 = v16 | v2;
        
        /* Different computation pattern */
        v4 = v3 + v6;
        v7 = v9 * v12;
        v10 = v15 - v2;
        v13 = v5 ^ v8;
        v16 = v11 | v14;
        
        /* Clobber registers */
        asm volatile("" : : : "memory");
    }
    
    /* Merge point: use all variables again to ensure they're live across edges */
    v1 = v1 + v2 + v3 + v4;
    v5 = v5 * v6 * v7 * v8;
    v9 = v9 - v10 - v11 - v12;
    v13 = v13 ^ v14 ^ v15 ^ v16;
    
    /* Final computations mixing all variables */
    v2 = v1 + v5;
    v6 = v9 * v13;
    v10 = v2 - v6;
    v14 = v10 ^ v1;
    
    /* Another branch to create more complex CFG */
    switch (v14 & 3) {
        case 0:
            v3 = v7 + v11;
            v15 = v4 * v8;
            break;
        case 1:
            v3 = v12 - v16;
            v15 = v5 ^ v9;
            break;
        case 2:
            v3 = v6 | v13;
            v15 = v10 + v14;
            break;
        default:
            v3 = v2 * v7;
            v15 = v11 - v16;
            break;
    }
    
    /* Use results to prevent dead code elimination */
    asm volatile("" : : "r"(v3), "r"(v15) : "memory");
}

/* Trivial main function to make the file compilable */
int main(void) {
    return 0;
}
