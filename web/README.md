# iPixel ESP32 Web Control Panel

Eine interaktive HTML-Webseite zur Steuerung aller iPixel-Funktionen über den Browser.

## Features

✅ **Geräteauswahl** - Wähle aus allen verbundenen iPixel-Geräten
✅ **Zeit & Uhr** - Stelle die aktuelle Zeit ein
✅ **Helligkeit & Geschwindigkeit** - Slider zur einfachen Anpassung
✅ **LED Kontrolle** - An/Aus, Löschen, Füllen
✅ **Pixel Zeichnen** - Setze einzelne Pixel mit Farbwähler
✅ **Text** - Sende Text mit verschiedenen Animationen
✅ **Datei Upload** - Lade PNG-Bilder und GIF-Animationen hoch
✅ **Einstellungen** - Orientierung und Fun Mode

## Installation & Deployment

Die Web-Dateien werden automatisch beim Kompilieren in das SPIFFS-Dateisystem des ESP32 eingebettet.

### Dateien-Struktur
```
web/
├── index.html      # Hauptseite (wird in data/web/ kopiert)
├── control.js      # JavaScript-Logik (wird in data/web/ kopiert)
└── README.md       # Diese Datei

data/
└── web/
    ├── index.html  # Wird in SPIFFS eingebettet
    └── control.js  # Wird in SPIFFS eingebettet
```

### Wie es funktioniert
1. Die Dateien in `web/` sind die Quellen
2. Sie werden in `data/web/` kopiert
3. PlatformIO bettet `data/` automatisch in SPIFFS ein
4. Der ESP32 serviert sie über `/` (Root-URL)

## Verwendung

### Erste Schritte
1. Kompiliere und lade den Code auf den ESP32
2. Verbinde dich mit dem ESP32 über WiFi
3. Öffne die IP-Adresse des ESP32 im Browser (z.B. `http://192.168.1.100`)
4. Die Webseite wird automatisch geladen

### Geräte steuern
1. Wähle ein Gerät aus der Dropdown-Liste oben
2. Klicke auf "🔄 Aktualisieren" um die Geräteliste zu aktualisieren
3. Nutze die verschiedenen Karten zur Steuerung:
   - **⏰ Zeit & Uhr** - Stelle Stunde, Minute, Sekunde ein
   - **💡 Helligkeit & Geschwindigkeit** - Nutze die Slider
   - **🔌 LED Kontrolle** - Schnelle Buttons für An/Aus/Löschen
   - **🎨 Pixel Zeichnen** - Setze einzelne Pixel mit RGB-Farbe
   - **📝 Text** - Sende Text mit Animationen
   - **📤 Datei Upload** - Lade PNG und GIF Dateien hoch
   - **⚙️ Einstellungen** - Orientierung und Fun Mode

### Datei Upload
1. Klicke auf das Upload-Feld für PNG oder GIF
2. Wähle eine Datei aus (max. 32x32 Pixel empfohlen)
3. Klicke "PNG senden" oder "GIF senden"
4. Warte auf die Bestätigung

## API-Endpoints

Die Web-Seite kommuniziert mit folgenden REST-API-Endpoints:

### Geräte
- `GET /devices/list` - Liste aller Geräte
- `GET /devices/last` - Zuletzt verbundenes Gerät
- `GET /devices/set?mac=XX:XX:XX:XX:XX:XX&name=Name` - Gerät hinzufügen/aktualisieren

### Befehle
- `GET /device/{mac}/setTime?hour=12&minute=30&second=0`
- `GET /device/{mac}/setBrightness?brightness=128`
- `GET /device/{mac}/setSpeed?speed=50`
- `GET /device/{mac}/setPixel?x=0&y=0&r=255&g=0&b=0`
- `GET /device/{mac}/sendText?text=Hello&animation=0&speed=50&colorR=255&colorG=255&colorB=255`
- `GET /device/{mac}/setOrientation?orientation=0`
- `GET /device/{mac}/setFunMode?funMode=true`
- `GET /device/{mac}/ledOn`
- `GET /device/{mac}/ledOff`
- `GET /device/{mac}/clear`
- `GET /device/{mac}/fill`

### Datei Upload
- `POST /device/sendPNG?mac=XX:XX:XX:XX:XX:XX` - PNG-Datei hochladen
- `POST /device/sendAnimation?mac=XX:XX:XX:XX:XX:XX` - GIF-Datei hochladen

## Responsive Design

Die Seite ist vollständig responsive und funktioniert auf:
- Desktop-Browsern
- Tablets
- Smartphones

## Styling

- Modernes Gradient-Design mit Lila-Tönen
- Intuitive Bedienung mit großen Buttons
- Echtzeit-Statusmeldungen
- Farbwähler für RGB-Werte
- Slider für Helligkeit und Geschwindigkeit

## Technologie

- HTML5
- CSS3 (Grid, Flexbox, Responsive)
- Vanilla JavaScript (keine Abhängigkeiten)
- Fetch API für HTTP-Requests
- Async/Await für asynchrone Operationen

