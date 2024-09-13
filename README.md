## spec 1

### maximum size of command is 4096 bytes and maximum size of working directory is 1024 bytes.

### After bckground process status is printed, DONOT PRESS ENTER and give command.

### TYPE "exit" TWICE TO EXIT THE SHELL.

## spec 2

### commands with both semicolon and ampersand have not been implemented (for part 1 submission)

### Edit: Its working!!

## spec 3

### hop .. , hop - , hop ../.. , hop , hop ., hop absolute path, hop relative path are implemented (including direct folder naem in current working directory)

## spec 4

### reveal also works for .. , . , ../.. , ~ , - , absolute and relative paths.(including direct folder naem in current working directory)

## spec 5

### commands with log are not stored.

## spec-6

### If erronous command is given for executing in the background,
-> It will be detected before fork if it has hop, log, proclore, reveal or seek
-> It will be detected during execution and that error will be printed.

## spec 7

### virtual memory prints size used in kB.

## spec-8

### All the files will be sorted and printed in that order. If subfolder (which is lexicographically before any file name with target) has a name matching with the target, first that will be printed before the filename.

### Given that executables will not be asked to search.

### if no file/folder is found, nothing is print

### Assumption: maximum size of a file is 4096 bytes. Do not give a file with more size 

## spec-9

### Implemented alias for reveall, reveala, home and functions for hop_seek and mk_hop.

## spec-10

### No assumptions made.

## spec-11

### maximum number of commands is 256

### maximum size of ezch command is 1024

### use "tr -d '[:space:]'" instead of sed command

## spec 12

### for writing to a file, printing contents of file and doing word count one after the other in piping and redirection continously, use this command instead " echo data | tee filename | wc "

### Maximum number of commands in piping is 256
