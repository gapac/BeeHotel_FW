# BeeHotel_FW

## BOM in tabela
- https://unilj-my.sharepoint.com/:x:/g/personal/fz8406_student_uni-lj_si/EUVIKQBwZVZEiQTID3_gR-cBUHhWG1S9oAQ9KZtY8Le_DA?rtime=Ik3cb0dz3Eg

## Taski
- integracija temp, hum, air senzorja, 
- LORA messagi
- integracija mikrofona z i2s
- posebej main funkcija brez AI

## Knjiznice
- BME680: https://github.com/adafruit/Adafruit_BME680/tree/master
- MIC: https://github.com/joextodd/listener/tree/master

## Navodila za izdelavo MVP programa
- Testiranje bme680 senzorja (temperatura, kvaliteta zraka, tlak, vlažnost) ostali senzor (svetlost, buzz/nobuzz)
- Obseg parametrov: 0...100 z decimalnim mestom,  0.3k...20k ohm zdeljeno z 10 ,  300...1100 hPa, 0...100%, 0...100%, 0...10000
- Testiranje ML algoritma
- Testiranje LoRaWAN modula

## Postopno vključevanje 
- ML algoritem
- Senzorji
- LoRaWAN modul