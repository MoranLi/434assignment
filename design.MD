How function of Router implemented

Unfortunatly, I can not get thread communicate to process , so I use seperate process for sever side and client side for router

for reach router(router() function)
    it create a process for server
    then run a infinate for loop for client (with sleep(2) to ensure only send each 2 second)

server
    it receive 2 kind of communication:
        1. from client of other router
            split the string and update it own distance and node names(recv_string() function)
        2. from client of same router
            the message is a "?", use that to split from other messages
            it will uses generate a string for own distance table and nodes connected
            and reply that string back to the client

client
    it contain a loop and a seperate client call
            the loop
                it will create client to all other nodes connected, send its distance table to them
            after that, it calls server process for same router, ask for the updated router table
        