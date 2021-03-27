# Flood-fill

This open-source repository is a part of the (yet private) hd-simulator project and serves for finding connected components in 3D rectangular shapes.
The sequential implementation relies on DSU data structure and requires a recent version of boost library installed (for reading arguments and options from a command line).

## Usage

Check connectivity of cubic `sample.raw` of size `N`:

```bash
make
./flood-fill </path/to/sample.raw> <N> [--flow-dir <uint>] [--wall <uint8_t>]
```

The result (the largest connected component in the chosen flow direction) will be saved to 
`sample_1c{flow-dir-char}.raw`, in the same folder where `sample.raw` resides.

To find connected components of rectangular shapes, just pass dimension sizes (separated by spaces) after the input file.

Compile debug version of the program

```bash
make debug
make
gdb --args ./flood-fill <args>
```

One can always get back to release by typying `make release`. Note that it doesn't compile the code. It just changes `g++` set of options.

## TODO List

- [x] Sequential implementation
- [ ] Separate from the main project environment (Dockerfile)
- [ ] Parallel implementation
