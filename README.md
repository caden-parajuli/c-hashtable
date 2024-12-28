# C-hashtable

A string-key hashtable implemented in C. This was originally created for use in the symbol table of a compiler, but can be used for any purpose.

## Implementation Details

The hashtable uses an FNV-1a hash, Robin Hood linear probing, and backward shifting deletion.
