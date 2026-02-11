SFTP ON / OFF Access Toggle for `/path/html/`

This document describes a **temporary and controlled SFTP permission escalation**
mechanism using POSIX ACLs (`setfacl`).

The goal is to allow **full SFTP access only while working**, then revoke it
cleanly to reduce attack surface and human error.

---

## Overview

- No root login via SFTP
- No permanent permission changes
- No `chmod 777`, no ownership changes
- Fully reversible
- Designed for hardened Linux servers

Target Configuration

**Target directory**

- /path/html

**SFTP user**

- name

## Script: `sftp-on.sh`

- Enables recursive write access and ensures new files inherit permissions.

```
#!/bin/bash

TARGET="/path/html"
USER="name"

echo "[+] Enabling SFTP write access for $USER on $TARGET"

setfacl -R -m u:$USER:rwx "$TARGET"
setfacl -R -d -m u:$USER:rwx "$TARGET"

echo "[✓] SFTP write access ENABLED"
```

## Script: `sftp-off.sh`

- Removes all ACL permissions added by `sftp-on.sh`.

```
#!/bin/bash

TARGET="/path/html"
USER="name"

echo "[-] Disabling SFTP write access for $USER on $TARGET"

setfacl -R -x u:$USER "$TARGET"
setfacl -R -k "$TARGET"

echo "[✓] SFTP write access DISABLED"
```

## Installation

- Place the scripts in `/usr/local/bin`:

```
sudo nano /usr/local/bin/sftp-on.sh
sudo nano /usr/local/bin/sftp-off.sh
```

- Set permissions (run once):

```
chmod 750 /usr/local/bin/sftp-on.sh /usr/local/bin/sftp-off.sh
chown root:root /usr/local/bin/sftp-on.sh /usr/local/bin/sftp-off.sh
```

## Usage

- Enable access:

```
sudo sftp-on.sh
```

- Disable access:

```
sudo sftp-off.sh
```

- Verify ACL:

```
getfacl /home/hitsukaya | head
```

Security Notes

- File contents are never modified
- Ownership and classic permissions remain unchanged
- SELinux contexts are not affected
- ACL changes are fully reversible
- Access is limited strictly to the target directory

---

COMPILE SOURCE C

gcc src/main.c -o bin/sftp-toggle

sudo cp bin/sftp-toggle /usr/local/bin/

sudo cp conf/sftp-toggle.conf /etc/

sudo cp systemd/sftp-toggle.service /etc/systemd/system/

sudo systemctl daemon-reload
