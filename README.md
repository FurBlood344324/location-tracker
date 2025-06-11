# Location Tracking System

Real-time web application that displays GPS coordinates from any device can send a POST request on an interactive map.

## 🚀 Features

- Real-time location tracking (Server-Sent Events)
- Interactive map with Leaflet.js
- Auto-follow mode
- Custom SVG icon support
- Mobile responsive design
- HTTPS support (Let's Encrypt)

## 📋 Requirements

- Node.js (v14 or higher)
- Nginx
- Certbot (for SSL certificate)
- Domain name (with DNS configured)

## 📁 Project Structure

```
location-tracker/
├── app/
│   ├── icon.svg        # Marker icon
│   ├── index.html      # Main map page
│   ├── package.json    # Node.js dependencies
│   ├── package-lock.json
│   └── server.js       # Express server
```

## 🛠️ Installation

### 1. Clone the Project

```bash
git clone https://github.com/metharda/location-tracker
cd location-tracker
```

### 2. Install Dependencies

```bash
npm install
```

### 3. Start Application with Express

```bash
# Install Express (if not installed)
npm install express

# Start application
npm run start
```

### 4. Nginx Configuration

Create `/etc/nginx/sites-available/location-tracker`:

```nginx
server {
    listen 80;
    server_name yourdomain.com;
    
    location / {
        proxy_pass http://localhost:3000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
    
    location /events {
        proxy_pass http://localhost:3000/events;
        proxy_http_version 1.1;
        proxy_set_header Connection '';
        proxy_set_header Cache-Control 'no-cache';
        proxy_set_header X-Accel-Buffering 'no';
        proxy_buffering off;
        proxy_cache off;
        proxy_read_timeout 86400s;
        keepalive_timeout 86400s;
    }
}
```

Enable site:
```bash
sudo ln -s /etc/nginx/sites-available/location-tracker /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### 5. SSL Certificate (Certbot)

```bash
# Install Certbot
sudo apt update
sudo apt install certbot python3-certbot-nginx

# Get SSL certificate
sudo certbot --nginx -d yourdomain.com

# Test automatic renewal
sudo certbot renew --dry-run
```

## 🧪 Testing

### Send Test Location with cURL

```bash
# Send location via POST
curl -X POST https://yourdomain.com/location \
  -H "Content-Type: application/json" \
  -d '{"lat":41.0082,"lng":28.9784}'

# Multiple test locations
curl -X POST https://yourdomain.com/location \
  -H "Content-Type: application/json" \
  -d '{"lat":40.9918,"lng":29.0237}'

curl -X POST https://yourdomain.com/location \
  -H "Content-Type: application/json" \
  -d '{"lat":41.0429,"lng":29.0069}'

```

## 📄 License
This project is licensed under the MIT License.