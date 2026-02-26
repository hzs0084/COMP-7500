Lecture 10 MQ and scheduling

1. Mach <- Much (Multi User Comm Kernel)
	1. CMU 1985 -> 1993 Mach 3.0 -> BSD (Berkely Software Distribution)
					   -> FreeBSD (based from Mach 3.0)
					   -> Apple XNU
	   Dario G
	   Rich R
	   
	   
2. Mailbox: port_allocate()

A port is an array of messages to a specific port-name

A queue of messages is referred as port

When port allocate is created, they will create port-name metadata

The maximum size: 8 slots

At that time in 80s they used task which now we know as processes

each task is a PCB - Process Control Block

The owner of the port name is referred to a specific task

The data structure for the 8 slots is a queue

msg_send()
msg_receive()
msg_rpc()

Remote Procedure Calls -> RPC

POSIX Message Queue

ms_open
mq_send
mq_receive
mq_notify

Shared memory was designed first before message queue

Shared memory is more popular compare to message queue

MQ - Message Queue

2. Rabbit MQ : 2007

	Main product of a startup company Rabbit and their design of Message Queue system design
	
	1.  Open Telecom Platform
	2. Publisher / Subscriber Model

		Whoever is going to send data, we don't call them senders, we call them publisher

Diagram

Queue is the message
Exchange is the port_name 

You need the metadata to describe what is going on in the data

Publishers can connect to any exchange and it makes it binding?

Amazon SQS
	Create an infinite asynchronous queue

RPC Frameworks in Modern Systems

Exercise 2. 
	It allows CPYs to share resources

Exercise 3
	The process moves to the end of the ready queue

Exercise 4
	Shortest Job Next
	Priority Scheduling

Exercise 5
	Minimize context switch overhead

Exercise 6
	The time taken from submitting a request until the first response is produced

Exercise 7
	The number of processes completed per unit time

Exercise 8
	The total time taken from submitting a request until the process is completed

Exercise 9
	User's password

Exercise 10
	Waiting