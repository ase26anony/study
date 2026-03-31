#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structure */
struct GlobalStruct {
    int id;
    float value;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    int counter;
    char name[32];
} UserStruct;

/* TYPE_UNION: Union declaration */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidCallback)(void);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char flag;
    int number;
    double precision;
};

/* External declarations for cross-file references */
extern struct GlobalStruct global_instance;
extern UserStruct user_instances[5];

#endif /* TEST_TYPES_H */
