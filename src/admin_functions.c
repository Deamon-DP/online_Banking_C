#define ADMIN_HELPER_FUNCTIONS
#include "./helper_structures.h"
#define max_str 4096
#define ADMIN_ID "dp1811"
#define ADMIN_PASSWORD "1234"
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>

#include<stdio.h>
bool admin_login_check(int socket_descriptor)
{
    struct Customer customer;

    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    // char var[max_str] = "ENTER THE ADMIN LOGIN CREDENTIALS\nLOGIN ID";

    strcpy(write_buffer, "ENTER THE ADMIN LOGIN CREDENTIALS\nLOGIN ID");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    if (wbytes == -1)
    {
        perror("Error while asking login credentials to admin");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

    if (rbytes == -1)
    {
        perror("ERROR while reading login ID of ADMIN");

        return false;
    }
    else
    {
        printf("\nADMIN Login ID received :%s\n", read_buffer);
    }

    if (strcmp(read_buffer, ADMIN_ID) == 0)
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "Password:");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking password of admin");

            return false;
        }
        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
        if (rbytes == -1)
        {
            perror("error while reading Password from ADMIN");

            return false;
        }
        else
        {
            printf("ADMIN password received:%s\n", read_buffer);
        }
        if (strcmp(read_buffer, ADMIN_PASSWORD) == 0)
        {
            return true;
        }
        else
        {
            bzero(write_buffer, sizeof(write_buffer));
            printf("ERROR:Invalid Admin Password\n");
            strcpy(write_buffer, "SHOW+\nERROR:Invalid Admin Password");

            send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
            return false;
        }
    }
    else
    {
        printf("Wrong Admin Login Id\n");

        bzero(write_buffer, sizeof(write_buffer));
        printf("ERROR:Invalid Admin ID\n");
        strcpy(write_buffer, "SHOW+\nERROR:Invalid Admin ID");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
        return false;
    }
    return false;
}
int add_customer(int socket_descriptor, int account_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    struct Customer new, last_added;

    //--------------------------------------------------CustomerID-----------------------------------------------
    int customer_descriptor = open("./database/customers.txt", O_CREAT, S_IRWXU);

    // if (errno == ENOENT)
    // {
    //     new.account_number = 0;
    //     creat("./database/customers.txt", S_IRWXU);
    // }
    rbytes = read(customer_descriptor, read_buffer, sizeof(read_buffer));

    if (rbytes == 0)
    {
        new.customer_ID = 0;
    }
    else
    {
        if (customer_descriptor == -1)
        {
            perror("error while opening customer database");

            return false;
        }

        off_t offset = lseek(customer_descriptor, -sizeof(struct Customer), SEEK_END);

        if (offset == -1)
        {
            perror("Error while getting offset of customer file");

            return false;
        }

        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_END;
        lock.l_len = sizeof(struct Customer);
        lock.l_start = offset;
        lock.l_pid = getpid();
        int lck = fcntl(customer_descriptor, F_SETLKW, &lock);

        if (lck == -1)
        {
            perror("error while acquring a lock on account file");

            return false;
        }
        rbytes = read(customer_descriptor, &last_added, sizeof(last_added));

        if (rbytes == -1)
        {
            perror("error while reading from account file");

            return false;
        }

        new.customer_ID = last_added.customer_ID + 1;

        lock.l_type = F_UNLCK;

        fcntl(customer_descriptor, F_SETLK, &lock);

        close(customer_descriptor);
    }
    //---------------------------------------------Customer Name---------------------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter The Customer Name:", sizeof("Enter The Customer Name:"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking name of customer");

        return -1;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading customer name");

        return -1;
    }
    if (strlen(read_buffer) <= 50)
    {
        strcpy(new.customer_name, read_buffer);
    }
    else
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "ERROR :: customer name should be less than 50 letters");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        return -1;
    }

    //--------------------------------------AGE----------------------------------------------

    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter The Customer Age:", sizeof("Enter The Customer Age:"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking age of customer");

        return -1;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading customer age");

        return -1;
    }

    new.age = atoi(read_buffer);
    //----------------------------------------------GENDER--------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter The Customer Gender:1.M for Male\n2.F for Female\n3.O for Others", sizeof("Enter The Customer Gender:1.M for Male\n2.F for Female\n3.O for Others"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking customer gender");

        return -1;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading customer gender");

        return -1;
    }

    if (strlen(read_buffer) > 1 || (read_buffer[0] != 'M' && (read_buffer[0] != 'F') && read_buffer[0] != 'O' && read_buffer[0] != 'm' && (read_buffer[0] != 'f') && read_buffer[0] != 'o'))
    {
        bzero(write_buffer, sizeof(write_buffer));
        printf("ERROR :: customer gender should be M or F or O\n");
        strcpy(write_buffer, "SHOW+\nERROR :: customer gender should be M or F or O");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        recv(socket_descriptor,write_buffer,sizeof(write_buffer),0);

        return -1;
    }
    new.gender = read_buffer[0];
    //------------------------------------------Login-ID-----------------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter The Customer Login-ID", sizeof("Enter The Customer Login-ID"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking customer login ID");

        return -1;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading customer login ID");

        return -1;
    }
    if (strlen(read_buffer) <= 50)
    {
        strcpy(new.login_id, read_buffer);
    }
    else
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "ERROR :: customer login ID should be less than 50 letters");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        return -1;
    }
    //------------------------------------Password-----------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter The Customer Password", sizeof("Enter The Customer Password"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking customer password");

        return -1;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading customer password");

        return -1;
    }
    if (strlen(read_buffer) <= 50)
    {
        strcpy(new.password, read_buffer);
    }
    else
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "ERROR :: customer password should be less than 50 letters");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        return -1;
    }
    customer_descriptor = open("./database/customers.txt", O_APPEND | O_RDWR);

    if (customer_descriptor == -1)
    {
        perror("Error while opening accounts data");

        return -1;
    }

    wbytes = write(customer_descriptor, &new, sizeof(new));

    if (wbytes == -1)
    {
        perror("error while writing new account details to accounts database");

        return -1;
    }
    printf("NEW CUSTOMER CREATED WITH ID:%d\n", new.customer_ID);
    close(customer_descriptor);
    return new.customer_ID;
}

bool add_account(int socket_descriptor)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    struct Account new, last_added;
    //-------------------------------Account Number---------------------------------------
    int account_descriptor = open("./database/accounts.txt", O_CREAT, S_IRWXU);
    rbytes = read(account_descriptor, read_buffer, sizeof(read_buffer));
    // if (errno == ENOENT)
    // {
    //     new.account_number = 0;
    //     creat("./database/accounts.txt", S_IRWXU);
    // }
    if (rbytes == 0)
    {
        new.account_number = 0;
    }
    else
    {
        if (account_descriptor == -1)
        {
            perror("error while opening accounts file");
            return false;
        }
        off_t offset = lseek(account_descriptor, -sizeof(struct Account), SEEK_END);

        if (offset == -1)
        {
            perror("Error while getting offset of account file");
            return false;
        }

        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_END;
        lock.l_len = sizeof(struct Account);
        lock.l_start = offset;
        lock.l_pid = getpid();
        int lck = fcntl(account_descriptor, F_SETLKW, &lock);
        if (lck == -1)
        {
            perror("error while acquring a lock on account file");
            return false;
        }
        rbytes = read(account_descriptor, &last_added, sizeof(last_added));
        if (rbytes == -1)
        {
            perror("error while reading from account file");
            return false;
        }

        new.account_number = last_added.account_number + 1;

        close(account_descriptor);
        lock.l_type = F_UNLCK;
        fcntl(account_descriptor, F_SETLK, &lock);
    }
    //---------------------------------Account Type-------------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter Type of account\n1.Regular Account\n2.Joint Account", sizeof("Enter Type of account\n1.Regular Account\n2.Joint Account"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking type of account");

        return false;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading account type");

        return false;
    }

    int account_type = atoi(read_buffer);
    //----------------------------------------------CustomerID-----------------------------------------
    if (account_type = 1)
    {

        new.regular_account = true;

        new.customer_ID[0] = add_customer(socket_descriptor, new.account_number);

        new.customer_ID[1] = -1;
    }

    else
    {
        new.regular_account = false;

        int cust_id = add_customer(socket_descriptor, new.account_number);

        if (cust_id >= 0)
            new.customer_ID[0] = cust_id; // assigning customer id for 1st customer;
        else
            return 0;

        cust_id = add_customer(socket_descriptor, new.account_number);

        if (cust_id >= 0)
            new.customer_ID[1] = cust_id; // assigning customer id for 2nd customer;
        else
            return 0;
    }
    //---------------------------------------------Balance--------------------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    wbytes = send(socket_descriptor, "Enter the Initial deposited amount in the account", sizeof("Enter the Initial deposited amount in the account"), 0);

    if (wbytes == -1)
    {
        perror("Error while asking balance in the account");

        return false;
    }

    bzero(read_buffer, sizeof(read_buffer));

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading account balance");
        return false;
    }

    new.balance = atoi(read_buffer);

    new.active = true;

    account_descriptor = open("./database/accounts.txt", O_APPEND | O_RDWR);

    if (account_descriptor == -1)
    {
        perror("Error while opening accounts data");

        return false;
    }
    // off_t offset = lseek(account_descriptor, 0, SEEK_SET);
    wbytes = write(account_descriptor, &new, sizeof(new));

    if (wbytes == -1)
    {
        perror("error while writing new account details to accounts database");

        return false;
    }
    printf("Account Created with ID:%ld\n", new.account_number);
    close(account_descriptor);
    return true;
}

bool send_new_account_info(int socket_descriptor)
{
    struct Account account;
    struct Customer customer;

    int account_descriptor = open("./database/accounts.txt", O_RDWR);
    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return false;
    }

    off_t offset = lseek(account_descriptor, -sizeof(struct Account), SEEK_END);
    off_t customer_offset = lseek(customer_descriptor, -sizeof(struct Customer), SEEK_END);

    if (offset == -1 || customer_offset == -1)
    {
        perror("Error while getting offset of account or customer file");

        return false;
    }

    struct flock lock, clock;
    lock.l_type = F_RDLCK;
    clock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    clock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Account);
    clock.l_len = sizeof(struct Customer);
    lock.l_start = offset;
    clock.l_start = customer_offset;
    lock.l_pid = getpid();
    clock.l_pid = getpid();

    int lck = fcntl(account_descriptor, F_SETLKW, &lock);
    int clck = fcntl(customer_descriptor, F_SETLKW, &clock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account or customer file");
        return false;
    }
    ssize_t rbytes, wbytes;
    char read_buffer[max_str], write_buffer[max_str];
    rbytes = read(account_descriptor, &account, sizeof(account));
    read(customer_descriptor, &customer, sizeof(customer));
    if (rbytes == -1)
    {
        perror("error while reading from account file");
        return false;
    }
    char *type;
    char *active;
    if (account.active == true)

        active = "Active";
    else
        active = "NOT Active";
    if (account.regular_account)
        type = "Regular";
    else
        type = "Joint";
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);
    clock.l_type = F_UNLCK;
    fcntl(customer_descriptor, F_SETLK, &lock);
    sprintf(write_buffer, "SHOW+\nNEWLY CREATED ACCOUNT INFORMATION \nAccount Number:%ld\nAccount Balance:%ld\nAccount Type:%s\nAccount Status:%s\nCustomer IDs:%d\nCustomer Name:%s\nCustomer Age: %d\nCustomer Gender:%c\nCustomer login:%s\n", account.account_number, account.balance, type, active, account.customer_ID[0], customer.customer_name, customer.age, customer.gender, customer.password);
    send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
    // printf("\n%s\n",read_buffer);
    close(account_descriptor);

    close(customer_descriptor);
}

int check_accountID(int socket_descriptor, int account_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    struct Account account;

    int account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    off_t offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(account_descriptor, F_SETLKW, &lock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account file");
        return false;
    }

    rbytes = read(account_descriptor, &account, sizeof(account));
    if (rbytes == -1)
    {
        perror("error while reading from account file");
        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);
    close(account_descriptor);
    if (!account.active)
    {
        bzero(write_buffer, sizeof(write_buffer));
        strcpy(write_buffer, "SHOW+\nERROR:Account is not ACTIVE\n");
        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        return -1;
    }

    if (account.account_number == account_number)
    {
        return offset;
    }
    return -1;
}
off_t check_customerID(int socket_descriptor, int customer_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    struct Customer customer;

    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    if (customer_descriptor == -1)
    {
        perror("error while opening customer file");

        return -1;
    }

    off_t offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of customer file");

        return -1;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Customer);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(customer_descriptor, F_SETLK, &lock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account file");
        return false;
    }

    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from account file");
        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(customer_descriptor, F_SETLK, &lock);
    close(customer_descriptor);
    if (customer.customer_ID == customer_number)

        return offset;
    return -1;
}

bool delete_account(int socket_descriptor)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    bzero(write_buffer, sizeof(write_buffer));

    strcpy(write_buffer, "Enter the account Number to be Deleted\n");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    if (wbytes == -1)
    {
        perror("Error while asking the account number to be deleted");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading account number");

        return false;
    }

    int account_number = atoi(read_buffer);

    int offset = check_accountID(socket_descriptor, account_number);

    if (offset == -1)
    {
        printf("The file you want to delete dosent exist");
        send(socket_descriptor, "SHOW+\nERROR: The account you want to delete dosent exist\n", sizeof("SHOW+\nERROR: The account you want to delete dosent exist\n"), 0);
        recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
        return false;
    }

    int account_descriptor = open("./database/accounts.txt", O_RDWR);
    offset = lseek(account_descriptor, sizeof(struct Account), SEEK_SET);
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(account_descriptor, F_SETLKW, &lock);

    if (lck == -1)
    {
        perror("error while acquring a lock on account file");

        return false;
    }
    struct Account delete;

    rbytes = read(account_descriptor, &delete, sizeof(delete));

    if (rbytes == -1)
    {
        perror("error while reading from account file");

        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);

    close(account_descriptor);
    wbytes = write(account_descriptor, &delete, sizeof(delete));

    bzero(write_buffer, sizeof(write_buffer));
    delete.active = false;

    account_descriptor = open("./database/accounts.txt", O_RDWR);

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    fcntl(account_descriptor, F_SETLKW, &lock);
    wbytes = write(account_descriptor, &delete, sizeof(delete));
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);
    printf("ACCOUNT with ID %ld is Deleted\n",delete.account_number);
    close(account_descriptor);
}
bool Reactivate_account(int socket_descriptor)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    bzero(write_buffer, sizeof(write_buffer));

    strcpy(write_buffer, "Enter the account Number to be Reactivate\n");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    if (wbytes == -1)
    {
        perror("Error while asking the account number to be Reactivate");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading account number");

        return false;
    }

    int account_number = atoi(read_buffer);

    // int offset = check_accountID(socket_descriptor, account_number);

    // if (offset == -1)
    // {
    //     printf("The file you want to Reactivate dosent exist");
    //     send(socket_descriptor, "ERROR: The account you want to Reactivate dosent exist\n", sizeof("ERROR: The account you want to delete dosent exist\n"), 0);
    //     return false;
    // }

    int account_descriptor = open("./database/accounts.txt", O_RDWR);
    int offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(account_descriptor, F_SETLKW, &lock);

    if (lck == -1)
    {
        perror("error while acquring a lock on account file");

        return false;
    }
    struct Account delete;

    rbytes = read(account_descriptor, &delete, sizeof(delete));

    if (rbytes == -1)
    {
        perror("error while reading from account file");

        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);

    close(account_descriptor);
    wbytes = write(account_descriptor, &delete, sizeof(delete));
    if (delete.active == true)
    {
        bzero(write_buffer, sizeof(write_buffer));
        strcpy(write_buffer, "Account is already Active\n");
        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        return false;
    }
    bzero(write_buffer, sizeof(write_buffer));
    delete.active = true;

    account_descriptor = open("./database/accounts.txt", O_RDWR);

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    fcntl(account_descriptor, F_SETLKW, &lock);
    wbytes = write(account_descriptor, &delete, sizeof(delete));
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);
    printf("Account with ID %ld is Reactivated\n",delete.account_number);
    close(account_descriptor);
}

bool modify_customer(int socket_descriptor)
{

    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    bzero(write_buffer, sizeof(write_buffer));

    strcpy(write_buffer, "Enter the Customer you want to modify\n");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    if (wbytes == -1)
    {
        perror("Error while asking the Customer to be modified");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading CustomerID");

        return false;
    }

    int customer_number = atoi(read_buffer);

    int offset = check_customerID(socket_descriptor, customer_number);

    if (offset == -1)
    {
        printf("The file you want to modify dosent exist");

        send(socket_descriptor, "SHOW+\nERROR: The CustomerID you want to modify dosent exist\n", sizeof("SHOW+\nERROR: The Customer you want to modify dosent exist\n"), 0);
        recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        return false;
    }

    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    offset = lseek(customer_descriptor, sizeof(struct Customer), SEEK_SET);

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Customer);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(customer_descriptor, F_SETLKW, &lock);

    if (lck == -1)
    {
        perror("error while acquring a lock on customer file");

        return false;
    }
    struct Customer modify;

    rbytes = read(customer_descriptor, &modify, sizeof(modify));

    if (rbytes == -1)
    {
        perror("error while reading from customer file");

        return false;
    }

    lock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);

    //--------------------------------------modify menu-----------------------------------------
    bzero(write_buffer, sizeof(write_buffer));

    strcat(write_buffer, "------------------Current Customer Information----------\n");
    sprintf(write_buffer, "Customer ID:%d\nCustomer Account number %d\nCustomer AGE:%d\nCutomer gender:%c\nCustomer LoginID:%s\n", modify.customer_ID, modify.account_number, modify.age, modify.gender, modify.login_id);

    strcat(write_buffer, "MODIFY MENU\n1.Modify Name\n2.Modify Age\n3.Modify gender\n4.Modify Customer loginID\n");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    if (wbytes == -1)
    {
        perror("Error while asking the modify menu");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading Modify medu");

        return false;
    }
    int choice = atoi(read_buffer);
    switch (choice)
    {
    case 1:
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "Enter the new Name of the customer\n");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking the modify customer name");

            return false;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {
            perror("Error while reading Modify customer name");

            return false;
        }
        strcpy(modify.customer_name, read_buffer);
        break;
    case 2:
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "Enter the new age of the customer\n");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking the modify customer age");

            return false;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {
            perror("Error while reading Modify customer age");

            return false;
        }
        modify.age = atoi(read_buffer);
        break;
    case 3:
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "modify new gender value\n");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking the modify customer gender");

            return false;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {
            perror("Error while reading Modify customer gender");

            return false;
        }

        modify.gender = read_buffer[0];

        break;
    case 4:
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "Enter the new loginID of the customer\n");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking the modify customer login_ID");

            return false;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {
            perror("Error while reading Modify customer login_ID");

            return false;
        }
        strcpy(modify.login_id, read_buffer);
        break;
    }

    bzero(write_buffer, sizeof(write_buffer));

    customer_descriptor = open("./database/customers.txt", O_RDWR);
    offset = lseek(customer_descriptor, sizeof(struct Customer), SEEK_SET);
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Customer);
    lock.l_start = offset;
    lock.l_pid = getpid();
    fcntl(customer_descriptor, F_SETLKW, &lock);
    wbytes = write(customer_descriptor, &modify, sizeof(modify));

    lock.l_type = F_UNLCK;
    fcntl(customer_descriptor, F_SETLK, &lock);
    printf("Customer with customerID %d details are modified\n",modify.customer_ID);
    close(customer_descriptor);
}

bool display_account_info(int socket_descriptor)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    bzero(write_buffer, sizeof(write_buffer));

    strcpy(write_buffer, "Enter the account Number to be searched\n");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    if (wbytes == -1)
    {
        perror("Error while asking the account number to be searched");

        return false;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

    if (rbytes == -1)
    {
        perror("Error while reading account number");

        return false;
    }

    int account_number = atoi(read_buffer);

    int offset = check_accountID(socket_descriptor, account_number);

    if (offset == -1)
    {
        printf("The AccountID:%d dosent exist\n", account_number);
        send(socket_descriptor, "SHOW+\nERROR: The Account ID you want to search dosent Exist\n", sizeof("SHOW+\nERROR: The Account ID you want to search dosent Exist\n"), 0);
        recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        return false;
    }

    int account_descriptor = open("./database/accounts.txt", O_RDWR);
    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);
    if (offset == -1)
    {
        printf("The file you want to delete dosent exist");
        send(socket_descriptor, "SHOW+\nERROR: The account you want to display dosent exist\n", sizeof("SHOW+\nERROR: The account you want to display dosent exist\n"), 0);
        recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        return false;
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    int lck = fcntl(account_descriptor, F_SETLKW, &lock);

    if (lck == -1)
    {
        perror("error while acquring a lock on account file");

        return false;
    }
    struct Account display;

    rbytes = read(account_descriptor, &display, sizeof(display));

    if (rbytes == -1)
    {
        perror("error while reading from account file");

        return false;
    }
    lock.l_type = F_UNLCK;

    fcntl(account_descriptor, F_SETLK, &lock);

    char *type;

    if (display.regular_account)
        type = "Regular";
    else
        type = "Joint";

    char *active;

    if (display.active)
        active = "Active";
    else
        active = "Not Active";

    int customer_number = display.customer_ID[0];
    // int customer_offset = check_customerID(socket_descriptor, customer_number);

    int customer_descriptor = open("./database/customers.txt", O_RDWR);
    int customer_offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

    struct flock clock;
    clock.l_type = F_RDLCK;
    clock.l_whence = SEEK_SET;
    clock.l_len = sizeof(struct Customer);
    clock.l_start = customer_offset;
    clock.l_pid = getpid();

    int clck = fcntl(customer_descriptor, F_SETLKW, &lock);

    if (clck == -1)
    {
        perror("error while acquring a lock on account file");

        return false;
    }
    struct Customer customer;

    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from account file");

        return false;
    }
    clock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &clock);

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "SHOW+\n ACCOUNT INFORMATION \nAccount Number:%ld\nAccount Balance:%ld\nAccount Type:%s\nAccount Status:%s\nCustomer ID%d\ncustomer custID:%d\nCustomer Name:%s\nCustomer age:%d\ncustomer loginID:%s\n", display.account_number, display.balance, type, active, display.customer_ID[0], customer.customer_ID, customer.customer_name, customer.age, customer.login_id);

    send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // dummy read
    close(customer_descriptor);
    close(account_descriptor);
}

bool admin_handler(int new_socket_descriptor)
{
    ssize_t rbytes, wbytes;
    char read_buffer[max_str], write_buffer[max_str];
    bool admin_login=admin_login_check(new_socket_descriptor);
    while (1)
    {
        if (!admin_login)
        {
            strcpy(write_buffer, "SHOW+\nERROR:Login Unsucessfull");
            send(new_socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            recv(new_socket_descriptor, read_buffer, sizeof(read_buffer), 0);
            printf("ADMIN_LOGIN_UNSUCESS\n");
            return false;
        }

        bzero(write_buffer, sizeof(write_buffer));

        char *admin_menu = "\nADMIN LOGIN SUCESSFULL\n------------ADMIN MENU----------\n1.Add Account\n2.Delete Account\n3.Reactivate Account\n4.Modify Account\n5.search Account Details\n6.ADMIN LOGOUT\n";

        strcpy(write_buffer, admin_menu);

        wbytes = send(new_socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking admin menu choice");

            return false;
        }

        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(new_socket_descriptor, read_buffer, sizeof(read_buffer), 0);
        if (rbytes == -1)
        {
            perror("error while reading Admin menu choice");

            return false;
        }

        int menu_choice = atoi(read_buffer);

        switch (menu_choice)
        {
        case 1:
            if (!add_account(new_socket_descriptor))
            {
                strcpy(write_buffer, "SHOW+\nERROR:Account Not Created\n");

                send(new_socket_descriptor, write_buffer, sizeof(write_buffer), 0);
                recv(new_socket_descriptor, read_buffer, sizeof(read_buffer), 0);
                continue;
            }
            else
            {
                send_new_account_info(new_socket_descriptor);
            }
            break;
        case 2:
            delete_account(new_socket_descriptor);

            break;
        case 3:
            Reactivate_account(new_socket_descriptor);
            break;
        case 4:
            modify_customer(new_socket_descriptor);
            break;

        case 5:
            display_account_info(new_socket_descriptor);
            break;
        case 6:
            return false;
            break;
        }
    }
    return true;
}