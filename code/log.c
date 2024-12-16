#include <stdio.h>
#include <time.h>
#include <string.h>

#define LOG_FILE_NAME "/app/log/json2sql.log"

void log_message(const char *message) {
    FILE *log_file;
    time_t now;
    char time_buffer[26];

    // Open the log file in append mode
    log_file = fopen(LOG_FILE_NAME, "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    // Get current time
    time(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Write the message to the log file
    fprintf(log_file, "[%s] %s\n", time_buffer, message);

    // Close the log file
    fclose(log_file);
}
