#pragma once

#include <stdbool.h>
#include <time.h>

enum action{
    login,
    transaction_money,
    Balance_enquiry,
    password_change,
    view_details,
    admin_all_users,
    Modify_account,
    delete_account,
    add_account,
    Exit
};

enum account_type{
    admin,
    normal
};

enum status_check{
    success,
    unauthorized,
    failure
};

enum transaction_type{
    deposit,
    withdraw
};

struct header{
    enum action action;
    int session_id;
};

struct response{
    enum status_check status;
};

struct user{
    enum account_type accountType;
    char name[100];
    char email[100];
    char password[100];
    int id;
    int account_id;
};

struct user_response {
    enum status_check status;
    struct user user;
};

struct account{
    int id;
    double balance;
};

struct login_request{
    char email[100];
    char password[100];
};

struct login_response{
    enum status_check status;
    int session_id;
    enum account_type loginType;
};

struct change_password_request{
    char new_password[20];
};

struct delete_user_request{
    int user_id;
};

struct transaction{
    int account_id;
    int user_id;
    char name[100];
    double opening_balance;
    double closing_balance;
    enum transaction_type transactionType;
    int id;
};

struct transaction_request{
    double amount;
    enum transaction_type transactionType;
};

struct balance_enquiry_response{
    enum status_check status;
    double balance;
};

struct metadata{
    int user_id;
    int account_id;
    int transaction_id;
};

struct response1{
    enum status_check status;
    int count;
};

struct session{
    bool isActive;
    struct user user;
};