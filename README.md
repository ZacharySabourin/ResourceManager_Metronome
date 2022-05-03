# ResourceManager_Metronome
Final Project for my Real-Time Programming class.

The task was to implement a Resource Manager and Metronome using the C language that functions with the BlackBerry QNX Neutrino OS.

There is a thread for each process. The main Resource Manager is the server, while the Metronome thread is the client

I register a Metronome device to the resource manager and send the device some messages through the Neutrino console.

The metronome has an API that can be accessed through messages.
The messages are then interpreted and the metronome will change it's output based on the message recieved.

What I learned:
- A firm grasp of resource managers
- A firm grasp of IPC
- A firm grasp of multi-threading and memory management
- More defensive programming techniques

If it were up to me, I would have designed this in a less confusing way, but my professor was adamant about everything being in the one file.
