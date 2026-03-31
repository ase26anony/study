/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a ^ b) + (a & b) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        int offset = j * 2;
        
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables */
            int idx1 = i + offset;
            int idx2 = i - 1;
            int idx3 = i + j;
            
            /* Recurrence: arr1 depends on its previous value */
            int base = arr1[idx2];
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Complex expression with function call */
                arr1[idx1] = helper(arr2[idx3], base) + arr3[i];
            } else if (idx1 % 3 == 1) {
                /* Different arithmetic operation */
                arr1[idx1] = (arr2[idx3] * 2) - base + arr3[i];
            } else {
                /* Pointer arithmetic access */
                int *ptr = arr3 + i;
                arr1[idx1] = arr2[idx3] + base + *ptr;
            }
            
            /* Additional recurrence with arr3 */
            if (i > 2) {
                arr3[i] = arr3[i-1] + arr3[i-2] + (i % 5);
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr2[offset] = arr1[offset - 1] + 1;
        }
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-constant patterns */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 2 - 1;
        arr3[i] = i % 7;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n = (argc > 1) ? 200 : 250;
    int m = (argc > 2) ? 3 : 4;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    long sum = 0;
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use system call to avoid stdio includes) */
    if (sum != 0) {
        /* Simple output mechanism */
        const char *msg = "Checksum: ";
        char buf[20];
        int pos = 0;
        
        /* Convert sum to string */
        long val = sum;
        if (val < 0) {
            val = -val;
        }
        
        do {
            buf[pos++] = '0' + (val % 10);
            val /= 10;
        } while (val > 0 && pos < 19);
        
        buf[pos] = '\n';
        
        /* Write output */
        __asm__ __volatile__ (
            "mov $1, %%rax\n"
            "mov $1, %%rdi\n"
            "mov %0, %%rsi\n"
            "mov %1, %%rdx\n"
            "syscall"
            :
            : "r"(msg), "r"(10)
            : "rax", "rdi", "rsi", "rdx"
        );
    }
    
    return 0;
}
