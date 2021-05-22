#include "../include/structs.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int lock(int fd, int lock_type, int size, int record)
{
    struct flock fl;
    fl.l_type = lock_type, fl.l_whence = SEEK_SET, fl.l_start = size * record, fl.l_len = size, fl.l_pid = getpid();
    return fcntl(fd, F_SETLKW, &fl);
}

void create_account(struct account *curr_account)
{
    if (curr_account->id == -1 )
    {
        int meta_fd = open("../DB/meta_db.dat", O_RDWR);
        struct metadata curr_metadata;
        lock(meta_fd, F_WRLCK, 0, 0);
        read(meta_fd, &curr_metadata, sizeof(curr_metadata));
        curr_account->id = curr_metadata.account_id;
        curr_metadata.account_id++;
        lseek(meta_fd, 0, SEEK_SET);
        write(meta_fd, &curr_metadata, sizeof(curr_metadata));
        lock(meta_fd, F_ULOCK, 0, 0);
        int account_fd = open("../DB/account_db.dat", O_APPEND | O_WRONLY);
        write(account_fd, curr_account, sizeof(*curr_account));
        close(account_fd);
        close(meta_fd);   
    } 
}

void create_user(struct user *curr_user)
{
    int meta_fd = open("../DB/meta_db.dat", O_RDWR);
    struct metadata curr_metadata;
    lock(meta_fd, F_WRLCK, 0, 0);
    read(meta_fd, &curr_metadata, sizeof(curr_metadata));
    curr_user->id = curr_metadata.user_id;
    curr_metadata.user_id++;
    lseek(meta_fd, 0, SEEK_SET);
    write(meta_fd, &curr_metadata, sizeof(curr_metadata));
    lock(meta_fd, F_ULOCK, 0, 0);
    struct account curr_account;
    curr_account.balance = 0, curr_account.id = curr_user->account_id;
    create_account(&curr_account);
    curr_user->account_id = curr_account.id;
    int user_fd = open("../DB/user_db.dat", O_APPEND | O_WRONLY);
    write(user_fd, curr_user, sizeof(*curr_user));
    close(user_fd);
    close(meta_fd);
}

void create_transaction(struct transaction *curr_transaction)
{
    int meta_fd = open("../DB/meta_db.dat", O_RDWR);
    struct metadata curr_metadata;
    lock(meta_fd, F_WRLCK, 0, 0);
    read(meta_fd, &curr_metadata, sizeof(curr_metadata));
    curr_transaction->id = curr_metadata.transaction_id;
    curr_metadata.transaction_id++;
    lseek(meta_fd, 0, SEEK_SET);
    write(meta_fd, &curr_metadata, sizeof(curr_metadata));
    lock(meta_fd, F_ULOCK, 0, 0);
    int transaction_fd = open("../DB/transaction_db.dat", O_APPEND | O_WRONLY);
    write(transaction_fd, curr_transaction, sizeof(*curr_transaction));
    close(transaction_fd);
    close(meta_fd);    
}

int get_user(char email[], struct user *curr_user)
{
    int user_fd = open("../DB/user_db.dat", O_RDONLY);
    lock(user_fd, F_RDLCK, 0, 0 );
    int siz;
    while(1)
    {
        siz = read(user_fd, curr_user, sizeof(*curr_user));
        if(siz == 0)break;
        if(strcmp(curr_user->email, email) == 0 && curr_user->id != -1)
        {
            lock(user_fd, F_UNLCK, 0, 0);
            close(user_fd);
            return 0;
        }
    }
    lock(user_fd, F_UNLCK, 0, 0);
    close(user_fd);
    return -1;
}

int get_user_by_id(int user_id, struct user *curr_user)
{
    int user_fd = open("../DB/user_db.dat", O_RDONLY);
    lock(user_fd, F_RDLCK, 0, 0);
    int siz;
    while(1)
    {
        siz = read(user_fd, curr_user, sizeof(*curr_user));
        if(siz == 0)break;
        if(curr_user->id == user_id)
        {
            lock(user_fd, F_UNLCK, 0, 0);
            close(user_fd);
            return 0;
        }
    }
    lock(user_fd, F_UNLCK, 0, 0);
    close(user_fd);
    return -1;   
}

int get_account(int account_id, struct account *curr_account)
{
    int account_fd = open("../DB/account_db.dat", O_RDONLY);
    lock(account_fd, F_RDLCK, 0, 0);
    int siz;
    while(1)
    {
        siz = read(account_fd, curr_account, sizeof(*curr_account));
        if(siz == 0)break;
        if(curr_account->id == account_id)
        {
            lock(account_fd, F_UNLCK, 0, 0);
            close(account_fd);
            return 0;
        }
    }
    lock(account_fd, F_UNLCK, 0, 0);
    close(account_fd);
    return -1;
}

int get_users(struct user curr_users[], int maxi)
{
    int user_fd = open("../DB/user_db.dat", O_RDONLY);
    lock(user_fd, F_RDLCK, 0, 0);
    int siz, count = 0;
    struct user curr_user;
    while (1)
    {
        siz = read(user_fd, &curr_user, sizeof(curr_user));
        if(siz == 0 || maxi == count)break;
        if(curr_user.id != -1)
        {
            curr_users[count] = curr_user;
            count++; 
        }
    }
    lock(user_fd, F_ULOCK, 0, 0);
    close(user_fd);
    return count;
}

int get_accounts(struct account curr_accounts[], int maxi)
{
    int account_fd = open("../DB/account_db.dat", O_RDONLY);
    lock(account_fd, F_RDLCK, 0, 0);
    int siz, count = 0;
    struct account curr_account;
    while (1)
    {
        siz = read(account_fd, &curr_account, sizeof(curr_account));
        if(siz == 0 || maxi == count)break;
        if(curr_account.id != -1)
        {
            curr_accounts[count] = curr_account;
            count++; 
        }
    }
    lock(account_fd, F_ULOCK, 0, 0);
    close(account_fd);
    return count;    
}

int get_transactions(int account_id, struct transaction curr_transactions[], int maxi)
{
    int transaction_fd = open("../DB/transaction_db.dat", O_RDONLY);
    lock(transaction_fd, F_RDLCK, 0, 0);
    int siz, count = 0;
    struct transaction curr_transaction;
    while (1)
    {
        siz = read(transaction_fd, &curr_transaction, sizeof(curr_transaction));
        if(siz == 0 || maxi == count)break;
        if(curr_transaction.account_id == account_id)
        {
            curr_transactions[count] = curr_transaction;
            count++; 
        }
    }
    lock(transaction_fd, F_ULOCK, 0, 0);
    close(transaction_fd);
    return count;  
}

int save_user(struct user *curr_user)
{
    int user_fd = open("../DB/user_db.dat", O_RDWR);
    int siz;
    struct user temp_user;
    while(1)
    {
        siz = read(user_fd, &temp_user, sizeof(temp_user));
        if(temp_user.id == curr_user->id || siz == 0)break;
    }
    if(temp_user.id == curr_user->id)
    {
        int posi = lseek(user_fd, -sizeof(temp_user), SEEK_CUR);
        lock(user_fd, F_WRLCK, sizeof(temp_user), posi/sizeof(temp_user));
        write(user_fd, curr_user, sizeof(*curr_user));
        lock(user_fd, F_UNLCK, 0, 0);
        close(user_fd);
        return 0;
    }
    close(user_fd);
    return -1;
}

int delete_user(int user_id)
{
    int user_fd = open("../DB/user_db.dat", O_RDWR);
    int siz;
    struct user temp_user;
    while(1)
    {
        siz = read(user_fd, &temp_user, sizeof(temp_user));
        if(temp_user.id == user_id || siz == 0)break;
    }
    if(temp_user.id == user_id)
    {
        int posi = lseek(user_fd, -sizeof(temp_user), SEEK_CUR);
        lock(user_fd, F_WRLCK, sizeof(temp_user), posi/sizeof(temp_user));
        temp_user.id = -1;
        write(user_fd, &temp_user, sizeof(temp_user));
        lock(user_fd, F_UNLCK, 0, 0);
        close(user_fd);
        return 0;   
    }
    close(user_fd);
    return -1;
}

int change_account_balance(int account_id, float amount)
{
    int account_fd = open("../DB/account_db.dat", O_RDWR);
    int siz;
    struct account curr_account;
    while(1)
    {
        siz = read(account_fd, &curr_account, sizeof(curr_account));
        if(account_id == curr_account.id)break;
    }
    if(curr_account.id == account_id)
    {
        int posi = lseek(account_fd, -sizeof(curr_account), SEEK_CUR);
        lock(account_fd, F_WRLCK, sizeof(curr_account), posi/sizeof(curr_account));
        if(curr_account.balance + amount < 0)
        {
            lock(account_fd, F_UNLCK, sizeof(curr_account), posi/sizeof(curr_account));
            close(account_fd);
            return -1;
        }
        else
        {
            curr_account.balance += amount;
            write(account_fd, &curr_account, sizeof(curr_account));
            lock(account_fd, F_UNLCK, sizeof(curr_account), posi/sizeof(curr_account));
            close(account_fd);
            return 0;           
        }
    }
    else
    {
        close(account_fd);
        return -1;
    }
}