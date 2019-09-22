# UDP
Basic UDP client Server program in C

It calculatess Round Trip Time (RTT) of a packet of predefined size. 

To run the server:
```
gcc server_fast.c -o srv
./srv 8080
```
To run the normal client application:
```
gcc client_udp.c -o clt
./clt 8080 127.0.0.1 100 4 50
```
Where argv[1]= port number of Server, argv[2]= ip of Server, argv[3]= length of Payload, argv[4]= Time To Live (TTL), argv[5]= number of Packets

In order to check the comparative performance of RTT, 50 packets of different size ranging from 100 to 1000 are sent each for TTL 2,4,8 and 16 respectively.

To run the client program for all this data logs:
```
gcc graph.c -o grp
./grp 8080 127.0.0.1 4
```
Where argv[1]= port number of Server, argv[2]= ip of Server, argv[3]= Time To Live (TTL).

To plot the comparative results: run the python script.
