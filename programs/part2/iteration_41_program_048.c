Device (GPU/Accelerator)
├── Multiple Teams (like thread blocks)
│   ├── Each team gets a chunk of j iterations
│   │   ├── Within each team: SIMD vectorization of i loop
│   │   │   ├── Vector length ≤ 16
│   │   │   └── Processes multiple i iterations simultaneously
