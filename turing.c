#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct State {
    uint32_t write;
    char move;
    uint32_t next_card;
};

struct Card {
    uint32_t id;
    struct State state[2];
    struct Card *next;
};

struct Card *get_cards(char *file) {
    /*
    Collects the data for the instruction cards from the file and creates a
    singly linked list

    args:
        file - path to the file which has the data in the specified format

    ret:
        head - pointer to the first instruction card
    */

    FILE *fptr = fopen(file, "r");
    if(fptr == NULL) {
        printf("Error reading file\n");
        exit(0);
    }

    // creating the hault card
    struct Card *head = (struct Card*)malloc(sizeof(struct Card));
    if(head == NULL) {
        printf("System out of memory");
        exit(0);
    }
    head->id = 0;
    head->state[0].write = 0, head->state[0].move = 'S', head->state[0].next_card = 0;
    head->state[1].write = 0, head->state[1].move = 'S', head->state[1].next_card = 0;

    head->next = (struct Card*)malloc(sizeof(struct Card));
    if(head->next == NULL) {
        printf("System out of memory");
        exit(0);
    }
    struct Card *dummy = head->next, *prev;

    while(fscanf(fptr, "%d", &dummy->id) != EOF) {

        fscanf(fptr, "%d %c %d", &dummy->state[0].write, &dummy->state[0].move, &dummy->state[0].next_card);
        fscanf(fptr, "%d %c %d", &dummy->state[1].write, &dummy->state[1].move, &dummy->state[1].next_card);

        dummy->next = (struct Card*)malloc(sizeof(struct Card));
        if(dummy == NULL) {
            printf("System out of memory");
            exit(0);
        }
        prev = dummy;
        dummy = dummy->next;
    }

    prev->next = NULL;
    free(dummy);
    fclose(fptr);

    return head;
}

struct Card *search_card(struct Card* head, uint32_t target) {
    /*
    Searches the singly linked list of cards to get the card with the target
    value as id

    args:
        head - pointer to the head of the singly linked list of cards
        target - id value of the card to find

    ret:
        if the card exists returns the pointer to the card
        else it returns 'NULL'
    */

    struct Card *dummy = head;

    while(dummy != NULL) {

        if(dummy->id == target)
            return dummy;
        dummy = dummy->next;
    }

    return NULL;
}

uint32_t find_duplicacy(struct Card *head, uint32_t target) {
    /*
    Searches the singly linked list for duplicacy with the target id
    args:
        head - the head of the singly linked lists of cards
        target - id value of the taarget card

    ret:
        0 if duplicate is found
        1 if duplicate is not found
    */

    uint32_t count = 0;
    struct Card *dummy = head;
    while(dummy != NULL) {

        if(dummy->id == target)
            count++;

        dummy = dummy->next;
    }

    if(count > 1)
        return 0;

    return 1;
}

uint32_t validate_cards(struct Card *head) {
    /*
    Validates the singly linked lists of cards

    args:
        head - the head of the singly linked lists of cards

    ret:
        if all cards are valid returns '1'
        else returns '0'
    */

    uint32_t ret = 1;
    printf("\nValidating Cards...\n");

    struct Card *dummy = head;
    int cards_count = 0;

    while(dummy->next != NULL) {

        dummy = dummy->next;

        uint32_t is_valid_card = 1;
        cards_count++;

        if(find_duplicacy(head, dummy->id) == 0) {
            printf("Card ID: %d is duplicate\n", dummy->id);
            ret = 0;
            continue;
        }

        for(int i = 0; i < 2; ++i) {

            uint32_t write = dummy->state[i].write == 0 || dummy->state[i].write == 1;
            uint32_t move = dummy->state[i].move == 'L' || dummy->state[i].move == 'R' || dummy->state[i].move == 'S';
            struct Card *find = search_card(head, dummy->state[i].next_card);

            if(!write || !move || (!find && dummy->state[i].next_card != 0)) {
                is_valid_card = 0;
                break;
            }
        }

        if(is_valid_card) {
            printf("Card ID: %d is valid\n", dummy->id);

        } else {
            printf("Card ID: %d is not valid\n", dummy->id);
            ret = 0;
        }
    }

    printf("\n");

    if(cards_count < 1) {
        printf("Invalid: No cards availabl\n");
        return 0;
    }
    return ret;
}

void print_cards(struct Card *head) {
    /*
    Prints all the cards in the singly linked lists

    args:
        head - pointer to the head of the card
    */

    printf("Printing cards...\n-----------------\n");

    struct Card *dummy = head->next;
    while(dummy != NULL) {

        printf("\nCard - %d\n", dummy->id);
        printf("S  W M N\n");
        printf("0  %d %c %d\n", dummy->state[0].write, dummy->state[0].move, dummy->state[0].next_card);
        printf("1  %d %c %d\n\n", dummy->state[1].write, dummy->state[1].move, dummy->state[1].next_card);

        dummy = dummy->next;
    }
    printf("-----------------\n");
}

void free_cards(struct Card *head) {
    /*
    Frees the memory of cards

    args:
        head - pointer to the head of the card
    */

    struct Card *temp1 = head, *temp2;
    while(temp1 != NULL) {
        temp2 = temp1->next;
        free(temp1);
        temp1 = temp2;
    }
}

struct Cell {
    uint32_t val;
    struct Cell *next;
    struct Cell *prev;
};

void print_tape(struct Cell *head) {
    /*
    Prints the tape (doubly linked list containing all cells)

    args:
        head - head of the tape (doubly linked list containing all cells)
    */

    struct Cell *dummy = head;

    while(dummy->prev != NULL)
        dummy = dummy->prev;

    while(dummy != NULL) {
        printf("%d ", dummy->val);
        dummy = dummy->next;
    }

    printf("\n\n");
}

struct Cell *move(struct Cell *head, char dir) {
    /*
    Moves the head of the tape (doubly linked list containing all cells)

    args:
        head - head of the tape (doubly linked list containing all cells)
        dir - direction to move
            L - move left
            R - move right
            S - stay
            if excluded options are given the program will exit
            (the validate_cards function will take care of this)

    ret:
        returns the head of the pointer moved to the specified direction

    If when the shifted cell is non-exixting it creates a new cell and initialises
    its valus to '0' and adds to the tape (doubly linked list containing all cells)
    */

    if(dir == 'R') {

        if(head->next == NULL) {
            struct Cell *temp = (struct Cell*)malloc(sizeof(struct Cell));
            if(temp == NULL) {
                printf("System out of memory");
                exit(0);
            }
            temp->val = 0;
            head->next = temp;
            temp->prev = head;
            temp->next = NULL;
        }

        head = head->next;

    } else if(dir == 'L') {

        if(head->prev == NULL) {
            struct Cell *temp = (struct Cell *)malloc(sizeof(struct Cell));
            if(temp == NULL) {
                printf("System out of memory");
                exit(0);
            }
            temp->val = 0;
            head->prev = temp;
            temp->next = head;
            temp->prev = NULL;
        }

        head = head->prev;

    } else if(dir == 'S') {

    } else {
        printf("Invalid Direction\n");
        exit(0);
    }

    return head;
}

void get_initial_tape(struct Cell *head, char *file) {
    /*
    Creates the initial tape (doubly linked list) from the file with the
    specified format

    args:
        head - the one cell created at the beginning
        file - the file with the specified format to create tape
    */

    FILE *fptr = fopen(file, "r");
    if(fptr == NULL) {
        printf("Error reading file\n");
        exit(0);
    }

    struct Cell *dummy = head;
    char temp;

    while(fscanf(fptr, "%c", &temp) != EOF) {
        dummy->val = temp - '0';
        dummy = move(dummy, 'R');
        fscanf(fptr, "%c", &temp);
    }

    dummy = move(dummy, 'L');
    free(dummy->next);
    dummy->next = NULL;
}

uint32_t validate_tape(struct Cell *head) {
    /*
    Validates the tape (the value of each cell must be '0'/'1')

    args:
        head - head of the tape (doubly linked list containing all cells)
    */

    struct Cell* dummy = head;

    while(dummy != NULL) {

        if(dummy->val != 0 && dummy->val != 1) {
            printf("Invalid Tape!\n");
            return 0;
        }

        dummy = dummy->next;
    }

    printf("Valid tape\n\n");
    return 1;
}

uint32_t tape_count_ones(struct Cell *head) {
    /*
    Counts the number of ones in the tape (doubly linked list containing all cells)

    args:
        head - head of the tape (doubly linked list containing all cells)

    ret:
        count - number of ones in the tape
    */

    uint32_t count = 0;

    struct Cell *dummy = head;
    while(dummy != NULL) {
        if(dummy->val == 1)
            count++;
        dummy = dummy->prev;
    }

    dummy = head->next;
    while(dummy != NULL) {
        if(dummy->val == 1)
            count++;
        dummy = dummy->next;
    }

    return count;
}

void free_tape(struct Cell *head) {
    /*
    Frees the memory of cells

    args:
        head - pointer to the head of the cell
    */

    struct Cell *temp1 = head, *temp2;
    while(temp1 != NULL) {
        temp2 = temp1->next;
        free(temp1);
        temp1 = temp2;
    }
}

// maximum number of shifts done by the program
const uint32_t MAX_STEPS = 10000;

int main(int argc, char *argv[]) {

    struct Card *cards_head = get_cards(argv[1]);
    if (cards_head == NULL) {
        printf("System out of memory");
        exit(0);
    }

    if (validate_cards(cards_head) == 0) {
        return 0;
    }

    print_cards(cards_head);

    struct Cell *tape_head = (struct Cell*)malloc(sizeof(struct Cell));
    if (!tape_head) {
        printf("System out of memory");
        exit(0);
    }
    tape_head->prev = NULL, tape_head->next = NULL;

    get_initial_tape(tape_head, argv[2]);

    if(validate_tape(tape_head) == 0)
        return 0;

    print_tape(tape_head);

    printf("Starting Execution...\n---------------------\n\n");
    struct Card *current_card = cards_head->next;

    int idx = 0;
    while(1) {
        printf("STEP: %d\n", idx);
        print_tape(tape_head);

        if (current_card->id == 0) {
            break;
        }

        uint32_t temp_val = tape_head->val;
        tape_head->val = current_card->state[temp_val].write;
        tape_head = move(tape_head, current_card->state[temp_val].move);
        current_card = search_card(cards_head, current_card->state[temp_val].next_card);

        idx++;
        if(idx == MAX_STEPS+1) {
            printf("Maximum steps exeeced\n");
            return 0;
        }
    }

    printf("---------------------\nEnd of Execution\n\n");
    printf("Result\n");
    print_tape(tape_head);
    printf("Number of ones: %d\n", tape_count_ones(tape_head));

    free_cards(cards_head);
    free_tape(tape_head);
}