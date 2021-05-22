#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>  
#include <sys/types.h> 
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/structs.h"

int connect_socket(int port_num)
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET, server.sin_addr.s_addr = INADDR_ANY, server.sin_port = htons (port_num);
    connect(sd, (struct sockaddr*) &server, sizeof(server));
    return sd;
}

void login_client(int *session_id, enum account_type *curr_account_type)
{
    int sd = connect_socket(5000);
    struct header curr_header;
    struct login_request curr_request;
    struct login_response curr_response;
    printf("Please enter the email: ");
    scanf("%s", curr_request.email);
    printf("Please enter the password: ");
    scanf("%s", curr_request.password);
    curr_header.action = login, curr_header.session_id = -1;
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_request, sizeof(curr_request));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)
    {
        *session_id = curr_response.session_id;
        *curr_account_type = curr_response.loginType;
    }
    else
    {
        printf("You are unauthorized.\n");
        exit(0);
    }
}

int next_work()
{
    int curr_operation_num;
    printf("1 for transaction_money\n");
    printf("2 for balance_enquiry\n");
    printf("3 for password_change\n");
    printf("4 for view_details\n");
    printf("5 for view_all_users\n");
    printf("6 for modify_account\n");
    printf("7 for delete_account\n");
    printf("8 for add_account\n");
    printf("9 for exit\n");
    printf("Please enter the operation number: ");
    scanf("%d", &curr_operation_num);
    return curr_operation_num;
}

int balance_client(int sd, int session_id)
{
    struct header curr_header;
    struct balance_enquiry_response curr_response;
    curr_header.action = Balance_enquiry, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Currrent Balance in your account: %f\n", curr_response.balance);
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

void exit_client(int sd, int session_id)
{
    struct header curr_header;
    struct response curr_response;
    curr_header.action = Exit, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully exit from the server.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
    close(sd);
}

void add_client(int sd, int session_id, bool joint)
{
    struct user curr_user;
    struct header curr_header;
    struct user_response curr_response;
    printf("Please enter the username: ");
    scanf("%s", curr_user.name);
    printf("Please enter the email Id: ");
    scanf("%s", curr_user.email);
    printf("Please enter the password: ");
    scanf("%s", curr_user.password);
    curr_user.accountType = normal;

    if (joint == true)
    {
        printf("Please enter the account ID: ");
        scanf("%d", &curr_user.account_id);
    }
    curr_header.action = add_account, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_user, sizeof(curr_user));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully created the user with Account ID: %d and User ID: %d\n", curr_response.user.account_id, curr_response.user.id);
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

void modify_client(int sd, int session_id)
{
    struct user curr_user;
    struct header curr_header;
    struct response curr_response;
    int temp;
    printf("Please enter the User ID: ");
    scanf("%d", &curr_user.id);
    printf("Please enter the new Name: ");
    scanf("%s", curr_user.name);
    printf("Please enter the new email Id: ");
    scanf("%s", curr_user.email);
    printf("Please enter the new password: ");
    scanf("%s", curr_user.password);
    curr_user.accountType = normal;

    curr_header.action = Modify_account, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_user, sizeof(curr_user));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully created the user.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

void delete_client(int sd, int session_id)
{
    struct header curr_header;
    struct delete_user_request curr_request;
    struct response curr_response;
    curr_header.action = delete_account, curr_header.session_id = session_id;
    printf("Pleade enter the user ID: ");
    scanf("%d", &curr_request.user_id);
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_request, sizeof(curr_request));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully deleted the user.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

void deposit_or_withdraw(int sd, int session_id)
{
    struct header curr_header;
    struct transaction_request curr_request;
    struct response curr_response;
    curr_header.action = transaction_money, curr_header.session_id = session_id;
    printf("Please enter the amount: ");
    scanf("%lf", &curr_request.amount);
    if(curr_request.amount < 0 )curr_request.transactionType = withdraw;
    else curr_request.transactionType = deposit;
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_request, sizeof(curr_request));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully done the transaction.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

void change_password_client(int sd, int session_id)
{
    struct header curr_header;
    struct change_password_request curr_request;
    struct response curr_response;
    curr_header.action = password_change, curr_header.session_id = session_id;
    printf("Pleade enter the new password: ");
    scanf("%s", curr_request.new_password);
    write(sd, &curr_header, sizeof(curr_header));
    write(sd, &curr_request, sizeof(curr_request));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)printf("Successfully change the password.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}   

void view_details_client(int sd, int session_id)
{
    struct header curr_header;
    struct response1 curr_response;
    struct user curr_user;
    struct account curr_account;
    curr_header.action = view_details, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    read(sd, &curr_response, sizeof(curr_response));
    read(sd, &curr_user, sizeof(curr_user));
    read(sd, &curr_account, sizeof(curr_account));
    struct transaction curr_transactions[curr_response.count];
    for(int i = 0; i < curr_response.count; i++)read(sd, &curr_transactions[i], sizeof(curr_transactions[i]));
    if(curr_response.status == success)printf("Successfully got the details.\n");
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");  
    if(curr_response.status == success)
    {
        printf("User ID: %d\n", curr_user.id);
        printf("Account ID: %d\n", curr_user.account_id);
        printf("Account type: %d\n", curr_user.accountType);
        printf("Password: %s\n", curr_user.password);
        printf("Name: %s\n", curr_user.name);
        printf("Email: %s\n", curr_user.email);
        printf("Balance: %f\n", curr_account.balance);
    }  
}

void view_all_users_client(int sd, int session_id)
{
    struct header curr_header;
    struct response1 curr_response;
    curr_header.action = admin_all_users, curr_header.session_id = session_id;
    write(sd, &curr_header, sizeof(curr_header));
    read(sd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)
    {
        printf("Successfully got all the users.\n");
        struct user curr_users[curr_response.count];
        for(int i = 0; i < curr_response.count; i++)read(sd, &curr_users[i], sizeof(curr_users[i]));
        for(int i = 0; i < curr_response.count; i++)
        {
            printf("User ID: %d\n", curr_users[i].id);
            printf("Account ID: %d\n", curr_users[i].account_id);
            printf("Account type: %d\n", curr_users[i].accountType);
            printf("Password: %s\n", curr_users[i].password);
            printf("Name: %s\n", curr_users[i].name);
            printf("Email: %s\n", curr_users[i].email);          
        }
    }
    else if(curr_response.status == failure)printf("Some error from the server side.\n");
    else printf("Unauthorized\n");
}

int main(void)
{
    int session_id;
    enum account_type curr_type;
    login_client(&session_id, &curr_type);
    while (1)
    {
        int curr_operation = next_work();
        int sd = connect_socket(5000);
        if(curr_operation == 1)deposit_or_withdraw(sd, session_id);
        else if(curr_operation == 2)balance_client(sd, session_id);
        else if(curr_operation == 3)change_password_client(sd, session_id);
        else if(curr_operation == 4)view_details_client(sd, session_id);
        else if(curr_operation == 5)view_all_users_client(sd, session_id);
        else if(curr_operation == 6)modify_client(sd, session_id);
        else if(curr_operation == 7)delete_client(sd, session_id);
        else if(curr_operation == 8)
        {
            int temp;
            bool val;
            printf("0 for Normal and 1 for Joint: ");
            scanf("%d", &temp);
            val = temp;
            add_client(sd, session_id, val);
        }
        else 
        {
            exit_client(sd, session_id);
            break;
        }
    }
}