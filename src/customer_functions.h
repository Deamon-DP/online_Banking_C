#ifndef CUSTOMER_HELPER_FUNCTIONS
#define CUSTOMER_HELPER_FUNCTIONS
#define max_str 4096

#include<time.h>
//#include "./admin_functions.h"
//#include "./helper_structures.h"



bool create_transaction(int from_account, int to_account, int from_account_balance, int to_account_balance, char operation[25], time_t time_stamp, int amount);

int view_details(int socket_descriptor, int customer_number);

int loginID_search(int socket_descriptor, char login_id[25]);

int balance_enquery(int socket_descriptor, int customer_number);

int deposit(int socket_descriptor, int customer_number);

int withdraw(int socket_descriptor, int customer_number);

int change_password(int socket_descriptor, int customer_number);

int customer_login_handler(int socket_descriptor);

bool customer_helper(int socket_descriptor);


#endif