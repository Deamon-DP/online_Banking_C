#ifndef ADMIN_HELPER_FUNCTIONS
#define ADMIN_HELPER_FUNCTIONS
//#include "./helper_structures.h"
#define max_str 4096
#define ADMIN_ID "dp1811"
#define ADMIN_PASSWORD "1234"


bool admin_login_check(int socket_descriptor);

int add_customer(int socket_descriptor, int account_number);

bool add_account(int socket_descriptor);

bool send_new_account_info(int socket_descriptor);

int check_accountID(int socket_descriptor, int account_number);

off_t check_customerID(int socket_descriptor, int customer_number);

bool delete_account(int socket_descriptor);

bool Reactivate_account(int socket_descriptor);

bool modify_customer(int socket_descriptor);

bool display_account_info(int socket_descriptor);

bool admin_handler(int new_socket_descriptor);


#endif