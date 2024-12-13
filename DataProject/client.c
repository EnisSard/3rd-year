#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Windows sockets library

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void display_menu() {
    printf("\nSelect an option:\n");
    printf("1. Add a new book\n");
    printf("2. List the books\n");
    printf("3. Update book info\n");
    printf("0. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int choice;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Connected to server.\n");

    while (1) {
        display_menu();
        scanf("%d", &choice);
        getchar(); // Consume the newline character left in the buffer

        if (choice == 0) {
            printf("Exiting...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

        switch (choice) {
            case 1: {
                // Add a new book
                int id;
                char name[50], department[50];

                printf("Enter book ID: ");
                scanf("%d", &id);
                getchar(); // Consume newline
                printf("Enter book name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline
                printf("Enter book department: ");
                fgets(department, sizeof(department), stdin);
                department[strcspn(department, "\n")] = 0; // Remove newline

                snprintf(buffer, BUFFER_SIZE, "ADD %d \"%s\" \"%s\"", id, name, department);
                break;
            }
            case 2:
                // List books
                strcpy(buffer, "GET ALL");
                break;
            case 3: {
                // Update book info
                int id;
                char name[50], department[50];

                printf("Enter book ID to update: ");
                scanf("%d", &id);
                getchar(); // Consume newline
                printf("Enter new book name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline
                printf("Enter new book department: ");
                fgets(department, sizeof(department), stdin);
                department[strcspn(department, "\n")] = 0; // Remove newline

                snprintf(buffer, BUFFER_SIZE, "UPDATE %d \"%s\" \"%s\"", id, name, department);
                break;
            }
            default:
                printf("Invalid choice. Please try again.\n");
                continue;
        }

        // Send the request to the server
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            printf("Send failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        // Receive the response from the server
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
            printf("Receive failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        printf("Server response: %s\n", buffer);
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();

    printf("Press any key to exit...\n");
getchar();

    return 0;
}
