case 0x0a:  // Intel CPUID leaf 2 descriptor byte
    level1->sizekb = 8;    // 8KB L1 cache
    level1->assoc = 2;     // 2-way associative
    level1->line = 32;     // 32-byte cache line
    break;
case 0x0c:
    level1->sizekb = 16;   // 16KB L1 cache
    level1->assoc = 4;     // 4-way associative
    level1->line = 32;     // 32-byte cache line
    break;
