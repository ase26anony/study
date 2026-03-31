// Creating a pointer to integer type
struct type *int_type = create_type(TYPE_SCALAR);
struct type *int_ptr = create_type(TYPE_POINTER);
int_ptr->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = create_type(TYPE_STRING);
struct type *string_array = create_type(TYPE_ARRAY);
string_array->u.element_type = string_type;

// Linking types in a list (e.g., function parameters)
struct type *param1 = create_type(TYPE_SCALAR);
struct type *param2 = create_type(TYPE_SCALAR);
param1->next = param2;
