# What is MeshNet?
Meshnet is a UDP-based file transfer protocol designed to have similar, and eventually superior, function to HTTP. I also plan to implement encryption to even rival HTTPS.
As its name suggests, it will (eventually) operate in a mesh. This includes servers and clients.
Servers, whether many on one system or many on many systems, can operate as one, making multithreading and multimachining easier.
Public meshes can be made to act like torrents, with the so-called "buddy" system's port open, meaning users can help in serving files.
Meshes will be able to share all their data to help with speed and redundancy, or each buddy can have its own set of files, meaning when a client asks for one, it will invisibly request from the buddy that has it.

<br />
<a href="https://trello.com/invite/b/68c7a84a4a505ffd33b44fd4/ATTI9e6f1db914ae9493f9fac9ccca7ae9b434536BBA/meshnetserver">Trello todo board</a>

# What's its current state?
Right now, almost none of the above features are implemented. Currently, it implements a reliable and fast file transfer method, but doesn't buddy yet.
It sends 2052-byte packets to the client, containing a uint32 packet index (for reliability) and a 2kb payload. 
After each packet, it waits for the client to respond simply with the string "-1" (ok) or a number (still in string form).
The number (if not -1) refers to the last packet index the client saw, so if packets 25 and 26 are dropped, the client reports 24, and the server will rewind to packet 24 and continue from there.

# How do I use it?
A basic client exists <a href="https://github.com/80HD-0/MeshNetClient">here</a>. It just grabs test.txt.

# How do I build it?
I run VSCode, and as much as I hate not just giving you a makefile, it would be a waste of time for an unfinished project.
With that said, if you want to use it I have builds in the releases, and you can always just use vscode.

WARNING:
The code you build is likely not stable or incompatible with the current client. Stable code will be publiushed in releases.

## Versioning scheme
Example: 1   .   2   .   3<br><br>
first number (1): version. This dictates the major client release. Should a major architecture change happen, this will update.<br><br>
second number (2): release. This is always consistant with protocol version. A 0.2.x server must bind to a 0.2.x client, though some later backwards compatibility is planned.<br><br>
third number (3): subrelease. This updates when bugfixes or general server improvements happen, and updates seperately from the client.
