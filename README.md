# What is MeshNet?
Meshnet is a UDP-based file transfer protocol designed to have similar, and eventually superior, function to HTTP. I also plan to implement encryption to even rival HTTPS.
As its name suggests, it will (eventually) operate in a mesh. This includes servers and clients.
Servers, whether many on one system or many on many systems, can operate as one, making multithreading and multimachining easier.
Public meshes can be made to act like torrents, with the so-called "buddy" system's port open, meaning users can help in serving files.
Meshes will be able to share all their data to help with speed and redundancy, or each buddy can have its own set of files, meaning when a client asks for one, it will invisibly request from the buddy that has it.

# What's its current state?
Right now, almost none of the above features are implemented. Currently, it implements a reliable and fast file transfer method, but doesn't buddy yet.
It sends 2052-byte packets to the client, containing a uint32 packet index (for reliability) and a 2kb payload. 
After each packet, it waits for the client to respond simply with the string "-1" (ok) or a number (still in string form).
The number (if not -1) refers to the last packet index the client saw, so if packets 25 and 26 are dropped, the client reports 24, and the server will rewind to packet 24 and continue from there.

# How do I use it?
I'm working on a client that does the above, but it is currently non-functional.
If you want to have a crack at making your own, be my guest.
