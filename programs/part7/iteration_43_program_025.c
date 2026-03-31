#ifndef FILE1_H
#define FILE1_H

/* Parentheses: Complex function pointers */
typedef int (*simple_fp)(int);
typedef void (*(*nested_fp)(int (*)(char), double))(float);
typedef int (*(*signal_proto)(int, void (*)(int)))(int);

/* Brackets: Multi-dimensional and VLAs */
typedef int matrix_2d[10][20];
typedef int vla_matrix[][(sizeof(int) * 8)];
struct flexible_array {
    int len;
    int data[];
};

/* Braces: Nested structs with initializers */
struct Point3D {
    int x;
    struct {
        int y, z;
    } coord;
};

struct NestedInitializer {
    int a;
    int b[3];
    struct Point3D point;
};

/* Combined: Function pointer returning array pointer */
typedef int (*(*callback_fp)(void))[10];

/* Combined: Struct with array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*handlers[3])(struct Point3D*);
};

/* Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX_FP(5);

/* Deeply nested parentheses */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(char))[4];

/* Extern declarations for cross-file testing */
extern struct Operations global_ops;
extern callback_fp get_callback(void);

#endif /* FILE1_H */
