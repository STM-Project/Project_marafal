# STM32F769 → Arkusz Google → demonstracyjny przekaźnik

Rozwiązanie działa na zwykłym bezpłatnym koncie Google/Gmail. Arkusz zapisuje się
na Twoim Dysku Google. Nie potrzebujesz płatnej usługi Google Cloud, Firebase,
klucza API Google ani hasła do Gmaila w STM32.

STM32 wykonuje **wychodzące połączenia HTTPS**. Nie przekierowuj portów routera.
Ta implementacja używa Ethernetu/LwIP, a nie połączenia AT przez ESP32.

```
co 10 s: STM32 -- HTTPS POST: 25.5°C + poprzedni stan DEMO --> Apps Script --> Arkusz
         STM32 <-- HTTPS: nowy stan DEMO z pola wyboru ------ Apps Script <-- Arkusz
```

**Żaden pin GPIO nie jest przełączany.** Stan przekaźnika jest tylko zmienną RAM
`googleCloudStatus.demo_relay`. Temperatura testowa wynosi stale 25,5°C.
Nie ma automatycznej reguły progowej: o stanie decydujesz polem wyboru w arkuszu.

## 1. Utwórz prywatny arkusz

1. Zaloguj się swoim kontem Gmail na <https://drive.google.com>.
2. Wybierz **Nowy → Arkusze Google → Pusty arkusz**.
3. Nadaj nazwę, np. `STM32 - temperatura i sterowanie`.
4. W **Plik → Ustawienia** wybierz swoją strefę czasową, np. Warszawa.
5. Pozostaw arkusz prywatny. Nie wybieraj „Opublikuj w internecie”.

## 2. Wklej kod Apps Script

1. W arkuszu wybierz **Rozszerzenia → Apps Script**.
2. Usuń przykładowy `myFunction` z pliku `Kod.gs`/`Code.gs`.
3. Wklej całą zawartość pliku
   [Code.gs](../../cloud/google-apps-script/Code.gs) z tego repozytorium.
4. Zapisz projekt, np. jako `STM32 Bridge`.

## 3. Jednorazowo przygotuj arkusz i token urządzenia

1. Z listy funkcji na górze edytora wybierz **setup** i kliknij **Uruchom**.
2. Google poprosi o autoryzację: wybierz własne konto i zezwól swojemu skryptowi
   na pracę z arkuszami. Przy komunikacie o niezweryfikowanej aplikacji sprawdź,
   że to Twój własny projekt z właśnie wklejonym kodem; osobista aplikacja
   testowa może wymagać przejścia przez **Zaawansowane** do tego projektu.
3. Wróć do arkusza. Powstaną zakładki **Sterowanie** i **Pomiary**.
4. W edytorze Apps Script otwórz **Ustawienia projektu** (ikona koła zębatego),
   potem **Właściwości skryptu / Script properties**. Zobaczysz:

   | Właściwość | Znaczenie |
   | --- | --- |
   | `SHEET_ID` | Identyfikator Twojego arkusza, ustawiony automatycznie |
   | `DEVICE_ID` | Domyślnie `stm32f769-demo` |
   | `DEVICE_TOKEN` | Prywatny token urządzenia: 64 znaki szesnastkowe |

5. Skopiuj `DEVICE_TOKEN` do swojej lokalnej konfiguracji STM32 opisanej niżej.
   Nie wklejaj go do arkusza, publicznego repozytorium ani adresu URL.
   Ponowne `setup()` zachowuje istniejący token i ustawienie checkboxa.

## 4. Wdróż aplikację internetową

1. Kliknij **Wdróż → Nowe wdrożenie**.
2. Wybierz typ **Aplikacja internetowa / Web app**.
3. Ustaw **Uruchamiaj jako: Ja / Me** (Twoje konto).
4. Ustaw **Kto ma dostęp: Każdy / Anyone**, również bez logowania.
   Samo „Każdy użytkownik z kontem Google” nie wystarczy dla STM32.
5. Kliknij **Wdróż**, zaakceptuj autoryzację, jeśli się pojawi.
6. Skopiuj **URL aplikacji internetowej**, np.:

   ```text
   https://script.google.com/macros/s/IDENTYFIKATOR_WDROZENIA/exec
   ```

   Użyj adresu kończącego się `/exec`, a nie testowego `/dev`.

Publiczny jest punkt wejścia skryptu, **nie arkusz**. Kod odrzuca pomiar bez
prawidłowego prywatnego tokena. Jeżeli administrator konta zabrania opcji
„Każdy”, ta metoda nie będzie działać bez zmiany zasad konta; na osobistym
koncie Google zwykle można ją wybrać.

Otwarcie URL w przeglądarce ma pokazać:
`STM32 cloud: wyslij POST z urzadzenia.` To sprawdza publikację, ale jeszcze
nie potwierdza zapisu temperatury.

## 5. Włącz klienta w STM32

1. Skopiuj plik `Application/google_cloud_config.local.h.example` pod nazwą
   `Application/google_cloud_config.local.h`.
2. W nowym pliku ustaw:

   ```c
   #define GOOGLE_CLOUD_ENABLED 1
   #define GOOGLE_CLOUD_DEVICE "stm32f769-demo"
   #define GOOGLE_CLOUD_TOKEN "TU_64_ZNAKI_Z_DEVICE_TOKEN"
   #define GOOGLE_CLOUD_PATH "/macros/s/TWOJ_IDENTYFIKATOR_WDROZENIA/exec"
   ```

   Ostatnia wartość to **sama ścieżka** z URL, bez `https://script.google.com`.
   Nazwa urządzenia musi odpowiadać `DEVICE_ID` oraz `Sterowanie!A2`.
   Plik lokalny jest ignorowany przez Git. Token trafia do firmware, więc
   nie udostępniaj publicznie również wynikowego ELF/BIN z własnym tokenem.
3. Odśwież projekt w STM32CubeIDE. Nowe pliki `.c` w `Application` są częścią
   konfiguracji źródeł projektu. Wykonaj **Project → Clean**, potem **Build**.
4. Wgraj firmware na płytkę.

Domyślnie, bez pliku lokalnego, klient jest wyłączony. Włączenie klienta
inicjalizuje Ethernet/LwIP także przy dotychczasowym `HTTP_THREAD_ON=0`.
Nie trzeba uruchamiać lokalnego serwera HTTP ani HTTPS, aby wysyłać dane do Google.

## 6. Sprawdź połączenie Ethernet i czas

1. Podłącz Ethernet do routera z internetem.
2. W bieżącym `LWIP/App/lwip.c` jest adres statyczny **192.168.1.40**, maska
   **255.255.255.0**, brama **192.168.1.1**. Dopasuj te ustawienia w CubeMX lub
   użyj wolnego adresu ze swojej sieci. Nie przypisuj adresu innego urządzenia.
3. Potrzebne są DNS, HTTPS/TCP 443 do `script.google.com` i
   `script.googleusercontent.com` oraz SNTP/UDP 123 do `time.google.com`.
   Jeśli DNS nie był ustawiony, klient ustawia 8.8.8.8 i 8.8.4.4.
4. Klient uruchamia SNTP i ustawia zegar UTC biblioteki mbedTLS. Pierwsza próba
   może czekać na synchronizację; kolejna następuje po 10 s.

Weryfikacja certyfikatu jest obowiązkowa, wraz z nazwą serwera i datą ważności.
Nie ustawiaj `MBEDTLS_SSL_VERIFY_NONE`, żeby obejść błąd. Dołączono publiczne
certyfikaty GTS Root R1–R4 z systemowego magazynu Debian `ca-certificates`.
Po zmianie łańcuchów Google trzeba aktualizować ten zestaw z zaufanego źródła
<https://pki.goog/repository/>. SNTP to zwykła synchronizacja NTP, nie
uwierzytelnione źródło czasu; docelowy produkt może wymagać zaufanego RTC.

## 7. Obejrzyj pomiary

1. Po uruchomieniu poczekaj na synchronizację SNTP (zwykle kilkadziesiąt sekund); potem pomiary są wysyłane co 10 s.
2. W zakładce **Pomiary** powinny przybywać wiersze z czasem odebrania i liczbą
   **25,5**. To liczba w komórce; sposób wyświetlania kropki/przecinka zależy
   od ustawień regionalnych arkusza.
3. W **Sterowanie** komórka **E2** pokazuje ostatnią temperaturę, **D2** czas
   ostatniego kontaktu. Godzina pochodzi z serwera Google.
4. Jeśli **D2** przestało się zmieniać, dane są nieaktualne — pole checkboxa
   samo w sobie nie potwierdza połączenia z urządzeniem.

Historia ma limit około 10 000 ostatnich wpisów (około 25–28 godzin przy pomiarze co 10 s).
Po osiągnięciu limitu skrypt usuwa 1000 najstarszych wierszy. Jeśli chcesz
archiwizować dłużej, najpierw skopiuj zakładkę lub zmień tę politykę w skrypcie.

## 8. Kliknij demonstracyjny przekaźnik

1. W zakładce **Sterowanie** zaznacz checkbox **B2**.
2. STM32 pobierze `1` w odpowiedzi na następny pomiar, zwykle w ciągu 10 s
   plus czas połączenia.
3. Zmienna `googleCloudStatus.demo_relay` w debuggerze przyjmie `1`.
   **Żaden GPIO ani rzeczywisty przekaźnik nie zostanie przełączony.**
4. Przy kolejnym pomiarze STM32 odeśle potwierdzenie. W **C2** pojawi się `1`,
   a **F2** pokaże `DEMO - potwierdzony`. Potwierdzenie może więc pojawić się
   dopiero po około 20 sekundach od kliknięcia, plus czas połączeń.
5. Odznacz **B2**, aby tak samo przetestować `0`.

Checkbox jest stanem żądanym, a nie przyciskiem chwilowym. Po restarcie STM32
stan RAM wraca do `0`; po kolejnym poprawnym odczycie przyjmie ustawienie B2.
Przy błędzie sieci, certyfikatu lub odpowiedzi stan RAM pozostaje bez zmian.

## 9. Diagnostyka w STM32CubeIDE

Dodaj do podglądu zmienną `googleCloudStatus`:

| Pole | Znaczenie |
| --- | --- |
| `successes` / `failures` | Liczba poprawnych / nieudanych wymian |
| `temperature10` | `255` oznacza 25,5°C |
| `demo_relay` | Symulowany stan 0 lub 1 |
| `last_error = 0` | Pełna wymiana i poprawna odpowiedź |
| `last_error = -1` | Błędna konfiguracja lokalna |
| `last_error = -2` | Brak aktualnego czasu SNTP |
| `last_error = -10, -11` | TCP/DNS: sprawdź bramę, Ethernet, DNS |
| `last_error = -16` | Nieoczekiwany HTTP lub zbyt duża odpowiedź |
| `last_error = -17` | Przekroczony czas odpowiedzi |
| `last_error = -22` | Odpowiedź aplikacji nie zgadza się z protokołem/nonce |
| `last_error = -23` | Nieoczekiwany adres przekierowania |
| `last_error = -20, -21` | Nie udało się uruchomić callbacka LwIP / zadania |
| `certificate_flags` | Flagi błędu sprawdzania certyfikatu mbedTLS |

Inne ujemne błędy mogą pochodzić bezpośrednio z mbedTLS. Nie loguj tokena ani
pełnego POST. `-22` często oznacza zły token/DEVICE_ID, brak `setup()`,
usuniętą zakładkę lub wpisany tekst zamiast checkboxa. Sprawdź też ekran
**Wykonania / Executions** w Apps Script, bez dodawania logowania żądań.

Google ContentService przekierowuje odpowiedź na `script.googleusercontent.com`.
Klient obsługuje 302/303, pobiera wynik przez GET i **nie przesyła tokena
ponownie do adresu przekierowania**. Odpowiedź musi zawierać losowy identyfikator
aktualnego pomiaru; stara lub niepoprawna odpowiedź nie zmieni stanu DEMO.

## 10. Zmiany skryptu, limity i dalsza rozbudowa

- Po zmianie `Code.gs`: **Wdróż → Zarządzaj wdrożeniami → Edytuj → Nowa wersja
  → Wdróż**. Zachowanie tego samego wdrożenia zachowuje URL. Sam zapis kodu
  nie aktualizuje opublikowanej wersji `/exec`.
- To wersja dla **jednego urządzenia**. Nie używaj równolegle tego samego
  identyfikatora/tokena na kilku STM32.
- Jeden pomiar co 10 sekund daje około **8640 wywołań dziennie**. Wolne połączenie lub błędy mogą wydłużyć odstęp; zadanie nie nadrabia zaległych pomiarów seriami żądań. Apps Script
  ma zmienne limity wykonania i dostępności; bezpłatne konto nie daje gwarancji
  pracy w czasie rzeczywistym. Endpoint może też zużywać limity przy cudzych
  wywołaniach. Dokumentacja limitów:
  <https://developers.google.com/apps-script/guides/services/quotas>.
- Przykład nie używa Gmail API ani `UrlFetchApp`; nie potrzebuje hasła aplikacji Gmail.
- Aby podłączyć rzeczywisty czujnik, zastąp słabą funkcję
  `GoogleCloud_ReadTemperature10()` własną implementacją zwracającą dziesiąte
  części stopnia. Zakres tej wersji: od -50,0 do +150,0°C.
- Do rzeczywistego sterowania GPIO potrzebna będzie osobna, świadoma zmiana
  kodu; w tej wersji pozostaje tylko demonstracja zgodnie z ustaleniem.

## Walidacja wykonana w środowisku chmurowym

Testy lokalne: `sh tests/run_google_cloud_tests.sh`. Sprawdzają ramki HTTP,
częściowe odczyty, chunked, dozwolone przekierowania, nonce i stan przekaźnika,
a także logikę Apps Script z atrapami usług Google: uwierzytelnienie, checkbox,
potwierdzenia, powtórzone żądanie, blokadę i limit historii.
Osobny test adaptera RNG sprawdza kopiowanie wszystkich bajtów, długości
niepodzielne przez cztery, granice bufora oraz propagację błędów HAL.

Kompilacja ARM: `python3 /workspace/cloud-setup/build.py`, wynik w
`Debug/cloud/firmware.elf`. To pomocnik przygotowany dla tego środowiska;
w STM32CubeIDE użyj zwykłego Build projektu.
Sprawdzono kompilację i linkowanie 260/260 plików zarówno z klientem
wyłączonym, jak i włączonym z konfiguracją testową bez prawdziwych danych konta.
Przed wgraniem zbuduj firmware z własnym plikiem konfiguracji lokalnej.

Testy lokalne i kompilacja **nie potwierdzają jeszcze połączenia z Twoim kontem
Google ani działania na płytce**. Te kroki wymagają Twojego wdrożenia `/exec`,
lokalnej konfiguracji i uruchomienia sprzętu. Nie tworzono arkusza na Twoim
koncie i nie publikowano skryptu w Twoim imieniu.

Dokumentacja Google:
[Web apps](https://developers.google.com/apps-script/guides/web),
[ContentService i przekierowania](https://developers.google.com/apps-script/guides/content).
