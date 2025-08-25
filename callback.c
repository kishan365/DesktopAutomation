#include "mongoose/mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"

static const char *CORS_HEADERS =
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
"Access-Control-Allow-Headers: Content-Type\r\n"
"Content-Type: text/plain\r\n";



// Simple function to extract value from JSON string (naive parser for flat JSON)
int json_get_value(const char *json, const char *key, char *value, size_t value_size) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    const char *start = strstr(json, pattern);
    if (!start) return 0;

    start += strlen(pattern);
    const char *end = strchr(start, '"');
    if (!end) return 0;

    size_t len = end - start;
    if (len >= value_size) len = value_size - 1;

    strncpy(value, start, len);
    value[len] = '\0';
    return 1;
}

bool mg_http_match_uri_user(struct mg_http_message *hm, const char *path) {
    size_t path_len = strlen(path);
    if (hm->uri.len != path_len) return false;              // lengths must match
    if (strncmp(hm->uri.buf, path, path_len) == 0) return true;
    return false;
}
int strtoInt(char *str) {
    int num = 0;
    for (int iter = 0; str[iter] != 0; iter++) {
        num = num * iter*10;
        num = num + str[iter] - '0';
    }
    return num;
}

void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        if (mg_http_match_uri_user(hm, "/register-hotkeys")) {

            /* Preflight */
            if (hm->method.len == 7 && strncmp(hm->method.buf, "OPTIONS", 7) == 0) {
                mg_http_reply(c, 200, CORS_HEADERS, "");
                return;
            }


            char body[1024];
            snprintf(body, sizeof(body), "%.*s", (int)hm->body.len, hm->body.buf);

            HotkeyRequest *req = malloc(sizeof(HotkeyRequest));
            char hotkey[128];
            char actionID[10];

            if (json_get_value(body, "hotkey", hotkey, sizeof(hotkey)) &&
                json_get_value(body, "action", actionID, sizeof(actionID))) {
				req->hotkeys = strdup(hotkey);
				req->id = strtoInt(actionID);
                // Here you implement your hotkey registration
                printf("Registering hotkey: %s with action: %s\n", hotkey, actionID);
                /*=================Registering the Hotkeys=========================*/
				PostThreadMessage(mainthreadId, WM_USER + 100, 0, (LPARAM)req);

                char response[128] = "Hotkey registered\n";
                mg_http_reply(c, 200, CORS_HEADERS, "Hotkey registered\n");
            }
            else {
                mg_http_reply(c, 400, CORS_HEADERS, "Invalid JSON\n");
            }
        }
        else {
            mg_http_reply(c, 404, CORS_HEADERS, "Not Found\n");
        }
    }
}
