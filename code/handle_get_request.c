#include "common.h"

#define GETLOG 1

char* handle_get_request(const char *url) {
// initialize the JSON answer
    cJSON *json = cJSON_CreateObject();
    char *answerget;
    unsigned int tokens;
    char version[64]="";
    char resource[64]="";
    char schema[64]="";
    char table[64]="";
    char column[64]="";
    char value[64]="";
    char query[512]="";

// extracting tokens from url
    tokens = sscanf(url, "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]%[/]", version, resource, schema, table, column, value);

    cJSON_AddStringToObject(json, "status", "HANDLER");
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
#if GETLOG == 1
    cJSON_AddNumberToObject(json, "tokens", tokens);
#endif //GETLOG

// analyzing toeksn to match patterns and resources
// 4 possibilites :
// - tokens = 2 && resource in (ping, healthcheck, status)
// - tokens = 4 && resource = tables
// - tokens = 6 && resource = tables
// - bad request format
if (tokens == 2 && strcmp(resource, "ping") == 0 ) {
// ping : verify API responsiveness
      cJSON_AddStringToObject(json, "action", "PING");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
      cJSON_AddStringToObject(json, "message", "pong");

} else if (tokens == 2 && strcmp(resource, "healtcheck") == 0 ) {
// healthcheck : verify API full availability
      cJSON_AddStringToObject(json, "action", "HEALTCHECK");
      snprintf(query, sizeof(query),"select now() as NOW");

} else if (tokens == 2 && strcmp(resource, "status") == 0 ) {
// global status
      snprintf(query, sizeof(query), "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
      cJSON_AddStringToObject(json, "action", "STATUS");

} else if (tokens == 4 && strcmp(resource, "tables") == 0 ) {
// TODO : check request body existence & content
// building query
//      snprintf(query, sizeof(query), "SELECT * FROM %s.%s", schema, table);
      cJSON_AddStringToObject(json, "action", "QUERY");
      cJSON_AddStringToObject(json, "message", "Being developped");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);

} else if (tokens == 6 && strcmp(resource, "tables") == 0 ) {
// building query
      snprintf(query, sizeof(query), "SELECT * FROM %s.%s WHERE %s = '%s'", schema, table, column, value);
      cJSON_AddStringToObject(json, "action", "QUERY");

} else {
// bad url KO
      cJSON_AddStringToObject(json, "action", "ERROR");
      cJSON_AddStringToObject(json, "error", "Invalid GET request");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
}

// if query => connect,execute,getresult
if (strlen(query) > 0) {
// step 1 : connexion
cJSON_AddStringToObject(json, "step","cnx init");
}

// housekeeping

printf("after cnx");
    answerget = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
return answerget;
} // end function
