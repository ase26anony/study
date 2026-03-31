#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
void undefined_function(void); /* void return type */

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern _Bool global_bool;

/* TYPE_STRING: String types */
extern char *global_string;
extern const char *global_const_string;
extern char global_char_array[];

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char *str;
};

/* TYPE_POINTER: Various pointer types */
typedef int *IntPtr;
typedef void *VoidPtr;
typedef struct Point *PointPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef float Matrix[5][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidCallback)(void);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* Complex nested type for recursive analysis */
typedef struct ComplexNode {
    int value;
    struct ComplexNode *next; /* Pointer to incomplete type */
    void (*processor)(struct ComplexNode *);
    union {
        int int_data;
        float float_data;
    } data_union;
} ComplexNode;

/* External declarations for multi-file testing */
extern void process_types(void);
extern int compute_checksum(void);

#endif /* TEST_TYPES_H */
