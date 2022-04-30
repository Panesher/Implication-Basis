# Implication-Basis
## Files Info 
1. main.cpp - Parallel runner for computing Canonical Basis
2. lib - Library for computing canonicalc basis
3. toCXT.cpp - Code to convert a Context in the format accepted by main.cpp to the cxt format
3. fromCXT.cpp - Code to convert a Context in cxt format to the format accepted by main.cpp
4. StandardDS - Contains some real-world contexts
5. StandardDSCXT - Contains some real-world contexts in cxt format
6. ArtificialDS - Contains some artificial contexts
7. ArtificialDSCXT - Contains some artificial contexts in cxt format

## Usage

### main.cpp
- Compilation:
- - make derictory for cmake (recommend: `./build`, you can use your own)
- - `cmake path/to/source`
- - `make`
- Running:
- - `./algo.out <path/to/context.txt> <Epsilon> <Delta> <strong/weak> <uniform/frequent/area-based/squared-frequency> <number of threads> none <print-format:csv/csv-with-header/readable>`
- - Example: 
- - `./algo.out ../StandardDS/mushroom.txt 0.01 0.1 weak uniform 40 none readable`

### toCXT.cpp
- Compilation - `g++ -o toCXT toCXT.cpp`
- Running - `./toCXT <Input> > <Output>`

### fromCXT.cpp
- Compilation - `g++ -o fromCXT fromCXT.cpp`
- Running - `./fromCXT < <Input> > <Output>`
