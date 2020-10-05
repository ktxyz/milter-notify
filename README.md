# milter-notify
Milter that notifies about new email by a POST command

## Instalacja

### 1. Instalacja postfixa

Przed zainstalowaniem miltera należy zainstalować postfixa.

### 2. Konfiguracja postfixa

Po zainstalowaniu postfixa, należy poinformować go, na jakim porcie milter będzie się z nim komunikował. Aby to zrobić należy dodać linijkę

```
smtpd_milters=inet:localhost:7695
```

do pliku ```/etc/postfix/main.cf```. Jeżeli chce się korzystać z innego portu, należy dostosować jego wartość w pliku ```milter_notify.service``` przed uruchomieniem skryptu instalacyjnego. Po zmianie pliku konfiguracyjnego należy przeładować postfix komendą ```sudo postfix reload```.

### 3. Uruchomienie skryptu instalacyjnego.

W folderze ```milter-notify/``` należy uruchomić skrypt instalacyjny za pomocą komendy

```
bash ./scripts/install.sh
```

Utworzy on pliki konfiguracyjne oraz skopiuje program wraz z plikiem ```.service``` do odpowiednich folderów.

### 4. Konfiguracja

Aby skonfigurować działanie miltera należy zmienić wartości zapisane w plikach

```
/etc/milter-notify/ping_url - adres, na który chcemy wysyłać powiadomienia

/etc/milter-notify/ping_url - dowolna wartość przesyłana, użyteczna w przypadku chęci ochrony przed spamem ze strony osób trzecich (jako np. hasło, lub secret_key). 
```

#### 5. Uruchomienie

Aby uruchomić usługe miltera należy uruchomić skrypt

```
bash scripts/enable.sh
```

#### 6. Zmiany w trakcie działania

Jeżeli w trakcie działania chce się zmienić jakąś wartość w plikach konfiguracyjnych należy zrestartować usługę komendą:

```
sudo systemctl restart milter_notify
```

## Usuwanie

Aby usunąć milter, wystarczy uruchomić skrypt

```
bash scripts/remove.sh
```

Po jego wykonaniu milter jest całkowicie wyłączony a wszystki związane z nim pliki usunięte (należy ręcznie usunąć informacje o nim z pliku konfiguracyjnego postfixa).
