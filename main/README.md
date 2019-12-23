Might have to Change Env Settings to your personal SFML directories while running.


FINAL SUBMISSION -----------------------------------------------------------------------

The solution for the remainder of the assignment is stored in server_main and client_main as follows:
\server_main has the server code
	Execute as server.exe

\client_main has the client code
	Execute as client.exe <ClientId>

CONTROLS : 

C : Disconnect and Close
Q : Pause
S / F : Slow or Speed Up
R (Deprecated) : Resize (Fixed size or scaling)



Note : 

The clients can connect at arbitary times and number of clients is limited to 5 presently since I have set the number of instances of the client 
to 5 and the game's physics systems performance degrades significantly as the number of clients increase. It can be changed to arbitary number 
since the rest of the code (server side handling as well as client side handling) is written without any such limiting case. The line containing
this limit is L44 in \client_main\client.cpp (	Character clients[5]; ).