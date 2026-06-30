#!/bin/bash
set -e

sudo mkdir -p /opt/cigorn/CigornGateway-5.0.1
sudo cp ../cigorn ../cigorn.ini /opt/cigorn/CigornGateway-5.0.1/
sudo chmod +x /opt/cigorn/CigornGateway-5.0.1/cigorn

sudo cp ../systemd/cigorn.service /etc/systemd/system/cigorn.service
sudo systemctl daemon-reload
sudo systemctl enable cigorn.service

sudo ufw allow 2300/tcp || true
sudo ufw allow 25002/tcp || true
sudo ufw allow 8000/tcp || true
sudo ufw allow 8001/tcp || true

echo "Installed Cigorn Gateway 5.0.1"
echo "Start with: sudo systemctl start cigorn"
