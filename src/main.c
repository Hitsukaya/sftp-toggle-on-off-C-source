#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONF_FILE "/etc/sftp-toggle.conf"

void load_config(char *target, size_t tlen, char *user, size_t ulen) {
    FILE *f = fopen(CONF_FILE, "r");
    if (!f) {
        perror("Failed to open config file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0; // remove newline

        if (strncmp(line, "TARGET=", 7) == 0) {
            strncpy(target, line+7, tlen-1);
            target[tlen-1] = 0;
        } else if (strncmp(line, "USER=", 5) == 0) {
            strncpy(user, line+5, ulen-1);
            user[ulen-1] = 0;
        }
    }

    fclose(f);
}

int main() {
    char input[16];
    char target[256] = {0};
    char user[64] = {0};
    char cmd[512] = {0};

    load_config(target, sizeof(target), user, sizeof(user));

    printf("SFTP toggle shell: type 'on' or 'off'\n> ");
    if (!fgets(input, sizeof(input), stdin)) return 1;
    input[strcspn(input, "\n")] = 0;

    if (strcmp(input, "on") == 0) {
        snprintf(cmd, sizeof(cmd),
                 "setfacl -R -m u:%s:rwx %s && setfacl -R -d -m u:%s:rwx %s",
                 user, target, user, target);
        system(cmd);
        printf("[✓] SFTP access ENABLED for %s\n", user);
    } else if (strcmp(input, "off") == 0) {
        snprintf(cmd, sizeof(cmd),
                 "setfacl -R -x u:%s %s && setfacl -R -k %s",
                 user, target, target);
        system(cmd);
        printf("[✓] SFTP access DISABLED for %s\n", user);
    } else {
        printf("Invalid command. Use 'on' or 'off'.\n");
    }

    return 0;
}
