# ClassConnect
Multi threaded Client-Server program to create a platform for student-professor communication

Class Connect
- Configuration of a multi-threaded server to handle multiple clients simultaneously using socket programming.
- Concurrency handled using pthread by creating TCP sockets for tasks such as sending messages for functionalities such as chat server, file transfer, setting appointments.

Note:
The project is built and developed on Linux environment.

1. Server would run in the background and provide different services to professor – client and student – client. 
2. The steps to run them are given below.




1.	Server Implementation

	Enter the server folder in terminal
	Compile the server with following command
•	gcc –pthread –o server server.c

	Run server by the following command:
•	./server 98765

2.	Professor Implementations

	Enter the client folder in a new terminal to run the client – Professor
	Compile the server with following command
•	gcc –o professor professor.c
	Run server by the following command:
•	./professor 127.0.0.1 98765

3.	Student Implementations

	Enter the client folder in a new terminal to run the client – Student
	Compile the server with following command
•	gcc –o student student.c
	Run server by the following command:
•	./student 127.0.0.1 98765



