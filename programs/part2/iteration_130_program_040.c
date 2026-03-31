#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer type with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int matrix_t[3][4];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        long d;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* Function pointer array type */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])

/* Attribute with deeply nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))

/* Complex structure with attributes */
struct ComplexStruct {
    int (*(*callback)(struct ComplexStruct *self, 
                      int (*(*nested)(int, ...))(void)))(void);
    long data[10][20];
} ALIGNED_ATTR PACKED_ATTR;

/* Function declarations */
void process_matrix(int matrix[][4], int rows);
complex_callback create_callback(void);

#endif /* COMPLEX_TYPES_H */
