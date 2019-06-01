# File-Transfer

<img src="https://i.ibb.co/4ZSTFpP/Screenshot-from-2019-06-01-16-01-46.png" width="800"><br>
A simple terminal-based multithreaded C application that allow two (or more) machine connected to the same local network<br>
communicate between them to send and receive files over the internet, using the TCP/IP socket paradigm.<br>
This small project has been designed for *personal use* and it is therefore open to some future *improvement*.<br>

#### Secure Networking
This project come with a custom networking safe library. The library contains functions to control efficiently the data<br>
passage over the network, the handling of common socket errors and the definition of custom data structure to organize<br>
the workflow of the socket definition, building and initialization more easily.<br>

# Installing
You can both download and compile from scratch the source code, or install it with the install script.

## Installing with the script
Just download the repository, extract the file to a folder and run <br>
```console
foo@bar:~/File-Transfer$ .install.sh
```

## Installing from source code
As above, download the repo and run the <b>make</b> into each folder (Client/Server) to generate the executable.<br>
Before compiling, you must specify the path of your Repository, both for the client and the server (they should be different)<br>
withing the first 3 lines of code of both the FTClient and FTServer, where the <b>#DEFINE "PATHTODIR"</b> is located. Just change *"PATHTODIR"* with your favorite directory, build it and run the executable.<br>

# Usage
Follow this guidelines to begin the file-trasnfer:

## Installed with the script
If you ran the install script, you can type the commands anywhere from the terminal to start the transfer.
Also, two folder are created into the Home folder: <b>FileDownload & FileUpload</b>
FileDownload is the folder that store the files download from the server.
FileUpload is the folder where the file that the server should share are put.
*Note: you can move this folder anywhere in your filesystem.*

To begin, you can type
```console
foo@bar:~$ ftserver
```
to start the server, and
```console
foo@bar:~$ ftclient <Server IP to reach>
```
to start the client. Be sure to put the ip address of the server as the first argument. <br>

Additional Note: Be sure to *put something into the FileUpload*, otherwise the client won't see anything.

## Installed from source
After you ran the make and specified the repository folder, both for the client and the server, you only need to start the exectuable:<br>
To start the client
```console
foo@bar:~$ ./FTClient <Server IP to reach>
```
and to start the server
```console
foo@bar:~$ ./FTServer
```

