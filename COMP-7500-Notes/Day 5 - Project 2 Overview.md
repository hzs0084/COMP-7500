
Design Ideas for Project 2 on next Friday

1. Message Queue Data Structure
	- Send and Receive
	- Receive ID is going to be included

ipc is going to be the metadata
msg_stime - when the message is delivered. The reason we try to track the time is because of synchronization order
msg_rtime - receiver receiving time, what's the time that the message was received. 
msg_ctime - change time, the mode and the way the message was changed during it's configuration when it was moving through different modes
msg_qnom - number of messages in the queue
msg-qbytes - this is the maximum bytes that can be delivered.
<- msg_first - points to the first message in the queue

the msg_msg data structure will have *msg_next* to point to the next msg in the queue, and ^ is used to identify the last message

the msg_type - all messages have a data type 
msg_stime - for this specific message
msg_rtime - for this receive message

<-msg_last - is used to point to the last message in the queue

Linked Lists is used to efficiently manage 

In old days, it could only hold up to ...

2. Project 2: Pipe
	1. Learning Objectives: How to write multiple processes in C, fork 
	2. File I/Os
	3. Ordinary Pipe
	4. Use GDB to debug it
	5. To learn Unix Pipe as it was one of the first IPC mechanism

Make use of ordinary pipes enabling two processes to communicate in a producer-consumer fashion 

One process takes the data and the other returns the data back

Process 1 is going to retrieve the data and give it to the second process. 

Process 2 is going to count the words and then process 2 is going to give that data to process 1

Process 1 is dealing with input and output, it will print the output


Process 1 is going to be a parent
Process 2 is going to be a child

Process 1 is going to create Process 2. 

Exercise 1. We will implement the WordCount tool as a CLI (pwordcount). What is are the WordCount's argument?

- `./pwordcount input.txt` `pwordcount <file_name>`
- `pwordcount <file_name> <number_of_cores>` number of cores / number of children

Exercise 2. User input should be checked for errors. What error-checking cases should you consider in pwordcount?

- Only reads from .txt file
- Can only create up to 32 cores
- File doesn't exist
- No file name provided
- File can't open
- File isn't a standard text document

fork() : create a process
	The children is going to duplicate the image of the parent in terms of parent's executable code. 

