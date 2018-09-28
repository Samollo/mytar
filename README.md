# mytar
An implementation of the tar command using C.


# INSTALL
Simply with a makefile:
```
make all
```

# RUN
The command has a particular format:
```
./mytar -opt [dst] [src]
```
There's 3 options with this command:

- -e to extract
- -a to create an archive
- -l to list the files into the tar


# TODO
This command still needs work. Errors may occur when trying to archive "big" files. Stay tuned..
