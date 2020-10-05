#!/usr/bin/bash

make

sudo cp bin/milter-notify /usr/local
sudo cp milter_notify.service /etc/systemd/system/milter_notify.service

sudo mkdir /etc/milter-notify
sudo chown postfix:mail /etc/milter-notify
sudo chmod a+r /etc/milter-notify

echo 'SECRET_KEY' | sudo tee -a /etc/milter-notify/ping_data
echo '0.0.0.0' | sudo tee -a /etc/milter-notify/ping_url

sudo chmod a+r /etc/milter-notify/ping_data
sudo chmod a+r /etc/milter-notify/ping_url

sudo chown postfix:mail /etc/milter-notify/ping_data
sudo chown postfix:mail /etc/milter-notify/ping_url

echo "Everything done. Dont forget to change files:"
echo -e "\t/etc/milter-notify/ping_data"
echo -e "\t/etc/milter-notify/ping_url"

