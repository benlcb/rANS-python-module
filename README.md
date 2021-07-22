# rANS-python-module
Asymmetric Numeral System Coder (range variant) implemented in C++; 
It can be loaded as a Python module with Boost.

# Dependencies

Boost is required to build the project as a Python Module. 
If you are compiling under Ubuntu, you can install it easily with  
`sudo apt install libboost-all-dev`

# Compiling
`$ cmake CMakeLists.txt`  
`$ make`

# Demo

The Jupyter Notebook file contains a demo that will show you how to use this module for encoding and decoding.

# Todo
- Package and release to pip (deal with boost dependency)
- Add GPU support 
