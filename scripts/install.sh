#!/usr/bin/bash

make

sudo cp bin/milter-notify /usr/local
sudo cp milter_notify.service /etc/systemd/system/milter_notify.service

sudo mkdir /etc/milter-notify
sudo chown postfix:postfix /etc/milter-notify
sudo chmod 755 /etc/milter-notify

sudo echo "SECRET_KEY" > /etc/milter-notify/ping_data
sudo echo "0.0.0.0" > /etc/milter-notify/ping_url

sudo chmod a+r /etc/milter-notify/ping_data
sudo chmod a+r /etc/milter-notify/ping_url

sudo chown postfix:postfix /etc/milter-notify/ping_data
sudo chown postfix:postifx /etc/milter-notify/ping_url
