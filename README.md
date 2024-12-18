# Inverted Index

## Build & Run
After cloning repository, open up a terminal and make sure you have cmake downloaded on your system. Then make a new directory which will have the build files:
    
    mkdir cmake-build
    cd cmake-build

Now create cmake files needed for build:

    cmake ..

Now build a project:

    cmake --build ./

Launch a server:

    ./server

Open a new terminal in this folder and launch a client:

    ./client1


## How to use / test
After running client and server, you now have a start up index already pre-build on server, which has 250 files. To search this files you can type:

    SEARCH <term>

Where \<term> is the word you want to search for.

---

If you want to add new files to the server, load them into the `files/` directory, and then type:

    REINDEX

This will start a reindexing process on the whole `files/` directory.

---

To see the status of reindexing process you can use:

    STATUS

This command will tell you if reindexing is still ongoing or have already finished.

---

To disconnect a client simply type in:

    DISCONNECT
