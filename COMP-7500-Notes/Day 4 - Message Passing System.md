Message Passing System

Shared ID
token

1. shmid_ds

ipc
shm_segs
...
shm_pages: pages is a pointer to memory. 
attach: processes: the processes that have access to those resources. 


All main memory is divided into pages, the main memory is organized in pages. 

The resource that is getting shared for everyone to use is shared memory. 

shm is a linked list of processes ??

shmat()?

str is application dependent

shmat - shared memory attach

shmdt - shared memory detach 

Exercise 2 (Review): Suppose two processes are exchanging a small amount of
data, which interprocess communication model will you choose? Why?
A. Shared Memory
B. Shared Buffer
C. Message Passing
D. Message Chatting

Answer: For small data when two processes are talking to each other, the setup time is quick with *message passing*, but shared memory is also right answer but that's for bigger data. 

Message Passing is implemented by system calls. 
Shared Memory is done through user land. 


message passing: small data [cons: copying]
(via system call)

Shared memory: large data [avoid copying, avoids sys calls, avoid kernel intervention.]
(in userland)

Exercise 3 (Review): Which interprocess communication model is faster than its
counterpart? Assumption: message-passing systems are implemented using system
calls; shared-memory regions are implemented in the user space.
A. Message Passing is faster
B. Shared Memory is faster
C. Shared Message is faster
D. Two models are both fast

Answer: B

Exercise 4 (Review): Which interprocess communication model is easier to
implement in a distributed system?
A. Shared Memory is easier
B. Message Passing is faster
C. Message Passing is easier [Answer]
D. Two models are equally easy

Thoughts: So, shared memory avoids sys calls, and kernel intervention, and i don't know what the user would be using the application for, so it would be ideal to use shared memory, but it might be difficult to implement. I personally think that message passing would be easier to implement. 

Memory is not naturally shared, they are distributed. You have to create a virtualized shared memory space and that is tough. 

Exercise 5 (MS Forms): Where should the shared memory region reside? (30 Seconds)
A. Process A
B. Process B
C. Randomly pick process A or B
D. The process that creates the shared memory region [Answer]



Synchronization problems with processes?

Turn a regular array into a circle array?

Exercise 9 (MS Forms): What issue hasn’t been addressed in the above producer-
consumer solution? (30 Seconds)
A. No issue. It is a perfect solution.
B. There a debugging problem
C. There is a race condition problem [Answer]
D. There is a user-space problem

You can use mutex or semaphores to solve the problem

Why do we need message queues? When we already have shared memory?


Message Queue has a data type: once a queue of integers then it can't be change. It also has the type and the message in the queue. 

Shared memory has it's own different type depending on the data. Very flexible on data type

Processes communicate with each other without resorting to shared variables. 

Some common function used:
	ftok()
	msgget()
	msgsnd()
	msgcv()
	msgctl()

First get the identifier and then send then receive. 

rcv just gets the message from the queue. 

What is the data type of message in `message.mesg_text`

	It's a struct
	
	It is an array of char

Do we need to include the PID of the sender or the receiver?
	The reason it is not being done right now because all of this, is a part of the code right now. When the executable code is executed. The OS will make a PID for us. 


