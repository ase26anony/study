// Example of what might be in those functions:
void test_invalid_address_of_bitfield() {
    struct S {
        unsigned int bf : 4;  // bitfield
    } s;
    
    // Taking address of bitfield is invalid in standard C
    // unsigned int* ptr = &s.bf;  // ERROR
}

void test_incomplete_sizeof() {
    struct incomplete;  // Forward declaration, incomplete type
    
    // sizeof(incomplete) would be invalid
    // size_t size = sizeof(struct incomplete);  // ERROR
}
