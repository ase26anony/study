GPU Hierarchy:
└── Gang (thread block)
    ├── Worker (warp)
    │   └── Vector (threads within warp)
    ├── Worker (warp)
    │   └── Vector (threads within warp)
    └── ...
