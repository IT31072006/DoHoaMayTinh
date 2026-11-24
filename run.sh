#!/bin/bash
# ==============================================================================
# SCRIPT TỰ ĐỘNG BIÊN DỊCH VÀ KHỞI CHẠY GAME CHROME T-REX RUNNER 2D TRÊN UBUNTU
# ==============================================================================

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

echo "======================================================================"
echo "    GAME CHROME T-REX DINOSAUR RUNNER (ĐỒ HỌA MÁY TÍNH - VKU)         "
echo "    BÀI TẬP GIỮA KỲ MÔN ĐỒ HỌA MÁY TÍNH - C++ / OPENGL / FREEGLUT     "
echo "======================================================================"

echo "[1/2] Đang kiểm tra và biên dịch mã nguồn C++..."
g++ -Wall -Wextra -O2 -std=c++11 main.cpp -lGL -lGLU -lglut -o dino_game
cp dino_game artillery_game

echo "[2/2] Biên dịch thành công! Đang mở cửa sổ game..."
./dino_game
