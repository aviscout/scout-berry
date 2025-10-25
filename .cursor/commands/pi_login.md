# log in to the raspbery pi via ssh using this password: scout123
# ssh aviscout@192.168.1.84 -P scout123

```
#!/usr/bin/expect -f
set timeout 20
spawn ssh aviscout@192.168.1.84
expect "password:"
send "scout123\r"
expect "$ "
send "ls -la /home/aviscout/MVP\r"
expect "$ "
send "exit\r"
expect eof
```