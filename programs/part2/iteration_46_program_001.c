int main() {
    struct Message msg = {
        .id = 1,
        .data = 0x0123456789ABCDEF0123456789ABCDEF  // 128-bit literal
    };
    
    process_msg(&msg);
    return 0;
}
