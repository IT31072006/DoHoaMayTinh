# ==============================================================================
# TRƯỜNG ĐẠI HỌC CÔNG NGHỆ THÔNG TIN & TRUYỀN THÔNG VIỆT - HÀN (VKU)
# BÀI TẬP GIỮA KỲ MÔN ĐỒ HỌA MÁY TÍNH (COMPUTER GRAPHICS)
# MAKEFILE BIÊN DỊCH VÀ KHỞI CHẠY GAME CHROME T-REX DINOSAUR RUNNER 2D
# ==============================================================================

CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++11

# Thư viện đồ họa trên Ubuntu Linux
LIBS = -lGL -lGLU -lglut

TARGET = dino_game
ALIAS  = artillery_game
SRC = main.cpp

.PHONY: all run clean help

all: $(TARGET)
	@cp $(TARGET) $(ALIAS)

$(TARGET): $(SRC)
	@echo "=================================================="
	@echo "Đang biên dịch Chrome T-Rex Dinosaur Runner 2D..."
	@echo "=================================================="
	$(CXX) $(CXXFLAGS) $(SRC) $(LIBS) -o $(TARGET)
	@echo "Biên dịch thành công! File thực thi: ./$(TARGET)"

run: all
	@echo "Khởi chạy Game..."
	./$(TARGET)

clean:
	@echo "Dọn dẹp file thực thi..."
	rm -f $(TARGET) $(ALIAS)

help:
	@echo "Hướng dẫn:"
	@echo "  make        : Biên dịch game thành $(TARGET)"
	@echo "  make run    : Biên dịch và chạy game ngay lập tức"
	@echo "  make clean  : Xóa file thực thi đã biên dịch"
