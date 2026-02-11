#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define CONF_FILE "/etc/sftp-toggle-root-safe.conf"

char target[256] = {0};
char user[64] = {0};
char allowed_ips[256] = {0};

int check_ip_allowed() {
    char ip[128] = {0};
    FILE *f = popen("hostname -I | awk '{print $1}'", "r");
    if (!f) return 0;
    if (!fgets(ip, sizeof(ip), f)) { pclose(f); return 0; }
    ip[strcspn(ip, "\n")] = 0;
    pclose(f);

    char ips[256];
    strncpy(ips, allowed_ips, sizeof(ips)-1);
    ips[sizeof(ips)-1] = 0;

    char *token = strtok(ips, ",");
    while(token) {
        if(strcmp(token, ip)==0) return 1;
        token = strtok(NULL, ",");
    }
    return 0;
}

void load_config() {
    FILE *f = fopen(CONF_FILE, "r");
    if(!f) { perror("Failed to open config file"); exit(1); }

    char line[256];
    while(fgets(line, sizeof(line), f)) {
        line[strcspn(line,"\n")] = 0;
        if(strncmp(line,"TARGET=",7)==0) strncpy(target,line+7,sizeof(target)-1);
        else if(strncmp(line,"USER=",5)==0) strncpy(user,line+5,sizeof(user)-1);
        else if(strncmp(line,"ALLOWED_IPS=",12)==0) strncpy(allowed_ips,line+12,sizeof(allowed_ips)-1);
    }
    fclose(f);

    if(target[0]!='/') { fprintf(stderr,"Invalid TARGET\n"); exit(1); }
    if(strcmp(user,"root")!=0) { fprintf(stderr,"USER must be root\n"); exit(1); }
}

int main(int argc, char *argv[]) {
    openlog("sftp-toggle-root-safe", LOG_PID|LOG_CONS, LOG_DAEMON);

    if(argc != 2) {
        printf("Usage: %s [on|off]\n", argv[0]);
        return 1;
    }

    load_config();

    if(!check_ip_allowed()) {
        syslog(LOG_ERR,"SFTP root toggle blocked: IP not in whitelist");
        printf("Your IP is not allowed to toggle root SFTP.\n");
        return 1;
    }

    char cmd[512] = {0};

    if(strcmp(argv[1],"on")==0) {
        snprintf(cmd,sizeof(cmd),
                 "setfacl -R -m u:%s:rwx %s && setfacl -R -d -m u:%s:rwx %s",
                 user,target,user,target);
        if(system(cmd)==0) {
            syslog(LOG_INFO,"Root SFTP ENABLED (safe mode) in %s",target);
            printf("[✓] Root SFTP ENABLED (safe mode) in %s\n",target);
        } else {
            syslog(LOG_ERR,"Failed to ENABLE root SFTP (safe mode)");
            printf("[✗] Failed to enable root SFTP\n");
            return 1;
        }
    } else if(strcmp(argv[1],"off")==0) {
        snprintf(cmd,sizeof(cmd),
                 "setfacl -R -x u:%s %s && setfacl -R -k %s",
                 user,target,target);
        if(system(cmd)==0) {
            syslog(LOG_INFO,"Root SFTP DISABLED (safe mode) in %s",target);
            printf("[✓] Root SFTP DISABLED (safe mode) in %s\n",target);
        } else {
            syslog(LOG_ERR,"Failed to DISABLE root SFTP (safe mode)");
            printf("[✗] Failed to disable root SFTP\n");
            return 1;
        }
    } else {
        printf("Invalid command. Use 'on' or 'off'.\n");
        return 1;
    }

    closelog();
    return 0;
}
