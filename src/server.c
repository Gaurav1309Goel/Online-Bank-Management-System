#include <stdio.h>
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
#include "database.c"

struct session curr_sessions[100000];
pthread_mutex_t sessions_lock;
int sd;

int create_socket(int port_num)
{
    int sd1 = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET, server.sin_addr.s_addr = INADDR_ANY, server.sin_port = htons (port_num);
    bind(sd1, (void *) &server, sizeof(server));
    listen(sd1, 100000);
    return sd1;
}

bool password_validate(struct login_request curr_request)
{
    struct user curr_user;
    int ret = get_user(curr_request.email, &curr_user);
    if(ret == -1 || strcmp(curr_request.password, curr_user.password) != 0)return false;
    return true;
}

void check_login(int nsd)
{
    struct login_request curr_request;
    read(nsd, &curr_request, sizeof(curr_request));
    bool val = password_validate(curr_request);
    struct login_response curr_response;
    curr_response.status = failure;
    if(!val)
    {
        curr_response.status = unauthorized;
        curr_response.session_id = -1;
    }
    else
    {
        struct user curr_user;
        get_user(curr_request.email, &curr_user);
        pthread_mutex_lock(&sessions_lock);
        for(int i = 0; i < 100000; i++)
        {
            if(!curr_sessions[i].isActive)
            {
                curr_sessions[i].isActive = true;
                curr_sessions[i].user = curr_user;
                curr_response.session_id = i;
                curr_response.loginType = curr_user.accountType;
                curr_response.status = success;
                break;
            }
        }
        pthread_mutex_unlock(&sessions_lock);
    }
    write(nsd, &curr_response, sizeof(curr_response));
}

void exit_client(int nsd, int session_id)
{
    struct response curr_response;
    curr_response.status = success;
    pthread_mutex_lock(&sessions_lock);
    curr_sessions[session_id].isActive = false;
    pthread_mutex_unlock(&sessions_lock);
    write(nsd, &curr_response, sizeof(curr_response));
}

void balance_enquiry(int nsd, int session_id)
{
    struct account curr_account;
    struct balance_enquiry_response curr_response;
    int ret = get_account(curr_sessions[session_id].user.account_id, &curr_account);
    if(ret == 0)
    {
        curr_response.status = success;
        curr_response.balance = curr_account.balance;
    }
    else curr_response.status = failure;
    write(nsd, &curr_response, sizeof(curr_response));
}

void account_add(int nsd, int session_id)
{
    struct user curr_user_request;
    struct user_response curr_response;
    read(nsd, &curr_user_request, sizeof(curr_user_request));
    if(curr_sessions[session_id].user.accountType == admin)
    {
        create_user(&curr_user_request);
        curr_response.status = success;
        curr_response.user = curr_user_request;
    }
    else curr_response.status = unauthorized;
    write(nsd, &curr_response, sizeof(curr_response));
}

void modify_account(int nsd, int session_id)
{
    struct user curr_user_request;
    struct response curr_response;
    read(nsd, &curr_user_request, sizeof(curr_user_request));
    if(curr_sessions[session_id].user.accountType == admin)
    {
        struct user user_temp;
        get_user_by_id(curr_user_request.id, &user_temp);
        curr_user_request.account_id = user_temp.account_id;
        int status = save_user(&curr_user_request);
        if (status != 0)curr_response.status = failure;
        else
        {
            curr_response.status = success;        
        }
    }
    else curr_response.status = unauthorized;
    write(nsd, &curr_response, sizeof(curr_response));  
}

void money_transaction(int nsd, int session_id)
{
    struct transaction_request curr_request;
    struct account curr_account;
    struct response curr_response;
    read(nsd, &curr_request, sizeof(curr_request));
    get_account(curr_sessions[session_id].user.account_id, &curr_account);

    int status, ini_balance = curr_account.balance;
    status = change_account_balance(curr_sessions[session_id].user.account_id, curr_request.amount);
    if(status != 0)curr_response.status = failure;
    else
    {
        get_account(curr_sessions[session_id].user.account_id, &curr_account);
        curr_response.status = success;
        struct transaction curr_transaction;
        curr_transaction.account_id = curr_sessions[session_id].user.account_id;
        curr_transaction.user_id = curr_sessions[session_id].user.id;
        curr_transaction.opening_balance = ini_balance;
        curr_transaction.closing_balance = curr_account.balance;
        curr_transaction.transactionType = curr_request.transactionType;
        strcpy(curr_transaction.name, curr_sessions[session_id].user.name);
        create_transaction(&curr_transaction);
    }
    write(nsd, &curr_response, sizeof(curr_request));
}

void change_password(int nsd, int session_id)
{
    struct change_password_request curr_request;
    struct response curr_response;
    read(nsd, &curr_request, sizeof(curr_request));
    strcpy(curr_sessions[session_id].user.password, curr_request.new_password);
    int status = save_user(&curr_sessions[session_id].user);
    if(status != 0)curr_response.status = failure;
    else curr_response.status = success;
    write(nsd, &curr_response, sizeof(curr_response));
}

void get_details(int nsd, int session_id)
{
    struct user curr_user;
    struct account curr_account;
    struct transaction curr_transactions[10000];
    struct response1 curr_response;
    int status = get_user(curr_sessions[session_id].user.email, &curr_user);
    if(status != 0)
    {
        curr_response.status = unauthorized;
        write(nsd, &curr_response, sizeof(curr_response));      
    }
    status = get_account(curr_user.account_id, &curr_account);
    if(status != 0)
    {
        curr_response.status = unauthorized;
        write(nsd, &curr_response, sizeof(curr_response));      
    }
    curr_response.count = get_transactions(curr_user.account_id, curr_transactions, 10000);
    curr_response.status = success;
    write(nsd, &curr_response, sizeof(curr_response));
    write(nsd, &curr_user, sizeof(curr_user));
    write(nsd, &curr_account, sizeof(curr_account));
    for(int i = 0; i < curr_response.count; i++)write(nsd, &curr_transactions[i], sizeof(curr_transactions[i]));
}

void get_users_server(int nsd, int session_id)
{
    struct user curr_users[10000];
    struct response1 curr_response;
    if(curr_sessions[session_id].user.accountType == admin)
    {
        curr_response.count = get_users(curr_users, 10000);
        curr_response.status = success; 
    }
    else curr_response.status = unauthorized;
    write(nsd, &curr_response, sizeof(curr_response));
    if(curr_response.status == success)
    {
        for(int i = 0; i < curr_response.count; i++)write(nsd, &curr_users[i], sizeof(curr_users[i]));
    }
}

void account_delete(int nsd, int session_id)
{
    struct delete_user_request curr_request;
    struct response curr_response;
    read(nsd, &curr_request, sizeof(curr_request));
    if(curr_sessions[session_id].user.accountType == admin)
    {
        int status = delete_user(curr_request.user_id);
        if(status != 0)curr_response.status = failure;
        else curr_response.status = success;
    }
    else curr_response.status = unauthorized;
    write(nsd, &curr_response, sizeof(curr_response));
}

void* request_handler(void *socketdesc)
{
    int nsd = *(int *)socketdesc;
    struct header curr_header;
    read(nsd, &curr_header, sizeof(curr_header));
    if(curr_header.action == login)check_login(nsd);
    else if(curr_header.action == Balance_enquiry)balance_enquiry(nsd, curr_header.session_id);    
    else if(curr_header.action == transaction_money)money_transaction(nsd, curr_header.session_id);
    else if(curr_header.action == password_change)change_password(nsd, curr_header.session_id);
    else if(curr_header.action == view_details)get_details(nsd, curr_header.session_id);
    else if(curr_header.action == admin_all_users)get_users_server(nsd, curr_header.session_id);
    else if(curr_header.action == Modify_account)modify_account(nsd, curr_header.session_id);
    else if(curr_header.action == delete_account)account_delete(nsd, curr_header.session_id);
    else if(curr_header.action == add_account)account_add(nsd, curr_header.session_id);
    else if(curr_header.action == Exit)exit_client(nsd, curr_header.session_id);
}

void kill_server(int sig)
{
    printf("\nClosing Server\n");
    close(sd);
    exit(0);
}

int setup_handler()
{
    int sd = create_socket(5000);
    for(int i = 0; i < 100000; i++)curr_sessions[i].isActive = false;
    signal(SIGINT, kill_server);
    pthread_mutex_init(&sessions_lock, NULL);
    return sd;
}

int main()
{
    sd = setup_handler();
    while (1)
    {
        int nsd = accept(sd, NULL, NULL);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &request_handler, &nsd);
    }
    close(sd);
}