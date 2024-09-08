#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_ISBN_LENGTH 13
#define MAX_GENRE_LENGTH 50
#define MAX_RACK 15
#define MAX_SIZE_USER_NAME 20
#define MAX_SIZE_PASSWORD 20
#define TABLE_SIZE 1000
#define FILE_HEADER_SIZE sizeof(sFileHeader)

struct Book
{
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char ISBN[MAX_ISBN_LENGTH];
    char genre[MAX_GENRE_LENGTH];
    char rack[MAX_RACK];
    int quantity;
} book;
struct HashNode
{
    char key[14];
    struct Book value;
    struct HashNode *next;
};

struct HashTable
{
    struct HashNode *table[TABLE_SIZE];
};
void initializeHashTable(struct HashTable *hashTable)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        hashTable->table[i] = NULL;
    }
}

unsigned int hashFunction(const char *key)
{
    unsigned int hash = 7573;
    while (*key)
    {
        hash = (hash * 31) + *key;
        key++;
    }
    return hash % TABLE_SIZE;
}
struct sFileHeader
{
    char username[MAX_SIZE_USER_NAME];
    char password[MAX_SIZE_PASSWORD];
} sFileHeader;

void getCurrentDate(char currentDate[])
{
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    strftime(currentDate, 20, "%Y-%m-%d", tm_info);
}

void insertBook(struct HashTable *hashTable, const struct Book *newBook)
{
    unsigned int index = hashFunction(newBook->ISBN);

    struct HashNode *newNode = (struct HashNode *)malloc(sizeof(struct HashNode));
    if (newNode == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    strcpy(newNode->key, newBook->ISBN);
    newNode->value = *newBook;
    newNode->next = NULL;

    if (hashTable->table[index] == NULL)
    {
        hashTable->table[index] = newNode;
    }
    else
    {
        newNode->next = hashTable->table[index];
        hashTable->table[index] = newNode;
    }
}

void loadBooksFromFile(struct HashTable *hashTable, const char *filePath)
{
    FILE *file = fopen(filePath, "r");
    if (file == NULL)
    {
        printf("Error opening file for reading: %s\n", filePath);
        return;
    }

    struct Book newBook;
    int readFields;
    int lineCount = 0;

    while ((readFields = fscanf(file, "%99[^,],%99[^,],%13[^,],%49[^,],%14[^,],%d\n",
                                newBook.title, newBook.author, newBook.ISBN, newBook.genre, newBook.rack, &newBook.quantity)) == 6)
    {
        insertBook(hashTable, &newBook);
        lineCount++;
    }

    if (readFields == EOF)
    {
        printf("End of file reached.\n");
    }
    else if (readFields != 6)
    {
        printf("Error reading from file: %s\n", filePath);
        printf("Unexpected number of fields or format at line %d.\n", lineCount + 1);
    }

    fclose(file);
}



void appendSearchTimeHash(const char *searchType, const char *searchTerm, double timeTaken)
{
    FILE *timeFile = fopen("hashtimetaken.txt", "a");
    int length = (int)strlen(searchTerm);

    if (timeFile == NULL)
    {
        perror("Error opening hashtimetaken.txt for appending");
        return;
    }

    fprintf(timeFile, "Search Type: %s, Search Term: %s, Time Taken: %.4f seconds,Length of the word :-%.4d\n", searchType, searchTerm, timeTaken, length);

    fclose(timeFile);
}
void printBook(const struct Book *book)
{
    printf("%-35s%-35s%-35s%-35s%-15s%-15d\n\n", book->title, book->author, book->ISBN, book->genre, book->rack, book->quantity);
    
}
// Replace strcmp with lcs in searchBookHashTable function
struct Book *searchBookHashTable(const struct HashTable *hashTable, const char *key, const char *searchType) {
    unsigned int index = hashFunction(key);

    struct HashNode *current = hashTable->table[index];
    while (current != NULL) {
        if ((strcmp(searchType, "ISBN") == 0 && lcs(current->key, key) == strlen(key)) ||
            (strcmp(searchType, "Author") == 0 && lcs(current->value.author, key) == strlen(key)) ||
            (strcmp(searchType, "Title") == 0 && lcs(current->value.title, key) == strlen(key))) {
            return &(current->value);
        }
        current = current->next;
    }

    return NULL;
}

// Replace strcmp with lcs in searchBooksHash function
void searchBooksHash(struct HashTable *hashTable, const char *searchType, const char *searchTerm) {
    clock_t startTime = clock();

    printf("\n////////////////////\n");
    printf("//   Search Result   //\n");
    printf("////////////////////\n\n");

    int count = 0;

    printf("%-4s%-35s%-35s%-35s%-20s%-15s%-15s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
    printf("=====================================================================================================================================================\n");

    for (int i = 0; i < TABLE_SIZE; i++) {
        struct HashNode *current = hashTable->table[i];
        while (current != NULL) {
            if ((strcmp(searchType, "ISBN") == 0 && lcs(current->key, searchTerm) == strlen(searchTerm)) ||
                (strcmp(searchType, "Author") == 0 && lcs(current->value.author, searchTerm) == strlen(searchTerm)) ||
                (strcmp(searchType, "Title") == 0 && lcs(current->value.title, searchTerm) == strlen(searchTerm))) {
                printf("%-4d", ++count);
                printBook(&(current->value));
            }
            current = current->next;
        }
    }

    if (count == 0) {
        printf("No matching records found.\n");
    } else {
        double timeTaken = ((double)(clock() - startTime)) / CLOCKS_PER_SEC;
        appendSearchTimeHash(searchType, searchTerm, timeTaken);
        printf("\nTime Taken: %.4f seconds\n", timeTaken);
    }
}

void showBooksHash(const struct HashTable *hashTable)
{
    printf("\n////////////////////\n");
    printf("//   Books List   //\n");
    printf("////////////////////\n\n");

    int count = 0;

    printf("%-4s%-35s%-35s%-35s%-20s%-15s%-10s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
    printf("============================================================================================================\n");

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        struct HashNode *current = hashTable->table[i];
        while (current != NULL)
        {
            printf("%-4d", ++count);
            printBook(&(current->value));
            current = current->next;
        }
    }
}

void headMessage(char *message)
{
    system("cls");
    printf("\t\t\t###########################################################################");
    printf("\n\t\t\t############                                                   ############");
    printf("\n\t\t\t############             LIBRARY MANAGEMENT SYSTEM             ############");
    printf("\n\t\t\t############                                                   ############");
    printf("\n\t\t\t###########################################################################");
    printf("\n\t\t\t---------------------------------------------------------------------------\n");
    // Print message center implementation
    printf("\n\t\t\t%s\n", message);
    printf("\n\t\t\t----------------------------------------------------------------------------");
}

void append_to_file(const char *file_path, char title[], char author[], char ISBN[], char genre[], char rack[], int quantity)
{
    FILE *file = fopen(file_path, "a");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    fprintf(file, "%s\t\t\t\t%s\t\t\t\t\t\t\t\t\t\t\t\t%s\t\t\t\t%s\t\t\t\t%s\t\t\t\t%d\n", title, author, ISBN, genre, rack, quantity);

    fclose(file);
}

void searchBooks(const char *searchType, const char *searchTerm)
{
    FILE *readfile;
    readfile = fopen("Book1.csv", "r");

    if (readfile == NULL)
    {
        perror("Error opening file");
        return;
    }

    printf("\n////////////////////\n");
    printf("//   Search Result   //\n");
    printf("////////////////////\n\n");

    char line[100];
    int count = 0;
    clock_t start_time = clock(); // Record start time

    // Print header
    printf("%-4s%-35s%-35s%-35s%-20s%-15s%-10s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
    printf("============================================================================================================\n");

    while (fgets(line, sizeof(line), readfile))
    {
        char *title = strtok(line, ",");
        char *author = strtok(NULL, ",");
        char *isbn = strtok(NULL, ",");
        char *genre = strtok(NULL, ",");
        char *rack = strtok(NULL, ",");
        char *quantity = strtok(NULL, ",");

        // Check if all fields are present
        if (title != NULL && author != NULL && isbn != NULL && genre != NULL && rack != NULL && quantity != NULL)
        {
            // Check if the search term matches the desired field
            if ((strcmp(searchType, "ISBN") == 0 && strcmp(isbn, searchTerm) == 0) ||
                (strcmp(searchType, "Author") == 0 && strcmp(author, searchTerm) == 0) ||
                (strcmp(searchType, "Title") == 0 && strcmp(title, searchTerm) == 0))
            {
                printf("%-4d%-35s%-35s%-35s%-20s%-15s%-8s\n", ++count, title, author, isbn, genre, rack, quantity);
            }
        }
        else
        {
            printf("Error parsing line: %s", line);
        }
    }

    fclose(readfile);

    clock_t end_time = clock();                                             // Record end time
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC; // Calculate time taken

    printf("\nTime Taken: %.4f seconds\n", time_taken);

    if (count == 0)
    {
        printf("No matching records found.\n");
    }

    // Append search details to a file
    FILE *log_file = fopen("search_time.txt", "a");
    if (log_file != NULL)
    {
        fprintf(log_file, "Search Type: %s, Search Term: %s, Time Taken: %.4f seconds, Length of the word: %04d\n",
                searchType, searchTerm, time_taken, (int)strlen(searchTerm));
        fclose(log_file);
    }
    else
    {
        printf("Error opening log file for appending.\n");
    }
}
void showBooks()
{
    int run = 1, count = 0;

    do
    {
        FILE *readfile;
        readfile = fopen("Book1.csv", "r");

        if (readfile == NULL)
        {
            perror("Error opening file");
            return;
        }

        printf("\n////////////////////\n");
        printf("//   Books List   //\n");
        printf("////////////////////\n\n");

        char line[100];

        // Print header
        printf("%-4s%-35s%-35s%-35s%-20s%-15s%-10s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
        printf("============================================================================================================\n");

        while (fgets(line, sizeof(line), readfile))
        {
            printf("%-4d", ++count);

            char *title = strtok(line, ",");
            char *author = strtok(NULL, ",");
            char *isbn = strtok(NULL, ",");
            char *genre = strtok(NULL, ",");
            char *rack = strtok(NULL, ",");
            char *quantity = strtok(NULL, ",");

            // Check if all fields are present
            if (title != NULL && author != NULL && isbn != NULL && genre != NULL && rack != NULL && quantity != NULL)
            {
                printf("%-35s%-35s%-35s%-20s%-15s%-8s\n", title, author, isbn, genre, rack, quantity);
            }
            else
            {
                printf("Error parsing line: %s", line);
            }
        }

        fclose(readfile);
        count = 0;

        int searchOption;
        printf("\nSearch Options:\n");
        printf("1. Search by ISBN\n");
        printf("2. Search by Author\n");
        printf("3. Search by Title\n");
        printf("0. Go back\n");
        printf("Enter your choice: ");
        scanf("%d", &searchOption);

        char searchTerm[100];
        getchar(); // Consume the newline character left in the input buffer

        switch (searchOption)
        {
        case 1:
            printf("Enter ISBN to search: ");
            fgets(searchTerm, sizeof(searchTerm), stdin);
            strtok(searchTerm, "\n"); // Remove the newline character from the input
            searchBooks("ISBN", searchTerm);
            break;
        case 2:
            printf("Enter Author to search: ");
            fgets(searchTerm, sizeof(searchTerm), stdin);
            strtok(searchTerm, "\n"); // Remove the newline character from the input
            searchBooks("Author", searchTerm);
            break;
        case 3:
            printf("Enter Title to search: ");
            fgets(searchTerm, sizeof(searchTerm), stdin);
            strtok(searchTerm, "\n"); // Remove the newline character from the input
            searchBooks("Title", searchTerm);
            break;
        case 0:
            run = 0;
            break;
        default:
            printf("Invalid choice. Please enter a valid option.\n");
        }

    } while (run != 0);
}
void borrowBook(const char *file_path, const char *ISBN)
{
    FILE *file = fopen(file_path, "r+");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    printf("\n////////////////////\n");
    printf("//   Borrow Book   //\n");
    printf("////////////////////\n\n");

    char line[100];
    int count = 0;
    int bookFound = 0;

    FILE *tempFile = fopen("temp.csv", "w");
    if (tempFile == NULL)
    {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    // Print header
    printf("%-4s%-35s%-35s%-35s%-20s%-15s%-10s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
    printf("============================================================================================================\n");

    while (fgets(line, sizeof(line), file))
    {
        char *title = strtok(line, ",");
        char *author = strtok(NULL, ",");
        char *isbn = strtok(NULL, ",");
        char *genre = strtok(NULL, ",");
        char *rack = strtok(NULL, ",");
        char *quantity_str = strtok(NULL, ",");

        if (title != NULL && author != NULL && isbn != NULL && genre != NULL && rack != NULL && quantity_str != NULL)
        {
            int quantity = atoi(quantity_str);
            printf("%-4d%-35s%-35s%-35s%-20s%-15s%-8d\n", ++count, title, author, isbn, genre, rack, quantity);

            if (strcmp(isbn, ISBN) == 0 && quantity > 0)
            {
                quantity--;
                fprintf(tempFile, "%s,%s,%s,%s,%s,%d\n", title, author, isbn, genre, rack, quantity);
                bookFound = 1;
                printf("Book with ISBN %s borrowed successfully.\n", ISBN);
            }
            else
            {
                fprintf(tempFile, "%s,%s,%s,%s,%s,%s", title, author, isbn, genre, rack, quantity_str);
            }
        }
        else
        {
            printf("Error parsing line: %s", line);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(file_path);
    rename("temp.csv", file_path);

    if (!bookFound)
    {
        printf("Book with ISBN %s not found or out of stock.\n", ISBN);
    }
}

void returnBook(const char *file_path, const char *ISBN)
{
    FILE *file = fopen(file_path, "r+");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    printf("\n////////////////////\n");
    printf("//   Return Book   //\n");
    printf("////////////////////\n\n");

    char line[100];
    int count = 0;
    int bookFound = 0;

    FILE *tempFile = fopen("temp.csv", "w");
    if (tempFile == NULL)
    {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    // Print header
    printf("%-4s%-35s%-35s%-35s%-20s%-15s%-10s\n", "No.", "Title", "Author", "ISBN", "Genre", "Rack", " Quantity");
    printf("============================================================================================================\n");

    while (fgets(line, sizeof(line), file))
    {
        char *title = strtok(line, ",");
        char *author = strtok(NULL, ",");
        char *isbn = strtok(NULL, ",");
        char *genre = strtok(NULL, ",");
        char *rack = strtok(NULL, ",");
        char *quantity_str = strtok(NULL, ",");

        if (title != NULL && author != NULL && isbn != NULL && genre != NULL && rack != NULL && quantity_str != NULL)
        {
            int quantity = atoi(quantity_str);
            printf("%-4d%-35s%-35s%-35s%-20s%-15s%-8d\n", ++count, title, author, isbn, genre, rack, quantity);

            if (strcmp(isbn, ISBN) == 0)
            {
                quantity++;
                fprintf(tempFile, "%s,%s,%s,%s,%s,%d\n", title, author, isbn, genre, rack, quantity);
                bookFound = 1;
                printf("Book with ISBN %s returned successfully.\n", ISBN);
            }
            else
            {
                fprintf(tempFile, "%s,%s,%s,%s,%s,%s", title, author, isbn, genre, rack, quantity_str);
            }
        }
        else
        {
            printf("Error parsing line: %s", line);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(file_path);
    rename("temp.csv", file_path);

    if (!bookFound)
    {
        printf("Book with ISBN %s not found.\n", ISBN);
    }
}
int lcs(const char *str1, const char *str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; ++i) {
        for (int j = 0; j <= len2; ++j) {
            if (i == 0 || j == 0)
                dp[i][j] = 0;
            else if (str1[i - 1] == str2[j - 1])
                dp[i][j] = dp[i - 1][j - 1] + 1;
            else
                dp[i][j] = (dp[i - 1][j] > dp[i][j - 1]) ? dp[i - 1][j] : dp[i][j - 1];
        }
    }

    return dp[len1][len2];
}


void print_menu(struct HashTable library)
{
    const char *file_path = "C:\\Users\nayak\\OneDrive\\Desktop\\rabinkarp\\BOOk.csv"; // Replace with your actual file path

    int choice;
    do
    {
        printf("\nPress 1. To find book\nPress 2. To add book to inventory\nPress 3. To borrow book\nPress 4. To return book\nPress 5. To view books\nPress 6 to use Hash table\nPress 7. to EXIT\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("\nPress 1. Search by ISBN\nPress 2. Search by Author\nPress 3. Search by Title\nPress 0. Go back\n");
            int searchOption;
            scanf("%d", &searchOption);
            getchar(); // consume the newline character left in the input buffer
            char searchTerm[100];
            switch (searchOption)
            {
            case 1:
                printf("Enter ISBN to search: ");
                fgets(searchTerm, sizeof(searchTerm), stdin);
                strtok(searchTerm, "\n"); // Remove the newline character from the input
                searchBooks("ISBN", searchTerm);
                break;
            case 2:
                printf("Enter Author to search: ");
                fgets(searchTerm, sizeof(searchTerm), stdin);
                strtok(searchTerm, "\n"); // Remove the newline character from the input
                searchBooks("Author", searchTerm);
                break;
            case 3:
                printf("Enter Title to search: ");
                fgets(searchTerm, sizeof(searchTerm), stdin);
                strtok(searchTerm, "\n"); // Remove the newline character from the input
                searchBooks("Title", searchTerm);
                break;
            case 0:
                break;
            default:
                printf("Invalid choice. Returning to main menu.\n");
            }
            break;
        case 2:
        {
            struct Book newBook;
            printf("Enter author name: ");
            scanf("%99s", newBook.author);
            printf("Enter book title: ");
            scanf("%99s", newBook.title);
            printf("Enter book ISBN: ");
            scanf("%12s", newBook.ISBN);
            printf("Enter genre: ");
            scanf("%49s", newBook.genre);
            printf("Enter rack number: ");
            scanf("%14s", newBook.rack);
            printf("Enter quantity: ");
            scanf("%d", &newBook.quantity);

            append_to_file(file_path, newBook.title, newBook.author, newBook.ISBN, newBook.genre, newBook.rack, newBook.quantity);
            break;
        }
        case 3:
        {
            char ISBN[13];
            printf("Enter ISBN of the book to borrow: ");
            scanf("%12s", ISBN);
            borrowBook(file_path, ISBN);
            break;
        }
        case 4:
        {
            char ISBN[13];
            printf("Enter ISBN of the book to return: ");
            scanf("%12s", ISBN);
            returnBook(file_path, ISBN);
            break;
        }
        case 5:
            showBooks();
            break;
        case 6:
        {
            int choice1; // Move the declaration to the beginning of the block
            do
            {
                printf("\nPress 1. To find book\nPress 2. To add book to inventory\nPress 3. To view books\nPress 4 to exit\n");
                scanf("%d", &choice1);
                getchar(); // Consume newline character left in the input buffer

                switch (choice1)
                {
                case 1:
                {
                    printf("\nPress 1. Search by ISBN\nPress 2. Search by Author\nPress 3. Search by Title\nPress 0. Go back\n");
                    int searchOption;
                    scanf("%d", &searchOption);
                    getchar(); // Consume newline character left in the input buffer
                    char searchTerm[100];

                    switch (searchOption)
                    {
                    case 1:
                        printf("Enter ISBN to search: ");
                        fgets(searchTerm, sizeof(searchTerm), stdin);
                        strtok(searchTerm, "\n");
                        searchBooksHash(&library, "ISBN", searchTerm);
                        break;
                    case 2:
                        printf("Enter Author to search: ");
                        fgets(searchTerm, sizeof(searchTerm), stdin);
                        strtok(searchTerm, "\n");
                        searchBooksHash(&library, "Author", searchTerm);
                        break;
                    case 3:
                        printf("Enter Title to search: ");
                        fgets(searchTerm, sizeof(searchTerm), stdin);
                        strtok(searchTerm, "\n");
                        searchBooksHash(&library, "Title", searchTerm);
                        break;
                    case 0:
                        break;
                    default:
                        printf("Invalid choice. Returning to main menu.\n");
                    }
                    break;
                }
                case 2:
                {
                    struct Book newBook;
                    printf("Enter author name: ");
                    scanf("%99s", newBook.author);
                    printf("Enter book title: ");
                    scanf("%99s", newBook.title);
                    printf("Enter book ISBN: ");
                    scanf("%13s", newBook.ISBN);
                    printf("Enter genre: ");
                    scanf("%49s", newBook.genre);
                    printf("Enter rack number: ");
                    scanf("%14s", newBook.rack);
                    printf("Enter quantity: ");
                    scanf("%d", &newBook.quantity);
                    getchar(); // Consume newline character left in the input buffer

                    insertBook(&library, &newBook);
                    break;
                }
                case 3:
                    showBooksHash(&library);
                    break;
                case 4:
                    printf("Exiting...\n");
                    break;
                default:
                    printf("Invalid choice. Please enter a valid option.\n");
                }
            } while (choice1 != 4);
            break; // Ensure to break out of the switch case 6 block
        }

        case 7:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice. Please enter a valid option.\n");
        }

    } while (choice != 7);
}

int login()
{
    char enteredUsername[MAX_SIZE_USER_NAME] = {0};
    char enteredPassword[MAX_SIZE_PASSWORD] = {0};

    headMessage("Login");

    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file for reading.\n");
        return 0; // Return 0 to indicate login failure
    }

    struct sFileHeader fileHeaderInfo;

    printf("\n\n\n\t\t\t\tUsername:");
    fgets(enteredUsername, MAX_SIZE_USER_NAME, stdin);
    strtok(enteredUsername, "\n"); // Remove the newline character

    printf("\n\t\t\t\tPassword:");
    fgets(enteredPassword, MAX_SIZE_PASSWORD, stdin);
    strtok(enteredPassword, "\n"); // Remove the newline character

    while (fscanf(file, "%[^,],%[^\n]\n", fileHeaderInfo.username, fileHeaderInfo.password) == 2)
    {
        if ((strcmp(enteredUsername, fileHeaderInfo.username) == 0) &&
            (strcmp(enteredPassword, fileHeaderInfo.password) == 0))
        {
            fclose(file);

            // Save the current user to current_user.txt
            FILE *currentUserFile = fopen("current_user.txt", "w");
            if (currentUserFile != NULL)
            {
                fprintf(currentUserFile, "%s\n", enteredUsername);
                fclose(currentUserFile);
            }
            else
            {
                printf("Error opening current_user.txt for writing.\n");
            }

            return 1; // Return 1 to indicate login success
        }
    }

    fclose(file);
    headMessage("Login Failed");
    printf("\t\t\t\tSorry, Unknown User.\n");
    printf("\t\t\t\tPress any key to go back to the main menu.");
    getchar(); // Wait for user input
    return 0;  // Return 0 to indicate login failure
}

int main()
{
    struct HashTable library;
    initializeHashTable(&library);

    loadBooksFromFile(&library, "C:\\Users\\nayak\\OneDrive\\Desktop\\rabinkarp\\BOOk.csv");
    headMessage("Library Management System");

    int log = login();
    if (log == 1)
    {
        print_menu(library);
    }

    return 0;
}
