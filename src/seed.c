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
#include "database.c"
#include "../include/structs.h"

int main(void)
{
    int account_fd = open("../DB/account_db.dat", O_CREAT|O_RDWR, 0744);
    int user_fd = open("../DB/user_db.dat", O_CREAT|O_RDWR, 0744);
    int transaction_fd = open("../DB/transaction_db.dat", O_CREAT|O_RDWR, 0744);
    int metadata_fd = open("../DB/meta_db.dat", O_CREAT|O_RDWR, 0744);

    struct metadata curr_metadata;
    struct user curr_admin, curr_user;
    curr_metadata.account_id = 0, curr_metadata.transaction_id = 0, curr_metadata.user_id = 0;
    write(metadata_fd, &curr_metadata, sizeof(curr_metadata));
    curr_admin.accountType = admin, curr_admin.id = -1, curr_admin.account_id = -1, strcpy(curr_admin.email, "admin@iiitb.org"), strcpy(curr_admin.password, "admin1234"), strcpy(curr_admin.name, "Admin");
    curr_user.accountType = normal, curr_user.id = -1, curr_user.account_id = -1, strcpy(curr_user.email, "gaurav@iiitb.org"), strcpy(curr_user.password, "gaurav1234"), strcpy(curr_user.name, "Gaurav");
    create_user(&curr_admin);
    create_user(&curr_user);
}