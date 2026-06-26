# Imagine Getting Noclipped (IGN)- 3D FPS Game

**"Imagine Getting Noclipped"** adalah game 3D First-Person Shooter (FPS) bertema *Backrooms & DooM*. Game ini dikembangkan menggunakan bahasa pemrograman C++ dan pustaka OpenGL untuk memenuhi Tugas Besar Praktikum mata kuliah Grafika dan Teknik Interaktif, Universitas Diponegoro.

## 📝 Deskripsi Proyek
Dalam game ini, pemain berada dalam sudut pandang orang pertama (First-Person) dan harus menjelajahi labirin gelap yang dirender dari struktur matriks 2D ke dalam ruang 3D. Tujuan utamanya adalah bertahan hidup, menavigasi rintangan, dan mengalahkan entitas musuh yang mengejar.

## ✨ Fitur Utama
- **Sistem Kamera Dinamis:** Navigasi kamera *First-Person* menggunakan fungsi `gluLookAt`.
- **Geometri & Transformasi Real-time:** Objek modular seperti tembok, musuh (*billboarding*), dan senjata pemain.
- **Pencahayaan & Bayangan:** Efek *flickering light* (lentera) dan implementasi *shadow mapping*.
- **Pemetaan Tekstur (Texturing):** Detail realistis pada lantai, atap, dan tembok labirin.
- **Deteksi Tabrakan (Collision):** Batasan solid pada tembok/lantai, serta sistem interaksi tembakan (pengurangan HP musuh).

## 🎮 Kontrol Permainan
- `W` `A` `S` `D` : Menggerakkan karakter
- `SPACE` / `Klik Kiri (LMB)` : Menembak
- `R` : Restart permainan
- `ESC` / `Q` : Keluar dari permainan

## 🛠️ Teknologi yang Digunakan
- C++
- OpenGL / GLUT

## 👥 Pengembang (Kelompok 7 - Kelas C)
- Azka Aqylla Maulana (24060124140195)
- Agil Yudis Wibawa (24060124120045)
- Akmal Kafli Anan (24060124120042)
- Adel Rayyan Hakim (24060124140173)
