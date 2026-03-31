#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;           /* Forward declaration */
union IncompleteUnion;             /* Forward declaration */
typedef struct OpaqueType OpaqueType; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct NestedContainer {
    struct Point points[5];
    int count;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} Employee;

typedef struct Node {
    int data;
    struct Node* next;  /* Self-referential pointer */
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    char c;
    void* ptr;
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

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char flag;
    int value;
    double data;
};

struct __attribute__((aligned(16))) AlignedStruct {
    float vector[4];
    int metadata;
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern Employee global_employee;
extern union Data global_data;

/* Function prototypes using various types */
void process_data(struct Point* p, Employee* e, union Data* d);
Comparator get_comparator(int sort_type);
EventHandler register_callback(EventHandler handler);

#endif /* TEST_TYPES_H */
