/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars to create various dependencies */
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    /* Main loop with complex dependency patterns */
    for (i = 0; i < SIZE; i++) {
        /* FLOW (RAW) dependency: a[i] depends on b[i] and c[i] */
        a[i] = b[i] + c[i];  /* MEM_DEP: array accesses */
        
        /* Another FLOW dependency using the computed value */
        x = a[i] * 2;        /* REG_DEP: scalar operation */
        
        /* ANTI (WAR) dependency: read then write to same location */
        y = a[i];            /* Read a[i] */
        if (i % 2 == 0) {    /* Control flow creates basic block boundary */
            /* OUTPUT (WAW) dependency: multiple writes to same location */
            a[i] = x + y;    /* Write a[i] - WAW with line 34 */
            
            /* Another FLOW dependency */
            z = a[i] + 1;    /* REG_DEP */
        } else {
            /* Different path with OUTPUT dependency */
            a[i] = y - 1;    /* Write a[i] - WAW with line 34 and 41 */
            
            /* FLOW dependency */
            z = a[i] * 3;    /* REG_DEP */
        }
        
        /* LOOP-CARRIED dependency: current iteration depends on previous */
        if (i > 0) {
            /* True loop-carried FLOW dependency with distance = 1 */
            b[i] = b[i-1] + a[i];  /* MEM_DEP with distance > 0 */
        }
        
        /* Nested loop to increase complexity */
        for (j = 0; j < 4; j++) {
            /* More dependencies in inner loop */
            c[j] = c[j] + i;      /* MEM_DEP */
            x = x + c[j];         /* REG_DEP */
        }
        
        /* OUTPUT dependency across loop iterations */
        if (i % 3 == 0) {
            a[i] = z;            /* WAW with previous a[i] writes */
        }
    }
    
    /* Second loop with different dependency pattern */
    for (i = 1; i < SIZE; i++) {
        /* Strong loop-carried dependency chain */
        a[i] = a[i-1] * 2 + b[i];  /* Distance = 1, MEM_DEP */
        
        /* Anti dependency in scalar */
        y = x;                     /* WAR: read x */
        x = i * 3;                 /* write x */
        
        /* Output dependency in scalar */
        z = y + 1;                 /* WAW with line 68 */
        if (i % 4 == 0) {
            z = x - 1;             /* WAW with line 68 */
        }
    }
    
    /* Prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i % 4];
    }
    
    /* Use result to prevent optimization */
    return sum % 100;
}
