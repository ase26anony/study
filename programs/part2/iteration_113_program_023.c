switch (descriptor_byte) {
    case 0x0a:
        level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
        break;
    case 0x0c:
        level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
        break;
    // ... more L1 cases
    case 0x86:
        level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
        break;
    case 0x87:
        level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
        break;
    // ... more L2/L3 cases
}
