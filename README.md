# ImGui Program Monitor

## Projektübersicht

Dieses Projekt ist eine frühe Alpha-Version eines Programms zur Überwachung der Nutzung spezifischer Anwendungen und zur Erstellung von Ereignissen im Google Kalender basierend auf diesen Daten. Die Benutzeroberfläche wurde mit ImGui erstellt und nutzt DirectX9 für die Darstellung.

## Funktionen

1. **Programmüberwachung**:
   - Überwacht die Nutzung von spezifischen Anwendungen (z.B. notepad.exe, devenv.exe, chrome.exe) und zeichnet die aktive Nutzungszeit auf.
   - Anzeige der Nutzungshistorie in einem Log-Fenster in Echtzeit.

2. **OAuth2-Authentifizierung**:
   - Integriert die Google Kalender API mittels OAuth2 zur Authentifizierung.
   - Generiert eine URL zur Authentifizierung, über die der Benutzer ein Zugriffstoken erhält.

3. **Erstellung von Kalenderereignissen**:
   - Ermöglicht die Erstellung von Ereignissen im Google Kalender für jede überwachte Anwendung.
   - Konfigurationsfenster zur Festlegung des Titels, der Beschreibung und der Farbe des Ereignisses.
   - Automatische oder manuelle Einstellung der Start- und Endzeiten des Ereignisses basierend auf der Nutzungshistorie der Anwendung.

## Installation

### Voraussetzungen

- Windows-Betriebssystem
- Visual Studio 2019 oder neuer
- C++ Compiler
- DirectX 9 SDK
- CURL
- nlohmann::json Bibliothek

### Schritte zur Installation

1. **Repository klonen**:
   - Klonen Sie das Repository von GitHub: `git clone https://github.com/IhrBenutzername/ImGui-Program-Monitor.git`
   - Wechseln Sie in das Verzeichnis des geklonten Repositorys: `cd ImGui-Program-Monitor`

2. **Abhängigkeiten installieren**:
   Stellen Sie sicher, dass die folgenden Abhängigkeiten installiert sind:
   - DirectX 9 SDK
   - CURL
   - nlohmann::json

3. **Projekt öffnen**:
   - Öffnen Sie die Projektdatei `ImGui-Loader-Base.sln` in Visual Studio.
   - Stellen Sie sicher, dass alle Pfade zu den Abhängigkeiten korrekt konfiguriert sind.

4. **Projektkonfiguration überprüfen**:
   - Stellen Sie sicher, dass die Konfiguration auf `Debug` oder `Release` und die Plattform auf `x64` gesetzt ist.

5. **Kompilieren und ausführen**:
   - Kompilieren Sie das Projekt in Visual Studio durch Drücken von `Strg + Umschalt + B`.
   - Starten Sie das Programm durch Drücken von `F5` oder wählen Sie im Menü `Debuggen > Starten`.

## Nutzung

1. **OAuth2-Authentifizierung**:
   - Starten Sie das Programm.
   - Öffnen Sie das Fenster "OAuth2 Authorization".
   - Kopieren Sie die generierte URL und autorisieren Sie die Anwendung.
   - Geben Sie den erhaltenen Autorisierungscode ein und klicken Sie auf "Authorize".

2. **Programmüberwachung**:
   - Fügen Sie Anwendungen zur Überwachungsliste im Fenster "Programme Hinzufuegen" hinzu.
   - Überwachte Anwendungen werden in Echtzeit im Fenster "Aktives Programm" angezeigt.

3. **Erstellung von Kalenderereignissen**:
   - Klicken Sie im Log-Fenster auf den "Push"-Button neben der gewünschten Anwendung.
   - Konfigurieren Sie das Ereignis im erscheinenden Fenster "Event Configuration".
   - Klicken Sie auf "Create Event", um das Ereignis im Google Kalender zu erstellen.

## Weiterentwicklung

Dieses Projekt befindet sich in einer frühen Alpha-Phase. Zukünftige Entwicklungspläne umfassen:
- Erweiterte Fehlerbehandlung und Logging.
- Verbesserte Benutzeroberfläche mit zusätzlichen Konfigurationsoptionen.
- Integration weiterer APIs und Dienste zur erweiterten Automatisierung und Analyse.
