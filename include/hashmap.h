#ifndef LAVA_HASHMAP_H
#define LAVA_HASHMAP_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct hash_table_entry {
	//TODO This should be a string view and the hash algo should just query it
    char* key;
    void* value;
    struct hash_table_entry* next;
} hash_table_entry;

typedef struct hash_table {
    size_t size;
    struct hash_table_entry **table;
} hash_table;

struct hash_table* init_hash_table(size_t number_of_rows) {
    if (!number_of_rows) {
        return NULL;
    }

    struct hash_table *new_table=(struct hash_table*)malloc(sizeof(struct hash_table)); //bartender, one hashtable please
    if (NULL==new_table) {
        return NULL;
    }

    new_table->table=(struct hash_table_entry **) malloc(number_of_rows * sizeof(struct hash_table_entry *));
    if (NULL == (new_table->table)) {
        free(new_table);
        return NULL;
    }

    memset(new_table->table, 0, number_of_rows * sizeof(struct hash_table_entry *) );
    new_table->size = number_of_rows;
    return new_table;
}

//The simplest hash function. Converts 'key' string into a value in range 0- (number_of_rows-1)
size_t hashFunction(const char *key, size_t number_of_rows) {
    unsigned sum=0;
    while('\0' != *key) {
        sum+=key[0]; //it's okay if sum overflows
        key++;
    }
    return sum%number_of_rows;
}

//returns 0 on success
int insert(struct hash_table *the_hash_table, char* key, void* value) {
    if (value == NULL) {
        printf("Pushed NULL value to map for '%s'!", key);
        exit(1);
    }
    size_t index = hashFunction(key, the_hash_table->size);
    struct hash_table_entry *new_hash_table_entry = malloc(sizeof(struct hash_table_entry));
    if(NULL == new_hash_table_entry)
        return -1;

    //make a whole copy of key and values
    new_hash_table_entry->key=malloc(strlen(key)+1/*+1 for delimiter*/);
    if(NULL == new_hash_table_entry->key)
        return -1;
    strcpy(new_hash_table_entry->key, key);
    //new_hash_table_entry->value=malloc(strlen(value)+1);
    //if(NULL == new_hash_table_entry->value)
        //return -1;
    //strcpy(new_hash_table_entry->value, value);
    new_hash_table_entry->value = value; //No need to copy, since we know this pointer will persist

    //insert at start of linked list
    new_hash_table_entry->next=the_hash_table->table[index];
    the_hash_table->table[index]=new_hash_table_entry;

    return 0;
}

//returns 0 on success
int find(struct hash_table *the_hash_table, char *key, struct hash_table_entry* ret_val) {
    //head to the current row
    struct hash_table_entry *head_ll=the_hash_table->table[hashFunction(key, the_hash_table->size)];

    while(NULL != head_ll) {
        if(0==strcmp(head_ll->key,key)) {
            //found match! Make a copy and transfer over value
            if (ret_val != NULL) {
                memcpy(ret_val, head_ll, sizeof(struct hash_table_entry));
            }
            return 0;
        }
        head_ll=head_ll->next;
    }
    return -1;
}

//returns 0 on success
int delete(struct hash_table *the_hash_table, char *key) {
    //head to the current row
    struct hash_table_entry *head_ll=the_hash_table->table[hashFunction(key, the_hash_table->size)];

    if (NULL == head_ll)
        return -1;
    if (0==strcmp(head_ll->key, key)) {
        //head is the element to be deleted
        the_hash_table->table[hashFunction(key, the_hash_table->size)]=head_ll->next;
        free(head_ll->key);
        free(head_ll->value);
        free(head_ll);
        return 0;
    }
    while (NULL!=(head_ll->next)) {
        if (0==strcmp(head_ll->next->key, key)) {
            //found element, bypass this element in the LL
            struct hash_table_entry *tmp=head_ll->next;
            head_ll->next=tmp->next;
            //delete the element
            free(tmp->key);
            free(tmp->value);
            free(tmp);
            return 0;
        }
        head_ll=head_ll->next;
    }
    return -1; //didnt find key
}

void deleteTable(struct hash_table *my_table) {
    //for each entry, delete the entry linked list
    size_t i=0;
    struct hash_table_entry *head_ll, *temp;
    while(i< my_table->size) {
        head_ll=my_table->table[i];
        while(NULL != head_ll) {
            temp=head_ll->next;
            free(head_ll->key);
            free(head_ll->value);
            free(head_ll);
            head_ll=temp;
        }
        i++;
    }
    //free the table
    free(my_table->table);
    //free hashtable
    free(my_table);
}
#endif //LAVA_HASHMAP_H
