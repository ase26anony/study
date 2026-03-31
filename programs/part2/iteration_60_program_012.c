#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep pointer arithmetic intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    // Force address calculation through pointer arithmetic
    // This should create MEM with REG0 as the base pointer
    while (ptr < end) {
        // Access via pointer dereference
        sum += *ptr;
        // Immediate increment after access - target for auto-inc-dec
        ptr++;
        
        // Complex addressing that might confuse pattern matcher
        if ((ptr - array) % 2 == 0) {
            // Alternate form: *(ptr + 0) which should simplify but requires analysis
            sum += *(ptr + 0);
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
    int sum = 0;
    
    // Use both array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array index form
        sum += array[i];
        
        // Convert to pointer and use offset
        int *ptr = &array[i];
        sum += *(ptr + 0);  // XEXP should extract the base register from this
        
        // Force non-trivial address expression
        volatile int *vptr = ptr;
        sum += *vptr;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_structure_pointer_walk(int n) {
    struct Data {
        int a;
        int b;
        short c;
        char d;
    };
    
    // Dynamically allocate to prevent stack optimization
    struct Data *data = malloc(n * sizeof(struct Data));
    if (!data) return 0;
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        data[i].a = i;
        data[i].b = i * 2;
        data[i].c = i % 100;
        data[i].d = i % 256;
    }
    
    long long total = 0;
    struct Data *ptr = data;
    struct Data *end = data + n;
    
    // Walk through structure array with pointer increment
    while (ptr < end) {
        // Multiple member accesses with same base pointer
        total += ptr->a;
        total += ptr->b;
        total += ptr->c;
        total += ptr->d;
        
        // Pointer increment after last access - target for pattern matching
        ptr++;
        
        // Additional offset calculation that might trigger XEXP analysis
        if ((ptr - data) % 3 == 0) {
            struct Data *temp = ptr;
            total += temp->a;  // Another MEM access with same base
        }
    }
    
    free(data);
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_multiple_pointer_types(int n) {
    // Different memory access sizes
    char *cptr = malloc(n * sizeof(char));
    short *sptr = malloc(n * sizeof(short));
    int *iptr = malloc(n * sizeof(int));
    
    if (!cptr || !sptr || !iptr) {
        free(cptr);
        free(sptr);
        free(iptr);
        return 0;
    }
    
    // Initialize
    for (int i = 0; i < n; i++) {
        cptr[i] = i % 128;
        sptr[i] = i % 32768;
        iptr[i] = i;
    }
    
    int sum = 0;
    
    // Process char array with pointer arithmetic
    char *cp = cptr;
    for (int i = 0; i < n; i++) {
        sum += *cp;
        cp++;  // char pointer increment
    }
    
    // Process short array with mixed forms
    short *sp = sptr;
    for (int i = 0; i < n; i++) {
        // Use offset form: *(sp + 0)
        sum += *(sp + 0);
        sp += 1;  // short pointer arithmetic
    }
    
    // Process int array with complex addressing
    int *ip = iptr;
    volatile int *vip = ip;  // volatile to prevent simplification
    for (int i = 0; i < n; i++) {
        // This creates a MEM with potentially complex address
        sum += *vip;
        ip++;
        vip = ip;  // Update volatile pointer
    }
    
    free(cptr);
    free(sptr);
    free(iptr);
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_copy(int n) {
    int *src = malloc(n * sizeof(int));
    int *dst = malloc(n * sizeof(int));
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return 0;
    }
    
    // Initialize source
    for (int i = 0; i < n; i++) {
        src[i] = i * 3;
    }
    
    // Classic pointer copy pattern that should trigger auto-inc-dec
    int *s = src;
    int *d = dst;
    int *end = src + n;
    
    while (s < end) {
        *d = *s;  // Memory access with two different bases
        s++;      // Source pointer increment
        d++;      // Dest pointer increment
    }
    
    // Verify copy
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += dst[i];
    }
    
    free(src);
    free(dst);
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_base_calculation(int n) {
    int *array = malloc(n * sizeof(int));
    if (!array) return 0;
    
    for (int i = 0; i < n; i++) {
        array[i] = i * 7;
    }
    
    int sum = 0;
    
    // Create complex base calculation that might simplify to base+0
    int *base1 = array;
    int *base2 = array + n/2;
    
    for (int i = 0; i < n/2; i++) {
        // Expression: *(base1 + (base2 - base2)) which should be *(base1 + 0)
        // But requires analysis to simplify
        int offset = base2 - base2;  // Should be 0, but not compile-time constant
        sum += *(base1 + offset);
        
        // Another form: base1[base2 - base2]
        sum += base1[base2 - base2];
        
        base1++;
    }
    
    free(array);
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  // Ensure reasonable size
    
    // Allocate and initialize test array
    int *test_array = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        test_array[i] = i * 2 + 1;
    }
    
    int total = 0;
    
    // Run all test patterns
    total += test_pointer_increment_sum(test_array, n);
    total += test_mixed_index_pointer(test_array, n);
    total += test_pointer_copy(n);
    
    // These use their own allocations
    total += test_multiple_pointer_types(n % 50 + 10);
    total += test_complex_base_calculation(n);
    
    long long struct_result = test_structure_pointer_walk(n % 30 + 10);
    total += (int)struct_result;
    
    free(test_array);
    
    printf("Result: %d\n", total);
    return 0;
}
