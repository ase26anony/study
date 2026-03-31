/* test-mcf-debug.c
 * Designed to trigger debug dumping of MCF fixup graph with artificial nodes
 * Compile with: gcc-debug -O2 -c test-mcf-debug.c -o test.o
 * Or for ARM: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 */

/* Force priority-based IRA algorithm */
#ifdef __GNUC__
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define IRA_PRIORITY
#endif

/* Target ARM for limited registers */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Function to create high register pressure */
IRA_PRIORITY TARGET_ARM
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent optimization */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4));
    asm volatile("" : "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8));
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12));
    asm volatile("" : "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16));
    
    /* Complex control flow to create many basic blocks */
    /* Each branch uses different subsets of variables to keep them live */
    
    /* Block 1: Use first subset */
    v1 = v2 + v3;
    v4 = v1 * v5;
    v6 = v4 - v7;
    
    /* Force register clobbering for ARM */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                  "r8", "r9", "r10", "r11", "r12", "memory");
    
    /* Conditional branch 1 */
    if (v1 > 0) {
        /* Block 2: Use second subset, keep previous live */
        v8 = v9 + v10;
        v11 = v8 * v12;
        v13 = v11 - v14;
        
        /* More computation to increase pressure */
        v2 = v3 + v4;
        v5 = v6 * v7;
        
        asm volatile("" : : : "memory");
        
        /* Nested condition */
        if (v8 < v11) {
            /* Block 3: Use third subset */
            v15 = v16 + v1;
            v2 = v15 * v3;
            v4 = v2 - v5;
            
            v9 = v10 + v11;
            v12 = v9 * v13;
            
            asm volatile("" : : : "memory");
        } else {
            /* Block 4: Use fourth subset */
            v6 = v7 + v8;
            v9 = v6 * v10;
            v11 = v9 - v12;
            
            v13 = v14 + v15;
            v16 = v13 * v1;
            
            asm volatile("" : : : "memory");
        }
        
        /* Block 5: Merge point - use all variables */
        v1 = v2 + v3 + v4 + v5;
        v6 = v7 + v8 + v9 + v10;
        v11 = v12 + v13 + v14 + v15;
        v16 = v1 * v6 - v11;
        
    } else {
        /* Block 6: Alternative path */
        v2 = v3 * v4;
        v5 = v6 + v7;
        v8 = v9 - v10;
        v11 = v12 * v13;
        v14 = v15 + v16;
        
        asm volatile("" : : : "memory");
        
        /* Another condition */
        if (v2 != 0) {
            /* Block 7 */
            v3 = v4 + v5 + v6;
            v7 = v8 * v9 * v10;
            v11 = v12 - v13 - v14;
            
            asm volatile("" : : : "memory");
        } else {
            /* Block 8 */
            v15 = v16 + v1 + v2;
            v3 = v4 * v5 * v6;
            v7 = v8 - v9 - v10;
            
            asm volatile("" : : : "memory");
        }
        
        /* Block 9: Merge point */
        v12 = v13 + v14 + v15;
        v16 = v1 * v2 * v3;
        v4 = v5 + v6 + v7 + v8;
    }
    
    /* Final block: Force all variables to be live at exit */
    asm volatile("" 
                 : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4),
                   "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8),
                   "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12),
                   "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16)
                 : 
                 : "memory");
}

/* Second function with different pressure pattern */
IRA_PRIORITY TARGET_ARM
void another_high_pressure_function(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    asm volatile("" : "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4));
    asm volatile("" : "=r"(a5), "=r"(a6), "=r"(a7), "=r"(a8));
    asm volatile("" : "=r"(a9), "=r"(a10), "=r"(a11), "=r"(a12));
    asm volatile("" : "=r"(a13), "=r"(a14));
    
    /* Switch statement creates multiple basic blocks */
    switch (param & 7) {
        case 0:
            a1 = a2 + a3; a4 = a5 * a6; a7 = a8 - a9;
            break;
        case 1:
            a2 = a3 + a4; a5 = a6 * a7; a8 = a9 - a10;
            break;
        case 2:
            a3 = a4 + a5; a6 = a7 * a8; a9 = a10 - a11;
            break;
        case 3:
            a4 = a5 + a6; a7 = a8 * a9; a10 = a11 - a12;
            break;
        case 4:
            a5 = a6 + a7; a8 = a9 * a10; a11 = a12 - a13;
            break;
        case 5:
            a6 = a7 + a8; a9 = a10 * a11; a12 = a13 - a14;
            break;
        case 6:
            a7 = a8 + a9; a10 = a11 * a12; a13 = a14 - a1;
            break;
        default:
            a8 = a9 + a10; a11 = a12 * a13; a14 = a1 - a2;
            break;
    }
    
    /* Loop to increase pressure */
    for (int i = 0; i < 4; i++) {
        a1 = a2 + a3 + i;
        a4 = a5 * a6 * (i + 1);
        a7 = a8 - a9 - i;
        a10 = a11 + a12 + (i * 2);
        
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4");
    }
    
    /* Final use */
    asm volatile("" 
                 : "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4),
                   "=r"(a5), "=r"(a6), "=r"(a7), "=r"(a8),
                   "=r"(a9), "=r"(a10), "=r"(a11), "=r"(a12),
                   "=r"(a13), "=r"(a14)
                 : 
                 : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* Call functions to ensure they're not optimized away */
    high_pressure_function();
    another_high_pressure_function(42);
    return 0;
}
