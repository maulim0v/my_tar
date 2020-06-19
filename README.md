# my_tar

## Command
./my_tar

## Description
Simple implementation of tar. my_tar is a command to manipulate tape archive. The first option to tar is a mode indicator from the following list:

...-c Create a new archive containing the specified items.
...-r Like -c, but new entries are appended to the archive. The -f option is required.
...-t List archive contents to stdout.
...-u Like -r, but new entries are added only if they have a modification date newer than the corresponding entry in the archive. The -f option is required.
...-x Extract to disk from the archive. If a file with the same name appears more than once in the archive, each copy will be extracted, with later copies overwriting (replacing) earlier copies.
In -c, -r, or -u mode, each specified file or directory is added to the archive in the order specified on the command line. By default, the contents of each directory are also archived.

Unless specifically stated otherwise, options are applicable in all operating modes:

...-C <directory>. In c and r mode, this changes the directory before adding the following files. In x mode,. change directories after opening the archive but before extracting entries from the archive..
...-f file Read the archive from or write the archive to the specified file. The filename can be standard input or standard output.

## Return
0 on success, and >0 if an error occurs.

## Resources
https://www.gnu.org/software/tar/manual/html_node/Standard.html

## Functions used
```
chmod
close
free
fstat
lseek
malloc
mkdir
open
opendir
read
readdir
readlink
stat
symlink
write
```
## Run
```
make
./my_tar
```

## Example
The following creates a new archive called file.tar that contains two files source.c and
source.h:
