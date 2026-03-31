// Creating a pointer to integer type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *int_ptr = malloc(sizeof(struct type));
int_ptr->kind = TYPE_POINTER;
int_ptr->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = malloc(sizeof(struct type));
string_type->kind = TYPE_STRING;

struct type *string_array = malloc(sizeof(struct type));
string_array->kind = TYPE_ARRAY;
string_array->u.element_type = string_type;
