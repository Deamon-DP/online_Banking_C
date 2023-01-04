#define max_str 4096

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <sys/socket.h>
#include<string.h>
#include<stdlib.h>

#include "./admin_functions.h"
#include"./helper_structures.h"

bool create_transaction(int from_account, int to_account, int from_account_balance, int to_account_balance, char operation[25], time_t time_stamp, int amount)
{
    int rbytes, wbytes;
    struct Transaction new_transac, last_added;
    int transaction_descriptor = open("./database/transactions.txt", O_CREAT, S_IRWXU);

    rbytes = read(transaction_descriptor, &last_added, sizeof(last_added));

    if (rbytes == 0)
    {

        new_transac.transaction_id = 0;
    }
    else
    {

        if (transaction_descriptor == -1)
        {
            perror("error while opening accounts file");
            return false;
        }
        off_t offset = lseek(transaction_descriptor, -sizeof(struct Transaction), SEEK_END);

        if (offset == -1)
        {
            perror("Error while getting offset of account file");
            return false;
        }

        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(struct Transaction);
        lock.l_start = offset;
        lock.l_pid = getpid();
        int lck = fcntl(transaction_descriptor, F_SETLKW, &lock);
        if (lck == -1)
        {
            return false;
        }
        rbytes = read(transaction_descriptor, &last_added, sizeof(last_added));
        if (rbytes == -1)
        {
            return false;
        }

        new_transac.transaction_id = last_added.transaction_id + 1;
        lock.l_type = F_UNLCK;
        fcntl(transaction_descriptor, F_SETLK, &lock);
        close(transaction_descriptor);
    }
    new_transac.from_account = from_account;
    new_transac.to_account = to_account;
    new_transac.from_account_balance = from_account_balance;
    new_transac.to_account_balance = to_account_balance;
    strcpy(new_transac.operation, operation);
    strcpy(new_transac.tracsaction_time, ctime(&time_stamp));
    new_transac.transaction_amount = amount;

    if (strcmp(operation, "Deposit") != 0)
    {
        new_transac.transaction_amount = -1 * amount;
    }

    transaction_descriptor = open("./database/transactions.txt", O_APPEND | O_RDWR);
    wbytes = write(transaction_descriptor, &new_transac, sizeof(new_transac));

    close(transaction_descriptor);
    return true;
}

int view_details(int socket_descriptor, int customer_number)
{

    char read_buffer[max_str], write_buffer[max_str];

    bzero(read_buffer, sizeof(read_buffer));
    bzero(write_buffer, sizeof(write_buffer));

    struct Transaction current_transac;

    struct flock lck;
    int rbytes;

    int transaction_descriptor = open("./database/transactions.txt", O_CREAT, S_IRWXU);

    off_t current_offset = lseek(transaction_descriptor, 0, SEEK_SET);
    int transaction_number = 0;
    // if (transaction_descriptor == -1 && errno == ENOENT)
    // {
    //     // no users exits yet
    //     bzero(write_buffer, sizeof(write_buffer));

    //     strcpy(write_buffer, "ERROR:Transactions file is not created\n");

    //     send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    //     recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
    //     return -1;
    // }
    // else
    if (transaction_descriptor == -1)
    {
        perror("Error while opening transactions file");
        return -1;
    }
    else
    {

        do
        {
            lck.l_type = F_RDLCK;
            lck.l_whence = SEEK_END;
            lck.l_start = current_offset;
            lck.l_len = sizeof(struct Transaction);
            lck.l_pid = getpid();

            fcntl(transaction_descriptor, F_SETLKW, &lck);

            rbytes = read(transaction_descriptor, &current_transac, sizeof(struct Transaction));
            lck.l_type = F_UNLCK;
            fcntl(transaction_descriptor, F_SETLK, &lck);
            transaction_number++;
            if (current_transac.from_account == customer_number)
            {
                bzero(write_buffer, sizeof(write_buffer));
                sprintf(write_buffer, "+++++\n--------TRANSACTION NUMBER : %d------\n\nTransaction From Account:%ld\tTransaction From Account Balance:%ld\tTransaction Receiving Account:%ld\tTransaction Receiving Account Balance:%ld\tTransaction Operation:%s\tTransaction Time:%s\tTransaction Amount:%d\t......press any key to view next transaction\n", transaction_number, current_transac.from_account, current_transac.from_account_balance, current_transac.to_account, current_transac.to_account_balance, current_transac.operation, current_transac.tracsaction_time, current_transac.transaction_amount);
                send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
                bzero(read_buffer, sizeof(read_buffer));
                recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
            }
            else if (current_transac.to_account == customer_number)
            {
                bzero(write_buffer, sizeof(write_buffer));
                sprintf(write_buffer, "+++++\n--------TRANSACTION NUMBER : %d------\n\nTransaction From Account:%ld\tTransaction From Account Balance:%ld\tTransaction Receiving Account:%ld\tTransaction Receiving Account Balance:%ld\tTransaction Operation:%s\tTransaction Time:%s\tTransaction Amount:%d\t......press any key to view next transaction\n", transaction_number, current_transac.from_account, current_transac.from_account_balance, current_transac.to_account, current_transac.to_account_balance, current_transac.operation, current_transac.tracsaction_time, current_transac.transaction_amount);

                send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
                bzero(read_buffer, sizeof(read_buffer));

                recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
            }

        } while (rbytes > 0);
    }
    return 0;
}
int loginID_search(int socket_descriptor, char login_id[25])
{
    char read_buffer[max_str], write_buffer[max_str];

    bzero(read_buffer, sizeof(read_buffer));
    bzero(write_buffer, sizeof(write_buffer));

    struct Customer current_customer;

    struct flock lck;
    int rbytes;

    int customer_descriptor = open("./database/customers.txt", O_CREAT | O_RDWR, S_IRWXU);

    off_t current_offset = lseek(customer_descriptor, 0, SEEK_SET);

    if (customer_descriptor == -1 && errno == ENOENT)
    {
        // no users exits yet
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "SHOW+\nERROR:Customers file is not created\n");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        return -1;
    }
    else if (customer_descriptor == -1)
    {
        perror("Error while opening Customer file");
        return -1;
    }

    else
    {
        int id_exists = 0;
        do
        {

            lck.l_type = F_RDLCK;
            lck.l_whence = SEEK_SET;
            lck.l_start = current_offset;
            lck.l_len = sizeof(struct Customer);
            lck.l_pid = getpid();

            fcntl(customer_descriptor, F_SETLKW, &lck);

            rbytes = read(customer_descriptor, &current_customer, sizeof(struct Customer));

            lck.l_type = F_UNLCK;

            fcntl(customer_descriptor, F_SETLK, &lck);

            // printf("logggggg:%s\n",current_customer.login_id);
            if (strcmp(login_id, current_customer.login_id) == 0)
            {
                id_exists = 1;
            }
        } while ((id_exists == 0) && rbytes > 0);

        if (!id_exists)
        {
            // bzero(write_buffer, sizeof(write_buffer));

            // strcpy(write_buffer, "SHOW+\nERROR:Invalid Login ID");
            // send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            return -1;
        }
        close(customer_descriptor);
        return current_customer.customer_ID;
    }
    return -1;
}

int balance_enquery(int socket_descriptor, int customer_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];
    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    int offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

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

        return -1;
    }

    struct Customer customer;
    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from customer file");

        return -1;
    }

    lock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);

    int account_number = customer.account_number;
    struct Account account;

    int account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    lck = fcntl(account_descriptor, F_SETLKW, &lock);
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

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "SHOW+\n CURRENT ACCOUNT BALANCE=%ld\n", account.balance);

    send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // dummy read
}

int deposit(int socket_descriptor, int customer_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];
    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    int offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

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

        return -1;
    }

    struct Customer customer;
    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from customer file");

        return -1;
    }

    lock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);

    int account_number = customer.account_number;
    struct Account account;

    int account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_RDLCK;

    lock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    lck = fcntl(account_descriptor, F_SETLKW, &lock);
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

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "----------------Deposit--------------\n\nBEFORE DEPOSIT ACCOUNT BALANCE: %ld\n\nEnter the Deposit Amount:", account.balance);

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    if (wbytes == -1)
    {
        perror("Error while asking deposit amount");

        return -1;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

    if (rbytes == -1)
    {
        perror("ERROR while reading deposit amount");

        return -1;
    }

    int amount = atoi(read_buffer);

    account.balance += amount;
    time_t time_stamp = time(NULL);

    account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();
    lck = fcntl(account_descriptor, F_SETLKW, &lock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account file");
        return false;
    }

    wbytes = write(account_descriptor, &account, sizeof(account));

    if (wbytes == -1)
    {
        perror("error while writing to account file");
        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);

    close(account_descriptor);
    char operation[25];
    strcpy(operation, "Deposit");

    create_transaction(customer.customer_ID, -1, account.balance, -1, operation, time_stamp, amount);

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "SHOW+\n AFTER DEPOSIT ACCOUNT BALANCE=%ld\n", account.balance);

    send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // dummy read
    printf("%d Rupees DEPOSITED TO ACCOUNT:%ld \n", amount, account.account_number);

    return true;
}

int withdraw(int socket_descriptor, int customer_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];
    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    int offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

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

        return -1;
    }

    struct Customer customer;
    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from customer file");

        return -1;
    }

    lock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);

    int account_number = customer.account_number;
    struct Account account;

    int account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    lck = fcntl(account_descriptor, F_SETLKW, &lock);
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

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "----------------Withdraw--------------\n\nBEFORE WITHDRAW ACCOUNT BALANCE: %ld\n\nEnter the Withdraw Amount:", account.balance);

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    if (wbytes == -1)
    {
        perror("Error while asking withdraw amount");

        return -1;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

    if (rbytes == -1)
    {
        perror("ERROR while reading withdraw amount");

        return -1;
    }

    int amount = atoi(read_buffer);

    account.balance -= amount;
    time_t time_stamp = time(NULL);
    account_descriptor = open("./database/accounts.txt", O_RDWR);

    if (account_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(account_descriptor, account_number * sizeof(struct Account), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Account);
    lock.l_start = offset;
    lock.l_pid = getpid();

    lck = fcntl(account_descriptor, F_SETLKW, &lock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account file");
        return false;
    }

    wbytes = write(account_descriptor, &account, sizeof(account));
    if (wbytes == -1)
    {
        perror("error while writing to account file");
        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(account_descriptor, F_SETLK, &lock);
    close(account_descriptor);

    char operation[25];
    strcpy(operation, "WithDraw");

    create_transaction(customer.customer_ID, -1, account.balance, -1, operation, time_stamp, amount);

    bzero(write_buffer, sizeof(write_buffer));

    sprintf(write_buffer, "SHOW+\n AFTER WITHDRAW ACCOUNT BALANCE=%ld\n", account.balance);

    send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

    recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // dummy read
    printf("%d Rupees WITHRAWN FROM ACCOUNT:%ld \n", amount, account.account_number);
}

int change_password(int socket_descriptor, int customer_number)
{
    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];
    int customer_descriptor = open("./database/customers.txt", O_RDWR);

    int offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

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

        return -1;
    }

    struct Customer customer;
    rbytes = read(customer_descriptor, &customer, sizeof(customer));

    if (rbytes == -1)
    {
        perror("error while reading from customer file");

        return -1;
    }

    lock.l_type = F_UNLCK;

    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);

    bzero(write_buffer, sizeof(write_buffer));

    strcpy(write_buffer, "----------------CHANGE PASSWORD--------------\n\nENTER THE OLD PASSWORD FOR VERIFICATION OF USER AUTHENTICATION:");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    if (wbytes == -1)
    {
        perror("Error while asking password");

        return -1;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

    if (rbytes == -1)
    {
        perror("ERROR while reading password");

        return -1;
    }
    if (strcmp(read_buffer, customer.password) == 0)
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "----------------CHANGE PASSWORD--------------\n\nENTER THE NEW PASSWORD:");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking password");

            return -1;
        }

        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {
            perror("ERROR while reading password");

            return -1;
        }
        char new_password[25];
        strcpy(new_password, read_buffer);

        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "----------------CHANGE PASSWORD--------------\n\nREENTER THE NEW PASSWORD:");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking password");

            return -1;
        }
        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

        if (rbytes == -1)
        {
            perror("ERROR while reading password");

            return -1;
        }
        if (strcmp(new_password, read_buffer) == 0)
        {
            strcpy(customer.password, new_password);
        }
        else
        {
            bzero(write_buffer, sizeof(write_buffer));

            strcpy(write_buffer, "SHOW+\nERROR: THE NEW PASSWORD AND RE-ENTERED PASSWORD ARE NOT MATCHING");
            printf("ERROR: THE NEW PASSWORD AND RE-ENTERED PASSWORD ARE NOT MATCHING\n");
            wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            if (wbytes == -1)
            {
                perror("Error while asking password");

                return -1;
            }
            bzero(read_buffer, sizeof(read_buffer));

            rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // dummy read

            if (rbytes == -1)
            {
                perror("ERROR while reading password");

                return -1;
            }
            return -1;
        }
    }
    else
    {
        bzero(write_buffer, sizeof(write_buffer));

        strcpy(write_buffer, "SHOW+\nERROR:OLD PASSWORD IS NOT ENTERED CORRECTLY\n......please try again");
        printf("ERROR:OLD PASSWORD ERROR\n");
        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking password");

            return -1;
        }
        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

        if (rbytes == -1)
        {
            perror("ERROR while reading password");

            return -1;
        }
        return -1;
    }
    customer_descriptor = open("./database/customers.txt", O_RDWR);

    if (customer_descriptor == -1)
    {
        perror("error while opening accounts file");

        return -1;
    }

    offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);

    if (offset == -1)
    {
        perror("Error while getting offset of account file");

        return -1;
    }

    // struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct Customer);
    lock.l_start = offset;
    lock.l_pid = getpid();
    lck = fcntl(customer_descriptor, F_SETLKW, &lock);
    if (lck == -1)
    {
        perror("error while acquring a lock on account file");
        return false;
    }

    wbytes = write(customer_descriptor, &customer, sizeof(customer));

    if (wbytes == -1)
    {
        perror("error while writing to account file");
        return false;
    }
    lock.l_type = F_UNLCK;
    fcntl(customer_descriptor, F_SETLK, &lock);

    close(customer_descriptor);
}

int customer_login_handler(int socket_descriptor)
{
    struct Customer customer;

    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    strcpy(write_buffer, "ENTER THE  Customer LOGIN CREDENTIALS\n\nLOGIN ID");

    wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
    if (wbytes == -1)
    {
        perror("Error while asking login credentials to customer");

        return -1;
    }

    rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client
    char loginID[25];
    strcpy(loginID, read_buffer);
    if (rbytes == -1)
    {
        perror("ERROR while reading login ID of Customer");

        return -1;
    }
    else
    {
        printf("\nCustomer Login ID received :%s\n", loginID);
    }

    int customer_number = loginID_search(socket_descriptor, read_buffer);
    if (customer_number < 0)
    {
        bzero(write_buffer, sizeof(write_buffer));

        sprintf(write_buffer, "SHOW+\nERROR:There is no customer with Customer loginID %s\n", loginID);
        printf("ERROR:Customer INVALID CUSTOMER LOGIN ID\n");

        send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        recv(socket_descriptor,read_buffer,sizeof(read_buffer),0);
        return -1;
    }
    else
    {
        printf("\nlogin ID %s is of customer with ID :%d\n", loginID, customer_number);

        // int offset = check_customerID(socket_descriptor, customer_number);

        // if (offset == -1)
        // {
        //     printf("The file you want to modify dosent exist");

        //     send(socket_descriptor, "ERROR: The CustomerID you want to modify dosent exist\n", sizeof("ERROR: The Customer you want to modify dosent exist\n"), 0);

        //     return -1;
        // }
        int customer_descriptor = open("./database/customers.txt", O_RDWR);
        int offset = lseek(customer_descriptor, customer_number * sizeof(struct Customer), SEEK_SET);
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

            return -1;
        }

        rbytes = read(customer_descriptor, &customer, sizeof(customer));

        if (rbytes == -1)
        {
            perror("error while reading from customer file");

            return -1;
        }

        lock.l_type = F_UNLCK;

        fcntl(customer_descriptor, F_SETLK, &lock);

        close(customer_descriptor);
        strcpy(write_buffer, "ENTER THE  Customer LOGIN CREDENTIALS\n\nPassword:");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking login credentials to customer");

            return -1;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

        if (rbytes == -1)
        {
            perror("ERROR while reading login ID of Customer");

            return -1;
        }
        else
        {
            printf("\nustomer Login password received :%s\n", read_buffer);
        }
        if (strcmp(read_buffer, customer.password) == 0)
        {
            return customer.customer_ID;
        }
        else
        {
            bzero(write_buffer, sizeof(write_buffer));

            strcpy(write_buffer, "SHOW+\nERROR:Invalid Customer Password");
            printf("ERROR:INVALID CUSTOMER PASSWORD\n");
            send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        }
    }

    return -1;
}

bool customer_helper(int socket_descriptor)
{

    ssize_t rbytes, wbytes;
    char read_buffer[max_str], write_buffer[max_str];
    int customer_number = customer_login_handler(socket_descriptor);
    while (1)
    {
        if (customer_number < 0)
        {
            strcpy(write_buffer, "SHOW+\nERROR:Customer Login Unsucessfull");
            printf("CUSTOMER LOGIN UNSUCESSFULL\n");
            send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            recv(socket_descriptor,read_buffer,sizeof(read_buffer),0);
            return false;
        }

        bzero(write_buffer, sizeof(write_buffer));

        char *customer_menu = "\nCUSTOMER LOGIN SUCESSFULL\n------------CUSTOMER MENU----------\n1.Balance Enquery\n2.Deposit\n3.Withdraw\n4.View Details\n5.Password Change\n6.CUSTOMER LOG OUT\n";

        strcpy(write_buffer, customer_menu);

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

        if (wbytes == -1)
        {
            perror("Error while asking customer menu choice");

            return false;
        }

        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);
        if (rbytes == -1)
        {
            perror("error while reading Customer menu choice");

            return false;
        }

        int menu_choice = atoi(read_buffer);

        switch (menu_choice)
        {
        case 1:
            balance_enquery(socket_descriptor, customer_number);
            break;
        case 2:
            deposit(socket_descriptor, customer_number);
            break;
        case 3:
            withdraw(socket_descriptor, customer_number);
            break;
        // case 4:
        //     // transfer(socket_descriptor, customer_number);
        //     break;
        case 4:
            view_details(socket_descriptor, customer_number);
            break;
        case 5:
            change_password(socket_descriptor, customer_number);
            break;
        case 6:
            return false;
            break;
        }
    }
    return true;
}
