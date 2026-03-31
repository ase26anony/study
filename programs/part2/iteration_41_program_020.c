Device (GPU/Accelerator)
├── Multiple Teams (distributed across j iterations)
│   ├── Each team processes a chunk of j iterations
│   │   ├── Each thread in team processes one or more j iterations
│   │   │   ├── Inner i loop vectorized with SIMD (up to 16 elements at once)
