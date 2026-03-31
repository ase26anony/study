#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} UserStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    char c;
    void *p;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(64))) AlignedStruct {
    int data[16];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* Complex nested type for thorough testing */
typedef struct ComplexNode {
    void *data;
    struct ComplexNode **children;  /* TYPE_POINTER to TYPE_POINTER */
    int (*compare)(struct ComplexNode *, struct ComplexNode *);  /* TYPE_CALLBACK */
    union {
        int int_val;
        double dbl_val;
    } value;
} ComplexNode;

/* External declarations for multi-file testing */
extern struct Point global_point;
extern UserStruct global_user_struct;

#endif /* TYPES_H */
