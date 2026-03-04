
1. TDD - Test Driven Development

	User-space development for system tool. It is ideal to make sure that the tool is built using test driven development

	Anything closely connected to the hardware, you don't use TDD. 

	TDD is useful for compilers, Databases. 

2. Mocking Support

	All about how to fake dependencies. 
	All about unit and strong likelihood that the other modules are not available/or out of scope from your testing. 

3. Unity

	Developing embedded systems, unity is useful. 
	Simple and light-weighted
	Easy to integrate
	Check: C unit test framework - mocks, good formatting
	Criterion: modern, they can facilitate parallel testing. 
	CuTest: Simple

4. Supporting Mocks
	CMocka - Specialize in Mocks (Automatically creates those fake dependencies)
	GoogleTest - for C and can also support C++
5. C++
	GoogleTest
	Catch2: Modern design, feature-rich. 
	Boost.Test: More comprehensive than Catch2

C Unit

Test if the source code was well developed. 

Source Code <--> Unit Testing Code <--> Unit test framework

max.c <--> max_test.c <--> lcunit

```shellcode
gcc -c max.c --> max.o
```
```
gcc -c max_test.c --> max_test.o
```
```shellcode
gcc max.0 max_test.o -lcunit -o max_test
```


```shellcode
./max_test
```


Create testing suites

Given each suite
	Create test cases
	Using macros than function calls in C

CU_ASSERT_EQUAL (max(1, 2), 2)

