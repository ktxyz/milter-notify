#!/usr/bin/bash

sudo rm -rf /usr/local/milter-notify
sudo systemctl stop milter_notify

sudo rm -rf /etc/systemd/system/milter_notify.service
sudo rm -rf /etc/milter-notify
