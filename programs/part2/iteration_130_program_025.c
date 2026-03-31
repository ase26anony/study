/* Complex type declarations with deeply nested delimiters */

/* 1. Function pointers with complex signatures */
int (*(*complex_callback)(int (*)(float)))[10];

/* 2. Multi-dimensional arrays with initializers */
int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};

/* 3. Structures with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* 4. Type definitions with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);

/* 5. Complex nested function pointer type */
typedef void (*(*(*nested_fp)(int, ...))(double))(char);

/* 6. Array of function pointers returning pointers to arrays */
int (*(*func_array[3])(int))[5];

/* 7. Structure with all delimiter types mixed */
struct MixedDelimiters {
    int (*fp)(int (*)[10], char (*)(void));
    struct {
        int data[2][3];
        union {
            char *ptr;
            void (*callback)(int, ...);
        } u;
    } nested;
};

/* 8. GCC attributes with nested parentheses */
struct __attribute__((aligned(32))) AlignedStruct {
    int data;
} __attribute__((packed));

int __attribute__((format(printf, 2, 3))) 
format_func(char *buf, const char *fmt, ...);

/* 9. Conditional compilation with macros */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])

#ifdef COMPLEX_TYPES
MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);
#endif

/* 10. Compound type in sizeof */
size_t get_size(void) {
    return sizeof(int (*(*)[10])(char (*)[5]));
}

/* 11. Cast expression with nested delimiters */
void *complex_cast = (void (*)(int (*(*)[5])(void)))0x1000;

/* 12. Nested initializer with all delimiter types */
struct NestedInit {
    int arr[2][3];
    struct {
        int a;
        int b;
    } s;
    union {
        int x;
        float y;
    } u;
} nested_var = {
    .arr = {{1,2,3}, {4,5,6}},
    .s = {7, 8},
    .u = {.x = 9}
};

/* 13. Function with complex parameter */
void process_complex(int (*(*callback)(int (*arr)[10]))[5],
                     struct Outer *out) {
    /* Use parameters to avoid unused warnings */
    (void)callback;
    (void)out;
}

/* 14. Variable with attribute and initializer */
int __attribute__((aligned(16))) aligned_array[4] = {1, 2, 3, 4};

/* Minimal main function */
int main(void) {
    /* Declare variables of complex types */
    array_of_5_funcs funcs = {NULL, NULL, NULL, NULL, NULL};
    
    /* Use sizeof on complex types */
    size_t s1 = sizeof(int (*(*)[10])(void));
    size_t s2 = sizeof(struct MixedDelimiters);
    
    /* Create compound literal */
    struct Outer temp = {
        .c = 42,
        .func_ptr_arr = {NULL, NULL}
    };
    
    /* Use the variables to avoid warnings */
    (void)funcs;
    (void)s1;
    (void)s2;
    (void)temp;
    (void)complex_callback;
    (void)matrix;
    (void)nested_var;
    (void)aligned_array;
    
    return 0;
}
