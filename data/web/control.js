// Global state
let selectedDevice = null;
let devices = [];
let deviceAttributes = {}; // Store device width/height

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    loadDevices();
    setupColorPickers();
});

// Setup color picker synchronization
function setupColorPickers() {
    const colorInputs = [
        { color: 'R', hex: 'colorR', num: 'colorRNum' },
        { color: 'G', hex: 'colorG', num: 'colorGNum' },
        { color: 'B', hex: 'colorB', num: 'colorBNum' }
    ];

    colorInputs.forEach(({ hex, num }) => {
        const hexInput = document.getElementById(hex);
        const numInput = document.getElementById(num);

        hexInput.addEventListener('input', () => {
            const rgb = hexToRgb(hexInput.value);
            numInput.value = rgb[colorInputs.findIndex(c => c.hex === hex)];
        });

        numInput.addEventListener('input', () => {
            updateColorHex();
        });
    });
}

function hexToRgb(hex) {
    const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? [
        parseInt(result[1], 16),
        parseInt(result[2], 16),
        parseInt(result[3], 16)
    ] : [0, 0, 0];
}

function updateColorHex() {
    const r = parseInt(document.getElementById('colorRNum').value) || 0;
    const g = parseInt(document.getElementById('colorGNum').value) || 0;
    const b = parseInt(document.getElementById('colorBNum').value) || 0;
    const hex = '#' + [r, g, b].map(x => x.toString(16).padStart(2, '0').toUpperCase()).join('');
    document.getElementById('colorR').value = hex;
}

// Load devices from ESP
async function loadDevices() {
    try {
        const response = await fetch('/devices/list');
        const data = await response.json();
        devices = data;

        // Store device attributes (width, height)
        data.forEach(device => {
            deviceAttributes[device.mac] = {
                width: device.width || 32,
                height: device.height || 32
            };
        });

        const select = document.getElementById('deviceSelect');
        select.innerHTML = '';

        if (data.length === 0) {
            select.innerHTML = '<option value="">Keine Geräte gefunden</option>';
            return;
        }

        data.forEach(device => {
            const option = document.createElement('option');
            option.value = device.mac;
            const dims = device.width && device.height ? ` (${device.width}x${device.height})` : '';
            option.textContent = `${device.name || 'Unbekannt'} (${device.mac})${dims}`;
            select.appendChild(option);
        });

        // Select last connected device
        const lastConnected = data.find(d => d.lastConnected);
        if (lastConnected) {
            select.value = lastConnected.mac;
            selectedDevice = lastConnected.mac;
        }
    } catch (error) {
        console.error('Fehler beim Laden der Geräte:', error);
        showStatus('error', 'Fehler beim Laden der Geräte', 'settings');
    }
}

// Update selected device
document.addEventListener('DOMContentLoaded', () => {
    const select = document.getElementById('deviceSelect');
    if (select) {
        select.addEventListener('change', (e) => {
            selectedDevice = e.target.value;
        });
    }
});

// Update file name display
function updateFileName(type) {
    const fileInput = document.getElementById(type + 'File');
    const fileNameDisplay = document.getElementById(type + 'FileName');
    if (fileInput.files.length > 0) {
        fileNameDisplay.textContent = '✓ ' + fileInput.files[0].name;
    } else {
        fileNameDisplay.textContent = '';
    }
}

// Show status message (global toast)
function showStatus(type, message, section) {
    // Create or get global toast element
    let toastEl = document.getElementById('global-toast');
    if (!toastEl) {
        toastEl = document.createElement('div');
        toastEl.id = 'global-toast';
        toastEl.className = 'status';
        document.body.appendChild(toastEl);
    }

    toastEl.textContent = message;
    toastEl.className = 'status ' + type;

    // Auto-hide after 4 seconds
    setTimeout(() => {
        toastEl.className = 'status';
    }, 4000);
}

// Send command to ESP
async function sendCommand(action) {
    if (!selectedDevice) {
        showStatus('error', 'Bitte wählen Sie ein Gerät aus', 'settings');
        return;
    }

    try {
        let url = `/device/${selectedDevice}/${action}`;
        let params = new URLSearchParams();

        switch (action) {
            case 'setTime':
                params.append('hour', document.getElementById('hour').value || 0);
                params.append('minute', document.getElementById('minute').value || 0);
                params.append('second', document.getElementById('second').value || 0);
                showStatus('success', 'Zeit wird gesetzt...', 'time');
                break;

            case 'setBrightness':
                const brightness = Math.min(255, parseInt(document.getElementById('brightness').value) || 0);
                params.append('brightness', brightness);
                showStatus('success', 'Helligkeit wird gesetzt...', 'brightness');
                break;

            case 'setSpeed':
                const speed = Math.min(255, parseInt(document.getElementById('speed').value) || 0);
                params.append('speed', speed);
                showStatus('success', 'Geschwindigkeit wird gesetzt...', 'brightness');
                break;

            case 'setClockMode':
                params.append('mode', document.getElementById('clockMode').value || 0);
                showStatus('success', 'Uhr-Modus wird gesetzt...', 'time');
                break;

            case 'setPixel':
                params.append('x', document.getElementById('pixelX').value || 0);
                params.append('y', document.getElementById('pixelY').value || 0);
                params.append('r', document.getElementById('colorRNum').value || 0);
                params.append('g', document.getElementById('colorGNum').value || 0);
                params.append('b', document.getElementById('colorBNum').value || 0);
                showStatus('success', 'Pixel wird gesetzt...', 'pixel');
                break;

            case 'sendText':
                const matrixHeight = deviceAttributes[selectedDevice]?.height || 32;
                params.append('text', document.getElementById('textContent').value);
                params.append('animation', document.getElementById('textAnimation').value || 0);
                params.append('save_slot', document.getElementById('textSaveSlot').value || 0);
                params.append('speed', document.getElementById('textSpeed').value || 50);
                params.append('colorR', document.getElementById('colorRNum').value || 255);
                params.append('colorG', document.getElementById('colorGNum').value || 255);
                params.append('colorB', document.getElementById('colorBNum').value || 255);
                params.append('rainbow_mode', 0);
                params.append('matrix_height', matrixHeight);
                showStatus('success', 'Text wird gesendet...', 'text');
                break;

            case 'setOrientation':
                params.append('orientation', document.getElementById('orientation').value);
                showStatus('success', 'Orientierung wird gesetzt...', 'settings');
                break;

            case 'setFunMode':
                params.append('funMode', document.getElementById('funMode').checked);
                showStatus('success', 'Fun Mode wird gesetzt...', 'settings');
                break;

            case 'sendPNG':
                await sendFile('png');
                return;

            case 'sendAnimation':
                await sendFile('gif');
                return;

            case 'ledOn':
            case 'ledOff':
            case 'clear':
            case 'fill':
                showStatus('success', 'Befehl wird ausgeführt...', 'led');
                break;
        }

        url += '?' + params.toString();
        const response = await fetch(url);
        const text = await response.text();

        if (response.ok) {
            showStatus('success', '✓ ' + text, action.replace('set', '').toLowerCase() || 'led');
        } else {
            showStatus('error', '✗ ' + text, action.replace('set', '').toLowerCase() || 'led');
        }
    } catch (error) {
        console.error('Fehler:', error);
        showStatus('error', 'Fehler beim Senden des Befehls', 'settings');
    }
}

// Send file to ESP
async function sendFile(type) {
    const fileInput = document.getElementById(type + 'File');
    if (!fileInput.files.length) {
        showStatus('error', 'Bitte wählen Sie eine Datei aus', 'upload');
        return;
    }

    const file = fileInput.files[0];
    const formData = new FormData();
    formData.append('file', file);

    try {
        const endpoint = type === 'png' ? '/device/sendPNG' : '/device/sendAnimation';
        const response = await fetch(endpoint + '?mac=' + selectedDevice, {
            method: 'POST',
            body: formData
        });

        const text = await response.text();
        if (response.ok) {
            showStatus('success', '✓ ' + text, 'upload');
            fileInput.value = '';
            document.getElementById(type + 'FileName').textContent = '';
        } else {
            showStatus('error', '✗ ' + text, 'upload');
        }
    } catch (error) {
        console.error('Fehler beim Upload:', error);
        showStatus('error', 'Fehler beim Datei-Upload', 'upload');
    }
}

