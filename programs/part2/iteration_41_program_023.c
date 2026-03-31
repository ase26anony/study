Device (GPU/Accelerator)
├── Team 0
│   ├── Threads: process j = 0..k
│   │   └── SIMD lanes: process i = 0..15 simultaneously
├── Team 1
│   ├── Threads: process j = k+1..l
│   │   └── SIMD lanes: process i = 0..15 simultaneously
└── ...
