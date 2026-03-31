#ifndef FILE1_H
#define FILE1_H

/* Parentheses: Complex function pointers */
typedef int (*fp_simple)(int);
typedef void (*(*fp_nested)(int (*)(char), double))(float);
typedef int (*(*signal_proto)(int, void (*)(int)))(int);

/* Brackets: Multi-dimensional and VLAs */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
struct vla_container {
    int len;
    int arr[];
};

/* Braces: Nested structs with initializers */
struct Point3D {
    struct { int x; int y; } coord;
    int z;
};

/* Combined: Function pointer returning array pointer */
typedef int (*(*callback_array)(void))[10];

/* Deep nesting: Array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced[2])(int))[3];
};

/* Macro generating complex types */
#define DECLARE_COMPLEX(n) int (*(*fp_vla##n)(int))[n]
DECLARE_COMPLEX(5);

/* Nested in struct with bitfields */
struct Container {
    union {
        struct {
            int (*handler)(int);
            int data[2][3];
        } s;
        long long raw;
    } u;
    unsigned flag : 1;
};

/* Extern declarations for cross-file testing */
extern matrix_t global_matrix;
extern struct Operations* get_operations(void);

#endif /* FILE1_H */
