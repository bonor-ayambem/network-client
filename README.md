# CSE 109 - Systems Software - Spring 2022 - Homework 8

**Due Date: 5/2/2022 EOD**

## Instructions

**Read thoroughly before starting your project:**

1. Fork this repository into your CSE109 project namespace. [Instructions](https://docs.gitlab.com/ee/workflow/forking_workflow.html#creating-a-fork)
2. Clone your newly forked repository onto your development machine. [Instructions](https://docs.gitlab.com/ee/gitlab-basics/start-using-git.html#clone-a-repository) 
3. As you are writing code you should commit patches along the way. *i.e.* don't just submit all your code in one big commit when you're all done. Commit your progress as you work. **You should make at least one commit per function.**
4. When you've committed all of your work, there's nothing left to do to submit the assignment.

## Assignment

For this assignment, you will need to combine all of the skills you learned throughout CSE 109. As this is your final project, I expect that you have acquired the necessary skills to complete it. Therefore, I will not tell you what to do step-by-step. You have a lot of latitude to complete this project as you see fit. This means you can use whichever language you want (either C or C++), you can lay out your project directory however you want, and you can use whatever tools you want (gcc, g++, clang, Make, etc.). Do whatever works for you in order to achieve the intended result.

For this assignment, you will be writing the first half of a client/server pair that will communicate over Unix sockets. The pair is:

1. File Server - A file server is a program that hosts files for clients. It receives files that clients want to store, and sends them back to the client (or other clients) when they are requested.
2. Client - This program connects to the server and can send files to it, which will be stored on the file server. The client can also request files from the file server. This is what you will be writing for this assignment.

## Part 1 - Client

The client program should accept the following flags:

1. `--hostname address:port` - Where `address` is a 32 bit IP address, and `port` is the desired port of the file server.

2. `--send filename`

Where `filename` is a path to a file on your local computer. 

3. `--request filename`

Where `filename` is the name of a file stored on the file server

For example, you could call the client like this:

```
./client --hostname localhost:8081 --send files/document.txt
```

Or like this:

```
./client --hostname localhost:8081 --request document.txt
```

If you call the program with the `--send` and `--request` options at the same time, it should exit with an error. If you call the program without a `--hostname`, it should exit with an error.

### Sending a File

I've included six sample files in the `files` directory. They are:

- document.txt - a plain text document
- index.html - an html document
- program.c - a C source file
- logo.svg - the Lehigh logo in SVG
- main - a compiled C executable
- picture.png - a picture in Portable Network Graphics (PNG) format

To send a file, you will have to code the following steps:

- The user will invoke the program with a hostname, and a path to the file to send. e.g. `./client --hostname localhost:8081 --send files/document.txt`.
- Parse the input arguments, and store the path of the file to read, as well as the host address and port numbers.
- Open the file and read it into memory.
- Store the file in the following struct:

```
struct File {
  string name,                 // The name of the file, excluding the path. e.g. just document.txt
  vector<unsigned char> bytes, // The byte contents of the file
}
```

*(You don't have to use this exact struct. Like I said, you have latitude in how you design this. If you are using C you won't have vector, but you can use a `char*` array instead, along with a length field.)*

- Serialize the struct using the PACK109 message protocol. You'll have to write serialization and deserialization code to do this.
- Encrypt the serialized vector using the simple encryption scheme laid out [here](https://codeforwin.org/2018/05/10-cool-bitwise-operator-hacks-and-tricks.html).
- Establish a Unix socket connection with a remote server using the provided hostname and port.
- Send the encrypted byte vector over the socket.

#### Serialized File

If you have a file called `file.txt` with the contents `Hello`, then the serialzied, unencrypted message should look like this:

```
┌───────────────────────────────────────────────────────────────────────────────┐
│0xae     // map tag                                                            │
│0x01     // 1 kv pair                                                          │
├───────────────────────────────────────────────────────┬───────────┬───────────┤
│0xaa     // string8 tag                                │           │           │
│0x04     // 4 characters                               │ key       │           │
│File     // the string "File"                          │           │           │
├───────────────────────────────────────────────────────┼───────────┤  pair 1   │
│0xae     // the value associated with the key is a map │           │           │
│0x02     // 2 kv pairs                                 │           │           │
├────────────────────────────────┬───────────┬──────────┤           │           │
│0xaa     // string8 tag         │           │          │           │           │
│0x04     // 4 characters        │ key       │          │           │           │
│name     // the string "name"   │           │          │           │           │
├────────────────────────────────┼───────────┤ pair 1   │           │           │
│0xaa     // string8 tag         │           │          │           │           │
│0x08     // 8 characters        │ value     │          │           │           │
│file.txt // the filename        │           │          │           │           │
├────────────────────────────────┼───────────┼──────────┤           │           │
│0xaa     // string8 tag         │           │          │ value     │           │
│0x05     // 5 characters        │ key       │          │           │           │
│bytes    // the string "bytes"  │           │          │           │           │
├────────────────────────────────┼───────────┤ pair 2   │           │           │
│0xac     // array8 tag          │ value     │          │           │           │
│0x05     // 5 elements          │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│H        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│e        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│l        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│l        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│o        // a byte              │           │          │           │           │
└────────────────────────────────┴───────────┴──────────┴───────────┴───────────┘           
```

In decimal:
```
[174, 1, 170, 4, 70, 105, 108, 101, 174, 2, 170, 4, 110, 97, 109, 101, 170, 8, 102, 105, 108, 101, 46, 116, 120, 116, 170, 5, 98, 121, 116, 101, 115, 172, 5, 162, 72, 162, 101, 162, 108, 162, 108, 162, 111]
```

### Requesting a File

To request a file, you will have to code the following steps:

- The user will invoke the program with a hostname, and a filename that is requested from the file server. e.g. `./client --hostname localhost:8081 --request document.txt`.
- Parse the input arguments, and store the name of the file to request, as well as the host address and port numbers.
- Store the requested file name in the following struct:
```
struct Request {
  string name, // The name of the requested file, e.g. document.txt
}
```
- Serialize the struct using the PACK109 message protocol. You'll have to write serialization and deserialization code to do this.
- Encrypt the serialized vector using the simple encryption scheme laid out [here](https://codeforwin.org/2018/05/10-cool-bitwise-operator-hacks-and-tricks.html).
- Establish a Unix socket connection with a remote server using the provided hostname and port.
- Send the encrypted byte vector over the socket.
- Wait for a response from the server. When a response is received, it will be a vector of bytes. Save this vector of bytes into your program's memory.
- Decrypt the vector of bytes using the same simple encryption scheme as in the previous section.
- Deserialize the message. It will be in the PACK109 format, and the response will be a `File` struct, as laid out in the previous section.
- Once you have deserialized the message from the server, you should have access to the original bytes of the file, and its name. Save the file in a folder called `received`.

#### Serialized Request

If you have a file called `file.txt` that you are requesting, then the unencrypted message should look like this:

```
┌───────────────────────────────────────────────────────────────────────────────┐
│0xae     // map tag                                                            │
│0x01     // 1 kv pair                                                          │
├───────────────────────────────────────────────────────┬───────────┬───────────┤
│0xaa     // string8 tag                                │           │           │
│0x07     // 7 characters                               │ key       │           │
│Request  // the string "Request"                       │           │           │
├───────────────────────────────────────────────────────┼───────────┤  pair 1   │
│0xae     // the value associated with the key is a map │           │           │
│0x01     // 1 kv pair                                  │           │           │
├──────────────────────────────┬───────────┬────────────┤           │           │
│0xaa     // string8 tag       │           │            │           │           │
│0x04     // 4 characters      │ key       │            │ value     │           │
│name     // the string "name" │           │            │           │           │
├──────────────────────────────┼───────────┤ pair 1     │           │           │
│0xaa     // string8 tag       │           │            │           │           │
│0x08     // 8 characters      │ value     │            │           │           │
│file.txt // the filename      │           │            │           │           │
└──────────────────────────────┴───────────┴────────────┴───────────┴───────────┘           
```

In decimal:
```
[174, 1, 170, 7, 82, 101, 113, 117, 101, 115, 116, 174, 1, 170, 4, 110, 97, 109, 101, 170, 8, 102, 105, 108, 101, 46, 116, 120, 116]
```


## Writeup

In a file called WRITEUP.md, provide a detailed account of how your client was constructed. First and foremost, tell me how to build it. What steps do I need to take to run it? Then, tell me how your program works. Go through the entire program and tell me what everything does. Explain your design decisions. Explain any code that is not straightforward. Note: If you would rather record a video walkthrough of your code, you can do this as well. Upload the video to your repository or google drive and add a link here (make sure to grant me viewing permissions).

Link: https://drive.google.com/file/d/1qECymQmg9OdXtPq-4x7DM8DustdHgCkm/view?usp=sharing
