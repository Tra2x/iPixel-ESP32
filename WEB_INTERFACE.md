# iPixel ESP32 Web Interface

Eine vollständig interaktive HTML-Webseite zur Steuerung aller iPixel-Funktionen über den Browser.

## 🎯 Überblick

Die Web-Seite bietet eine benutzerfreundliche Oberfläche für alle Funktionen des iPixel-Displays:

- ✅ Geräteauswahl und -verwaltung
- ✅ Zeit und Uhr einstellen
- ✅ Helligkeit und Geschwindigkeit anpassen
- ✅ LED an/aus, Löschen, Füllen
- ✅ Einzelne Pixel mit RGB-Farbe setzen
- ✅ Text mit Animationen senden
- ✅ PNG-Bilder hochladen
- ✅ GIF-Animationen hochladen
- ✅ Orientierung und Fun Mode einstellen

## 📁 Dateien

```
web/
├── index.html       # Hauptseite (HTML + CSS)
├── control.js       # JavaScript-Logik
└── README.md        # Dokumentation

data/web/           # Wird automatisch von PlatformIO erstellt
├── index.html       # Kopie für SPIFFS
└── control.js       # Kopie für SPIFFS
```

## 🚀 Wie es funktioniert

### Build-Prozess
1. `copy_web_files.py` wird vor dem Build ausgeführt
2. Dateien aus `web/` werden nach `data/web/` kopiert
3. PlatformIO bettet `data/` in SPIFFS ein
4. ESP32 serviert die Dateien über HTTP

### Zugriff
- Öffne `http://<ESP32-IP>/` im Browser
- Die Seite wird automatisch geladen
- Alle Funktionen sind sofort verfügbar

## 🎨 Design

- **Responsive Layout** - Funktioniert auf Desktop, Tablet, Smartphone
- **Modernes Design** - Gradient-Hintergrund, klare Struktur
- **Intuitive Bedienung** - Große Buttons, Slider, Farbwähler
- **Echtzeit-Feedback** - Statusmeldungen nach jedem Befehl

## 🔧 Technologie

- **HTML5** - Semantische Struktur
- **CSS3** - Grid, Flexbox, Responsive Design
- **Vanilla JavaScript** - Keine externen Abhängigkeiten
- **Fetch API** - Asynchrone HTTP-Requests
- **SPIFFS** - Dateisystem auf dem ESP32

## 📝 API-Endpoints

Die Web-Seite nutzt folgende REST-API-Endpoints:

### Geräte
```
GET /devices/list              # Alle Geräte
GET /devices/last              # Zuletzt verbundenes Gerät
GET /devices/set?mac=...&name=...  # Gerät hinzufügen/aktualisieren
```

### Befehle
```
GET /device/{mac}/setTime?hour=12&minute=30&second=0
GET /device/{mac}/setBrightness?brightness=128
GET /device/{mac}/setSpeed?speed=50
GET /device/{mac}/setPixel?x=0&y=0&r=255&g=0&b=0
GET /device/{mac}/sendText?text=Hello&animation=0&speed=50&colorR=255&colorG=255&colorB=255
GET /device/{mac}/setOrientation?orientation=0
GET /device/{mac}/setFunMode?funMode=true
GET /device/{mac}/ledOn
GET /device/{mac}/ledOff
GET /device/{mac}/clear
GET /device/{mac}/fill
```

### Datei Upload
```
POST /device/sendPNG?mac=...       # PNG hochladen
POST /device/sendAnimation?mac=... # GIF hochladen
```

## 🛠️ Entwicklung

### Änderungen an der Web-Seite
1. Bearbeite `web/index.html` oder `web/control.js`
2. Kompiliere mit `pio run`
3. Das Build-Script kopiert die Dateien automatisch
4. Lade auf den ESP32

### Debugging
- Öffne die Browser-Konsole (F12)
- Überprüfe die Netzwerk-Requests
- Schaue in die ESP32-Logs (Serial Monitor)

## 📦 Speicherverbrauch

- `index.html` - ~15 KB
- `control.js` - ~8 KB
- **Total** - ~23 KB (SPIFFS)

Der ESP32 hat genug Speicher für die Web-Seite und alle anderen Funktionen.

## 🔐 Sicherheit

Die Web-Seite läuft lokal im Netzwerk. Für Produktivumgebungen sollte man erwägen:
- HTTPS zu aktivieren
- Authentifizierung hinzuzufügen
- CORS-Richtlinien zu setzen

## 📚 Weitere Ressourcen

- [web/README.md](web/README.md) - Detaillierte Dokumentation
- [src/Webserver.h](src/Webserver.h) - Webserver-Implementierung
- [platformio.ini](platformio.ini) - Build-Konfiguration

