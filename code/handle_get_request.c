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
    char object[64]="";
    char column[64]="";
    char value[64]="";
    char query[512]="";
    unsigned int decision = 1;
// 2 = exec not needed
// 1 = exec needed
// 0 = exec not possible
    
// extracting tokens from url
    tokens = sscanf(url, "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]", version, resource, schema, object, column, value);
// end token extraction
    
    cJSON_AddStringToObject(json, "status", "HANDLER");
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
#if GETLOG == 1
    cJSON_AddNumberToObject(json, "tokens",   tokens);
    cJSON_AddStringToObject(json, "version",  version);
    cJSON_AddStringToObject(json, "resource", resource);
    cJSON_AddStringToObject(json, "schema",   schema);
    cJSON_AddStringToObject(json, "table",    object);
    cJSON_AddStringToObject(json, "column",   column);
    cJSON_AddStringToObject(json, "value",    value); 
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
// end ping
} else if (tokens == 2 && strcmp(resource, "healtcheck") == 0 ) {
// healthcheck : verify API full availability
      cJSON_AddStringToObject(json, "action", "HEALTCHECK");
      size_t query_length = snprintf(NULL, 0, "select now() as NOW");
      query = malloc(query_length + 1);
      snprintf(query, query_length + 1,"select now() as NOW");
// end healthcheck
} else if (tokens == 2 && strcmp(resource, "status") == 0 ) {
// global status
      cJSON_AddStringToObject(json, "action", "STATUS");
      size_t query_length = snprintf(NULL, 0, "SELECT  * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
      query = malloc(query_length + 1);
      snprintf(query, query_length + 1, "SELECT  * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
/ end status
} else if (tokens == 4 && strcmp(resource, "tables") == 0 ) {
// TODO : check request body existence & content
//      size_t query_length = snprintf(NULL, 0, "SELECT * FROM %s.%s WHERE %s = '%s'", URL[2],URL[3],URL[4],URL[5]);
//      query = malloc(query_length + 1);
//      snprintf(query, query_length + 1, "SELECT * FROM %s.%s WHERE %s = '%s'", URL[2],URL[3],URL[4],URL[5]);
      cJSON_AddStringToObject(json, "action",   "QUERY");
      cJSON_AddStringToObject(json, "message",  "Being developped");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
// end query(body)
} else if (tokens == 6 && strcmp(resource, "tables") == 0 ) {
// simple query
      cJSON_AddStringToObject(json, "action", "QUERY");
      size_t query_length = snprintf(NULL, 0, "SELECT * FROM %s.%s WHERE %s = '%s'", schema, object, column, value);
      query = malloc(query_length + 1);
      snprintf(query, query_length + 1, "SELECT * FROM %s.%s WHERE %s = '%s'", schema, object, column, value);
// end simple query
} else {
// bad url KO
      cJSON_AddStringToObject(json, "action", "ERROR");
      cJSON_AddStringToObject(json, "error", "Invalid GET request");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
// end bad url
}
// end decision making


// housekeeping
    printf("after cnx");
    answerget = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    free(query);
//end housekeeping

return answerget;
} // end function
