Lecture 02: Interprocess Communication

1. What is a process?   Vs. Program

	Process is an executable action 

Exercise 1. Which one of the following is not a basic component in Microkernel?
	- File Manager (File Manager's is implemented outside of the microkenel; In a monolithic environment, it can be)

Exercise 2. What is a process in the context of an operating system?
	- A program in execution, including its code, data, and allocated resources. 

Exercise 3. Which one of the following items is not a design goal of a process manager? 
	- To allocate memory resources to processes

Exercise 4. The context switch time is OS overhead. The context-switch overhead depends on the following factors except?
	- Open source operating systems

Exercise 5. Which of the following is NOT a state in the typical process life cycle?
	- Assigned


States:
- Running: 
- Ready: Only needs CPU
- Wait: For anything

When it comes to interrupt, it connects to the corresponding handlers. The reason for interrupting is also important. 

2. There are two types of processes:
	1. Independent: When you create a program with .code, .text, .data; once it's ready, it doesn't need anything except for moving around states. 
	2. Cooperating: Processes are working together as team

3. Advantages of cooperating processes
	1. Quick execution aka speed up in performance
	2. Data sharing, even if the processes are collaborating, it avoid locs and it's a functionality
	3. Modularity: design
	4. Convenience: Usage / Implementation 
	5. Low Cost of execution
	6. Reliability

Many IPCs that make this possible

4. Example: Chrome Browser
	1. Interface
	2. Render: It's dealing with the important logic
	3. Plug-ins: For extensions, example, flash, quick-time. 

Each tab of the google chrome are independent processes

If one tab is crashed then every other process can still survive. It improves the reliability. 

5. All Available IPC Mechanisms
	1. Linux
		1. Signal: Unix-v4 in 1973
		2. Pipe: Unix-v3 in 1973
		3. Message Queue
		4. POSIX Shared Memory (Portable Operating System Interface)
	2. Windows-32
		1. Clipboard
		2. COM
		3. File Mapping, in terms of implementation ideas, Linux piping are quite close
		4. Mail Slots: One-way communication


6. Two IPC models
	1. Message Passing
	2. Shared memory

Parallel Computing is supported by message passing

Exercise 7. What are the two fundamentals models of interprocess communication? hint: How do two process talk with each other?

Answer: Message Passing and Shared Memory

7. PIpe
	- The state of the process does not matter 
	- In an address space, the heap is growing down, the stack is growing up, data segment, text segment. 
	- Only the process itself can access the contents inside the address space. 
	- You need only one IPC to make one process to talk to another process.
	- Pipe belongs to message passing
	- Pipe has write-end and read-end, which allows it to send and receive from one process to another. 
	- You cannot connect text, the pipe only works with the data segment of the process. 
	- Bidirectional is possible? But need more than one pipe

Using files so that the output of one process into a temporary file to pass into the second process. 

