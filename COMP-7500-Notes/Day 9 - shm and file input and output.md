Lecture SHM and File l/Os

1. SHM_UNLINK("/my_shm_x") and MUNMAP(addr, length)

Unmap is all about removing it from address space

During unlink, you only unlink the shared object name

If no one creates the object then shn_open will create it for you

1. Unlink is all about removing the shared object name; the object is still there, we just remove the link to it. *This is a system wide removal*
2. Once it is removed, it can't be open again. 
3. can read and write if it has been opened
4. Destroy? Automatically handled by the kernel
5. Example:
--------------------------------------------------------------------> time
		proc A: shm_open, mmap, write/read, shm_unlink, munmap, close | kernel destroys shm object
		proc B:         shm_open, mmap, write/read, write/read, munmap, close | kernel destroys shm object
		proc C:                                                                            shm_open(/using the same name); this wouldn't work
	
Unmap
- Removes the mapping from caller's address space
- It has nothing to do with the resource itself, it's just a pointer to that resource. 
- It is a local operator, individual process need to unmap from it's own local space. 

Destroy
- We don't have destroy in shared memory because IPC takes care of it. Automatically handled by the kernel. 
- All the processes must unmap and close it. Closing it is the signal for the kernel to destroy it. 

2. mmap()
	1. flag: MAP_SHARED, 
	   MAP_PRIVATE
	2. fd: remember that shm_open will have a fd; open is going to prepare the data structure internally and fd is responsible then, so another reason why you use file descriptors to close it. 

\#define DEBUG 

\#ifdef DEBUG
	/* source code*/
\# end if

*if it's in pink then compiler is going to handle it*


What is atoi()?

	Ascii to integer conversion

Argc - Argument Count
Argv - Argument Value

