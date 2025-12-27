#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif
#include "game.h"

#define PORT 8080
#define BUF_SIZE 8192
#define FILE_SIZE 65536

static Game game;
static int sock = -1;

static int read_file(const char* path, char* buf, int size) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    int n = (int)fread(buf, 1, size - 1, f);
    buf[n] = 0;
    fclose(f);
    return n;
}

static void send_resp(int client, const char* status, const char* type, const char* body) {
    char resp[BUF_SIZE];
    int len = snprintf(resp, sizeof(resp),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n%s",
        status, type, (int)strlen(body), body
    );
    send(client, resp, len, 0);
}

static void parse_query(const char* q, int* mode, int* init, int* tgt, int* act, int* val) {
    char* copy = strdup(q);
    char* tok = strtok(copy, "&");
    while (tok) {
        if (strncmp(tok, "mode=", 5) == 0) *mode = atoi(tok + 5);
        else if (strncmp(tok, "initial=", 8) == 0) *init = atoi(tok + 8);
        else if (strncmp(tok, "target=", 7) == 0) *tgt = atoi(tok + 7);
        else if (strncmp(tok, "action=", 7) == 0) *act = atoi(tok + 7);
        else if (strncmp(tok, "value=", 6) == 0) *val = atoi(tok + 6);
        tok = strtok(NULL, "&");
    }
    free(copy);
}

static void handle(int client, const char* req) {
    char method[16], path[256];
    sscanf(req, "%s %s", method, path);
    
    char* q = strchr(path, '?');
    if (q) *q++ = 0;
    
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        char html[FILE_SIZE];
        if (read_file("public/index.html", html, sizeof(html)) ||
            read_file("../public/index.html", html, sizeof(html))) {
            send_resp(client, "200 OK", "text/html", html);
        } else {
            send_resp(client, "404", "text/plain", "Not found");
        }
    } else if (strcmp(path, "/style.css") == 0) {
        char css[FILE_SIZE];
        if (read_file("public/style.css", css, sizeof(css)) ||
            read_file("../public/style.css", css, sizeof(css))) {
            send_resp(client, "200 OK", "text/css", css);
        } else {
            send_resp(client, "404", "text/plain", "Not found");
        }
    } else if (strcmp(path, "/script.js") == 0) {
        char js[FILE_SIZE];
        if (read_file("public/script.js", js, sizeof(js)) ||
            read_file("../public/script.js", js, sizeof(js))) {
            send_resp(client, "200 OK", "application/javascript", js);
        } else {
            send_resp(client, "404", "text/plain", "Not found");
        }
    } else if (strcmp(path, "/api/state") == 0) {
        char json[256];
        game_json(&game, json, sizeof(json));
        send_resp(client, "200 OK", "application/json", json);
    } else if (strcmp(path, "/api/new") == 0) {
        int mode = 0, init = 10, tgt = 50;
        if (q) parse_query(q, &mode, &init, &tgt, NULL, NULL);
        game_new(&game, (GameMode)mode, init, tgt);
        char json[256];
        game_json(&game, json, sizeof(json));
        send_resp(client, "200 OK", "application/json", json);
    } else if (strcmp(path, "/api/move") == 0) {
        int act = 0, val = 0;
        if (q) parse_query(q, NULL, NULL, NULL, &act, &val);
        
        if (act == 0 && game.state.mode == VS_COMPUTER && 
            game.state.current == PLAYER2 && !game.state.over) {
            game_computer_move(&game);
        } else if (act > 0) {
            game_move(&game, act, val);
            if (game.state.mode == VS_COMPUTER && game.state.current == PLAYER2 && !game.state.over) {
                game_computer_move(&game);
            }
        }
        
        char json[256];
        game_json(&game, json, sizeof(json));
        send_resp(client, "200 OK", "application/json", json);
    } else {
        send_resp(client, "404", "text/plain", "Not found");
    }
}

int main() {
    game_init(&game);
    
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 5);
    
    printf("Server: http://localhost:%d\n", PORT);
    
    while (1) {
        struct sockaddr_in client_addr;
#ifdef _WIN32
        int len = sizeof(client_addr);
#else
        socklen_t len = sizeof(client_addr);
#endif
        int client = accept(sock, (struct sockaddr*)&client_addr, &len);
        if (client < 0) continue;
        
        char buf[BUF_SIZE];
        int n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = 0;
            handle(client, buf);
        }
        
#ifdef _WIN32
        closesocket(client);
#else
        close(client);
#endif
    }
    
    return 0;
}

