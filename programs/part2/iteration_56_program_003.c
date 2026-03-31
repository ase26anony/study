#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

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
    int int_val;
    float float_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(2))) PackedGlobal {
    char flag;
    int counter;
    double value;
};

/* TYPE_ARRAY: Global arrays */
extern int global_int_array[100];
extern UserStruct global_struct_array[10];

/* TYPE_POINTER: Global pointers */
extern struct GlobalPoint *global_point_ptr;
extern void **global_void_ptr_ptr;

/* TYPE_STRING: String declarations */
extern const char *global_const_string;
extern char global_char_array[];

#endif /* TEST_TYPES_H */
