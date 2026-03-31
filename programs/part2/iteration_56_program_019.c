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

struct ComplexData {
    struct Point location;
    double timestamp;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} UserStruct;

typedef struct Node {
    int value;
    struct Node* next;
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char c;
};

union TaggedUnion {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
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

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* External declarations for multi-file testing */
extern struct IncompleteStruct* global_incomplete_ptr;
extern UserStruct global_user_struct;

#endif /* TEST_TYPES_H */
