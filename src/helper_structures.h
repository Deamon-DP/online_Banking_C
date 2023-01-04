#include<time.h>

struct Account
{
long int account_number;
int customer_ID[2];
int regular_account;
long int balance;
int active;
//int transactions[10];
};

struct Transaction
{
    int transaction_id;
    long int from_account;
    char  operation[25];
    long int to_account;
    long int from_account_balance;
    long int to_account_balance;
    char  tracsaction_time[100];
    int transaction_amount;
};

struct Customer
{
    int customer_ID;
    char customer_name[50];
    int account_number;
    int age;
    char gender;
    char login_id[25];
    char password[25];

};