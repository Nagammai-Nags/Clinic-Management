#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 8192

// Simple function to serve static files
void serve_file(SOCKET client_socket, const char *path, const char *content_type) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, not_found, strlen(not_found), 0);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send header
    char header[512];
    sprintf(header, "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n\r\n", content_type, fsize);
    send(client_socket, header, (int)strlen(header), 0);

    // Send content
    char *buffer = malloc(fsize);
    fread(buffer, 1, fsize, file);
    send(client_socket, buffer, fsize, 0);

    free(buffer);
    fclose(file);
}

// Very primitive JSON value extractor
void get_json_value(const char *json, const char *key, char *output) {
    char search_key[128];
    sprintf(search_key, "\"%s\":\"", key);
    char *start = strstr(json, search_key);
    if (start) {
        start += strlen(search_key);
        char *end = strchr(start, '\"');
        if (end) {
            size_t len = end - start;
            strncpy(output, start, len);
            output[len] = '\0';
        }
    }
}

void handle_api_auth(SOCKET client_socket, const char *payload) {
    char action[32] = {0}, email[128] = {0}, password[128] = {0}, clinic_name[128] = {0}, staff_id[128] = {0};
    
    get_json_value(payload, "action", action);
    get_json_value(payload, "email", email);
    get_json_value(payload, "password", password);
    get_json_value(payload, "clinic_name", clinic_name);
    get_json_value(payload, "staff_id", staff_id);

    printf("Auth Request: %s for %s\n", action, (strlen(email) > 0 ? email : staff_id));

    char response_body[512];
    int success = 0;

    if (strcmp(action, "signup") == 0) {
        FILE *f = fopen("backend/owners.csv", "a");
        if (f) {
            fprintf(f, "%s,%s,%s\n", email, password, clinic_name);
            fclose(f);
            success = 1;
            sprintf(response_body, "{\"success\":true,\"message\":\"Clinic '%s' registered successfully!\"}", clinic_name);
        } else {
            sprintf(response_body, "{\"success\":false,\"message\":\"Server Error: Storage full.\"}");
        }
    } else if (strcmp(action, "owner_login") == 0) {
        FILE *f = fopen("backend/owners.csv", "r");
        char line[256];
        while (f && fgets(line, sizeof(line), f)) {
            char f_email[128], f_pass[128], f_clinic[128];
            if (sscanf(line, "%[^,],%[^,],%[^\n]", f_email, f_pass, f_clinic) >= 2) {
                if (strcmp(f_email, email) == 0 && strcmp(f_pass, password) == 0) {
                    success = 1;
                    sprintf(response_body, "{\"success\":true,\"message\":\"Welcome back, %s! Dashboard loading...\"}", f_clinic);
                    break;
                }
            }
        }
        if (f) fclose(f);
        if (!success) sprintf(response_body, "{\"success\":false,\"message\":\"Invalid credentials.\"}");
    } else if (strcmp(action, "staff_login") == 0) {
        FILE *f = fopen("backend/staff.csv", "r");
        char line[256];
        while (f && fgets(line, sizeof(line), f)) {
            char f_id[128], f_pass[128], f_role[128], f_owner[128];
            if (sscanf(line, "%[^,],%[^,],%[^,],%[^\n]", f_id, f_pass, f_role, f_owner) >= 2) {
                if (strcmp(f_id, staff_id) == 0 && strcmp(f_pass, password) == 0) {
                    success = 1;
                    sprintf(response_body, "{\"success\":true,\"message\":\"Logged in as %s (%s).\"}", f_id, f_role);
                    break;
                }
            }
        }
        if (f) fclose(f);
        if (!success) sprintf(response_body, "{\"success\":false,\"message\":\"Staff ID or Password incorrect.\"}");
    }

    char header[512];
    sprintf(header, "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %d\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n\r\n", (int)strlen(response_body));
    send(client_socket, header, (int)strlen(header), 0);
    send(client_socket, response_body, (int)strlen(response_body), 0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }

    listen(server_socket, 3);
    printf("Server started at http://localhost:%d\nWaiting for connections...\n", PORT);

    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        char buffer[BUFFER_SIZE] = {0};
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        // Simple Request Parsing
        if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index.html", 15) == 0) {
            serve_file(client_socket, "index.html", "text/html");
        } 
        else if (strncmp(buffer, "GET /assets/css/style.css", 25) == 0) {
            serve_file(client_socket, "assets/css/style.css", "text/css");
        }
        else if (strncmp(buffer, "GET /assets/js/auth.js", 22) == 0) {
            serve_file(client_socket, "assets/js/auth.js", "application/javascript");
        }
        else if (strncmp(buffer, "POST /auth", 10) == 0) {
            char *payload = strstr(buffer, "\r\n\r\n");
            if (payload) {
                payload += 4;
                handle_api_auth(client_socket, payload);
            }
        }
        else {
            char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(client_socket, not_found, strlen(not_found), 0);
        }

        closesocket(client_socket);
    }

    WSACleanup();
    return 0;
}
