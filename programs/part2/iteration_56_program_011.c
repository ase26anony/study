#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;      /* Forward declaration */
union IncompleteUnion;        /* Forward declaration */
typedef struct Opaque Opaque; /* Opaque pointer type */

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char c;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);

/* Complex nested type for thorough testing */
struct ComplexContainer {
    void** data;                    /* TYPE_POINTER to TYPE_POINTER */
    int (*operations[10])(void*);   /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* External declarations for cross-file testing */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Data global_union;

#endif /* TEST_TYPES_H */
