
1. Modern Storage Architecture

Storage -- Data Storage

	1. Direct attached storage: The media is attached directly to the storage and no stability issues. 
	2. Network Attached storage (NAS): NAS has performance issues and isn't performance friendly.
	3. Storage Area Network (SAN): Addresses on network performancing issues in NAS. 
	4. Object Storage: Started from research papers, but now is in practice. 
	5. Software Defined Storage: Addresses Scalability and extensibility issues. 
	6. Cloud Storage: Built on Virtualization technologies. 

Modern storage solutions like Object Storage, Software Defined Storage, Cloud Storage are scalable and efficient. 

- Scale
- Performance
- fault-tolerance/redundancy

Scalability:
	Virtualization


2. HDD: Hard Drive Disk

![[Pasted image 20260306091723.png]]

Top-down view

![[Pasted image 20260306091821.png]]

	1) Seek the track is handled by the motor. Slow because of the physical time. The time spent by the hard drive is called `seek_time`
	2) In order to find the sector, you need to rotate the disk so that the disk head can find it. Measure how fast the rotation moves. It's called `rotation_dealy`. The time spent to find the corresponding sector. 
	3) Data transfer. Need to transfer the data from the media, if it's read then move it through file system, or dump data from main memory to the disks. //This is where burn comes in?
   `transfer_time`
	4) Controller. This is negligent in comparison to the above steps so it's very small and can be ignored. 

3. Disk Access Time

How to compute average rotational delays? Example: A hard drive's RPM is 120. 

RPM - Round Per Minute

	1. Convert minute into seconds
	   120 RPM = 120 Round/60 Sec = 2 Round / Sec
	2. Then you flip it
	   1 sec / 2 round = 0.5 sec/round
	3. 0.5 sec / 2 = 0.25 sec = 250 ms

7200 / 60  = 120 round / sec

1/120 = 0.00833333333

0.00833333333 / 2 = 0.00416666666 sec 

0.00416666 x 1000 = **4.17 ms**

2. Transfer time 

Data Size = 4 KB

Transfer Rate = 8 Mb/Sec 

Need to convert MegaBIT to MegaByte

Data size / Transfer Rate = 4Kb / 8Mb 


4 KB / 1 K \* KB / sec

4 Sec / 1024  = 0.0039 sec

3.9 ms

Example:

Suppose a 7200 RPM disk transfers a 4KB block with a 5 ms average seek time, 1Gb/sec transfer rate with a 0.1 ms controller overhead?

Rotational Delay is 4.17 ms

Data Size = 4 KB

Average seek time = 5ms

Transfer rate = 1Gb/sec = 0.125 GB/sec

B is the answer  // Figure out how to do it

^ Will be on the final
## HDD vs SSD

Cost low on HDD
Reliability more HDD
Performance high on SSD


Disk Attachment: How to access disk storage?


## 3. NAS: Network Attached Storage

- Storage is made available over a network
- NFS and CIFS are common protocols
- Implemented via remote procedure calls (RPCs) between host and storage over typically TCP or UDP on IP network

In the network diagram, as NAS and clients increase, the bottle neck on the network increases. 


### 4. SAN - Storage Area Network 

A network dedicated to network storage. 

Why wouldn't this be another bottleneck? - Look at the end of lecture 

Server as a form of cache? 