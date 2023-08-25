#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define delim "*****"
#define MAX 5000
void printerror(char *errmsg)
{
    perror(errmsg);
    exit(0);
}
// customer code
void mini_statement(int sockfd)
{
    char packet[MAX];
    // delimeter string
    bzero(packet, MAX);
    strcpy(packet, "size");
    int n = write(sockfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR writing to socket");

    // file size
    bzero(packet, MAX);
    n = read(sockfd, packet, MAX - 1);
    if (n < 0)
        printerror("ERROR reading from socket");

    int file_size = atoi(packet);
    int remain_data = file_size;

    // delimeter string
    bzero(packet, MAX);
    strcpy(packet, "content");
    n = write(sockfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR writing to socket");

    // mini statement
    printf("MINI STATEMENT: \n");
    bzero(packet, MAX);
    while ((remain_data > 0) && ((n = read(sockfd, packet, MAX)) > 0))
    {
        printf("%s", packet);
        remain_data -= n;
        bzero(packet, MAX);
    }
    printf("\n\n");
}
void balance(int sockfd)
{
    char packet[MAX];
    // delimiter string
    bzero(packet, MAX);
    strcpy(packet, "content");
    int n = write(sockfd, packet, strlen(packet));
    if (n < 0)
        printerror("ERROR writing to socket");
    // balance
    bzero(packet, MAX);
    n = read(sockfd, packet, MAX - 1);
    if (n < 0)
        printerror("ERROR reading from socket");
    printf("BALANCE: %s\n\n", packet);
}
void customers(int sockfd)
{
    char packet[MAX];
    char operation[MAX];
    int n;
    char flag;

    printf("Do u want to continue (y/n): ");
    scanf("%c", &flag);
    getchar();

    while (flag == 'y')
    {
        // sending the flag
        bzero(packet, MAX);
        packet[0] = flag;
        packet[1] = '\0';
        n = write(sockfd, packet, strlen(packet));
        if (n < 0)
            printerror("ERROR writing to socket");

        printf("Operation to perform(BALANCE/MINI_STATEMENT): ");
        bzero(operation, MAX);
        fgets(operation, MAX, stdin);

        // sending the operation to perform
        n = write(sockfd, operation, strlen(operation));
        if (n < 0)
            printerror("ERROR writing to socket");

        operation[strlen(operation) - 1] = '\0';

        // true or false
        bzero(packet, MAX);
        n = read(sockfd, packet, MAX - 1);
        if (n < 0)
            printerror("ERROR reading from socket");

        if (!strcmp(packet, "not_matched"))
        {
            printf("Invalid Operation.\n\n");
        }
        else if (!strcmp(packet, "matched"))
        {
            if (!strcmp(operation, "MINI_STATEMENT"))
            {
                mini_statement(sockfd);
            }
            else if (!strcmp(operation, "BALANCE"))
            {
                balance(sockfd);
            }
        }
        printf("Do u want to continue (y/n): ");
        scanf("%c", &flag);
        getchar();
    }
    // sending flag
    bzero(packet, MAX);
    packet[0] = flag;
    packet[1] = '\0';
    n = write(sockfd, packet, strlen(packet));
}
// admin code
void get_id_trans_amt(char *packet)
{
    char id[MAX], trans[MAX], amount[MAX];
    // char packet[MAX];
    printf("User ID of Customer: ");
    bzero(id, MAX);
    fgets(id, MAX, stdin);

    printf("Transaction Type(debit/credit): ");
    bzero(trans, MAX);
    fgets(trans, MAX, stdin);

    printf("Amount: ");
    bzero(amount, MAX);
    fgets(amount, MAX, stdin);

    bzero(packet, MAX);
    strcat(packet, id);
    strcat(packet, delim);
    strcat(packet, trans);
    strcat(packet, delim);
    strcat(packet, amount);
    strcat(packet, delim);
}
void reply_client(char *packet)
{
    if (!strcmp(packet, "false"))
    {
        printf("Transaction denied.\n\n");
    }
    else if (!strcmp(packet, "true"))
    {
        printf("Transaction successful.\n\n");
    }
    else if (!strcmp(packet, "deficit"))
    {
        printf("Insufficient Amount.\n\n");
    }
}
void admin(int sockfd)
{
    char packet[MAX];

    int n;
    char flag;

    printf("Do u want to continue (y/n): ");
    scanf("%c", &flag);
    getchar();

    while (flag == 'y')
    {
        bzero(packet, MAX);
        packet[0] = flag;
        packet[1] = '\0';
        n = write(sockfd, packet, strlen(packet));
        if (n < 0)
            printerror("ERROR: writing to socket");
        get_id_trans_amt(packet);
        // sending command
        n = write(sockfd, packet, strlen(packet));
        if (n < 0)
            printerror("ERROR: writing to socket");

        // true or false
        bzero(packet, MAX);
        n = read(sockfd, packet, MAX - 1);
        if (n < 0)
            printerror("ERROR: reading from socket");
        reply_client(packet);
        printf("Do u want to continue (y/n): ");
        scanf("%c", &flag);
        getchar();
    }
    // sending flag
    bzero(packet, MAX);
    packet[0] = flag;
    packet[1] = '\0';
    n = write(sockfd, packet, strlen(packet));
}
void getCredentials(char *packet)
{
    char username[MAX];
    char password[MAX];

    printf("Enter Username: ");
    bzero(username, MAX);
    fgets(username, MAX, stdin);

    printf("Enter Password: ");
    bzero(password, MAX);
    fgets(password, MAX, stdin);

    bzero(packet, MAX);
    strcat(packet, username);
    strcat(packet, delim);
    strcat(packet, password);
}
// police code
void send_flag(int sockfd, char *packet, char f)
{
    bzero(packet, MAX);
    packet[0] = f;
    packet[1] = '\0';
    int x = write(sockfd, packet, strlen(packet));
    if (x < 0)
        printerror("ERROR!! writing to socket..");
}

char ask_continuation(int sockfd, char *packet, char f)
{
    printf("Do u want to continue (y/n): ");
    scanf("%c", &f);
    getchar();
    send_flag(sockfd, packet, f);
    return f;
}

void input_services(char *packet, char *user_id, char *operation)
{
    bzero(packet, MAX);
    bzero(operation, MAX);
    bzero(user_id, MAX);

    printf("Operation to perform(balance/mini_statement): ");
    fgets(operation, MAX, stdin);
    strcat(packet, operation);
    strcat(packet, delim);
    operation[strlen(operation) - 1] = '\0';

    printf("User ID of Customer: ");
    fgets(user_id, MAX, stdin);
    strcat(packet, user_id);
}

void read_socket(int sockfd, char *packet)
{
    bzero(packet, MAX);
    int x = read(sockfd, packet, MAX - 1);
    if (x < 0)
        printerror("ERROR reading from socket");
}

void print_miniStatement(int socketfd, char *packet)
{
    int file_size = atoi(packet);
    int remain_data = file_size;

    bzero(packet, MAX);
    strcpy(packet, "content");
    int x = write(socketfd, packet, strlen(packet));
    if (x < 0)
        printerror("ERROR writing to socket");

    printf("MINI STATEMENT: \n");
    bzero(packet, MAX);
    while ((remain_data > 0) && ((x = read(socketfd, packet, MAX)) > 0))
    {
        printf("%s", packet);
        remain_data -= x;
        bzero(packet, MAX);
    }
    printf("\n\n");
}

void police(int sockfd)
{
    char packet[MAX], user_id[MAX], operation[MAX];
    int n;
    char flag;

    flag = ask_continuation(sockfd, packet, flag);

    while (flag == 'y')
    {
        /* input for services */
        input_services(packet, user_id, operation);

        // sending command
        n = write(sockfd, packet, strlen(packet));
        if (n < 0)
            printerror("ERROR writing to socket");

        // true or false
        read_socket(sockfd, packet);

        if (!strcmp(packet, "false"))
        {
            printf("Invalid Operation.\n\n");
        }
        else if (!strcmp(packet, "true"))
        {
            if (!strcmp(operation, "balance"))
            {
                // delimeter string
                bzero(packet, MAX);
                strcpy(packet, "content");
                n = write(sockfd, packet, strlen(packet));
                if (n < 0)
                    printerror("ERROR writing to socket");

                // balance
                // printf("fasd");
                read_socket(sockfd, packet);
                printf("BALANCE: %s\n\n", packet);
            }
            else if (!strcmp(operation, "mini_statement"))
            {
                // delimeter string
                bzero(packet, MAX);
                strcpy(packet, "size");
                n = write(sockfd, packet, strlen(packet));
                if (n < 0)
                    printerror("ERROR writing to socket");

                // file size
                read_socket(sockfd, packet);

                print_miniStatement(sockfd, packet);
            }
        }

        flag = ask_continuation(sockfd, packet, flag);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, port, n;
    char packet[MAX];

    struct sockaddr_in server_addr;
    struct hostent *server;

    if (argc < 3)
    {
        fprintf(stderr, "ERROR: Please provide hostname(ip address) and port");
        exit(0);
    }

    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printerror("ERROR: Opening socket failed.");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR: Host not found.\n");
        exit(0);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);

    // connecting to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        printerror("ERROR: Connecting to server failed.");

    char user_type;
    // Authenticating the client
    printf("Hello Sir, How are you? In order to help you we first need to authenticate you.\n So please enter your credentials as asked:\n");

    for (;;)
    {
        bzero(packet, MAX);
        getCredentials(packet);

        // sending the credentials to server
        int x = write(sockfd, packet, strlen(packet));
        if (x < 0)
        {
            printerror("ERROR: Sending credentials to the server failed.");
        }

        //
        bzero(packet, MAX);
        x = read(sockfd, packet, MAX - 1);
        // printf("%s",packet);
        if (x < 0)
            printerror("ERROR: Getting response on credentials failed.");

        if (!strcmp(packet, "exit"))
        {
            printf("You have entered the invalid credentials for three times. Exiting...\n");
            return 0;
        }

        if (strcmp(packet, "attempt_failed"))
        {
            user_type = packet[0];
            break;
        }
        printf("Invalid Credentitals.Please enter valid credentials. \n");
    }

    printf("Authentication Successful.\n");
    // diverting the services as per the user_type
    if (user_type == 'C')
    {
        printf("Welcome Bank Customer.\n");
        customers(sockfd);
    }
    else if (user_type == 'A')
    {
        printf("Welcome Bank Admin.\n");
        admin(sockfd);
    }
    else if (user_type == 'P')
    {
        printf("Welcome Police.\n");
        police(sockfd);
    }

    // closing the socket
    close(sockfd);
    return 0;
}