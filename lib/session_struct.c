#include "session_struct.h"

/**
 * insert a new tcp_session item into the linked list
 * 
 * @param pointer to the array of pointers of session_linked_list (add a head to it if not yet exist)
 * @param session the tcp session pointer
 * 
*/
void insert_tcp_session(struct session_linked_list** head, struct tcp_session* session);

/**
 * search a tcp session from the linked list by client ip & port
 * 
 * @param head the pointer to the head of the tcp_session
 * @param client_ip the client ip addr
 * @param client_port the client port
 * 
 * @return pointer to the tcp_session if found
 *         NULL if not found
*/
struct tcp_session* search_tcp_session_by_client(struct session_linked_list* head, uint32_t client_ip, uint16_t client_port);

/**
 * search a tcp session from the linked list by server port
 * 
 * @param head the pointer to the head of the tcp_session
 * @param server_port the server port
 * 
 * @return pointer to the tcp_session if found
 *         NULL if not found
*/
struct tcp_session* search_tcp_session_by_server(struct session_linked_list* head, uint16_t server_port);

/**
 * delete expired session (free the memory)
 * 
 * @param pointer to the array of pointers of session_linked_list
 * 
*/
void delete_expired_session(struct session_linked_list** head);

/**
 * delete all session (free the memory of linked-list and also sessions)
 * 
 * @param pointer to the head of the session_linked_list
 * @param remove_session indicate whether session also need to be "freed" or not (in case of two linked-list using same session)
*/
void delete_all_session(struct session_linked_list* head, int remove_session);

void print_session_linked_list(struct session_linked_list* head) {
    while (head != NULL) {
        printf("\t\t client ip: %d\n", head->session->client_ip);
        printf("\t\t client port: %d\n", head->session->client_port);
        printf("\t\t server port: %d\n", head->session->server_port);
        printf("\t\t state: %d\n", head->session->state);
        if ((difftime(time(NULL), head->session->expire)) > 0.0) {
            printf("\t\t expired\n");
        } else {
            printf("\t\t not expired\n");
        }
        printf("\n");
        head = head->next;
    }
    return;
}

void print_session_hash_table(struct session_hash_table* table) {
    printf("session info:\n");
    int i;
    for(i=0; i<table->len; ++i) {
        print_session_linked_list(table->list_client_key[i]);
    }
    printf("\n\n");
}

void insert_tcp_session(struct session_linked_list** _head, struct tcp_session* session) {
    struct session_linked_list* head = *_head;
    // if head is null
    if (head == NULL) {
        *_head = calloc(1, sizeof(struct session_linked_list));
        (*_head)->session = session;
        return;
    }
    // append to the end
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = calloc(1, sizeof(struct session_linked_list));
    head->next->session = session;
    return;
}

struct tcp_session* search_tcp_session_by_client(struct session_linked_list* head, uint32_t client_ip, uint16_t client_port) {
    // iterate the list
    while (head != NULL) {
        if (head->session->client_ip == client_ip && head->session->client_port == client_port) {
            return head->session;
        }
        head = head->next;
    }
    return NULL;
}

struct tcp_session* search_tcp_session_by_server(struct session_linked_list* head, uint16_t server_port) {
    // iterate the list
    while (head != NULL) {
        if (head->session->server_port == server_port) {
            return head->session;
        }
        head = head->next;
    }
    return NULL;
}

void delete_expired_session(struct session_linked_list** _head) {
    struct session_linked_list* current = *_head;
    if (current == NULL) {
        return;
    }
    struct session_linked_list* to_be_deleted;
    while (current != NULL && current->next != NULL) {
        // if the next is expired
        if (difftime(time(NULL), current->next->session->expire) > 0.0) {
            to_be_deleted = current->next;
            current->next = to_be_deleted->next;
            free(to_be_deleted);
        } else {
            current = current->next;
        }
    }
    // check the head
    if (difftime(time(NULL), (*_head)->session->expire) > 0.0) {
        to_be_deleted = *_head;
        *_head = to_be_deleted->next;
        free(to_be_deleted);
    }
    return;
}

void delete_all_session(struct session_linked_list* head, int remove_session) {
    struct session_linked_list* to_be_deleted;
    while (head != NULL) {
        to_be_deleted = head;
        head = head->next;
        if (remove_session)
            free(to_be_deleted->session);
        free(to_be_deleted);
    }
    return;
}

void init_hash_table(struct session_hash_table* hash_table, int num_entry, uint16_t min_port, uint16_t max_port) {
    hash_table->len = num_entry;
    hash_table->min_port = min_port;
    hash_table->mod_num = max_port - min_port + 1;
    hash_table->next_num = 0;
    hash_table->list_client_key = (struct session_linked_list**)(calloc(num_entry, sizeof(struct session_linked_list*)));
    hash_table->list_server_key = (struct session_linked_list**)(calloc(num_entry, sizeof(struct session_linked_list*)));
    return;
}

void delete_hash_table(struct session_hash_table* hash_table) {
    int i;
    for(i=0; i<hash_table->len; ++i) {
        delete_all_session(hash_table->list_client_key[i], 0);
        delete_all_session(hash_table->list_server_key[i], 1);
    }
    free(hash_table->list_client_key);
    free(hash_table->list_server_key);
    return;
}

void clean_expired_session(struct session_hash_table* hash_table) {
    int i;
    for(i=0; i<hash_table->len; ++i) {
        delete_expired_session(&(hash_table->list_client_key[i]));
        delete_expired_session(&(hash_table->list_server_key[i]));
    }
    return;
}

struct tcp_session* add_tcp_session(struct session_hash_table* hash_table, uint32_t client_ip, uint16_t client_port, int default_timeout_in_minute) {
    // get the hash value
    int hash_client = ((client_ip + client_port)%(hash_table->len));
    struct tcp_session* session = search_tcp_session_by_client(hash_table->list_client_key[hash_client], client_ip, client_port);
    // already exist
    if (session != NULL) {
        return session;
    }
    uint16_t server_port;
    int count;
    do {
        server_port = (hash_table->next_num % (hash_table->mod_num)) + hash_table->min_port;
        hash_table->next_num++;
        session = search_tcp_session_by_server(hash_table->list_server_key[server_port%hash_table->len], server_port);
        if (session == NULL) {
            break;
        }
        ++count;
        // no available port
        if (count > hash_table->mod_num) {
            return NULL;
        }
    } while(1);
    int hash_server = server_port%hash_table->len;
    // create a new session
    session = (struct tcp_session*)calloc(1, sizeof(struct tcp_session));
    session->client_ip = client_ip;
    session->client_port = client_port;
    session->server_port = server_port;
    session->state = 0;
    session->expire = time(NULL) + (default_timeout_in_minute * MINUTE);
    // insert session to two list
    insert_tcp_session(&(hash_table->list_client_key[hash_client]), session);
    insert_tcp_session(&(hash_table->list_server_key[hash_server]), session);
    return session;
}

struct tcp_session* get_tcp_session_by_client(struct session_hash_table* hash_table, uint32_t client_ip, uint32_t client_port) {
    return search_tcp_session_by_client(hash_table->list_client_key[(client_ip + client_port)%hash_table->len], client_ip, client_port);
}

struct tcp_session* get_tcp_session_by_server(struct session_hash_table* hash_table, uint32_t server_port) {
    return search_tcp_session_by_server(hash_table->list_server_key[server_port%hash_table->len], server_port);
}

