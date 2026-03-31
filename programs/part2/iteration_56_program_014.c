#ifndef TEST_TYPES_H
#define TEST_TYPES_H

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
    char name[32];
} UserStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char c;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* Complex nested type declarations */
typedef struct Node {
    int value;
    struct Node *next;  /* TYPE_POINTER to incomplete type */
} ListNode;

/* Function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern UserStruct global_user_struct;

#endif /* TEST_TYPES_H */
