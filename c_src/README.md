# Polyedr C Translation

This directory contains a complete C translation of the Python polyedr project for 3D polyhedron projection with shadow computation.

## Files

### Core Library

- **r3.h** - 3D vector operations (R³ space)
  - Vector arithmetic (add, subtract, multiply)
  - Rotations around Oz and Oy axes
  - Dot and cross products

- **segment.h** - One-dimensional segment operations
  - Segment creation and degeneracy testing
  - Segment intersection
  - Segment subtraction (set difference)

- **polyedr.h/polyedr.c** - Polyhedron data structures and algorithms
  - `Edge` - Edge with gap tracking for shadow computation
  - `Facet` - Polygonal face with normal calculations
  - `Polyedr` - Complete polyhedron with vertices, edges, and facets
  - File loading from `.geom` format
  - Shadow computation algorithm

- **tk_drawer.h/tk_drawer.c** - X11-based graphics rendering
  - Window creation and management
  - Line drawing with coordinate transformation

### Executables

- **run_shadow.c** - Main program with shadow computation (equivalent to `run_shadow.py`)
- **run_noshadow.c** - Simplified version without shadows (equivalent to `run_noshadow.py`)
- **test_polyedr.c** - Comprehensive unit tests

## Building

```bash
cd c_src
make all      # Build everything
make test     # Build and run tests
make clean    # Remove build artifacts
```

### Requirements
- GCC with C99 support
- X11 development libraries (`libx11-dev`)
- Math library (linked automatically)

## Running

### With Shadows
```bash
./run_shadow
```
Loads polyhedra from `../data/*.geom` files and renders them with shadow computation.

### Without Shadows
```bash
./run_noshadow
```
Renders polyhedra without shadow computation (faster, simpler visualization).

### Tests
```bash
./test_polyedr
```
Runs 51 unit tests covering:
- Segment operations (degeneracy, intersection, subtraction)
- Edge operations (3D coordinate conversion, half-space intersection)
- Facet operations (verticality, normals, center calculation)
- Polyhedron loading and shadow computation

## Key Differences from Python Version

### Memory Management
- Explicit allocation/deallocation with `malloc`/`free`
- Dynamic arrays for edges, gaps, and facet vertices
- Proper cleanup via `*_free()` functions

### Performance Optimizations
- Inline functions for simple vector operations
- Pre-allocated capacities for dynamic arrays
- Direct struct access instead of property methods

### API Changes
- Constructor functions instead of `__init__`
- Explicit `V` (projection vector) parameter passed to functions
- Return-by-value for small structs (R3, Segment)
- Pointer parameters for mutable operations

## Data Format (.geom files)

```
<scale> <alpha> <beta> <gamma>
<num_vertices> <num_facets> <num_edges>
<x> <y> <z>  (repeated for each vertex)
<num_vertices_in_facet> <v1> <v2> ... (repeated for each facet)
```

Where:
- `scale` - Homothety coefficient
- `alpha, beta, gamma` - Euler angles in degrees
- Vertex indices in facet definitions are 1-based

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                   Application Layer                  │
│  run_shadow.c / run_noshadow.c / test_polyedr.c     │
└─────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────┐
│                   Polyedr Layer                      │
│              polyedr.h / polyedr.c                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐   │
│  │   Edge   │  │  Facet   │  │    Polyedr       │   │
│  │ +gaps[]  │  │+vertexes │  │ +vertexes[]      │   │
│  └──────────┘  └──────────┘  │ +edges[]         │   │
│                               │ +facets[]        │   │
│                               └──────────────────┘   │
└─────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────┐
│                   Geometry Layer                     │
│    r3.h           │          segment.h              │
│  ┌──────────┐     │      ┌──────────────────┐       │
│  │   R3     │     │      │    Segment       │       │
│  │ x, y, z  │     │      │ beg, fin         │       │
│  └──────────┘     │      └──────────────────┘       │
└─────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────┐
│                   Rendering Layer                    │
│            tk_drawer.h / tk_drawer.c                 │
│              (X11-based graphics)                    │
└─────────────────────────────────────────────────────┘
```

## License

Translation of the original Python project to C.
