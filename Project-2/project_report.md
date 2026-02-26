## Project Overviewn and Design

The goal of this project is to create a unix pipe based word counting tool that's written in C. 

The tar submission includes this report and the DFD which was handdrawn on a paper. 

When it comes to design issues, I had to ensure that the pipes don't go into deadlock because a process blocks indefinitelty while waiting for data or EOF signal. 

I ensured that each process closes all unused pipes immediately after `fork()`

The parent process closes the write-end of the first pipe after sending the entire file, which then allows the child process to detect EOF and terminate the the read loop correctly. 

I understand why the producer-consumer works here because the parent performs the file I/O (producer) and the child performs computation (consumer). 

Closing unused pipe ends prevents deadlocks and EOF is delivered quickly. Chunk-based streaming avoids large-memory dependence and would support both small and large files. 

## Function Prototypes

``` c
void print_usage(void);

int count_words_in_bytes(const unsigned char *buf,
                          ssize_t n,
                          int *in_word);

int write_all(int fd,
              const void *data,
              size_t len);

ssize_t read_retry(int fd,
                   void *buf,
                   size_t len);
```

`print_usage()` - Prints the usage instructions when no input file is provided

`count_words_in_bytes()` - Counts the words in a buffer using whitespace separation. A word is defined as a sequence of non-whitespace characters

`write_all()` - ensures all bytes are written to a pipe, handling partial writes. 

`read_retry()` - handles interrupted system calls by retrying `read()` when needed. 


## Data Structures

Nothing crazy other than a Charcter Buffer and Integer count in 

```c
unsigned char buffer[4096]; // for reading file chunks and pipe communication 

int total_words; // this stores the final word count result passed between processes
```

## Sample Input and Output

`input.txt`

### Input

```
This is the second project of COMP7500 class.
There was a lot going on so i could not submit the project on time.

Tabs	should	also	count	as	words.
Newlines
should
not
break
anything.

End.
```

### Output

```
Process 1 is reading file "input.txt" now ...
Process 1 starts sending data to Process 2 ...
Process 2 finishes receiving data from Process 1 ...
Process 2 is counting words now ...
Process 2 is sending the result back to Process 1 ...
Process 1: The total number of words is 35.
```
