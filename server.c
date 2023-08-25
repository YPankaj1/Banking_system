#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#define MAX 5000
#define delim "*****"

void printerror(char *errmsg)
{
    perror(errmsg);
    exit(0);
}
// costomer code
void read_balance_from_file(char *fn, char *balance)
{
    FILE *fp = fopen(fn, "r");
    if (fp == NULL)
        printerror("ERROR: Opening user file for balance failed.");

    char *transaction = NULL;
    size_t len = 0;
    char *bal;

    while (getline(&transaction, &len, fp) != -1)
    {
        strtok(transaction, " ");
        strtok(NULL, " ");
        bal = strtok(NULL, " ");
    }
    bzero(balance, MAX - 1);
    strcat(balance, bal);
    // balance = bal;
    free(transaction);
    fclose(fp);
}
void send_available_balance(int clientfd, char *cust_id, char *client_ip)
{
    char filename[MAX];
    sprintf(filename, "%s.txt", cust_id);

    char balance[MAX];
    read_balance_from_file(filename, balance);
    fprintf(stdout, "Sending balance of customer '%s' to client with ip '%s'. \n", cust_id, client_ip);
    // balance
    int x = write(clientfd, balance, strlen(balance));
    if (x < 0)
        printerror("ERROR writing to socket");
}
void send_statement_from_file(char *filename, int clientfd, char *cust_id)
{
    struct stat file_stat;
    char packet[MAX];

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        printerror("Error in opening user file for mini_statement");

    // finding stats of file
    if (fstat(fd, &file_stat) < 0)
        printerror("Error in getting statistics of file.");

    // writing size of file
    bzero(packet, MAX);
    sprintf(packet, "%d", (int)file_stat.st_size);
    int n = write(clientfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR writing to socket");

    // delimeter string
    bzero(packet, MAX);
    n = read(clientfd, packet, MAX - 1);
    if (n < 0)
        printerror("ERROR reading from socket");

    // sending mini statement
    fprintf(stdout, "Sending mini statement of customer '%s' to client . \n", cust_id);
    while (1)
    {
        bzero(packet, MAX);
        int bytes_read = read(fd, packet, sizeof(packet));
        if (bytes_read == 0)
            break;
        if (bytes_read < 0)
            printerror("ERROR reading from file.");

        void *ptr = packet;
        while (bytes_read > 0)
        {
            int bytes_written = write(clientfd, ptr, bytes_read);
            if (bytes_written <= 0)
                printerror("ERROR writing to socket");
            bytes_read -= bytes_written;
            ptr += bytes_written;
        }
    }
    close(fd);
}
void send_mini_statement(int clientfd, char *cust_id)
{
    char filename[MAX];
    sprintf(filename, "%s.txt", cust_id);
    printf("%s\n", filename);
    send_statement_from_file(filename, clientfd, cust_id);
}
void customer(int clientfd, char *cust_id, char *client_ip)
{
    char packet[MAX];
    char id[MAX];
    sprintf(id, "%s", cust_id);
    // asking whether customer want to continue
    bzero(packet, MAX);
    int x = read(clientfd, packet, MAX - 1);

    if (x < 0)
        printerror("ERROR reading from socket");

    while (packet[0] == 'y')
    {
        // reading the command of customer
        bzero(packet, MAX);
        int x = read(clientfd, packet, MAX - 1);
        if (x < 0)
            printerror("ERROR: reading from socket");
        // removing the newline character from the end
        packet[strlen(packet) - 1] = '\0';
        if (!strcmp(packet, "MINI_STATEMENT"))
        {
            // sending operation matched
            bzero(packet, MAX);
            strcpy(packet, "matched");
            int x = write(clientfd, packet, strlen(packet));
            if (x < 0)
                printerror("ERROR: Writing in socket failed.");

            // delimeter string
            bzero(packet, MAX);
            x = read(clientfd, packet, MAX - 1);
            if (x < 0)
                printerror("ERROR: Reading from socket failed.");

            send_mini_statement(clientfd, id);
        }
        else if (!strcmp(packet, "BALANCE"))
        {
            // sending operation matched
            bzero(packet, MAX);
            strcpy(packet, "matched");
            int x = write(clientfd, packet, strlen(packet));
            if (x < 0)
                printerror("ERROR: Writing in socket failed.");

            // delimeter string
            bzero(packet, MAX);
            x = read(clientfd, packet, MAX - 1);
            if (x < 0)
                printerror("ERROR: Reading from socket failed.");

            send_available_balance(clientfd, id, client_ip);
            // send_available_balance
        }
        else
        {
            printf("Request from client with ip '%s' declined. \n", client_ip);
            // sending false
            bzero(packet, MAX);
            strcpy(packet, "not_matched");
            int n = write(clientfd, packet, strlen(packet));
            if (n < 0)
                printerror("ERROR writing to socket");
        }
        /* Reading flag */
        bzero(packet, MAX);
        int n = read(clientfd, packet, MAX - 1);
        if (n < 0)
            printerror("ERROR reading from socket");
    }
}
// admin code
int is_valid(char *amount)
{
    // checking validity of amount
    int i = 0;
    int cnt = 0;
    while (amount[i])
    {
        if (amount[i] <= '9' && amount[i] >= '0')
        {
            i++;
            continue;
        }
        else if (amount[i] == '.')
        {
            i++;
            cnt++;
            if (cnt > 1)
                return 0;
        }
        else
            return 0;
    }
    return 1;
}
void writing_transaction_to_file(char *filename, char *trans, double amt)
{
    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
        printerror("Error in opening user file for crediting.");

    time_t c_t = time(NULL);
    struct tm tm = *localtime(&c_t);
    fprintf(fp, "\n%.2d-%.2d-%.4d %s %f", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, trans, amt);
    fclose(fp);
}
void credit_amount(char *id, char *amount, char *trans)
{
    char filename[MAX];
    sprintf(filename, "%s.txt", id);
    char balance[MAX];
    read_balance_from_file(filename, balance);

    double amt, amt_cred;
    sscanf(balance, "%lf", &amt);

    // crediting amount
    sscanf(amount, "%lf", &amt_cred);
    amt += amt_cred;
    writing_transaction_to_file(filename, trans, amt);
}

int debit_amount(char *id, char *amount, char *trans)
{
    char filename[MAX];
    sprintf(filename, "%s.txt", id);

    char balance[MAX];
    read_balance_from_file(filename, balance);

    double amt, req_amt;
    sscanf(balance, "%lf", &amt);

    sscanf(amount, "%lf", &req_amt);
    if (amt < req_amt)
        return 0;

    // debiting amount
    amt -= req_amt;
    writing_transaction_to_file(filename, trans, amt);
    return 1;
}
int verify_valid_request(int check, char *trans, char *amount)
{
    return (check == 3 || check == 1 || (strcmp(trans, "credit") && strcmp(trans, "debit")) || !is_valid(amount));
}
void get_id_trans_amount(int sockfd, char *amount, char *id, char *trans)
{
    char packet[MAX];

    // reading command
    bzero(packet, MAX);
    int n = read(sockfd, packet, MAX - 1);
    if (n < 0)
        printerror("ERROR reading from socket");

    bzero(amount, MAX);
    bzero(id, MAX);
    bzero(trans, MAX);

    char *ptr = strtok(packet, delim);
    strcpy(id, ptr);
    ptr = strtok(NULL, delim);
    strcpy(trans, ptr);
    ptr = strtok(NULL, delim);
    strcpy(amount, ptr);

    // removing new line
    id[strlen(id) - 1] = '\0';
    trans[strlen(trans) - 1] = '\0';
    amount[strlen(amount) - 1] = '\0';
}

void send_true(int sockfd)
{
    char packet[MAX];
    bzero(packet, MAX);
    strcpy(packet, "true");
    int n = write(sockfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR: writing to socket");
}

void send_denial(int sockfd, char *msg)
{
    char packet[MAX];
    bzero(packet, MAX);
    strcpy(packet, msg);
    int n = write(sockfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR: writing to socket");
}
int check_if_customer(char *id)
{
    char *cred = NULL;
    size_t len = 0;
    int check = 3;
    FILE *fp = fopen("login_file.txt", "r");
    if (fp == NULL)
        printerror("ERROR: opening the file");

    while (getline(&cred, &len, fp) != -1)
    {
        char *username = strtok(cred, " ");
        strtok(NULL, " ");
        char *usertype = strtok(NULL, " ");

        if (!strcmp(username, id))
        {
            check = 1;
            if (usertype[0] == 'C')
            {
                check = 2;
            }
            break;
        }
    }
    free(cred);
    fclose(fp);
    return check;
}
void admin(int sockfd, char *client_ip)
{
    int n;
    char packet[MAX];

    /* Reading flag */
    bzero(packet, MAX);
    n = read(sockfd, packet, MAX - 1);
    if (n < 0)
        printerror("ERROR: reading from socket");

    while (packet[0] == 'y')
    {
        char id[MAX], trans[MAX], amount[MAX];
        get_id_trans_amount(sockfd, amount, id, trans);
        // checking for validity of user_id
        char *cred = NULL;
        size_t len = 0;
        int check = check_if_customer(id);
        // sending false
        if (verify_valid_request(check, trans, amount))
        {
            fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);
            send_denial(sockfd, "false");
        }
        else
        {
            if (!strcmp(trans, "credit"))
            {
                credit_amount(id, amount, trans);
                fprintf(stdout, "Credit request from client with ip '%s' for customer '%s' successfully executed. \n", client_ip, id);
                // sending true
                send_true(sockfd);
            }
            else if (!strcmp(trans, "debit"))
            {
                int f = debit_amount(id, amount, trans);
                // sending true
                if (f == 1)
                {
                    fprintf(stdout, "Debit request from client with ip '%s' for customer '%s' successfully executed. \n", client_ip, id);
                    send_true(sockfd);
                }
                else
                {
                    // insufficient amount
                    fprintf(stdout, "Debit request from client with ip '%s' declined. \n", client_ip);
                    bzero(packet, MAX);
                    send_denial(sockfd, "deficit");
                }
            }
        }
        /* Reading flag */
        bzero(packet, MAX);
        n = read(sockfd, packet, MAX - 1);
        if (n < 0)
            printerror("ERROR: reading from socket");
    }
}
// police

void read_client(int sockfd, char *packet)
{
    bzero(packet, MAX);
    int x = read(sockfd, packet, MAX - 1);
    if (x < 0)
        printerror("ERROR!! reading from socket...");
}

void break_command(char *packet, char *op, char *id)
{
    char *ptr = strtok(packet, delim);
    strcpy(op, ptr);
    ptr = strtok(NULL, delim);
    strcpy(id, ptr);

    op[strlen(op) - 1] = '\0';
    id[strlen(id) - 1] = '\0';
}

void validate_customer(char *id, int *check)
{
    char *credentials = NULL;
    size_t len = 0;

    FILE *login = fopen("login_file.txt", "r");
    if (login == NULL)
        printerror("Error in opening the login_file.");

    while (getline(&credentials, &len, login) != -1)
    {
        char *username = strtok(credentials, " ");
        strtok(NULL, " ");
        char *usertype = strtok(NULL, " ");

        if (!strcmp(username, id))
        {
            *check = 1;
            if (usertype[0] == 'C')
            {
                *check = 2;
            }
            break;
        }
    }
    fclose(login);
    free(credentials);
}

void send_bool(int sockfd, char *packet, int valid)
{
    bzero(packet, MAX);
    strcpy(packet, valid ? "true" : "false");
    int x = write(sockfd, packet, strlen(packet));
    if (x < 0)
        printerror("ERROR writing to socket");
}

void police(int sockfd, char *client_ip)
{
    int n;
    char packet[MAX];
    char filename[100], op[MAX], id[MAX];

    // reading flag
    read_client(sockfd, packet);

    while (packet[0] == 'y')
    {
        int valid = 0;
        // reading command
        read_client(sockfd, packet);

        // breaking command into operation and user_id
        break_command(packet, op, id);

        // checking for validity of user_id
        validate_customer(id, &valid);

        // sending false
        if (valid <= 1 || (strcmp(op, "balance") && strcmp(op, "mini_statement")))
        {
            printf("Request from client with ip '%s' declined. \n", client_ip);
            send_bool(sockfd, packet, 0);
        }
        else
        {
            if (!strcmp(op, "balance"))
            {
                // sending true
                send_bool(sockfd, packet, 1);

                // delimeter string
                read_client(sockfd, packet);

                send_available_balance(sockfd, id, client_ip);
            }
            else if (!strcmp(op, "mini_statement"))
            {
                // sending true
                send_bool(sockfd, packet, 1);

                // delimeter string
                read_client(sockfd, packet);

                send_mini_statement(sockfd, id);
            }
        }
        // reading flag
        read_client(sockfd, packet);
    }
}

// authentication code
int verify_credentials(int clientfd, char *user_type, char *cust_id)
{
    char packet[MAX];

    bzero(packet, MAX);
    int x = read(clientfd, packet, MAX - 1);
    if (x < 0)
    {
        printerror("ERROR: Reading username and passsword failed.");
    }
    char *username_recieved, *password_recieved;

    // breaking the massage in the packet to get username_recieved and password_recieved
    username_recieved = strtok(packet, delim);
    password_recieved = strtok(NULL, delim);

    // removing the newline character at the end
    username_recieved[strlen(username_recieved) - 1] = '\0';
    password_recieved[strlen(password_recieved) - 1] = '\0';

    if (!strlen(username_recieved) || !strlen(password_recieved))
        return 0;

    FILE *fp = fopen("login_file.txt", "r");
    if (fp == NULL)
        printerror("ERROR: Opening login file failed.\n");

    char *cred = NULL;
    size_t len = 0;
    int id;
    while (getline(&cred, &len, fp) != -1)
    {
        char *username = strtok(cred, " ");
        char *password = strtok(NULL, " ");
        char *usertype = strtok(NULL, " ");
        if (!strcmp(username, username_recieved) && !strcmp(password, password_recieved))
        {
            *user_type = usertype[0];
            id = atoi(username);
            printf("%d", id);
            sprintf(cust_id, "%d", id);
            // cust_id = username;
            free(cred);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    free(cred);
    return 0;
}
void serve_client(int clientfd, char *cip_addr)
{
    // int x;
    char packet[MAX];
    // int count = 1;
    char user_type;
    char cust_id[MAX];
    // we will limit the attempts user can make for authentication
    int attempt;
    for (attempt = 1; attempt <= 3 && !verify_credentials(clientfd, &user_type, cust_id); attempt++)
    {
        printf("Verification for client with ip address %s is failed.\n", cip_addr);
        bzero(packet, MAX);
        if (attempt < 3)
        {
            strcpy(packet, "attempt_failed");
            int x = write(clientfd, packet, strlen(packet));
            if (x < 0)
            {
                printerror("ERROR: Writing to socket failed.");
            }
        }
    }
    if (attempt >= 4)
    {
        // sending exit
        printf("attempts exceeded.");
        bzero(packet, MAX);
        strcpy(packet, "exit");
        int x = write(clientfd, packet, strlen(packet));
        return;
    }
    // fprintf(stdout, "Verification for client with ip address '%s' successful. \n", client_ip);
    // printf("customer id %s", cust_id);
    printf("Verification for client with ip address %s is successful %s.\n", cip_addr, cust_id);
    // sending user type
    bzero(packet, MAX);
    packet[0] = user_type;
    packet[1] = '\0';
    int x = write(clientfd, packet, strlen(packet));
    if (x < 0)
        printerror("ERROR writing to socket");

    // calling corresponding function
    if (user_type == 'C')
        customer(clientfd, cust_id, cip_addr);
    else if (user_type == 'A')
        admin(clientfd, cip_addr);
    else if (user_type == 'P')
        police(clientfd, cip_addr);

    return;
}
int main(int argc, char *argv[])
{
    /*
    here,   sockfd   = socket file descripter
            clientfd = client file descripter
            pid      = process id
    */
    int sockfd, clientfd, port, client_len;
    int pid, process_id;
    int child_count = 0;
    int enable = 1;
    struct sockaddr_in server_addr, client_addr;
    char *client_ip;
    int client_port;
    if (argc < 2)
    {
        fprintf(stderr, "ERROR: port is not provided. Please provide port number.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        printerror("ERROR: Opening socket failed.");

    bzero((char *)&server_addr, sizeof(server_addr));

    // converting port number into interger
    port = atoi(argv[1]);

    // setting server address family and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // seting options in the socket so that we can resuse the port addresses later
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        printerror("ERROR: Setting of socket as Reusable failed.");

    // binding socket to address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        printerror("ERROR: Binding failed.");

    fprintf(stdout, "Binding done.\n");

    // socket listening, though the parameter is set to 5, it may accept upto 8 connections.
    listen(sockfd, 5);
    fprintf(stdout, "Listening started with queue length 5.\n");

    client_len = sizeof(client_addr);
    while (1)
    {
        // accepting client request
        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (clientfd < 0)
            printerror("ERROR: Accepting connection failed.");
        pid = fork();
        if (pid < 0)
            printerror("ERROR: Forking the process failed.");
        if (pid)
        {
            // parent process
            // closing client
            close(clientfd);
            // handling zombie process
            while (child_count)
            {
                // Non-blocking wait
                process_id = waitpid((pid_t)-1, NULL, WNOHANG);
                if (process_id < 0)
                    printerror("ERROR: Cleaning zombie process failed.");
                else if (process_id == 0)
                    break;
                else
                    child_count--;
            }
        }
        else
        {
            // child process
            // closing listening socket here as we will handle connected client
            close(sockfd);

            client_ip = inet_ntoa(client_addr.sin_addr);
            client_port = ntohs(client_addr.sin_port);

            fprintf(stdout, "Connection accepted for client with ip address '%s' on port '%d'. \n", client_ip, client_port);

            // handling client
            serve_client(clientfd, client_ip);

            close(clientfd);
            fprintf(stdout, "Connection closed for client with ip address '%s' on port '%d'. \n", client_ip, client_port);

            exit(0);
        }
    }
}