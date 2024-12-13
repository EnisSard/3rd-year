#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_NOTES 100

// Structure to store book details
typedef struct {
    int id;
    char name[50];
    char department[50];
} Book;

Book books[MAX_NOTES];
int book_count = 0;

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        char response[BUFFER_SIZE] = "";
        char command[10];
        sscanf(buffer, "%9s", command);

        if (strcmp(command, "ADD") == 0) {
            // Add a new book
            if (book_count < MAX_NOTES) {
                int id;
                char name[50], department[50];
                if (sscanf(buffer, "%*s %d \"%49[^\"]\" \"%49[^\"]\"", &id, name, department) == 3) {
                    books[book_count].id = id;
                    strncpy(books[book_count].name, name, sizeof(books[book_count].name));
                    strncpy(books[book_count].department, department, sizeof(books[book_count].department));
                    book_count++;

                    snprintf(response, BUFFER_SIZE, "Book added: ID=%d, Name=%s, Department=%s", id, name, department);
                } else {
                    snprintf(response, BUFFER_SIZE, "Invalid ADD command format.");
                }
            } else {
                snprintf(response, BUFFER_SIZE, "Error: Book list is full.");
            }
        } else if (strcmp(command, "GET") == 0) {
            // Handle "GET ALL" to list all books
            if (strstr(buffer, "ALL")) {
                if (book_count == 0) {
                    snprintf(response, BUFFER_SIZE, "No books available.");
                } else {
                    for (int i = 0; i < book_count; i++) {
                        char book_info[100];
                        snprintf(book_info, sizeof(book_info), "ID=%d, Name=%s, Department=%s\n",
                                 books[i].id, books[i].name, books[i].department);
                        strncat(response, book_info, BUFFER_SIZE - strlen(response) - 1);
                    }
                }
            }
        } else if (strcmp(command, "UPDATE") == 0) {
            // Update book information
            int id;
            char name[50], department[50];
            if (sscanf(buffer, "%*s %d \"%49[^\"]\" \"%49[^\"]\"", &id, name, department) == 3) {
                int found = 0;
                for (int i = 0; i < book_count; i++) {
                    if (books[i].id == id) {
                        strncpy(books[i].name, name, sizeof(books[i].name));
                        strncpy(books[i].department, department, sizeof(books[i].department));
                        snprintf(response, BUFFER_SIZE, "Book updated: ID=%d, Name=%s, Department=%s", id, name, department);
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    snprintf(response, BUFFER_SIZE, "No book found with ID=%d", id);
                }
            } else {
                snprintf(response, BUFFER_SIZE, "Invalid UPDATE command format.");
            }
        } else {
            snprintf(response, BUFFER_SIZE, "Unknown command. Use ADD, GET, or UPDATE.");
        }

        // Send response to the client
        send(client_socket, response, strlen(response), 0);
    }

    closesocket(client_socket);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    // Accept and handle clients
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) != INVALID_SOCKET) {
        printf("Client connected\n");
        handle_client(client_socket);
    }

    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
    }

    // Clean up
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
