GPU/Accelerator
├── Team 0 (handles j=0..k)
│   ├── Threads execute SIMD operations on i=0..M
│   └── Vector length up to 16
├── Team 1 (handles j=k+1..l)
│   └── ...
└── Team n (handles remaining j)
