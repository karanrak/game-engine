Might have to Change Env Settings to your personal SFML directories while running.


FINAL SUBMISSION -----------------------------------------------------------------------

The solution for the remainder of the assignment is stored in MultithreadedGame and MultithreadedGame2 as follows:
\MultithreadedGame has the server code
	Execute as MultithreadedGame.exe

\MultithreadedGame2 has the client code
	Execute as MultithreadedGame2.exe <ClientId>

List of Files:
1. MultiThreadedGame - Main Server
2. MultiThreadedGame2 - Main Client
3. Distributed_* - For Performance evaluation
4. ServerSide_* - For Performance evaluation
5. tester_* - The implementation for the modified time that didnt work. More info in Design Doc
6. Performance Evaluation - Excel sheet with values.

CONTROLS : 

I : Start a replay-recording
O : Stop a replay-recording. Start Replay. (Check Note 1)
C : Disconnect and Close
Q : Pause
S / F : Slow or Speed Up (Client Side)
U / D : Slow or Speed Up (Not working properly, Check Design Doc Part 2 for details)
R (Deprecated) : Resize (Fixed size or scaling)


Note : 

1.	The replay seems to be not working when running it the first time in a client. Ss just hit the I and O key once. And following that the replays
should work properly. Check the Design Doc part 2 for more details on the bug. 

2.	The clients can connect at arbitary times and number of clients is limited to 5 presently since I have set the number of instances of the client 
to 5 and the game's physics systems performance degrades significantly as the number of clients increase. It can be changed to arbitary number 
since the rest of the code (server side handling as well as client side handling) is written without any such limiting case. The line containing
this limit is L44 in \MultithreadedGame2\Client.cpp (	Character clients[5]; ).