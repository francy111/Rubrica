#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "./../include/utility.h"
#include <unistd.h>
#include <termios.h>

// Deve essere invocata soltanto se si è sicuri che il buffer di input non è vuoto altrimenti verrà richiesto all'utente di digitare un carattere
void cleanInputBuffer(void) { 
    char c;
    while((c = getchar()) != '\n' && c != EOF){
        // Scarta tutti i caratteri fino a newline o EOF
    }
}

long getOptionalInput(char* string, size_t size) {
    if(fgets(string, size, stdin) != NULL){
        size_t len = strlen(string);

        // Controlla se la stringa è lunga quanto il massimo e se l'ultimo carattere è una newline (Invio dell'utente)
        if (len == size - 1 && string[len-1] != '\n')
            cleanInputBuffer(); // Se non c'è il newline, allora la stringa inserita dall'utente era troppo lunga e il buffer è sicuramente sporco
        
        if (len > 0 && string[len-1] == '\n') {
            string[len-1] = '\0'; // Rimuove il newline
        } 
        return len;
    } else {
        return -1;
    }
}

char getSingleChar(void){
    // acquisisco dall'utente una stringa di massimo 2 caratteri + '\0'
    char input[3] = {'x','x','\0'};
    getOptionalInput(input, 3);
    // controlliamo se il secondo carattere è vuoto (quindi se l'utente ha digitato un solo carattere)
    if(input[1] == '\0')
        return input[0]; // se è un solo carattere lo restituisce
    else
        return EOF; // uso EOF come carattere per il caso di errore
}


void getOptionalContact(Contact *contact, char* title){
    
    printTitle(title);
    printf("Digitare i dati del contatto (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
    printf("  Nome: " CYAN);
    getOptionalInput(contact->name,CONTACT_STRINGS_LENGTH+1); // acquisisco una stringa di 11 + '\0' così posso verificare se è più lunga di 10
    printf(RESET_COLOR);
    // se il nome non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isNameValid(contact->name)){
        printf(CLEAR);
        if(strlen(contact->name) > CONTACT_STRINGS_LENGTH - 1) 
            printCommunication("Nome digitato non valido, digitare massimo 10 caratteri", RED);
        else 
            printCommunication("Nome digitato non valido, utilizzare solo lettere e numeri", RED);
        printTitle(title);
        printf("Digitare i dati del contatto (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
        printf("  Nome: " CYAN);
        getOptionalInput(contact->name,CONTACT_STRINGS_LENGTH + 1);
        printf(RESET_COLOR);
    }
    printf(CLEAR);
    printTitle(title);
    printf("Digitare i dati del contatto (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
    printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
    printf("  Cognome: " CYAN);
    getOptionalInput(contact->surname,CONTACT_STRINGS_LENGTH - 1);
    printf(RESET_COLOR);
    // se il cognome non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isSurnameValid(contact->surname)){
        printf(CLEAR);
        if(strlen(contact->surname) > 10) 
            printCommunication("Cognome digitato non valido, digitare massimo 10 caratteri", RED);
        else 
            printCommunication("Cognome digitato non valido, utilizzare solo lettere e numeri", RED);
        printTitle(title);
        printf("Digitare i dati del contatto da leggere (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
        printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
        printf("  Cognome: " CYAN);
        getOptionalInput(contact->surname,CONTACT_STRINGS_LENGTH + 1);
        printf(RESET_COLOR);
    }
    printf(CLEAR);
    printTitle(title);
    printf("Digitare i dati del contatto (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
    printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
    printf("  Cognome: " CYAN "%s\n" RESET_COLOR, contact->surname);
    printf("  Numero di telefono: " CYAN);
    getOptionalInput(contact->phoneNumber,CONTACT_STRINGS_LENGTH+1);
    printf(RESET_COLOR);
    // se il numero di telefono non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isPhoneNumberValid(contact->phoneNumber)){
        printf(CLEAR);
        printCommunication("Numero di telefono digitato non valido, digitare esattamente 0 o 10 cifre", RED);
        printTitle(title);
        printf("Digitare i dati del contatto (" WHITEBG " ENTER " RESET_COLOR " per lasciarli vuoti):\n\n");
        printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
        printf("  Cognome: " CYAN "%s\n" RESET_COLOR, contact->surname);
        printf("  Numero di telefono: " CYAN);
        getOptionalInput(contact->phoneNumber,CONTACT_STRINGS_LENGTH+1);
        printf(RESET_COLOR);
    }
    printf(CLEAR);
}

void getNotEmptyContact(Contact *contact, char* title){
    
    // Stampa di un menu per acquisire un contatto
    printTitle(title); // Stampa il titolo passato
    printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
    printf("  Nome: " CYAN);
    getOptionalInput(contact->name,CONTACT_STRINGS_LENGTH+1);
    printf(RESET_COLOR);
    // Se il nome non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isNameValidAndNotEmpty(contact->name)){
        printf(CLEAR);
        // Se è valido ma non ha superato il controllo allora è vuoto quindi
        if(isNameValid(contact->name))
            printCommunication("Nome digitato non valido, non può essere lasciato vuoto", RED);
        else{ // Altrimenti se non è valido è per forza oltre 10 caratteri
            if(strlen(contact->name) > CONTACT_STRINGS_LENGTH - 1) 
                printCommunication("Nome digitato non valido, digitare massimo 10 caratteri", RED);
            else
                printCommunication("Nome digitato non valido, utilizzare solo lettere e numeri", RED);
        }   
        // Ripropone l'acquisizione del nome
        printTitle(title);
        printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
        printf("  Nome: " CYAN);
        getOptionalInput(contact->name,CONTACT_STRINGS_LENGTH+1);
        printf(RESET_COLOR);
    }
    printf(CLEAR);
    printTitle(title);
    printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
    printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
    printf("  Cognome: " CYAN);
    getOptionalInput(contact->surname,CONTACT_STRINGS_LENGTH+1);
    printf(RESET_COLOR);
    // Se il cognome non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isSurnameValidAndNotEmpty(contact->surname)){
        printf(CLEAR);
        // Se è valido ma non ha superato il controllo allora è vuoto quindi
        if(isNameValid(contact->surname))
            printCommunication("Cognome digitato non valido, non può essere lasciato vuoto", RED);
        else{ // Altrimenti se non è valido è per forza oltre 10 caratteri
            if(strlen(contact->surname) > CONTACT_STRINGS_LENGTH - 1) 
                printCommunication("Cognome digitato non valido, digitare massimo 10 caratteri", RED);
            else 
                printCommunication("Cognome digitato non valido, utilizzare solo lettere e numeri", RED);
        } 
        printTitle(title);
        printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
        printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
        printf("  Cognome: " CYAN);
        getOptionalInput(contact->surname,CONTACT_STRINGS_LENGTH + 1);
        printf(RESET_COLOR);
    }

    printf(CLEAR);
    printTitle(title);
    printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
    printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
    printf("  Cognome: " CYAN "%s\n" RESET_COLOR, contact->surname);
    printf("  Numero di telefono: " CYAN);
    getOptionalInput(contact->phoneNumber,CONTACT_STRINGS_LENGTH+1);
    printf(RESET_COLOR);
    // Se il numero di telefono non è valido viene riacquisito comunicando l'errore e ristampando il menu
    while(!isPhoneNumberValidAndNotEmpty(contact->phoneNumber)){
        printf(CLEAR);
        // Se è valido ma non ha superato il controllo allora è vuoto quindi
        if(isPhoneNumberValid(contact->phoneNumber))
            printCommunication("Numero di telefono digitato non valido, non può essere lasciato vuoto", RED);
        else // Altrimenti se non è valido è per forza oltre 10 caratteri
            printCommunication("Numero di telefono digitato non valido, digitare esattamente 10 cifre", RED);
        printTitle(title);
        printf("Digitare i dati del contatto (" UNDERLINE "non" RESET_COLOR " possono essere lasciati vuoti):\n\n");
        printf("  Nome: " CYAN "%s\n" RESET_COLOR, contact->name);
        printf("  Cognome: " CYAN "%s\n" RESET_COLOR, contact->surname);
        printf("  Numero di telefono: " CYAN);
        getOptionalInput(contact->phoneNumber,CONTACT_STRINGS_LENGTH+1);
        printf(RESET_COLOR);
    }
    printf(CLEAR);
}

int isNameValid(char *name){
    size_t len = strlen(name);
    // Se è vuoto va bene, altrimenti deve essere valido e non vuoto (<= 10 lettere/cifre)
    if(len > 0) 
        return isNameValidAndNotEmpty(name);
    else
        return 1;
}
int isSurnameValid(char *surname){
    size_t len = strlen(surname);
    // Se è vuoto va bene, altrimenti deve essere valido e non vuoto (<= 10 lettere/cifre)
    if(len > 0) 
        return isNameValidAndNotEmpty(surname);
    else
        return 1;
}

int isPhoneNumberValid(char *phoneNumber){
    size_t len = strlen(phoneNumber); 
    // Se è vuoto va bene, altrimenti deve essere valido e non vuoto (10 cifre)
    if(len > 0)
        return isPhoneNumberValidAndNotEmpty(phoneNumber);
    else
        return 1;
}
 
int isNameValidAndNotEmpty(char *name){
    size_t len = strlen(name); 
    int result = 0;
    // Non deve essere vuoto ma nemmeno più lungo di 10 caratteri
    if(0 < len && len < CONTACT_STRINGS_LENGTH) { // Compreso tra 1 e 10
        result = 1;
        for(int i = 0; i < len; i++){
            if(!(isDigit(name[i]) || isLetter(name[i]))) // Se non è una lettera/cifra non è valido
                result = 0;
        }
    }
    return result;
}
int isSurnameValidAndNotEmpty(char *surname){
    size_t len = strlen(surname); 
    int result = 0;
    // Non deve essere vuoto ma nemmeno più lungo di 10 caratteri
    if(0 < len && len < CONTACT_STRINGS_LENGTH) { // Compreso tra 1 e 10
        result = 1;
        for(int i = 0; i < len; i++){
            if(!(isDigit(surname[i]) || isLetter(surname[i]))) // Se non è una lettera/cifra non è valido
                result = 0;
        }
    }
    return result;
}

int isPhoneNumberValidAndNotEmpty(char *phoneNumber){
    size_t len = strlen(phoneNumber); 
    int result = 0;
    // Deve essere lungo esattamente dieci cifre
    if(len == CONTACT_STRINGS_LENGTH - 1) { // 11 - 1 
        result = 1;
        for(int i = 0; i < len; i++){
            if(!isDigit(phoneNumber[i])) // Se non è una cifra non è valido
                result = 0;
        }
    }
    return result;
}

int isUsernameValidAndNotEmpty(char *username){
    size_t len = strlen(username); 
    int result = 0;
    // Non deve essere vuoto ma nemmeno più lungo di 20 caratteri
    if(0 < len && len < AUTH_STRING_LENGTH) { // Compreso tra 1 e 20
        result = 1;
        for(int i = 0; i < len; i++){
            if(!(isDigit(username[i]) || isLetter(username[i]))) // Se non è una lettera/cifra non è valido
                result = 0;
        }
    }
    return result;
}

int isPasswordValid(char *password){
    size_t len = strlen(password); 
    int result = 0;
    if(len < AUTH_STRING_LENGTH) { // Minore o uguale a 20
        result = 1;
        for(int i = 0; i < len; i++){
            if(!(isDigit(password[i]) || isLetter(password[i]))) // Se non è una lettera/cifra non è valido
                result = 0;
        }
    }
    return result;
}

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int isLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

void printTitle(const char* message){
    size_t len = strlen(message);

    /*
     * ==========================================================
     *  Utilizziamo questa funzione per ottenere questo risultato
     * ==========================================================
     */

    // Per presentare un bordo scriviamo tanti uguali quanti caratteri sono nel messaggio, questa è la riga superiore
    for(int i = 0; i < len; i++)
        printf("=");
    printf("\n");
    printf(BOLD "%s\n" RESET_COLOR,message);

    // Per presentare un bordo scriviamo tanti uguali quanti caratteri sono nel messaggio, questa è la riga inferiore
    for(int i = 0; i < len; i++)
        printf("=");
     printf("\n\n");
}

void printCommunication(const char* message,const char* color){
    printf("%s" "%s\n\n" RESET_COLOR, color, message); // Stampa la stringa colorata con color
}

void printContactIndex(Contact contact, int matchIndex){
    printf("Corrispondenza " BMAGENTA "%d" RESET_COLOR ":\n", matchIndex);
    printf("  Nome: " CYAN "%s" RESET_COLOR "\n", contact.name);
    printf("  Cognome: " CYAN "%s" RESET_COLOR "\n", contact.surname);
    //printf("  Numero di telefono: " CYAN "%s" RESET_COLOR "\n\n\n", contact.phoneNumber);
    printf("  Numero di telefono: " CYAN);
    size_t len = strlen(contact.phoneNumber); 
    if(len == 10) {
        for(int i = 0; i < 3; i++)
            printf("%c",contact.phoneNumber[i]);
        printf(" ");
        for(int i = 3; i < 6; i++)
            printf("%c",contact.phoneNumber[i]);
        printf(" ");
        for(int i = 6; i < 10; i++)
            printf("%c",contact.phoneNumber[i]);
        
    } else{
        printf(RED "non valido");
    }
    printf(RESET_COLOR "\n\n\n");
}

void printContact(Contact contact){
    printf("  Nome: ");
    if(isSurnameValidAndNotEmpty(contact.name))  // Scriviamo il nome se corretto
        printf(CYAN "%s" RESET_COLOR "\n", contact.name);
    else
        printf(RED "non valido" RESET_COLOR); // Altrimenti lo indichiamo

    printf("  Cognome: ");
    if(isSurnameValidAndNotEmpty(contact.surname)) // Scriviamo il cognome se corretto
        printf(CYAN "%s" RESET_COLOR "\n", contact.surname);
    else
        printf(RED "non valido" RESET_COLOR); // Altrimenti lo indichiamo

    printf("  Numero di telefono: " CYAN);
    if(isPhoneNumberValidAndNotEmpty(contact.phoneNumber)) { // Formattiamo il numero di telefono per scriverlo nella forma [xxx yyy zzzz]
        for(int i = 0; i < 3; i++)
            printf("%c",contact.phoneNumber[i]);
        printf(" ");
        for(int i = 3; i < 6; i++)
            printf("%c",contact.phoneNumber[i]);
        printf(" ");
        for(int i = 6; i < 10; i++)
            printf("%c",contact.phoneNumber[i]);
        
    } else{ // Se non è corretto lo indichiamo
        printf(RED "non valido");
    }
    printf(RESET_COLOR "\n\n\n");
}


void disableEcho(void) {
    struct termios term;

    // Ottiene le impostazioni correnti del terminale
    tcgetattr(STDIN_FILENO, &term);

    // Disabilita l'ECHO, non mostrando l'input
    term.c_lflag &= ~ECHO;

    // Applica le nuove impostazioni
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void enableEcho(void) {
    struct termios term;

    // Ottiene le impostazioni correnti del terminale
    tcgetattr(STDIN_FILENO, &term);

    // Disabilita l'ECHO, non mostrando l'input
    term.c_lflag |= ECHO;

    // Applica le nuove impostazioni
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
