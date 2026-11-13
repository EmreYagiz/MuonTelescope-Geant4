# B1 müon teleskobu — rapor referansı

Bu paket, rapor için referans alınan blok öncesi B1.1 kaynak yedeğinden hazırlanmıştır. İki sintilatör paraleldir, aktif yüzleri arasında 30 cm açıklık vardır ve arada hedef yoktur. C++ kaynakları yedekten aynen alınmıştır. Çalıştırma makrosu eski sürümün komutlarına uyarlanmıştır.

## Gereksinimler

Linux veya WSL üzerinde C++ derleyicisi, CMake ve çok iş parçacığı desteğiyle derlenmiş Geant4 11.3.2 gerekir. Mevcut CMake yapılandırması Geant4 UI ve görselleştirme bileşenlerini de ister. Geant4 fizik veri setleri kurulu olmalıdır. ROOT, çıktı dosyasını incelemek için isteğe bağlıdır.

## Derleme ve çalıştırma

Arşivi açıp bu README'nin bulunduğu klasöre girin. İlk satırdaki yolu kendi Geant4 kurulumunuza göre değiştirin.

```bash
source /path/to/geant4-install/bin/geant4.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build
./exampleB1 reference.mac
```

CMake Geant4'ü bulamazsa ilk CMake komutuna `-DGeant4_DIR=/path/to/geant4-install/lib/cmake/Geant4` ekleyin. Kuruluma göre `lib` yerine `lib64` kullanılabilir.

`reference.mac` 1000 olay çalıştırır. Olay sayısı için `/run/beamOn` satırını düzenleyin. Kalıcı değişiklikleri proje kökündeki makroda yapıp CMake yapılandırmasını yeniden çalıştırın. ROOT çıktısını build klasöründe `muon_telescope_data*.root` adıyla arayın. Olay tablosu `MuonTelescope`, dalga biçimi tablosu `Oscilloscope` adını taşır.

GitHub'a yüklemek için arşivi açıp bu klasörün içeriğini depoya ekleyin.
