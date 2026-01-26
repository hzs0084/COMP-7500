How should communicating processes be associated with a link in indirect communication?
- A link is associated with multiple pair of processes? Here it's in pairs but we need group
- D? is the answer

How many communication links should each pair of processes share?

- 3 is the magic number in real world systems
- This is not a design decision but a implementation decision. 
- A) Each pair of processes may share several communication links

The design of indirect communications


What are the four basic operations for indirect communication?
- Create
- Destroy
- Send
- Receive 

Please design the prototypes for the following two operations for indirect communication?
- Send(mailboxID, pointer that points to the location where the message is stored);
- Receive();

Mailbox Sharing

- Process priority to break the tie
- Timestamps
- Random pick

### P Thread

1. POSIX [Portable OS Interface] - UNIX created by Bell Lab in 1970s 

BSD (Berkeley Software Distributor) in 1980s 

Every vendor came together and decided on using Posix so regardless of the design, the shell commands are the same for unix.

P in Pthread stands for Posix

Thread is multi-threading

If you use PThread the performance will improve in project 2; piping is expensive to implement

What is POSIX shared memory?
- API: To allow processes to communicate information by sharing a region of memory.


2. Persistance

When and who someone can access an object

Persistence
- available for access: kernel persistence vs no persistent
- All processes have unmapped objects and they have been deleted until
	- It is unmapped and disconnected
	- If the object itself is completely deleted
	- Or the system is shut down
	- Any undestroyed object will be destroyed

Exercise 1. What security feature should be be built in the shared memory system in POSIX?
- Only authorized parties can access it, create a region where only authorized parties can access it. 
- ACL - Access Control List; Have a list of PIDs that are authorized to access the shared region
- It is mapped in a virtual file system

Exercise 2. Can you design four basic POSIX shared memory operations?
- Create: shm_open
- mmap() (very important ) as it maps main memory to files 
- Destroy: shm_close
- munmap(): reverse implementation of the mmap
- ftruncate(): truncate to shrink or enlarge the files and f is a reference to files
- shm_unlink(): disconnect the participating processes 

Exercise 3. Given the following four operations, what is the correct sequence of these operations for producers and consumers?

fopen()
mmap()
shm_unlink()
ftruncate()
sprintf() /* write */
printf() /* read */

Answer: 

Producer:
fopen()
ftruncate()
mmap()
sprintf()

Consumer:

fopen()
mmap()
read()
unlink()


Exercise 4. Can you design two security-related operations for the POSIX shared memory?

Answer:

fchown() - to change the ownership of a shared memory object

fchomd() - To change the permissions of a shared memory object






