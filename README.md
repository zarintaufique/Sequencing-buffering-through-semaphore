# Sequencing-buffering-through-semaphore
Four processes are involved in printing files. Process A reads the data from the
first file to Buffer 1, Now Process B, will read the second file and store it in the Buffer 2. Process
C copies the data from Buffer 1 and Buffer 2 and places it in Buffer 3, finally Process D takes
the data from Buffer 3 and prints it onto the console. This task must be done using shared
memory. Write a program to coordinate the three processes using semaphores. Specially taking
care of completely reading from the file i.e. you must read from both the files into the buffers 1
and 2 in the shared memory and then start reading, take care of writing to buffer 3 when data
from both buffer 1 and buffer 2 has been read by process C.

Process A -> | Buffer 1 | ->
Read from File 1

Process C -> | Buffer 2 | -> Process D
Copy from buffers Print to console

Process D -> | Buffer 2 | ->
Read from File 2
