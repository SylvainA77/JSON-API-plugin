#include "common.h"

#define LOG 1

char* handle_get_request(const char *url) {
// initialize the JSON answer
    cJSON *json = cJSON_CreateObject();
    char *json_string;
#if LOG  == 1
  FILE *log = fopen("/var/lib/mysql/json2sql.log", "a");
  fwrite("INIT:",sizeof(char),5,log);
  fwrite(url, sizeof(char), sizeof(url), log);
#endif // end LOG
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
#if GETCORK == 1
    cJSON_AddStringToObject(json, "status", "GETCORK");
    cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
    cJSON_AddNumberToObject(json, "action", "CORK");
#else
    cJSON_AddStringToObject(json, "status", "HANDLER");
// initializing the variables
    char query[512]="";
    char schema[64];
    char table[64];
    char colname[64];
    char colvalue[64];
    unsigned int bypass=0;
// check request format, parameter extraction & statement exec
  if (sscanf(url, "/v1/tables/%64[^/]/%64[^/]/%64[^/]/%64s", schema, table, colname, colvalue) == 4) {
// building query
      snprintf(query, sizeof(query), "SELECT * FROM %s.%s WHERE %s = '%s'", schema, table, colname, colvalue);
      cJSON_AddStringToObject(json, "action", "QUERY");
    } else if (strcmp(url, "/v1/status") == 0) {
// global status
      snprintf(query, sizeof(query), "SHOW GLOBAL STATUS");
      cJSON_AddStringToObject(json, "action", "STATUS");
    } else if (strcmp(url, "/v1") == 0) {
// healthcheck
      cJSON_AddStringToObject(json, "action", "HEALTCHECK");
      snprintf(query, sizeof(query),"select now() as NOW");
    } else if (strcmp(url, "/v1/ping")) {
// ping
      bypass=1;
      cJSON_AddStringToObject(json, "action", "PING");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
      cJSON_AddStringToObject(json, "message", "pong");
   } else {
// bad url KO
      bypass=1;
      cJSON_AddStringToObject(json, "action", "ERROR");
      cJSON_AddStringToObject(json, "error", "Invalid GET request");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
#if LOG == 1
  fwrite("ERROR\n",sizeof(char),6,log);
#endif  // end LOG
  } // end if sscanf
// we establish internal local connexion
   if (bypass == 0) {
#if LOG == 1
  fwrite("CNX\n",sizeof(char), 4, log);
#endif  // end LOG
        MYSQL *connection = mysql_init(NULL);
        if (mysql_real_connect_local(connection) == NULL) {
#if LOG == 1
          fwrite("CNX FAILED\n",sizeof(char), 11,log);
#endif  // end LOG
          cJSON_AddStringToObject(json, "connexionstatus", "FAILED");
          cJSON_AddNumberToObject(json, "mariadbcode", mysql_errno(connection));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        } else if (mysql_real_query(connection, STRING_WITH_LEN(query))) {
// executing the query
#if LOG == 1
  fwrite("QUERY FAILED\n",sizeof(char),12,log);
#endif // end LOG
          cJSON_AddStringToObject(json, "connexionstatus", "OK");
          cJSON_AddStringToObject(json, "querystatus", "FAILED");
          cJSON_AddNumberToObject(json, "mariadbcode",mysql_errno(connection));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
// housekeeping
          mysql_close(connection);
        } else { // else if connect
// check resulset existence
#if LOG == 1
  fwrite("RESULT\n",sizeof(char),7,log);
#endif // end LOG
          cJSON_AddStringToObject(json, "connexionstatus", "OK");
          cJSON_AddStringToObject(json, "querystatus", "OK");
          cJSON_AddNumberToObject(json, "mariadbcode",mysql_errno(connection));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
          MYSQL_RES *resultset = mysql_store_result(connection);
          if (resultset == NULL) {
#if LOG == 1
  fwrite("EMPTY\n",sizeof(char),6,log);
#endif // end LOG
// either no data or error
             cJSON_AddStringToObject(json, "status", "NO DATA FOUND");
             cJSON_AddNumberToObject(json, "rows", 0);
             cJSON_AddNumberToObject(json, "mariadbcode",mysql_errno(connection));
             cJSON_AddStringToObject(json, "data", "");
// housekeeping
          } else { // else if resultset
// resulset to json translation
#if LOG == 1
  fwrite("NOT EMPTY\n",sizeof(char),10,log);
#endif // end LOG
             int num_rows = (int)mysql_num_rows(resultset);
             cJSON_AddNumberToObject(json, "rows", num_rows);
             cJSON_AddStringToObject(json, "status", "DATA FOUND");
             cJSON_AddNumberToObject(json, "mariadbcode", 0);
#if ARRAYCORK == 1
             cJSON_AddStringToObject(json, "data", "");
#if LOG == 1
  fwrite("ARRAYCORK\n",sizeof(char),10,log);
#endif // end LOG
#else
// Create a JSON array to hold result rows
             cJSON *rows_array = cJSON_CreateArray();
             unsigned int num_fields = mysql_num_fields(resultset);
             MYSQL_FIELD *fields = mysql_fetch_fields(resultset);
             MYSQL_ROW row;
#if LOG == 1
  fwrite("ARRAY\n",sizeof(char),6,log);
#endif // end LOG
             while ((row = mysql_fetch_row(resultset))) {
// Create a JSON object for each row
                   cJSON *row_object = cJSON_CreateObject();
// handling of rows in the following format : "columnname":"columnvalue"
                    for (unsigned int i = 0; i < num_fields; i++) {
                        cJSON_AddStringToObject(row_object, fields[i].name, row[i] ? row[i] : "");
                    } // end for
 // Add the row object to the array
                    cJSON_AddItemToArray(rows_array, row_object);
              } // end while
              cJSON_AddItemToObject(json, "data", rows_array);
#endif // ARRAYCORK
     } // end if resultset
// housekeeping
          mysql_free_result(resultset);
          mysql_close(connection);
     } // end if connect
     } // end if bypass
#endif // GETCORK
#if LOG == 1
  fwrite("END\n",sizeof(char),10,log);
#endif // end LOG
  json_string = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);
#if LOG == 1
  fwrite(json_string,sizeof(char),sizeof(json_string),log);
  fclose(log);
#endif // end LOG
  return json_string; // Caller is responsible for freeing this memory
} // end function
