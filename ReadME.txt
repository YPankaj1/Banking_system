There are 3 types of user, customer denoted by C, police denoted by P, admin denoted by A in the login_file.txt
There are 10 customers from 1 to 10, 2 admin (11-12), 3 police (13-15)'
We can add new users by adding them in the login_file
(we can also add functionality of dynamically creating user)

For running the file:
run the following code in a terminal in same folder where the files are present:

gcc server.c -o server
gcc client.c -o client

after that on two separate terminals(in the same folder where we have compiled the code) you can run server and client:
for running server ./server <port_number> (for example ./server 6500)
for running client ./client <ip_address> <port_numebr> (for example ./client 127.0.0.1 6500)

you can run multiple clients as my server can run upto 8 connections practically 
(theretically 5 - can be a good discussion point in viva).

for customers they have 2 options to view their balance or print mini_statement.
for admin they have 2 options to either debit or credit some amount for some customer.
for police they have 2 options to either view any customer's balance or mini_statement.
