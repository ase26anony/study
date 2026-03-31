// Create a pointer to int
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *ptr_type = malloc(sizeof(struct type));
ptr_type->kind = TYPE_POINTER;
ptr_type->u.pointer_to = int_type;

// Create an array of pointers
struct type *arr_type = malloc(sizeof(struct type));
arr_type->kind = TYPE_ARRAY;
arr_type->u.element_type = ptr_type;
