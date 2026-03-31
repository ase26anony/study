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
    char *str;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char flag;
    int value;
    double data;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long a;
    long long b;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* Complex nested type declarations */
typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} ListNode;

/* External declarations for multi-file testing */
extern struct IncompleteStruct *global_incomplete_ptr;
extern UserStruct global_user_struct;

#endif /* TEST_TYPES_H */
