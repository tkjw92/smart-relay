# Fitur #
* Internet Remote Access
  * Blynk APP
  * Telegram Bot
  * Restfull API

* Local Remote Access
  * Web Server

* Task Scheduler


# Indikator #

Indikator  | Fungsi
------------- | -------------
LED Merah Menyala  | Perangkat mendapatkan supply power
LED Biru Berkedip 5 kali   |  Perangkat selesai melakukan proses init (done booting proccess)
LED BIRU Menyala  |   Perangkat terkoneksi dengan Blynk APP (Online Blynk Server Connected)


# Default Configuration #

You can Setup any configuration with Local Web Panel by connected via default Access Point Name:<br>

**Smart Relay**<br>

Then access Web Panel in:<br>

**192.168.0.1**


# Blynk Virtual Pin Configuration #

Virtual Pin | Digital Pin
------------  | ------------
V0  | Relay 1
V1  | Relay 2
V2  | Relay 3
V3  | Relay 4
V4  | LCD WIDGET IN BLYNK APP
V5  | LCD WIDGET IN BLYNK APP

# Scheduler Setup #

Scheduler Task akan mengontrol relay1 secara otomatis sesuai dengan range waktu yang ditentukan, untuk range waktu adalah waktu 
relay menyala misal. Range wkatu<br>

start = 1310<br>
end = 1412<br>

Konfigurasi di atas akan mengontrol relay1 secara otomatis jika waktu saat ini berada di dalam range yang telah di konfigurasi,<br>
13:10<br>

Relay1 menyala hingga<br>

14:12<br>

Relay1 akan otomatis mati jika waktu saat ini diluar range yang telah dikonfigurasi.

**NOTE:**<br>
* Relay1 tidak dapat di kontrol ketika scheduler aktif.

# Hard Reset #

Untuk melakukan hard reser anda dapat menekan tombol boot, dan menunggu LED indikator berkedip 5 kali.

# Restfull API #

Untuk REST API menggunakan method GET<br>
anda dapat melakukan request ke endpoint BLYNK API dengan format url seperti berikut<br>

**https://blynk.cloud/external/api/update?token={token}&{pin}={value}**
