#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>

#define CONF_FILE "/etc/sftp-toggle.conf"

char target[256] = {0};
char user[64] = {0};

int is_valid_user(const char *u) {
    for (int i = 0; u[i]; i++) {
        if (!isalnum(u[i]) && u[i] != '-' && u[i] != '_') return 0;
    }
    return 1;
}

int is_valid_target(const char *t) {
    if (t[0] != '/') return 0; 
    if (strchr(t, ';') || strchr(t, '&') || strchr(t, '|')) return 0;
    return 1;
}

void load_config() {
    FILE *f = fopen(CONF_FILE, "r");
    if (!f) {
        perror("Failed to open config file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0; // remove newline

        if (strncmp(line, "TARGET=", 7) == 0) {
            strncpy(target, line+7, sizeof(target)-1);
            target[sizeof(target)-1] = 0;
        } else if (strncmp(line, "USER=", 5) == 0) {
            strncpy(user, line+5, sizeof(user)-1);
            user[sizeof(user)-1] = 0;
        }
    }
    fclose(f);

    if (!is_valid_user(user) || !is_valid_target(target)) {
        fprintf(stderr, "Invalid user or target in config\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    openlog("sftp-toggle", LOG_PID|LOG_CONS, LOG_DAEMON);

    if (argc != 2) {
        printf("Usage: %s [on|off]\n", argv[0]);
        return 1;
    }

    load_config();

    char cmd[512] = {0};

    if (strcmp(argv[1], "on") == 0) {
        snprintf(cmd, sizeof(cmd),
                 "setfacl -R -m u:%s:rwx %s && setfacl -R -d -m u:%s:rwx %s",
                 user, target, user, target);
        if (system(cmd) == 0) {
            syslog(LOG_INFO, "SFTP access ENABLED for %s", user);
            printf("[✓] SFTP access ENABLED for %s\n", user);
        } else {
            syslog(LOG_ERR, "Failed to ENABLE SFTP for %s", user);
            printf("[✗] Failed to enable SFTP for %s\n", user);
            return 1;
        }
    } else if (strcmp(argv[1], "off") == 0) {
        snprintf(cmd, sizeof(cmd),
                 "setfacl -R -x u:%s %s && setfacl -R -k %s",
                 user, target, target);
        if (system(cmd) == 0) {
            syslog(LOG_INFO, "SFTP access DISABLED for %s", user);
            printf("[✓] SFTP access DISABLED for %s\n", user);
        } else {
            syslog(LOG_ERR, "Failed to DISABLE SFTP for %s", user);
            printf("[✗] Failed to disable SFTP for %s\n", user);
            return 1;
        }
    } else {
        printf("Invalid command. Use 'on' or 'off'.\n");
        return 1;
    }

    closelog();
    return 0;
}
