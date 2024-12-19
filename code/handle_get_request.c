#include "common.h"

// this function aims at resolving the http GET requests upon various resources
// this fucntion should send back a json string in form of a char*
char* handle_get_request(const char *url) {
// initialize the JSON answer
    cJSON *json = cJSON_CreateObject();
    char *answerget;
    char version[64]="";
    char resource[64]="";
    char schema[64]="";
    char object[64]="";
    char column[64]="";
    char value[64]="";
    unsigned int action = 0;
// 2 = sql needed
// 1 = direct answere
// 0 = exec not possible

// extracting tokens from url
    int tokens = sscanf(url, "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]", version, resource, schema, object, column, value);
// end token extraction

    int isresourcent=0;
    if (strchr(resource, '\0') != NULL) {
// String is null-terminated
        isresourcent=1;
    }
    cJSON_AddStringToObject(json, "status", "HANDLER");
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
    cJSON_AddNumberToObject(json, "tokens", tokens);

// analyzing toeksn to match patterns and resources
// 4 possibilites :
// - tokens = 2 && resource in (ping, healthcheck, status)
// - tokens = 4 && resource = tables
// - tokens = 6 && resource = tables
// - bad request format
switch(tokens) {
    case 2:
      cJSON_AddStringToObject(json, "version", version);
      cJSON_AddStringToObject(json, "resource", resource);
      cJSON_AddNumberToObject(json, "strlen resource",strlen(resource));
      cJSON_AddNumberToObject(json, "sizeof resrouce",sizeof(resource));
      cJSON_AddNumberToObject(json, "strchr resource null is not null", isresourcent);
      if (strncasecmp(resource, "ping", 63) == 0 ) {
         cJSON_AddStringToObject(json, "action", "PING");
         cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
         cJSON_AddStringToObject(json, "message", "pong");
         action=1;
      } else if (strncasecmp(resource, "status", 63) == 0) {
         cJSON_AddStringToObject(json, "action", "STATUS");
         action=2;
      }  else if (strncasecmp(resource, "healthcheck",63) == 0 ) {
         cJSON_AddStringToObject(json, "action", "HEALTHCHECK");
         action=2;
      } else {
         cJSON_AddStringToObject(json, "action", "ERROR");
         cJSON_AddStringToObject(json, "error", "Invalid GET request");
         cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
         answerget = cJSON_PrintUnformatted(json);
         cJSON_Delete(json);
         return answerget;
      }
    break; // end tokens=2
    case 4:
      cJSON_AddStringToObject(json, "action",   "QUERY");
      cJSON_AddStringToObject(json, "message",  "developped later");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
    break; // end tokens=4
    case 6:
      cJSON_AddStringToObject(json, "action",   "QUERY");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
    break; // end tokens=6
    default:
      cJSON_AddStringToObject(json, "action", "ERROR");
      cJSON_AddStringToObject(json, "error", "Invalid GET request");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
      answerget = cJSON_PrintUnformatted(json);
      cJSON_Delete(json);
      return answerget;
}
// end decision making

// housekeeping
    answerget = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
//end housekeeping

return answerget;
} // end function
