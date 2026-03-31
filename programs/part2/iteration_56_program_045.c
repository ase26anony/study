#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct GlobalPoint {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[32];
} UserStruct;

/* TYPE_UNION: Union declarations */
union GlobalData {
    int i;
    float f;
    double d;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryOp)(int, int);
typedef void (*Callback)(void *data);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(2))) PackedGlobal {
    char flag;
    int value;
    double data;
};

/* External declarations for cross-file references */
extern struct GlobalPoint global_point;
extern UserStruct global_user_struct;

#endif /* TEST_TYPES_H */
