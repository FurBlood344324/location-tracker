const express = require('express');
const path = require('path');
const app = express();
const PORT = 3000;

app.set('trust proxy', true);
app.use(express.json());
app.use(express.static(__dirname));

let lastLocation = null;
let locations = [];
const MAX_LOCATIONS = 100;

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'app/index.html'));
});

app.post('/location', (req, res) => {
    const { lat, lng } = req.body;
    
    if (!lat || !lng) {
        return res.status(400).json({ error: 'lat ve lng gerekli' });
    }

    
    const location = {
        lat: parseFloat(lat),
        lng: parseFloat(lng),
        timestamp: new Date().toISOString()
    };
    
    lastLocation = location;
    locations.push(location);

    
    if (locations.length > MAX_LOCATIONS) {
        locations.shift();
    }
    
    console.log(`Yeni konum: ${lat}, ${lng}`);
    
    res.json({ 
        status: 'success', 
        message: 'Konum güncellendi',
        location: location 
    });
});

app.get('/location', (req, res) => {
    res.json({
        current: lastLocation,
        history: locations
    });
});

app.delete('/locations', (req, res) => {
    locations = [];
    lastLocation = null;
    res.json({ status: 'success', message: 'Konumlar temizlendi' });
});

app.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.setHeader('Access-Control-Allow-Origin', '*');

    
    if (lastLocation) {
        res.write(`data: ${JSON.stringify(lastLocation)}\n\n`);
    }

    
    const sendLocation = setInterval(() => {
        if (lastLocation) {
            res.write(`data: ${JSON.stringify(lastLocation)}\n\n`);
        }
    }, 1000);

    
    req.on('close', () => {
        clearInterval(sendLocation);
    });
});

app.listen(PORT, () => {
    console.log(`Sunucu http://localhost:${PORT} adresinde çalışıyor`);
    console.log('\nTest komutları:');
    console.log('POST: curl -X POST http://localhost:3000/location -H "Content-Type: application/json" -d \'{"lat":41.0082,"lng":28.9784}\'');
    console.log('GET:  curl http://localhost:3000/location');
});
