/* reload_coverage.c - Comprehensive test to trigger various reload types in GCC */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 3, vi2 = 7, vi3 = 11;
volatile long vl1 = 5, vl2 = 9;

/* ==================== Pattern 1: Complex Addressing Modes ==================== */
void complex_addressing(int n) {
    /* Large multi-dimensional array with non-constant indices */
    int arr[100][50];
    static int counter = 0;
    
    /* Force address reloads with complex index calculations */
    for (int i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr[vi1 + i][vi2 * 2] = arr[vi3][i + vl1] + arr[i][vi1 * vi2];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr[vi1][i];
        int *ptr2 = &arr[i][vi2];
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        *((int*)((char*)ptr1 + vi3 * sizeof(int))) = 
            *((int*)((char*)ptr2 + vl2 * sizeof(int))) + counter++;
    }
}

/* ==================== Pattern 2: Structure Passing ==================== */
struct SmallStruct {
    int a, b, c, d;
};

struct MediumStruct {
    int data[8];
    char padding[7]; /* Uneven size to complicate alignment */
};

/* Chain of functions passing/returning structures by value */
struct SmallStruct func1(struct SmallStruct s) {
    s.a += vi1;
    s.b ^= vi2;
    return s;
}

struct SmallStruct func2(struct SmallStruct s) {
    s.c *= vi3;
    s.d -= vl1;
    return s;
}

struct MediumStruct func3(struct MediumStruct m) {
    for (int i = 0; i < 8; i++) {
        m.data[i] += i * vi1;
    }
    return m;
}

void structure_ops(void) {
    struct SmallStruct s1 = {1, 2, 3, 4};
    struct MediumStruct m1;
    
    /* Chain structure operations - forces address reloads for temporaries */
    /* RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    s1 = func2(func1(s1));
    
    for (int i = 0; i < 8; i++) m1.data[i] = i * 10;
    m1 = func3(m1);
    
    /* Use results to prevent optimization */
    vi1 = s1.a + m1.data[0];
}

/* ==================== Pattern 3: Inline Assembly ==================== */
void asm_reload_patterns(void) {
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int result1, result2, result3;
    
    /* Chain of inline asm with register/memory constraints */
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result1)
        : "r" (a), "r" (b)
        : "%eax", "memory"
    );
    
    /* Memory operand with complex addressing */
    int mem_buffer[10];
    for (int i = 0; i < 10; i++) mem_buffer[i] = i * 100;
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        : "+r" (result1)
        : "m" (mem_buffer[vi1]), "r" (c)
        : "cc"
    );
    
    /* Multiple output operands */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        "leal (%3, %2, 4), %1\n\t"
        : "=&r" (result2), "=&r" (result3)
        : "r" (a), "r" (b), "r" (d)
        : "cc"
    );
    
    vi2 = result1 + result2 + result3;
}

/* ==================== Pattern 4: Vector Extensions ==================== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v8hi vec3 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector operations that may need decomposition */
    v4si result;
    
    /* Complex vector operation with shuffle */
    result = vec1 + vec2 * (v4si){vi1, vi2, vi3, vl1};
    
    /* Use __builtin_shuffle for non-contiguous access */
    v4si shuffled = __builtin_shuffle(vec1, vec2, 
        (v4si){1, 3, 0, 2});
    
    /* Store to memory with complex addressing */
    v4si memory[4];
    memory[vi1 & 3] = result + shuffled;
    
    /* Extract elements to force register pressure */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += result[i] + shuffled[i];
    }
    vi3 = sum;
}

/* ==================== Pattern 5: Control Flow with Live Range Splitting ==================== */
void control_flow_reloads(int n) {
    int x = 0, y = 0, z = 0;
    int array[100];
    
    /* Initialize array with volatile-dependent values */
    for (int i = 0; i < 100; i++) {
        array[i] = i * vi1;
    }
    
    /* Complex control flow with goto */
    int i = 0;
    start_loop:
    if (i >= n) goto end_loop;
    
    /* Values defined here used in distant blocks */
    x = array[i] + vi2;
    y = array[i + 1] * vi3;
    
    if (i % 3 == 0) {
        goto case0;
    } else if (i % 3 == 1) {
        goto case1;
    } else {
        goto case2;
    }
    
    case0:
    /* Use x and y here - live ranges cross goto boundaries */
    z = x + y + array[vi1];
    goto loop_end;
    
    case1:
    z = x - y + array[vi2];
    goto loop_end;
    
    case2:
    z = x * y + array[vi3];
    /* fall through */
    
    loop_end:
    array[i] = z;
    i++;
    goto start_loop;
    
    end_loop:
    /* Use results */
    vl1 = array[0] + array[n-1];
}

/* ==================== Pattern 6: Mixed Operations ==================== */
void mixed_operations(void) {
    /* Large local structure */
    struct {
        int data[64];
        char padding[31];
    } big_struct;
    
    /* Initialize with volatile values */
    for (int i = 0; i < 64; i++) {
        big_struct.data[i] = i * vi1 + vi2;
    }
    
    /* Pointer arithmetic with different types */
    char *char_ptr = (char*)big_struct.data;
    int *int_ptr = (int*)(char_ptr + vi3);
    
    /* Complex memory access pattern */
    for (int i = 0; i < 10; i++) {
        /* RELOAD_FOR_OTHER_ADDRESS types */
        int index = (i * vi1 + vi2) & 63;
        big_struct.data[index] = 
            *int_ptr + 
            *(int*)(char_ptr + index * sizeof(int) + vl1);
        int_ptr = (int*)((char*)int_ptr + vl2);
    }
}

/* ==================== Main Orchestrator ==================== */
int main(void) {
    int checksum = 0;
    
    printf("Starting reload coverage test...\n");
    
    /* Execute all patterns */
    complex_addressing(20);
    structure_ops();
    asm_reload_patterns();
    vector_ops();
    control_flow_reloads(50);
    mixed_operations();
    
    /* Compute checksum from volatile variables */
    checksum = vi1 + vi2 + vi3 + vl1 + vl2;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
