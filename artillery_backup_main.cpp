/* ============================================================================
 * TRƯỜNG ĐẠI HỌC CÔNG NGHỆ THÔNG TIN & TRUYỀN THÔNG VIỆT - HÀN (VKU)
 * BÀI TẬP GIỮA KỲ MÔN ĐỒ HỌA MÁY TÍNH (COMPUTER GRAPHICS)
 * 
 * ĐỀ TÀI: GAME ĐẠI CHIẾN PHÁO BINH 2D (ARTILLERY BATTLE: BÉZIER 2D MASTER EDITION)
 * ============================================================================
 * NÂNG CẤP TOÀN DIỆN VỀ GIAO DIỆN (UI/UX), CHỨC NĂNG VÀ VẬT PHẨM TRONG GAME:
 * 
 * 1. THUẬT TOÁN ĐỒ HỌA CỐT LÕI (SLIDE CG.5 & LAB 5.3):
 *    - Quadratic Bézier Curve (3 điểm P0, P1, P2): Đạn pháo AP, Pháo cối, Đạn chùm.
 *          P(t) = (1-t)^2*P0 + 2t(1-t)*P1 + t^2*P2
 *    - Cubic Bézier Curve (4 điểm P0, P1, P2, P3): Tên lửa hành trình uốn lượn S-Curve.
 *          P(t) = (1-t)^3*P0 + 3t(1-t)^2*P1 + 3t^2(1-t)*P2 + t^3*P3
 *    - Đạo hàm tiếp tuyến P'(t) xoay đầu đạn & ngọn lửa phản lực theo hướng bay.
 *    - Đa giác kiểm soát (Control Polygon) & Tam giác bao lồi (Convex Hull).
 * 
 * 2. CÁC PHÉP BIẾN ĐỔI HÌNH HỌC AFFINE 2D TRÊN MA TRẬN 3X3 (SLIDE CG.3 & LAB 3):
 *    - Ma trận thuần nhất vector dòng: [x' y' 1] = [x y 1] * M.
 *    - Xoay nòng pháo quanh khớp nối T(-x0,-y0)*R(theta)*T(x0,y0).
 *    - Thân xe tăng tự động nghiêng theo độ dốc địa hình sườn núi.
 *    - Nan hoa bánh xe quay tròn theo quãng đường tịnh tiến dX / R_wheel.
 * 
 * 3. GIAO DIỆN NGƯỜI DÙNG HIỆN ĐẠI (ADVANCED GAME GUI / HUD):
 *    - Màn hình Menu mở đầu (Start Menu) chọn 3 chế độ: Chiến Dịch, Đấu với AI, 1v1 PvP.
 *    - Giao diện chiến đấu với Avatar, thanh máu HP chuyển màu mượt mà.
 *    - Thước đo góc ngắm hình bán nguyệt (Protractor Gauge) quanh tháp pháo.
 *    - Thanh lực bắn dạng LED phân đoạn nhiều nấc màu (Segmented Power Gauge).
 *    - Thanh Dock vũ khí dưới đáy màn hình với 4 ô icon trang bị (Click chuột hoặc phím 1-4).
 *    - La bàn gió quân sự (Military Wind Compass) hiển thị hướng và tốc độ gió.
 * 
 * 4. TÍNH NĂNG VẬT PHẨM & GAMEPLAY MỞ RỘNG (IN-GAME ITEMS & MECHANICS):
 *    - Hòm tiếp tế thả dù từ bầu trời (Airdrop Supply Crate): Nhặt máu HP hoặc tên lửa!
 *    - Kho vũ khí 4 loại: Đạn pháo AP, Pháo cối Mortar nổ lớn, Đạn chùm Cluster, Tên lửa Cruise.
 *    - Trí tuệ nhân tạo (AI Artillery Bot) giải ngược Bézier bằng tìm kiếm nhị phân.
 *    - Địa hình biến dạng (Destructible Terrain) khoét hố bom và xe tăng sụt lún theo trọng lực.
 *    - Hiệu ứng ngày/đêm với ánh sáng điểm động (Point Light) và mưa tuyết.
 *    - Hỗ trợ thao tác chuột toàn diện: click chọn vũ khí, click menu, kéo thả chuột ngắm bắn.
 * ============================================================================
 */

#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HẰNG SỐ CẤU HÌNH CỬA SỔ & MÀN HÌNH
// ============================================================================
const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;
const float GRAVITY = 460.0f;

// ============================================================================
// CẤU TRÚC TOÁN HỌC 2D & MA TRẬN THUẦN NHẤT 3X3 (VECTOR DÒNG)
// ============================================================================
struct Point2D {
    float x, y;
    Point2D() : x(0.0f), y(0.0f) {}
    Point2D(float _x, float _y) : x(_x), y(_y) {}

    Point2D operator+(const Point2D& o) const { return Point2D(x + o.x, y + o.y); }
    Point2D operator-(const Point2D& o) const { return Point2D(x - o.x, y - o.y); }
    Point2D operator*(float s) const { return Point2D(x * s, y * s); }
};

struct Matrix3x3 {
    float m[3][3];

    Matrix3x3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    Matrix3x3 operator*(const Matrix3x3& o) const {
        Matrix3x3 res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.m[i][j] = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    res.m[i][j] += m[i][k] * o.m[k][j];
                }
            }
        }
        return res;
    }

    Point2D transform(const Point2D& p) const {
        Point2D res;
        res.x = p.x * m[0][0] + p.y * m[1][0] + 1.0f * m[2][0];
        res.y = p.x * m[0][1] + p.y * m[1][1] + 1.0f * m[2][1];
        return res;
    }

    static Matrix3x3 translation(float tx, float ty) {
        Matrix3x3 res;
        res.m[2][0] = tx;
        res.m[2][1] = ty;
        return res;
    }

    static Matrix3x3 rotation(float angleRad) {
        Matrix3x3 res;
        float c = cos(angleRad);
        float s = sin(angleRad);
        res.m[0][0] =  c; res.m[0][1] = s;
        res.m[1][0] = -s; res.m[1][1] = c;
        return res;
    }

    static Matrix3x3 rotationAround(float x0, float y0, float angleRad) {
        Matrix3x3 t1 = Matrix3x3::translation(-x0, -y0);
        Matrix3x3 r  = Matrix3x3::rotation(angleRad);
        Matrix3x3 t2 = Matrix3x3::translation(x0, y0);
        return t1 * r * t2;
    }
};

// ============================================================================
// HỖ TRỢ VẼ CHỮ & CHỤP ẢNH MÀN HÌNH BMP
// ============================================================================
void drawText(float x, float y, const string& text, float r = 1.0f, float g = 1.0f, float b = 1.0f, void* font = GLUT_BITMAP_9_BY_15) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

bool saveScreenshotBMP(const char* filename, int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT) {
    std::vector<unsigned char> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels.data());

    int rowPadded = (width * 3 + 3) & (~3);
    int imageSize = rowPadded * height;

    unsigned char fileHeader[14] = {
        'B', 'M',
        (unsigned char)(54 + imageSize), (unsigned char)((54 + imageSize) >> 8),
        (unsigned char)((54 + imageSize) >> 16), (unsigned char)((54 + imageSize) >> 24),
        0, 0, 0, 0, 54, 0, 0, 0
    };

    unsigned char infoHeader[40] = {
        40, 0, 0, 0,
        (unsigned char)(width), (unsigned char)(width >> 8), (unsigned char)(width >> 16), (unsigned char)(width >> 24),
        (unsigned char)(height), (unsigned char)(height >> 8), (unsigned char)(height >> 16), (unsigned char)(height >> 24),
        1, 0, 24, 0, 0, 0, 0, 0,
        (unsigned char)(imageSize), (unsigned char)(imageSize >> 8), (unsigned char)(imageSize >> 16), (unsigned char)(imageSize >> 24),
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(fileHeader, 1, 14, f);
    fwrite(infoHeader, 1, 40, f);
    std::vector<unsigned char> pad(rowPadded - width * 3, 0);
    for (int y = 0; y < height; y++) {
        fwrite(&pixels[y * width * 3], 1, width * 3, f);
        if (!pad.empty()) fwrite(pad.data(), 1, pad.size(), f);
    }
    fclose(f);
    cout << "Da chup anh man hinh: " << filename << endl;
    return true;
}

// ============================================================================
// THUẬT TOÁN ĐỒ HỌA CƠ SỞ: BRESENHAM & MIDPOINT (LAB 1)
// ============================================================================
void drawBresenhamLine(int x1, int y1, int x2, int y2, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy, curX = x1, curY = y1;
    while (true) {
        glVertex2i(curX, curY);
        if (curX == x2 && curY == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; curX += sx; }
        if (e2 < dx)  { err += dx; curY += sy; }
    }
    glEnd();
}

void drawMidpointCircle(int xc, int yc, int radius, float r, float g, float b, bool fill = false) {
    glColor3f(r, g, b);
    int x = 0, y = radius, p = 1 - radius;

    auto plot8 = [&](int cx, int cy, int px, int py) {
        if (!fill) {
            glBegin(GL_POINTS);
            glVertex2i(cx + px, cy + py); glVertex2i(cx - px, cy + py);
            glVertex2i(cx + px, cy - py); glVertex2i(cx - px, cy - py);
            glVertex2i(cx + py, cy + px); glVertex2i(cx - py, cy + px);
            glVertex2i(cx + py, cy - px); glVertex2i(cx - py, cy - px);
            glEnd();
        } else {
            glBegin(GL_LINES);
            glVertex2i(cx - px, cy + py); glVertex2i(cx + px, cy + py);
            glVertex2i(cx - px, cy - py); glVertex2i(cx + px, cy - py);
            glVertex2i(cx - py, cy + px); glVertex2i(cx + py, cy + px);
            glVertex2i(cx - py, cy - px); glVertex2i(cx + py, cy - px);
            glEnd();
        }
    };

    plot8(xc, yc, x, y);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plot8(xc, yc, x, y);
    }
}

// Vẽ hình hộp bo góc (Rounded Rectangle)
void drawRoundedRect(float x, float y, float w, float h, float r, float red, float green, float blue, float alpha = 1.0f) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(red, green, blue, alpha);

    // Thân giữa
    glRectf(x + r, y, x + w - r, y + h);
    glRectf(x, y + r, x + w, y + h - r);

    // 4 góc bo tròn bằng Midpoint Circle
    drawMidpointCircle((int)(x + r), (int)(y + r), (int)r, red, green, blue, true);
    drawMidpointCircle((int)(x + w - r), (int)(y + r), (int)r, red, green, blue, true);
    drawMidpointCircle((int)(x + r), (int)(y + h - r), (int)r, red, green, blue, true);
    drawMidpointCircle((int)(x + w - r), (int)(y + h - r), (int)r, red, green, blue, true);
    glDisable(GL_BLEND);
}

// ============================================================================
// THUẬT TOÁN ĐƯỜNG CONG BÉZIER BẬC 2 VÀ BẬC 3 (SLIDE CG.5)
// ============================================================================

// 1. Bézier bậc 2 (Quadratic)
Point2D computeBezierQuadratic(const Point2D& P0, const Point2D& P1, const Point2D& P2, float t) {
    float u = 1.0f - t;
    float B0 = u * u;
    float B1 = 2.0f * t * u;
    float B2 = t * t;
    return Point2D(B0 * P0.x + B1 * P1.x + B2 * P2.x,
                   B0 * P0.y + B1 * P1.y + B2 * P2.y);
}

Point2D computeBezierTangentQuad(const Point2D& P0, const Point2D& P1, const Point2D& P2, float t) {
    float u = 1.0f - t;
    return Point2D(2.0f * u * (P1.x - P0.x) + 2.0f * t * (P2.x - P1.x),
                   2.0f * u * (P1.y - P0.y) + 2.0f * t * (P2.y - P1.y));
}

// 2. Bézier bậc 3 (Cubic Bézier - 4 điểm kiểm soát cho Tên lửa hành trình)
Point2D computeBezierCubic(const Point2D& P0, const Point2D& P1, const Point2D& P2, const Point2D& P3, float t) {
    float u = 1.0f - t;
    float B0 = u * u * u;
    float B1 = 3.0f * t * u * u;
    float B2 = 3.0f * t * t * u;
    float B3 = t * t * t;
    return Point2D(B0 * P0.x + B1 * P1.x + B2 * P2.x + B3 * P3.x,
                   B0 * P0.y + B1 * P1.y + B2 * P2.y + B3 * P3.y);
}

Point2D computeBezierTangentCubic(const Point2D& P0, const Point2D& P1, const Point2D& P2, const Point2D& P3, float t) {
    float u = 1.0f - t;
    return Point2D(3.0f * u * u * (P1.x - P0.x) + 6.0f * t * u * (P2.x - P1.x) + 3.0f * t * t * (P3.x - P2.x),
                   3.0f * u * u * (P1.y - P0.y) + 6.0f * t * u * (P2.y - P1.y) + 3.0f * t * t * (P3.y - P2.y));
}

// ============================================================================
// HỆ THỐNG ĐỊA HÌNH BIẾN DẠNG ĐƯỢC (DESTRUCTIBLE TERRAIN)
// ============================================================================
float terrainHeights[WINDOW_WIDTH];

void generateTerrain(int level) {
    float baseHeight = 135.0f;
    float mountainPeak = (level == 1) ? 370.0f : (level == 2 ? 435.0f : 495.0f);
    float mountainCenter = 640.0f;
    float mountainWidth = 200.0f;

    for (int x = 0; x < WINDOW_WIDTH; ++x) {
        float wave = 18.0f * sin(x * 0.008f) + 8.0f * cos(x * 0.022f);
        float dist = abs(x - mountainCenter);
        float mountain = 0.0f;
        if (dist < mountainWidth) {
            float f = (mountainWidth - dist) / mountainWidth;
            mountain = (mountainPeak - baseHeight) * (f * f * (3.0f - 2.0f * f));
        }
        terrainHeights[x] = baseHeight + wave + mountain;
    }
}

float getTerrainHeight(float x) {
    int ix = (int)x;
    if (ix < 0) return terrainHeights[0];
    if (ix >= WINDOW_WIDTH - 1) return terrainHeights[WINDOW_WIDTH - 1];
    float frac = x - ix;
    return terrainHeights[ix] * (1.0f - frac) + terrainHeights[ix + 1] * frac;
}

void applyTerrainCrater(float cx, float cy, float radius) {
    int minX = max(0, (int)(cx - radius));
    int maxX = min(WINDOW_WIDTH - 1, (int)(cx + radius));
    for (int x = minX; x <= maxX; ++x) {
        float dx = x - cx;
        if (dx * dx <= radius * radius) {
            float dy = sqrt(radius * radius - dx * dx);
            float craterBottom = cy - dy * 0.8f;
            if (craterBottom < terrainHeights[x]) {
                terrainHeights[x] = craterBottom;
            }
        }
    }
}

// ============================================================================
// VẬT PHẨM HÒM TIẾP TẾ RƠI DÙ (AIRDROP SUPPLY CRATE)
// ============================================================================
struct AirdropCrate {
    bool active;
    float x, y;
    float vy;
    float parachuteAngle;
    int rewardType; // 0: Hoi mau +35 HP, 1: Tang 1 Ten lua Cruise

    AirdropCrate() : active(false), x(0), y(0), vy(-35.0f), parachuteAngle(0), rewardType(0) {}

    void spawn(float spawnX) {
        active = true;
        x = spawnX;
        y = WINDOW_HEIGHT + 30.0f;
        vy = -38.0f;
        parachuteAngle = 0.0f;
        rewardType = rand() % 2;
    }

    void update(float dt) {
        if (!active) return;
        y += vy * dt;
        parachuteAngle += 2.0f * dt;

        float groundY = getTerrainHeight(x);
        if (y <= groundY + 14.0f) {
            y = groundY + 14.0f;
            vy = 0.0f; // Tiep dat nam tren mat dat cho nguoi choi nhat
        }
    }

    void draw() {
        if (!active) return;
        float groundY = getTerrainHeight(x);
        bool isLanded = (y <= groundY + 15.0f);

        // 1. Ve Du luon (neu dang roi)
        if (!isLanded) {
            float chuteR = 22.0f;
            float chuteY = y + 26.0f;

            // Vom du trang vien cam
            drawMidpointCircle((int)x, (int)chuteY, (int)chuteR, 0.95f, 0.5f, 0.1f, true);
            glColor3f(0.9f, 0.9f, 0.95f);
            drawMidpointCircle((int)(x - 10), (int)chuteY, 11, 0.9f, 0.9f, 0.95f, true);
            drawMidpointCircle((int)(x + 10), (int)chuteY, 11, 0.9f, 0.9f, 0.95f, true);

            // Day du
            glColor3f(0.8f, 0.8f, 0.8f);
            glLineWidth(1.2f);
            glBegin(GL_LINES);
                glVertex2f(x - chuteR, chuteY); glVertex2f(x, y + 10.0f);
                glVertex2f(x + chuteR, chuteY); glVertex2f(x, y + 10.0f);
            glEnd();
        }

        // 2. Ve Hom tiep te (Hop go vien kim loai + chu thap do)
        glColor3f(0.6f, 0.35f, 0.15f); // Go nau
        glRectf(x - 12.0f, y - 10.0f, x + 12.0f, y + 10.0f);
        glColor3f(0.2f, 0.2f, 0.2f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x - 12.0f, y - 10.0f); glVertex2f(x + 12.0f, y - 10.0f);
            glVertex2f(x + 12.0f, y + 10.0f); glVertex2f(x - 12.0f, y + 10.0f);
        glEnd();

        // Bieu tuong: Dau Cong Y Te hoac Icon Ten Lua
        if (rewardType == 0) {
            glColor3f(1.0f, 0.1f, 0.1f);
            glRectf(x - 2.5f, y - 7.0f, x + 2.5f, y + 7.0f);
            glRectf(x - 7.0f, y - 2.5f, x + 7.0f, y + 2.5f);
        } else {
            glColor3f(0.2f, 0.9f, 1.0f);
            glRectf(x - 3.0f, y - 7.0f, x + 3.0f, y + 5.0f);
            glBegin(GL_TRIANGLES);
                glVertex2f(x - 6.0f, y + 2.0f); glVertex2f(x + 6.0f, y + 2.0f); glVertex2f(x, y + 8.0f);
            glEnd();
        }

        drawText(x - 24.0f, y + 18.0f, "AIRDROP", 1.0f, 0.9f, 0.2f, GLUT_BITMAP_HELVETICA_10);
    }
};

AirdropCrate airdrop;

// ============================================================================
// HỆ THỐNG HẠT VẬT LÝ 2D (PARTICLE SYSTEM)
// ============================================================================
enum ParticleType { PARTICLE_SPARK, PARTICLE_SMOKE, PARTICLE_SHOCKWAVE, PARTICLE_SNOW };

struct Particle {
    float x, y, vx, vy;
    float r, g, b, a;
    float size;
    float life, maxLife;
    ParticleType type;
};

vector<Particle> particles;
vector<Particle> snowflakes;

void initSnow() {
    snowflakes.clear();
    for (int i = 0; i < 120; ++i) {
        Particle sn;
        sn.x = ((float)rand() / RAND_MAX) * WINDOW_WIDTH;
        sn.y = ((float)rand() / RAND_MAX) * WINDOW_HEIGHT;
        sn.vx = (((float)rand() / RAND_MAX) - 0.5f) * 20.0f;
        sn.vy = -30.0f - ((float)rand() / RAND_MAX) * 45.0f;
        sn.r = sn.g = sn.b = 0.95f;
        sn.a = 0.6f + ((float)rand() / RAND_MAX) * 0.35f;
        sn.size = 1.5f + ((float)rand() / RAND_MAX) * 2.0f;
        sn.life = sn.maxLife = 999.0f;
        sn.type = PARTICLE_SNOW;
        snowflakes.push_back(sn);
    }
}

void spawnExplosion(float x, float y, int count = 75, float maxSpeed = 240.0f) {
    Particle sw;
    sw.x = x; sw.y = y; sw.vx = sw.vy = 0.0f;
    sw.r = 1.0f; sw.g = 0.9f; sw.b = 0.4f; sw.a = 1.0f;
    sw.size = 5.0f; sw.life = sw.maxLife = 0.45f;
    sw.type = PARTICLE_SHOCKWAVE;
    particles.push_back(sw);

    for (int i = 0; i < count; ++i) {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float speed = 40.0f + ((float)rand() / RAND_MAX) * maxSpeed;
        Particle p;
        p.x = x; p.y = y;
        p.vx = cos(angle) * speed; p.vy = sin(angle) * speed;
        float pick = (float)rand() / RAND_MAX;
        if (pick < 0.35f) { p.r = 1.0f; p.g = 0.2f; p.b = 0.05f; }
        else if (pick < 0.75f) { p.r = 1.0f; p.g = 0.75f; p.b = 0.1f; }
        else { p.r = 1.0f; p.g = 0.98f; p.b = 0.4f; }
        p.a = 1.0f;
        p.size = 2.0f + ((float)rand() / RAND_MAX) * 3.5f;
        p.life = p.maxLife = 0.6f + ((float)rand() / RAND_MAX) * 0.75f;
        p.type = PARTICLE_SPARK;
        particles.push_back(p);
    }

    for (int i = 0; i < count / 2; ++i) {
        float angle = M_PI * 0.25f + ((float)rand() / RAND_MAX) * M_PI * 0.5f;
        float speed = 20.0f + ((float)rand() / RAND_MAX) * 85.0f;
        Particle p;
        p.x = x + (((float)rand() / RAND_MAX) - 0.5f) * 20.0f;
        p.y = y;
        p.vx = cos(angle) * speed; p.vy = sin(angle) * speed;
        float grey = 0.2f + ((float)rand() / RAND_MAX) * 0.25f;
        p.r = p.g = p.b = grey;
        p.a = 0.7f;
        p.size = 5.0f + ((float)rand() / RAND_MAX) * 9.0f;
        p.life = p.maxLife = 1.0f + ((float)rand() / RAND_MAX) * 1.0f;
        p.type = PARTICLE_SMOKE;
        particles.push_back(p);
    }
}

struct FloatingText {
    float x, y;
    string text;
    float r, g, b;
    float life, maxLife;
};
vector<FloatingText> floatingTexts;

void addFloatingText(float x, float y, const string& msg, float r, float g, float b) {
    FloatingText ft;
    ft.x = x; ft.y = y;
    ft.text = msg;
    ft.r = r; ft.g = g; ft.b = b;
    ft.life = ft.maxLife = 1.8f;
    floatingTexts.push_back(ft);
}

// ============================================================================
// ĐỐI TƯỢNG XE TĂNG (PLAYER 1 & PLAYER 2 / AI BOT TANK)
// ============================================================================
enum TankSide { SIDE_LEFT, SIDE_RIGHT };

// Kho dan dac biet cua xe tang
struct TankInventory {
    int ammoAP;      // Vo han (-1)
    int ammoMortar;  // Mac dinh 3
    int ammoCluster; // Mac dinh 2
    int ammoCruise;  // Mac dinh 1

    TankInventory() : ammoAP(-1), ammoMortar(3), ammoCluster(2), ammoCruise(1) {}
};

struct Tank {
    float x;
    float barrelAngleDeg;
    float shootPower;
    float barrelLength;
    float wheelRotationDeg;
    float recoilOffset;
    float hp, maxHp;
    TankSide side;
    bool isAI;
    float colorR, colorG, colorB;
    TankInventory inv;

    Tank(float initX = 160.0f, TankSide s = SIDE_LEFT, bool ai = false) {
        x = initX;
        side = s;
        isAI = ai;
        barrelAngleDeg = (side == SIDE_LEFT) ? 45.0f : 135.0f;
        shootPower = 65.0f;
        barrelLength = 48.0f;
        wheelRotationDeg = 0.0f;
        recoilOffset = 0.0f;
        maxHp = hp = 100.0f;
        if (side == SIDE_LEFT) {
            colorR = 0.28f; colorG = 0.44f; colorB = 0.26f;
        } else {
            colorR = 0.65f; colorG = 0.24f; colorB = 0.22f;
        }
    }

    float getSlopeAngleRad() const {
        float y1 = getTerrainHeight(x - 20.0f);
        float y2 = getTerrainHeight(x + 20.0f);
        return atan2(y2 - y1, 40.0f);
    }

    Point2D getPivot() const {
        float groundY = getTerrainHeight(x);
        float slopeRad = getSlopeAngleRad();
        float ox = -sin(slopeRad) * 22.0f;
        float oy =  cos(slopeRad) * 22.0f;
        return Point2D(x + ox, groundY + oy);
    }

    Point2D getMuzzle() const {
        Point2D pivot = getPivot();
        float slopeRad = getSlopeAngleRad();
        float totalAngleRad = slopeRad + barrelAngleDeg * M_PI / 180.0f;
        float actualLen = barrelLength - recoilOffset;
        return Point2D(pivot.x + actualLen * cos(totalAngleRad), pivot.y + actualLen * sin(totalAngleRad));
    }

    void getAABB(float& minX, float& minY, float& maxX, float& maxY) const {
        float gy = getTerrainHeight(x);
        minX = x - 35.0f; maxX = x + 35.0f;
        minY = gy; maxY = gy + 32.0f;
    }

    // Ve xe tang va thuoc do goc Protractor quanh thap phao
    void draw(bool isCurrentTurn = false) {
        if (hp <= 0.0f) {
            float gy = getTerrainHeight(x);
            glColor3f(0.18f, 0.15f, 0.15f);
            glRectf(x - 30.0f, gy, x + 30.0f, gy + 12.0f);
            return;
        }

        float groundY = getTerrainHeight(x);
        Point2D pivot = getPivot();
        float slopeRad = getSlopeAngleRad();

        // 1. THUOC DO GOC HINH BAN NGUYET (PROTRACTOR GAUGE QUANH KHOP PHAO)
        if (isCurrentTurn) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.2f, 0.85f, 1.0f, 0.15f);
            drawMidpointCircle((int)pivot.x, (int)pivot.y, 42, 0.2f, 0.85f, 1.0f, false);

            // Cac vach chia do tren cung
            glColor4f(0.3f, 0.95f, 1.0f, 0.6f);
            glLineWidth(1.0f);
            glBegin(GL_LINES);
            for (int deg = 15; deg <= 165; deg += 15) {
                float drad = deg * M_PI / 180.0f;
                float r1 = 38.0f, r2 = 44.0f;
                glVertex2f(pivot.x + r1 * cos(drad), pivot.y + r1 * sin(drad));
                glVertex2f(pivot.x + r2 * cos(drad), pivot.y + r2 * sin(drad));
            }
            glEnd();
            glDisable(GL_BLEND);
        }

        // 2. Ve nong phao Affine
        float totalBarrelRad = slopeRad + barrelAngleDeg * M_PI / 180.0f;
        Matrix3x3 barrelMatrix = Matrix3x3::rotationAround(pivot.x, pivot.y, totalBarrelRad);
        float blen = barrelLength - recoilOffset;
        Point2D bV[4] = {
            Point2D(pivot.x, pivot.y - 3.5f), Point2D(pivot.x + blen, pivot.y - 3.5f),
            Point2D(pivot.x + blen, pivot.y + 3.5f), Point2D(pivot.x, pivot.y + 3.5f)
        };

        glColor3f(0.35f, 0.42f, 0.48f);
        glBegin(GL_QUADS);
        for (int i = 0; i < 4; ++i) {
            Point2D p = barrelMatrix.transform(bV[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();

        // 3. Than xe tang nghieng theo doc
        Matrix3x3 bodyMatrix = Matrix3x3::rotationAround(x, groundY, slopeRad);
        Point2D hullV[4] = {
            Point2D(x - 32.0f, groundY + 14.0f), Point2D(x + 32.0f, groundY + 14.0f),
            Point2D(x + 24.0f, groundY + 24.0f), Point2D(x - 24.0f, groundY + 24.0f)
        };

        glColor3f(colorR, colorG, colorB);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 4; ++i) {
            Point2D p = bodyMatrix.transform(hullV[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();

        Point2D treadV[4] = {
            Point2D(x - 38.0f, groundY + 4.0f), Point2D(x + 38.0f, groundY + 4.0f),
            Point2D(x + 34.0f, groundY + 16.0f), Point2D(x - 34.0f, groundY + 16.0f)
        };
        glColor3f(0.2f, 0.22f, 0.22f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 4; ++i) {
            Point2D p = bodyMatrix.transform(treadV[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();

        // Banh xe Midpoint + nan hoa quay
        float wheelOffsets[4] = {-24.0f, -8.0f, 8.0f, 24.0f};
        for (int i = 0; i < 4; ++i) {
            Point2D wc = bodyMatrix.transform(Point2D(x + wheelOffsets[i], groundY + 7.0f));
            drawMidpointCircle((int)wc.x, (int)wc.y, 7, 0.15f, 0.15f, 0.15f, true);
            drawMidpointCircle((int)wc.x, (int)wc.y, 5, 0.5f, 0.55f, 0.5f, true);

            float wrad = wheelRotationDeg * M_PI / 180.0f;
            Matrix3x3 wheelRot = Matrix3x3::rotationAround(wc.x, wc.y, wrad);
            Point2D sp1 = wheelRot.transform(Point2D(wc.x - 6, wc.y));
            Point2D sp2 = wheelRot.transform(Point2D(wc.x + 6, wc.y));
            glColor3f(0.1f, 0.1f, 0.1f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
                glVertex2f(sp1.x, sp1.y); glVertex2f(sp2.x, sp2.y);
            glEnd();
        }

        // Thap phao ban nguyet
        drawMidpointCircle((int)pivot.x, (int)pivot.y, 11, colorR * 1.2f, colorG * 1.2f, colorB * 1.2f, true);
        drawMidpointCircle((int)pivot.x, (int)pivot.y, 11, 0.15f, 0.2f, 0.15f, false);
    }
};

// ============================================================================
// LÔ CỐT PHÒNG THỦ (TARGET BUNKER TRONG CHẾ ĐỘ CAMPAIGN)
// ============================================================================
struct TargetBunker {
    float x, width, height;
    float hp, maxHp;
    float radarAngleDeg;
    float hitFlashTimer;
    bool destroyed;

    TargetBunker() {
        x = 1060.0f; width = 72.0f; height = 46.0f;
        maxHp = hp = 100.0f; radarAngleDeg = 0.0f; hitFlashTimer = 0.0f; destroyed = false;
    }

    void reset(float newX, float newHp = 100.0f) {
        x = newX; maxHp = hp = newHp; radarAngleDeg = 0.0f; hitFlashTimer = 0.0f; destroyed = false;
    }

    void getAABB(float& minX, float& minY, float& maxX, float& maxY) const {
        float gy = getTerrainHeight(x);
        minX = x - width * 0.5f; maxX = x + width * 0.5f;
        minY = gy; maxY = gy + height;
    }

    void draw(bool showDebugAABB = false) {
        float gy = getTerrainHeight(x);
        float minX, minY, maxX, maxY;
        getAABB(minX, minY, maxX, maxY);

        if (showDebugAABB) {
            glColor3f(0.2f, 1.0f, 0.4f);
            glLineWidth(1.5f);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(2, 0xAAAA);
            glBegin(GL_LINE_LOOP);
                glVertex2f(minX, minY); glVertex2f(maxX, minY);
                glVertex2f(maxX, maxY); glVertex2f(minX, maxY);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
            drawText(minX - 8.0f, maxY + 38.0f, "Target AABB Hitbox", 0.3f, 1.0f, 0.4f, GLUT_BITMAP_HELVETICA_10);
        }

        if (destroyed) {
            glColor3f(0.18f, 0.15f, 0.15f);
            glRectf(minX - 5.0f, gy, maxX + 5.0f, gy + 12.0f);
            return;
        }

        if (hitFlashTimer > 0.0f) glColor3f(1.0f, 0.3f, 0.3f);
        else glColor3f(0.48f, 0.5f, 0.55f);

        glBegin(GL_POLYGON);
            glVertex2f(minX, gy); glVertex2f(maxX, gy);
            glVertex2f(maxX - 10.0f, maxY); glVertex2f(minX + 10.0f, maxY);
        glEnd();

        glColor3f(0.1f, 0.12f, 0.15f);
        glRectf(x - 18.0f, gy + 16.0f, x + 18.0f, gy + 24.0f);

        // Anten radar Affine
        float radarBaseX = x + 12.0f, radarBaseY = maxY + 8.0f;
        glColor3f(0.3f, 0.35f, 0.4f);
        glRectf(radarBaseX - 3.0f, maxY, radarBaseX + 3.0f, radarBaseY);

        float rrad = radarAngleDeg * M_PI / 180.0f;
        Matrix3x3 radarMat = Matrix3x3::rotationAround(radarBaseX, radarBaseY, rrad);
        Point2D dishPts[3] = {
            Point2D(radarBaseX - 12.0f, radarBaseY + 6.0f),
            Point2D(radarBaseX, radarBaseY - 2.0f),
            Point2D(radarBaseX + 12.0f, radarBaseY + 6.0f)
        };
        glColor3f(0.2f, 0.85f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < 3; ++i) {
            Point2D p = radarMat.transform(dishPts[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }
};

// ============================================================================
// ĐỐI TƯỢNG ĐẠN PHÁO & TÊN LỬA HÀNH TRÌNH BÉZIER (PROJECTILE)
// ============================================================================
enum WeaponType {
    WEAPON_STANDARD,   // Đạn AP tiêu chuẩn (Bézier bậc 2)
    WEAPON_MORTAR,     // Pháo cối vòng cung cao (Bézier bậc 2)
    WEAPON_CLUSTER,    // Đạn chùm tách 3 bom con (Bézier bậc 2)
    WEAPON_CRUISE      // Tên lửa hành trình uốn lượn S-Curve (CUBIC BÉZIER BẬC 3!)
};

struct SubBomb {
    bool active;
    Point2D P0, P1, P2;
    float t, speed;
    Point2D currentPos, tangent;
};

struct Projectile {
    bool active;
    WeaponType type;
    TankSide firedBy;

    Point2D P0, P1, P2, P3;
    float t, speed;
    Point2D currentPos, tangent;
    bool hasSplit;
    vector<Point2D> smokeTrail;
    vector<SubBomb> subBombs;

    Projectile() : active(false), t(0.0f), speed(0.85f), hasSplit(false) {}
};

// ============================================================================
// QUẢN LÝ TRẠNG THÁI GAME & MENU (GAME MANAGER & STATE MACHINE)
// ============================================================================
enum GameMode {
    MODE_CAMPAIGN, // Người chơi vs Lô cốt phòng thủ
    MODE_PVP,      // Đối kháng 2 người chơi luân phiên (Turn-based)
    MODE_AI_BOT    // Người chơi vs AI Bot thông minh giải ngược Bézier
};

enum GameState {
    STATE_MENU,         // Màn hình Menu mở đầu game
    STATE_AIMING,       // Đang ngắm bắn trong trận
    STATE_CHARGING,     // Giữ nạp lực
    STATE_FLYING,       // Đạn đang bay
    STATE_AI_THINKING,  // Lượt AI đang suy nghĩ
    STATE_ROUND_CLEAR,  // Tiêu diệt mục tiêu thành công
    STATE_GAME_OVER     // Hết đạn / Thua trận
};

GameState gameState = STATE_MENU; // Mặc định mở Menu trước
GameMode gameMode = MODE_CAMPAIGN;
WeaponType currentWeapon = WEAPON_STANDARD;
TankSide currentTurn = SIDE_LEFT;

Tank playerTank(160.0f, SIDE_LEFT, false);
Tank enemyTank(1060.0f, SIDE_RIGHT, false);
TargetBunker enemyBunker;
Projectile bullet;

float windForce = 4.0f;
int score = 0;
int currentLevel = 1;
float aiThinkTimer = 0.0f;
float airdropTimer = 8.0f; // Thả dù tiếp tế sau mỗi 8-12 giây

// Cờ tùy chọn giao diện
bool showDebugHUD = false;       // Bảng toán học (TAB)
bool showControlPolygon = true;  // Bao lồi & Đa giác kiểm soát
bool isChargingForward = true;
float screenShakeTimer = 0.0f;
bool isNightMode = false;
bool isSnowing = false;

// Mây trôi
struct Cloud { float x, y, speed, scale; };
vector<Cloud> clouds;

void initClouds() {
    clouds.clear();
    for (int i = 0; i < 6; ++i) {
        Cloud c;
        c.x = ((float)rand() / RAND_MAX) * WINDOW_WIDTH;
        c.y = 510.0f + ((float)rand() / RAND_MAX) * 150.0f;
        c.speed = 10.0f + ((float)rand() / RAND_MAX) * 25.0f;
        c.scale = 0.7f + ((float)rand() / RAND_MAX) * 0.6f;
        clouds.push_back(c);
    }
}

// ============================================================================
// TÍNH TOÁN CÁC ĐIỂM KIỂM SOÁT BÉZIER BẬC 2 VÀ BẬC 3
// ============================================================================
void calculateControlPointsForTank(const Tank& tk, WeaponType wtype, Point2D& outP0, Point2D& outP1, Point2D& outP2, Point2D& outP3) {
    outP0 = tk.getMuzzle();
    float slopeRad = tk.getSlopeAngleRad();
    float totalAngleRad = slopeRad + tk.barrelAngleDeg * M_PI / 180.0f;
    float powerRatio = tk.shootPower / 100.0f;

    if (wtype == WEAPON_CRUISE) {
        // CUBIC BÉZIER (4 ĐIỂM KIỂM SOÁT):
        float forwardSign = (tk.side == SIDE_LEFT) ? 1.0f : -1.0f;
        float dist = 450.0f + powerRatio * 650.0f;

        outP1.x = outP0.x + dist * 0.35f * cos(totalAngleRad) + windForce * 15.0f;
        outP1.y = outP0.y + dist * 0.45f * sin(totalAngleRad) + 120.0f;

        outP2.x = outP0.x + forwardSign * dist * 0.7f + windForce * 25.0f;
        outP2.y = 260.0f - powerRatio * 50.0f; // Luồn thấp qua khe núi

        float landX = outP0.x + forwardSign * dist + windForce * 35.0f;
        landX = max(0.0f, min((float)WINDOW_WIDTH, landX));
        outP3 = Point2D(landX, getTerrainHeight(landX));
        outP2 = Point2D(outP2.x, max(getTerrainHeight(outP2.x) + 40.0f, outP2.y));
    } else {
        // QUADRATIC BÉZIER (3 ĐIỂM KIỂM SOÁT):
        float baseDist = 380.0f + powerRatio * 620.0f;
        if (wtype == WEAPON_MORTAR) baseDist *= 0.82f;

        outP1.x = outP0.x + baseDist * cos(totalAngleRad) + windForce * 22.0f;
        outP1.y = outP0.y + baseDist * sin(totalAngleRad) * (wtype == WEAPON_MORTAR ? 1.45f : 1.0f);

        float landingX = outP0.x + 2.0f * (outP1.x - outP0.x) + windForce * 35.0f;
        landingX = max(0.0f, min((float)WINDOW_WIDTH, landingX));
        outP2 = Point2D(landingX, getTerrainHeight(landingX));
        outP3 = outP2;
    }
}

// ============================================================================
// TRÍ TUỆ NHÂN TẠO (AI ARTILLERY BOT) - GIẢI NGƯỢC BÉZIER (INVERSE SOLVER)
// ============================================================================
void aiComputeShot(const Tank& bot, Point2D targetPos, float& outAngle, float& outPower) {
    float bestAngle = 135.0f;
    float bestPower = 60.0f;
    float minError = 99999.0f;

    for (float angle = 110.0f; angle <= 165.0f; angle += 3.0f) {
        Tank simBot = bot;
        simBot.barrelAngleDeg = angle;
        float lowP = 25.0f, highP = 100.0f;
        for (int iter = 0; iter < 12; ++iter) {
            float midP = (lowP + highP) * 0.5f;
            simBot.shootPower = midP;
            Point2D P0, P1, P2, P3;
            calculateControlPointsForTank(simBot, WEAPON_STANDARD, P0, P1, P2, P3);

            float err = abs(P2.x - targetPos.x);
            if (err < minError) {
                minError = err;
                bestAngle = angle;
                bestPower = midP;
            }
            if (P2.x > targetPos.x) lowP = midP;
            else highP = midP;
        }
    }

    float inaccuracy = (((float)rand() / RAND_MAX) - 0.5f) * 4.0f;
    outAngle = max(95.0f, min(175.0f, bestAngle + inaccuracy));
    outPower = max(20.0f, min(100.0f, bestPower + inaccuracy));
}

// Bắn pháo
void fireCannon() {
    if (bullet.active) return;

    Tank& activeTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
    if (activeTank.hp <= 0.0f) return;

    // Kiem tra so luong dan dac biet
    if (currentWeapon == WEAPON_MORTAR) {
        if (activeTank.inv.ammoMortar <= 0) {
            addFloatingText(activeTank.x, getTerrainHeight(activeTank.x) + 40.0f, "OUT OF MORTAR!", 1.0f, 0.4f, 0.4f);
            currentWeapon = WEAPON_STANDARD;
            return;
        }
        activeTank.inv.ammoMortar--;
    } else if (currentWeapon == WEAPON_CLUSTER) {
        if (activeTank.inv.ammoCluster <= 0) {
            addFloatingText(activeTank.x, getTerrainHeight(activeTank.x) + 40.0f, "OUT OF CLUSTER!", 1.0f, 0.4f, 0.4f);
            currentWeapon = WEAPON_STANDARD;
            return;
        }
        activeTank.inv.ammoCluster--;
    } else if (currentWeapon == WEAPON_CRUISE) {
        if (activeTank.inv.ammoCruise <= 0) {
            addFloatingText(activeTank.x, getTerrainHeight(activeTank.x) + 40.0f, "OUT OF CRUISE!", 1.0f, 0.4f, 0.4f);
            currentWeapon = WEAPON_STANDARD;
            return;
        }
        activeTank.inv.ammoCruise--;
    }

    activeTank.recoilOffset = 12.0f;

    Point2D P0, P1, P2, P3;
    calculateControlPointsForTank(activeTank, currentWeapon, P0, P1, P2, P3);

    bullet.active = true;
    bullet.type = currentWeapon;
    bullet.firedBy = currentTurn;
    bullet.P0 = P0; bullet.P1 = P1; bullet.P2 = P2; bullet.P3 = P3;
    bullet.t = 0.0f;
    bullet.speed = (currentWeapon == WEAPON_MORTAR) ? 0.72f : (currentWeapon == WEAPON_CRUISE ? 0.65f : 0.95f);
    bullet.currentPos = P0;
    bullet.tangent = (currentWeapon == WEAPON_CRUISE) ? computeBezierTangentCubic(P0, P1, P2, P3, 0.0f)
                                                      : computeBezierTangentQuad(P0, P1, P2, 0.0f);
    bullet.hasSplit = false;
    bullet.smokeTrail.clear();
    bullet.subBombs.clear();

    spawnExplosion(P0.x, P0.y, 25, 90.0f);
    gameState = STATE_FLYING;
}

// ============================================================================
// VẼ ĐƯỜNG NGẮM DỰ KIẾN (BÉZIER TRAJECTORY PREVIEW)
// ============================================================================
void drawTrajectoryPreview() {
    if (bullet.active) return;
    if (gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT) return;

    Tank& activeTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
    Point2D P0, P1, P2, P3;
    calculateControlPointsForTank(activeTank, currentWeapon, P0, P1, P2, P3);

    // 1. Đa giác kiểm soát & Bao lồi
    if (showControlPolygon) {
        if (currentWeapon == WEAPON_CRUISE) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.2f, 0.8f, 1.0f, 0.07f);
            glBegin(GL_POLYGON);
                glVertex2f(P0.x, P0.y); glVertex2f(P1.x, P1.y);
                glVertex2f(P2.x, P2.y); glVertex2f(P3.x, P3.y);
            glEnd();
            glDisable(GL_BLEND);

            glColor3f(0.95f, 0.6f, 0.15f);
            glLineWidth(1.5f);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(3, 0xAAAA);
            glBegin(GL_LINE_STRIP);
                glVertex2f(P0.x, P0.y); glVertex2f(P1.x, P1.y);
                glVertex2f(P2.x, P2.y); glVertex2f(P3.x, P3.y);
            glEnd();
            glDisable(GL_LINE_STIPPLE);

            drawMidpointCircle((int)P0.x, (int)P0.y, 5, 0.2f, 1.0f, 0.3f, true);
            drawMidpointCircle((int)P1.x, (int)P1.y, 6, 1.0f, 0.85f, 0.1f, true);
            drawMidpointCircle((int)P2.x, (int)P2.y, 6, 0.2f, 0.9f, 1.0f, true);
            drawMidpointCircle((int)P3.x, (int)P3.y, 5, 1.0f, 0.25f, 0.25f, true);
        } else {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.2f, 0.8f, 1.0f, 0.07f);
            glBegin(GL_TRIANGLES);
                glVertex2f(P0.x, P0.y); glVertex2f(P1.x, P1.y); glVertex2f(P2.x, P2.y);
            glEnd();
            glDisable(GL_BLEND);

            glColor3f(0.95f, 0.6f, 0.15f);
            glLineWidth(1.5f);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(3, 0xAAAA);
            glBegin(GL_LINES);
                glVertex2f(P0.x, P0.y); glVertex2f(P1.x, P1.y);
                glVertex2f(P1.x, P1.y); glVertex2f(P2.x, P2.y);
            glEnd();
            glDisable(GL_LINE_STIPPLE);

            drawMidpointCircle((int)P0.x, (int)P0.y, 5, 0.2f, 1.0f, 0.3f, true);
            drawMidpointCircle((int)P1.x, (int)P1.y, 6, 1.0f, 0.85f, 0.1f, true);
            drawMidpointCircle((int)P2.x, (int)P2.y, 5, 1.0f, 0.25f, 0.25f, true);
        }
    }

    // 2. Duong ngam cham bi dong
    const int SAMPLES = 65;
    static float animOffset = 0.0f;
    animOffset += 0.05f;
    if (animOffset > 1.0f) animOffset -= 1.0f;

    for (int i = 0; i < SAMPLES; ++i) {
        float t = (float)i / (float)SAMPLES;
        Point2D pt = (currentWeapon == WEAPON_CRUISE) ? computeBezierCubic(P0, P1, P2, P3, t)
                                                      : computeBezierQuadratic(P0, P1, P2, t);
        float pulse = sin((t * 20.0f) - animOffset * 6.28f);
        if (pulse > 0.0f) {
            float dotSize = 2.0f + 2.5f * (1.0f - t);
            if (currentWeapon == WEAPON_CRUISE) {
                drawMidpointCircle((int)pt.x, (int)pt.y, (int)dotSize, 0.2f, 1.0f, 0.8f, true);
            } else {
                drawMidpointCircle((int)pt.x, (int)pt.y, (int)dotSize, 0.2f, 0.95f, 1.0f, true);
            }
        }
    }
}

// ============================================================================
// VẼ MÔI TRƯỜNG & CHIẾU SÁNG ĐỘNG
// ============================================================================
void drawBackground() {
    if (!isNightMode) {
        glBegin(GL_QUADS);
            glColor3f(0.06f, 0.09f, 0.18f);
            glVertex2i(0, WINDOW_HEIGHT); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
            glColor3f(0.28f, 0.22f, 0.38f);
            glVertex2i(WINDOW_WIDTH, 140); glVertex2i(0, 140);
        glEnd();

        drawMidpointCircle(920, 480, 50, 1.0f, 0.65f, 0.25f, true);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawMidpointCircle(920, 480, 68, 1.0f, 0.5f, 0.1f, false);
        drawMidpointCircle(920, 480, 85, 1.0f, 0.4f, 0.05f, false);
        glDisable(GL_BLEND);
    } else {
        glBegin(GL_QUADS);
            glColor3f(0.01f, 0.02f, 0.05f);
            glVertex2i(0, WINDOW_HEIGHT); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
            glColor3f(0.08f, 0.07f, 0.15f);
            glVertex2i(WINDOW_WIDTH, 140); glVertex2i(0, 140);
        glEnd();

        drawMidpointCircle(920, 520, 42, 0.9f, 0.95f, 1.0f, true);
        drawMidpointCircle(905, 530, 36, 0.05f, 0.06f, 0.12f, true);

        glPointSize(1.5f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 60; ++i) {
            float sx = (i * 123) % WINDOW_WIDTH;
            float sy = 400 + (i * 77) % 300;
            float bright = 0.5f + 0.5f * sin(i + bullet.t * 5.0f);
            glColor3f(bright, bright, bright * 1.2f);
            glVertex2f(sx, sy);
        }
        glEnd();
    }

    for (const auto& c : clouds) {
        float r = 24.0f * c.scale;
        float grey = isNightMode ? 0.25f : 0.85f;
        drawMidpointCircle((int)(c.x), (int)(c.y), (int)(r), grey, grey, grey * 1.05f, true);
        drawMidpointCircle((int)(c.x - r * 0.8f), (int)(c.y - r * 0.2f), (int)(r * 0.75f), grey, grey, grey * 1.05f, true);
        drawMidpointCircle((int)(c.x + r * 0.8f), (int)(c.y - r * 0.1f), (int)(r * 0.8f), grey, grey, grey * 1.05f, true);
    }

    float mCol = isNightMode ? 0.07f : 0.2f;
    glColor3f(mCol, mCol * 0.9f, mCol * 1.3f);
    glBegin(GL_POLYGON);
        glVertex2i(0, 140); glVertex2i(220, 310); glVertex2i(480, 180);
        glVertex2i(700, 380); glVertex2i(960, 220); glVertex2i(1200, 340);
        glVertex2i(WINDOW_WIDTH, 140);
    glEnd();
}

void drawTerrain() {
    Point2D lightPos = bullet.active ? bullet.currentPos : Point2D(-1000, -1000);
    float lightIntensity = bullet.active ? 1.0f : 0.0f;

    glBegin(GL_QUAD_STRIP);
    for (int x = 0; x < WINDOW_WIDTH; x += 3) {
        float gy = terrainHeights[x];

        float baseR = isNightMode ? 0.12f : 0.38f;
        float baseG = isNightMode ? 0.08f : 0.24f;
        float baseB = isNightMode ? 0.06f : 0.18f;

        if (bullet.active) {
            float dist = sqrt((x - lightPos.x) * (x - lightPos.x) + (gy - lightPos.y) * (gy - lightPos.y));
            float attenuation = 1.0f / (1.0f + 0.00015f * dist * dist);
            float lightAdd = lightIntensity * attenuation * 0.85f;
            baseR = min(1.0f, baseR + lightAdd * 1.0f);
            baseG = min(1.0f, baseG + lightAdd * 0.85f);
            baseB = min(1.0f, baseB + lightAdd * 0.3f);
        }

        glColor3f(baseR, baseG, baseB);
        glVertex2f((float)x, gy);
        glColor3f(baseR * 0.3f, baseG * 0.3f, baseB * 0.3f);
        glVertex2f((float)x, 0.0f);
    }
    glEnd();

    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for (int x = 0; x < WINDOW_WIDTH; x += 2) {
        float grassG = isNightMode ? 0.3f : 0.62f;
        glColor3f(0.12f, grassG, 0.12f);
        glVertex2f((float)x, terrainHeights[x]);
    }
    glEnd();
}

// ============================================================================
// VẼ ĐẠN PHÁO & TÊN LỬA HÀNH TRÌNH BÉZIER
// ============================================================================
void drawSingleBullet(const Point2D& pos, const Point2D& tan, float scale = 1.0f, bool isCruise = false) {
    float bulletAngleRad = atan2(tan.y, tan.x);
    Matrix3x3 bulletMat = Matrix3x3::rotationAround(pos.x, pos.y, bulletAngleRad);
    float bx = pos.x, by = pos.y;

    if (isCruise) {
        Point2D mPts[6] = {
            Point2D(bx - 14.0f * scale, by - 4.0f * scale),
            Point2D(bx + 6.0f * scale, by - 4.0f * scale),
            Point2D(bx + 16.0f * scale, by),
            Point2D(bx + 6.0f * scale, by + 4.0f * scale),
            Point2D(bx - 14.0f * scale, by + 4.0f * scale),
            Point2D(bx - 10.0f * scale, by)
        };
        glColor3f(0.9f, 0.95f, 1.0f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 6; ++i) {
            Point2D p = bulletMat.transform(mPts[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();

        Point2D fin1 = bulletMat.transform(Point2D(bx - 12.0f * scale, by - 9.0f * scale));
        Point2D fin2 = bulletMat.transform(Point2D(bx - 12.0f * scale, by + 9.0f * scale));
        Point2D root1 = bulletMat.transform(Point2D(bx - 6.0f * scale, by));
        glColor3f(0.2f, 0.8f, 1.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(fin1.x, fin1.y); glVertex2f(root1.x, root1.y); glVertex2f(bx - 14.0f * scale, by);
            glVertex2f(fin2.x, fin2.y); glVertex2f(root1.x, root1.y); glVertex2f(bx - 14.0f * scale, by);
        glEnd();

        Point2D flamePos = bulletMat.transform(Point2D(bx - 16.0f * scale, by));
        drawMidpointCircle((int)flamePos.x, (int)flamePos.y, (int)(5 * scale), 0.2f, 0.95f, 1.0f, true);
    } else {
        Point2D bPts[4] = {
            Point2D(bx - 9.0f * scale, by - 3.5f * scale), Point2D(bx + 4.0f * scale, by - 3.5f * scale),
            Point2D(bx + 11.0f * scale, by), Point2D(bx + 4.0f * scale, by + 3.5f * scale)
        };
        glColor3f(1.0f, 0.9f, 0.2f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 4; ++i) {
            Point2D p = bulletMat.transform(bPts[i]);
            glVertex2f(p.x, p.y);
        }
        glEnd();

        Point2D flamePos = bulletMat.transform(Point2D(bx - 10.0f * scale, by));
        drawMidpointCircle((int)flamePos.x, (int)flamePos.y, (int)(4 * scale), 1.0f, 0.3f, 0.05f, true);
    }
}

void drawProjectile() {
    if (!bullet.active) return;

    if (bullet.smokeTrail.size() >= 2) {
        glLineWidth(2.5f);
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < bullet.smokeTrail.size(); ++i) {
            float alpha = (float)i / (float)bullet.smokeTrail.size();
            if (bullet.type == WEAPON_CRUISE) glColor4f(0.2f, 0.85f, 1.0f, alpha * 0.85f);
            else glColor4f(0.9f, 0.7f, 0.3f, alpha * 0.8f);
            glVertex2f(bullet.smokeTrail[i].x, bullet.smokeTrail[i].y);
        }
        glEnd();
    }

    if (!bullet.hasSplit) {
        drawSingleBullet(bullet.currentPos, bullet.tangent, 1.0f, (bullet.type == WEAPON_CRUISE));
    }

    for (const auto& sb : bullet.subBombs) {
        if (sb.active) drawSingleBullet(sb.currentPos, sb.tangent, 0.65f, false);
    }
}

// ============================================================================
// GIAO DIỆN START MENU (MÀN HÌNH MỞ ĐẦU CHỌN CHẾ ĐỘ)
// ============================================================================
void drawStartMenu() {
    drawBackground();
    drawTerrain();

    // Khung phông mờ trung tâm
    float mw = 680.0f, mh = 480.0f;
    float mx = (WINDOW_WIDTH - mw) * 0.5f;
    float my = (WINDOW_HEIGHT - mh) * 0.5f;

    drawRoundedRect(mx, my, mw, mh, 16.0f, 0.04f, 0.07f, 0.12f, 0.93f);

    glColor3f(0.2f, 0.7f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(mx, my); glVertex2f(mx + mw, my);
        glVertex2f(mx + mw, my + mh); glVertex2f(mx, my + mh);
    glEnd();

    // Tieu de Game
    drawText(mx + 65, my + mh - 55, "ARTILLERY BATTLE: BEZIER 2D", 0.2f, 0.95f, 1.0f, GLUT_BITMAP_HELVETICA_18);
    drawText(mx + 115, my + mh - 85, "Do An Do Hoa May Tinh (Computer Graphics) - VKU", 0.7f, 0.8f, 0.9f, GLUT_BITMAP_9_BY_15);

    // Cac nut chon che do choi
    auto drawMenuButton = [&](float bx, float by, float bw, float bh, const string& keyStr, const string& title, const string& desc, float br, float bg, float bb) {
        drawRoundedRect(bx, by, bw, bh, 8.0f, 0.08f, 0.12f, 0.2f, 0.9f);
        glColor3f(br, bg, bb);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx, by); glVertex2f(bx + bw, by);
            glVertex2f(bx + bw, by + bh); glVertex2f(bx, by + bh);
        glEnd();
        drawText(bx + 18, by + bh - 24, keyStr, br, bg, bb, GLUT_BITMAP_HELVETICA_18);
        drawText(bx + 85, by + bh - 24, title, 1.0f, 1.0f, 1.0f, GLUT_BITMAP_9_BY_15);
        drawText(bx + 85, by + 12, desc, 0.7f, 0.75f, 0.8f, GLUT_BITMAP_HELVETICA_10);
    };

    float btnW = 560.0f, btnH = 55.0f;
    float btnX = mx + (mw - btnW) * 0.5f;

    drawMenuButton(btnX, my + 265, btnW, btnH, "[ 1 ]", "CHE DO CHIEN DICH (CAMPAIGN)", "Vuot nui ban pha lo cot phong thu, vuot qua cac cap do gio bao.", 0.2f, 0.9f, 1.0f);
    drawMenuButton(btnX, my + 195, btnW, btnH, "[ 2 ]", "DAU VOI AI BOT (INVERSE SOLVER)", "Doi dau voi AI Bot tu dong giai nguoc phuong trinh Bezier de ban tra.", 1.0f, 0.85f, 0.2f);
    drawMenuButton(btnX, my + 125, btnW, btnH, "[ 3 ]", "DOI KHANG 2 NGUOI (1v1 LOCAL DUEL)", "2 nguoi choi luan phien doi luot, dia hinh bi tan pha duoi chan xich.", 0.3f, 1.0f, 0.5f);
    drawMenuButton(btnX, my + 55,  btnW, btnH, "[ H ]", "HUONG DAN & CONG THUC DO HOA (HELP)", "Xem giai thich toan hoc duong cong Bezier bac 2, bac 3 va bien doi Affine.", 0.9f, 0.4f, 1.0f);

    drawText(mx + 130, my + 20, "Bam phim so 1, 2, 3 hoac Click chuot de bat dau choi!", 0.85f, 0.9f, 0.95f, GLUT_BITMAP_9_BY_15);
}

// ============================================================================
// GIAO DIỆN CHIẾN ĐẤU (BATTLE HUD & INVENTORY DOCK)
// ============================================================================
void drawBattleHUD() {
    // 1. THANH THÔNG TIN TRÊN CÙNG (TOP HEADER STATUS)
    drawRoundedRect(10, WINDOW_HEIGHT - 60, WINDOW_WIDTH - 20, 52, 10.0f, 0.05f, 0.08f, 0.14f, 0.92f);
    glColor3f(0.2f, 0.4f, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(10, WINDOW_HEIGHT - 60); glVertex2f(WINDOW_WIDTH - 10, WINDOW_HEIGHT - 60);
        glVertex2f(WINDOW_WIDTH - 10, WINDOW_HEIGHT - 8); glVertex2f(10, WINDOW_HEIGHT - 8);
    glEnd();

    // Avatar P1 (Xe Xanh)
    drawRoundedRect(22, WINDOW_HEIGHT - 54, 180, 40, 6.0f, 0.1f, 0.18f, 0.12f, 0.85f);
    drawText(32, WINDOW_HEIGHT - 32, "P1 (XANH)", 0.3f, 1.0f, 0.5f, GLUT_BITMAP_HELVETICA_12);
    // Thanh mau P1
    float p1HpRatio = max(0.0f, playerTank.hp / playerTank.maxHp);
    glColor3f(0.1f, 0.1f, 0.1f);
    glRectf(32, WINDOW_HEIGHT - 48, 190, WINDOW_HEIGHT - 38);
    glColor3f((p1HpRatio > 0.5f ? 2.0f * (1.0f - p1HpRatio) : 1.0f), (p1HpRatio > 0.5f ? 1.0f : 2.0f * p1HpRatio), 0.1f);
    glRectf(32, WINDOW_HEIGHT - 48, 32 + 158 * p1HpRatio, WINDOW_HEIGHT - 38);
    stringstream ssP1Hp; ssP1Hp << (int)playerTank.hp << "/100 HP";
    drawText(110, WINDOW_HEIGHT - 47, ssP1Hp.str(), 1.0f, 1.0f, 1.0f, GLUT_BITMAP_HELVETICA_10);

    // Luot choi hien tai (Turn Badge)
    float badgeX = 230.0f;
    stringstream ssTurn;
    ssTurn << "LUOT: " << (currentTurn == SIDE_LEFT ? "P1 (XANH)" : (gameMode == MODE_AI_BOT ? "AI BOT (DO)" : "P2 (DO)"));
    drawText(badgeX, WINDOW_HEIGHT - 35, ssTurn.str(), (currentTurn == SIDE_LEFT ? 0.3f : 1.0f), (currentTurn == SIDE_LEFT ? 1.0f : 0.3f), 0.3f, GLUT_BITMAP_HELVETICA_18);

    // La ban gio quan su (Wind Indicator)
    float windBoxX = 540.0f;
    drawRoundedRect(windBoxX, WINDOW_HEIGHT - 54, 200, 40, 6.0f, 0.08f, 0.12f, 0.2f, 0.85f);
    stringstream ssWind;
    ssWind << "GIO: " << fixed << setprecision(1) << abs(windForce) << " m/s " << (windForce >= 0 ? ">>>" : "<<<");
    drawText(windBoxX + 15, WINDOW_HEIGHT - 35, ssWind.str(), (windForce >= 0 ? 0.3f : 1.0f), 0.9f, 1.0f, GLUT_BITMAP_9_BY_15);

    // Avatar P2 hoac Lo Cot dich
    float p2BoxX = WINDOW_WIDTH - 205;
    drawRoundedRect(p2BoxX, WINDOW_HEIGHT - 54, 185, 40, 6.0f, 0.2f, 0.1f, 0.1f, 0.85f);
    if (gameMode == MODE_CAMPAIGN) {
        drawText(p2BoxX + 10, WINDOW_HEIGHT - 32, "LO COT DICH", 1.0f, 0.4f, 0.4f, GLUT_BITMAP_HELVETICA_12);
        float bkHpRatio = max(0.0f, enemyBunker.hp / enemyBunker.maxHp);
        glColor3f(0.1f, 0.1f, 0.1f);
        glRectf(p2BoxX + 10, WINDOW_HEIGHT - 48, p2BoxX + 175, WINDOW_HEIGHT - 38);
        glColor3f((bkHpRatio > 0.5f ? 2.0f * (1.0f - bkHpRatio) : 1.0f), (bkHpRatio > 0.5f ? 1.0f : 2.0f * bkHpRatio), 0.1f);
        glRectf(p2BoxX + 10, WINDOW_HEIGHT - 48, p2BoxX + 10 + 165 * bkHpRatio, WINDOW_HEIGHT - 38);
        stringstream ssBkHp; ssBkHp << (int)enemyBunker.hp << "/100 HP";
        drawText(p2BoxX + 90, WINDOW_HEIGHT - 47, ssBkHp.str(), 1.0f, 1.0f, 1.0f, GLUT_BITMAP_HELVETICA_10);
    } else {
        drawText(p2BoxX + 10, WINDOW_HEIGHT - 32, (gameMode == MODE_AI_BOT ? "AI BOT (DO)" : "P2 (DO)"), 1.0f, 0.4f, 0.4f, GLUT_BITMAP_HELVETICA_12);
        float p2HpRatio = max(0.0f, enemyTank.hp / enemyTank.maxHp);
        glColor3f(0.1f, 0.1f, 0.1f);
        glRectf(p2BoxX + 10, WINDOW_HEIGHT - 48, p2BoxX + 175, WINDOW_HEIGHT - 38);
        glColor3f((p2HpRatio > 0.5f ? 2.0f * (1.0f - p2HpRatio) : 1.0f), (p2HpRatio > 0.5f ? 1.0f : 2.0f * p2HpRatio), 0.1f);
        glRectf(p2BoxX + 10, WINDOW_HEIGHT - 48, p2BoxX + 10 + 165 * p2HpRatio, WINDOW_HEIGHT - 38);
        stringstream ssP2Hp; ssP2Hp << (int)enemyTank.hp << "/100 HP";
        drawText(p2BoxX + 90, WINDOW_HEIGHT - 47, ssP2Hp.str(), 1.0f, 1.0f, 1.0f, GLUT_BITMAP_HELVETICA_10);
    }

    // Cac nut chuc nang nhanh (Quick Action Buttons)
    auto drawActionButton = [](float bx, float by, float bw, float bh, const string& label) {
        drawRoundedRect(bx, by, bw, bh, 5.0f, 0.12f, 0.18f, 0.28f, 0.9f);
        glColor3f(0.3f, 0.7f, 1.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx, by); glVertex2f(bx + bw, by);
            glVertex2f(bx + bw, by + bh); glVertex2f(bx, by + bh);
        glEnd();
        drawText(bx + 10, by + 10, label, 0.9f, 0.95f, 1.0f, GLUT_BITMAP_HELVETICA_10);
    };

    drawActionButton(760, WINDOW_HEIGHT - 54, 75, 38, "[ESC] MENU");
    drawActionButton(845, WINDOW_HEIGHT - 54, 75, 38, "[TAB] TOAN");
    drawActionButton(930, WINDOW_HEIGHT - 54, 75, 38, "[N] DEM/NGAY");
    drawActionButton(1015, WINDOW_HEIGHT - 54, 50, 38, "[T] TUYET");

    // 2. THANH DOCK DUOI DAY: KHO VU KHI (INVENTORY DOCK) & THANH LUC BAN
    drawRoundedRect(10, 10, WINDOW_WIDTH - 20, 68, 10.0f, 0.05f, 0.08f, 0.14f, 0.95f);
    glColor3f(0.2f, 0.35f, 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(10, 10); glVertex2f(WINDOW_WIDTH - 10, 10);
        glVertex2f(WINDOW_WIDTH - 10, 78); glVertex2f(10, 78);
    glEnd();

    Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;

    // Goc ban & Vi tri xe
    stringstream ssAngle, ssPos;
    ssAngle << "Goc Ban: " << (int)curTank.barrelAngleDeg << " deg [W/S]";
    ssPos   << "Vi Tri: X=" << (int)curTank.x << " [A/D]";
    drawText(25, 52, ssAngle.str(), 0.3f, 0.95f, 1.0f, GLUT_BITMAP_9_BY_15);
    drawText(25, 26, ssPos.str(), 0.75f, 0.8f, 0.9f, GLUT_BITMAP_9_BY_15);

    // THANH LUC BAN DANG LED NHIEU NAC (SEGMENTED POWER GAUGE)
    float gaugeX = 270.0f;
    drawText(gaugeX, 52, "LUC BAN [GIU SPACE]:", 1.0f, 0.85f, 0.2f, GLUT_BITMAP_9_BY_15);

    int totalSegments = 16;
    float segW = 10.0f, segH = 20.0f, segGap = 3.0f;
    float pRatio = (curTank.shootPower - 20.0f) / 80.0f;
    int litSegments = (int)(pRatio * totalSegments);

    for (int s = 0; s < totalSegments; ++s) {
        float sx = gaugeX + s * (segW + segGap);
        float sy = 22.0f;
        if (s < litSegments) {
            float sRatio = (float)s / totalSegments;
            glColor3f(sRatio, 1.0f - sRatio * 0.5f, 0.1f);
        } else {
            glColor3f(0.12f, 0.15f, 0.2f);
        }
        glRectf(sx, sy, sx + segW, sy + segH);
    }
    stringstream ssP; ssP << (int)curTank.shootPower << "%";
    drawText(gaugeX + totalSegments * (segW + segGap) + 10, 26, ssP.str(), 1.0f, 1.0f, 1.0f, GLUT_BITMAP_9_BY_15);

    // 4 O CHON VU KHI TRANG BI (CLICKABLE WEAPON DOCK)
    auto drawWeaponSlot = [&](int slotIdx, float x, float y, float w, float h, const string& keyStr, const string& name, const string& ammoStr, WeaponType type) {
        bool isSelected = (currentWeapon == type);
        if (isSelected) {
            drawRoundedRect(x, y, w, h, 6.0f, 0.2f, 0.35f, 0.55f, 0.95f);
            glColor3f(1.0f, 0.85f, 0.2f);
            glLineWidth(2.5f);
        } else {
            drawRoundedRect(x, y, w, h, 6.0f, 0.08f, 0.12f, 0.2f, 0.8f);
            glColor3f(0.2f, 0.3f, 0.45f);
            glLineWidth(1.0f);
        }
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y); glVertex2f(x + w, y);
            glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();

        drawText(x + 8, y + h - 18, keyStr, (isSelected ? 1.0f : 0.6f), (isSelected ? 0.9f : 0.7f), 0.3f, GLUT_BITMAP_HELVETICA_12);
        drawText(x + 36, y + h - 18, name, 0.95f, 0.95f, 1.0f, GLUT_BITMAP_HELVETICA_10);
        drawText(x + 36, y + 8, ammoStr, 0.75f, 0.85f, 0.9f, GLUT_BITMAP_HELVETICA_10);
        (void)slotIdx;
    };

    float dockX = 580.0f;
    float slotW = 160.0f, slotH = 52.0f, slotGap = 10.0f;

    drawWeaponSlot(0, dockX + 0 * (slotW + slotGap), 18, slotW, slotH, "[1]", "AP Shell", "Dan: Vo han", WEAPON_STANDARD);

    stringstream ssMortar; ssMortar << "Dan: " << curTank.inv.ammoMortar;
    drawWeaponSlot(1, dockX + 1 * (slotW + slotGap), 18, slotW, slotH, "[2]", "Mortar", ssMortar.str(), WEAPON_MORTAR);

    stringstream ssCluster; ssCluster << "Dan: " << curTank.inv.ammoCluster;
    drawWeaponSlot(2, dockX + 2 * (slotW + slotGap), 18, slotW, slotH, "[3]", "Cluster", ssCluster.str(), WEAPON_CLUSTER);

    stringstream ssCruise; ssCruise << "Dan: " << curTank.inv.ammoCruise;
    drawWeaponSlot(3, dockX + 3 * (slotW + slotGap), 18, slotW, slotH, "[4]", "Cruise (B3)", ssCruise.str(), WEAPON_CRUISE);

    // 3. BANG TOAN HOC DEBUG (HIEN THI KHI BAM TAB)
    if (showDebugHUD) {
        float panelW = 480.0f, panelH = 340.0f;
        float px = WINDOW_WIDTH - panelW - 15.0f;
        float py = WINDOW_HEIGHT - panelH - 75.0f;

        drawRoundedRect(px, py, panelW, panelH, 12.0f, 0.04f, 0.06f, 0.1f, 0.94f);
        glColor3f(0.2f, 0.7f, 1.0f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(px, py); glVertex2f(px + panelW, py);
            glVertex2f(px + panelW, py + panelH); glVertex2f(px, py + panelH);
        glEnd();

        Point2D P0, P1, P2, P3;
        calculateControlPointsForTank(curTank, currentWeapon, P0, P1, P2, P3);

        float textY = py + panelH - 24.0f;
        drawText(px + 14, textY, "THUAT TOAN BEZIER BAC 2 & BAC 3 (SLIDE CG.5)", 0.3f, 1.0f, 0.5f, GLUT_BITMAP_9_BY_15);
        textY -= 16.0f;
        drawText(px + 14, textY, "---------------------------------------------------", 0.4f, 0.5f, 0.6f, GLUT_BITMAP_9_BY_15);

        if (currentWeapon == WEAPON_CRUISE) {
            textY -= 18.0f;
            drawText(px + 14, textY, "Da thuc Bernstein Bac 3 (Cubic Bezier):", 0.9f, 0.9f, 0.9f, GLUT_BITMAP_9_BY_15);
            textY -= 16.0f;
            drawText(px + 18, textY, "P(t) = (1-t)^3*P0 + 3t(1-t)^2*P1 + 3t^2(1-t)*P2 + t^3*P3", 0.2f, 0.95f, 1.0f, GLUT_BITMAP_9_BY_15);
            textY -= 20.0f;
            stringstream ssP0, ssP1, ssP2, ssP3;
            ssP0 << "* P0 (Muzzle): (" << (int)P0.x << ", " << (int)P0.y << ")";
            ssP1 << "* P1 (Apex)  : (" << (int)P1.x << ", " << (int)P1.y << ") [Phong cao]";
            ssP2 << "* P2 (S-Turn): (" << (int)P2.x << ", " << (int)P2.y << ") [Vong luon khe nui]";
            ssP3 << "* P3 (Target): (" << (int)P3.x << ", " << (int)P3.y << ") [Tiep dat]";
            drawText(px + 14, textY, ssP0.str(), 0.3f, 1.0f, 0.4f, GLUT_BITMAP_9_BY_15); textY -= 16.0f;
            drawText(px + 14, textY, ssP1.str(), 1.0f, 0.85f, 0.2f, GLUT_BITMAP_9_BY_15); textY -= 16.0f;
            drawText(px + 14, textY, ssP2.str(), 0.2f, 0.9f, 1.0f, GLUT_BITMAP_9_BY_15);  textY -= 16.0f;
            drawText(px + 14, textY, ssP3.str(), 1.0f, 0.35f, 0.35f, GLUT_BITMAP_9_BY_15);
        } else {
            textY -= 18.0f;
            drawText(px + 14, textY, "Da thuc Bernstein Bac 2 (Quadratic Bezier):", 0.9f, 0.9f, 0.9f, GLUT_BITMAP_9_BY_15);
            textY -= 16.0f;
            drawText(px + 18, textY, "P(t) = (1-t)^2*P0 + 2t(1-t)*P1 + t^2*P2", 0.2f, 0.95f, 1.0f, GLUT_BITMAP_9_BY_15);
            textY -= 20.0f;
            stringstream ssP0, ssP1, ssP2;
            ssP0 << "* P0 (Muzzle): (" << (int)P0.x << ", " << (int)P0.y << ")";
            ssP1 << "* P1 (Apex)  : (" << (int)P1.x << ", " << (int)P1.y << ") [Goc & Gio]";
            ssP2 << "* P2 (Target): (" << (int)P2.x << ", " << (int)P2.y << ") [Diem roi]";
            drawText(px + 14, textY, ssP0.str(), 0.3f, 1.0f, 0.4f, GLUT_BITMAP_9_BY_15); textY -= 16.0f;
            drawText(px + 14, textY, ssP1.str(), 1.0f, 0.85f, 0.2f, GLUT_BITMAP_9_BY_15); textY -= 16.0f;
            drawText(px + 14, textY, ssP2.str(), 1.0f, 0.35f, 0.35f, GLUT_BITMAP_9_BY_15);
        }

        textY -= 22.0f;
        drawText(px + 14, textY, "PHEP BIEN DOI AFFINE 2D & ANH SANG DONG:", 0.3f, 1.0f, 0.5f, GLUT_BITMAP_9_BY_15);
        textY -= 16.0f;
        drawText(px + 18, textY, "Quay nong: M = T(-x0,-y0) * R(theta) * T(x0,y0)", 0.2f, 0.95f, 1.0f, GLUT_BITMAP_9_BY_15);
        textY -= 16.0f;
        drawText(px + 18, textY, "Point Light: I = I0 / (1 + k*d^2) * max(0, N.L)", 0.85f, 0.85f, 0.85f, GLUT_BITMAP_9_BY_15);

        textY -= 22.0f;
        drawText(px + 14, textY, "PHIM TAT:", 0.3f, 1.0f, 0.5f, GLUT_BITMAP_9_BY_15);
        textY -= 16.0f;
        drawText(px + 18, textY, "[1-4]: Chon vu khi | [TAB]: An/Hien | [P]: Bao loi", 0.7f, 0.75f, 0.8f, GLUT_BITMAP_9_BY_15);
    }
}

// ============================================================================
// HÀM HIỂN THỊ CHÍNH (OPENGL DISPLAY)
// ============================================================================
bool autoCaptureScreenshot = false;
int renderFrameCount = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (screenShakeTimer > 0.0f) {
        float shakeX = (((float)rand() / RAND_MAX) - 0.5f) * 12.0f * (screenShakeTimer / 0.5f);
        float shakeY = (((float)rand() / RAND_MAX) - 0.5f) * 12.0f * (screenShakeTimer / 0.5f);
        glTranslatef(shakeX, shakeY, 0.0f);
    }

    if (gameState == STATE_MENU) {
        drawStartMenu();
    } else {
        drawBackground();
        drawTerrain();

        // Ve xe tang & muc tieu
        playerTank.draw(currentTurn == SIDE_LEFT);
        if (gameMode == MODE_CAMPAIGN) {
            enemyBunker.draw(showDebugHUD);
        } else {
            enemyTank.draw(currentTurn == SIDE_RIGHT);
        }

        // Hom tiep te tha du
        airdrop.draw();

        drawTrajectoryPreview();
        drawProjectile();

        // Hat va tuyet roi
        if (isSnowing) {
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            for (const auto& sn : snowflakes) {
                glColor4f(sn.r, sn.g, sn.b, sn.a);
                glVertex2f(sn.x, sn.y);
            }
            glEnd();
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        for (const auto& p : particles) {
            float alpha = p.life / p.maxLife;
            if (p.type == PARTICLE_SHOCKWAVE) {
                float r = (1.0f - alpha) * 65.0f;
                drawMidpointCircle((int)p.x, (int)p.y, (int)r, p.r, p.g, p.b, false);
            } else if (p.type == PARTICLE_SPARK || p.type == PARTICLE_SMOKE) {
                glColor4f(p.r, p.g, p.b, alpha * (p.type == PARTICLE_SMOKE ? 0.5f : 1.0f));
                drawMidpointCircle((int)p.x, (int)p.y, (int)p.size, p.r, p.g, p.b, true);
            }
        }
        glDisable(GL_BLEND);

        for (const auto& ft : floatingTexts) {
            drawText(ft.x, ft.y, ft.text, ft.r, ft.g, ft.b, GLUT_BITMAP_HELVETICA_18);
        }

        glLoadIdentity();
        drawBattleHUD();
    }

    glutSwapBuffers();

    if (autoCaptureScreenshot) {
        renderFrameCount++;
        if (renderFrameCount >= 5) {
            if (gameState == STATE_MENU) {
                saveScreenshotBMP("ket_qua_menu.bmp");
            } else {
                saveScreenshotBMP("ket_qua_game_artillery_master.bmp");
            }
            exit(0);
        }
    }
}

// ============================================================================
// VÒNG LẶP CẬP NHẬT TRẠNG THÁI (60 FPS UPDATE)
// ============================================================================
void update(int value) {
    (void)value;
    float dt = 0.016f;

    // 1. May troi
    for (auto& c : clouds) {
        c.x += (c.speed + windForce * 2.0f) * dt;
        if (c.x > WINDOW_WIDTH + 80.0f) c.x = -80.0f;
        if (c.x < -80.0f) c.x = WINDOW_WIDTH + 80.0f;
    }

    // 2. Tuyet roi
    if (isSnowing) {
        for (auto& sn : snowflakes) {
            sn.y += sn.vy * dt;
            sn.x += (sn.vx + windForce * 3.0f) * dt;
            if (sn.y < 0) {
                sn.y = WINDOW_HEIGHT;
                sn.x = ((float)rand() / RAND_MAX) * WINDOW_WIDTH;
            }
        }
    }

    if (playerTank.recoilOffset > 0.0f) playerTank.recoilOffset = max(0.0f, playerTank.recoilOffset - 40.0f * dt);
    if (enemyTank.recoilOffset > 0.0f) enemyTank.recoilOffset = max(0.0f, enemyTank.recoilOffset - 40.0f * dt);
    if (screenShakeTimer > 0.0f) screenShakeTimer = max(0.0f, screenShakeTimer - dt);

    if (gameMode == MODE_CAMPAIGN) {
        enemyBunker.radarAngleDeg += 90.0f * dt;
        if (enemyBunker.radarAngleDeg >= 360.0f) enemyBunker.radarAngleDeg -= 360.0f;
        if (enemyBunker.hitFlashTimer > 0.0f) enemyBunker.hitFlashTimer = max(0.0f, enemyBunker.hitFlashTimer - dt);
    }

    // 3. CAP NHAT HOM TIEP TE THA DU (AIRDROP SUPPLY)
    if (gameState != STATE_MENU) {
        airdropTimer -= dt;
        if (airdropTimer <= 0.0f && !airdrop.active) {
            float dropX = 350.0f + ((float)rand() / RAND_MAX) * 580.0f;
            airdrop.spawn(dropX);
            addFloatingText(dropX - 40.0f, WINDOW_HEIGHT - 80.0f, "AIRDROP INCOMING!", 1.0f, 0.9f, 0.2f);
            airdropTimer = 18.0f + ((float)rand() / RAND_MAX) * 12.0f;
        }
        if (airdrop.active) {
            airdrop.update(dt);
            // Kiem tra nguoi choi hoac xe dich nhat hom tiep te
            auto checkPickup = [](Tank& tk) {
                if (!airdrop.active) return;
                if (abs(tk.x - airdrop.x) < 32.0f && abs(getTerrainHeight(tk.x) - airdrop.y) < 35.0f) {
                    airdrop.active = false;
                    spawnExplosion(airdrop.x, airdrop.y, 40, 140.0f);
                    if (airdrop.rewardType == 0) {
                        tk.hp = min(tk.maxHp, tk.hp + 35.0f);
                        addFloatingText(tk.x, getTerrainHeight(tk.x) + 40.0f, "+35 HP REPAIRED!", 0.2f, 1.0f, 0.4f);
                    } else {
                        tk.inv.ammoCruise++;
                        addFloatingText(tk.x, getTerrainHeight(tk.x) + 40.0f, "+1 CRUISE MISSILE!", 0.2f, 0.9f, 1.0f);
                    }
                }
            };
            checkPickup(playerTank);
            if (gameMode != MODE_CAMPAIGN) checkPickup(enemyTank);
        }
    }

    // 4. Tu dong nap luc khi giu phim Space
    if (gameState == STATE_CHARGING) {
        Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
        float chargeSpeed = 70.0f * dt;
        if (isChargingForward) {
            curTank.shootPower += chargeSpeed;
            if (curTank.shootPower >= 100.0f) { curTank.shootPower = 100.0f; isChargingForward = false; }
        } else {
            curTank.shootPower -= chargeSpeed;
            if (curTank.shootPower <= 20.0f) { curTank.shootPower = 20.0f; isChargingForward = true; }
        }
    }

    // 5. XU LY LUOT BAN CUA AI BOT
    if (gameState == STATE_AI_THINKING) {
        aiThinkTimer += dt;
        if (aiThinkTimer >= 1.2f) {
            aiThinkTimer = 0.0f;
            float aiAngle, aiPower;
            aiComputeShot(enemyTank, Point2D(playerTank.x, getTerrainHeight(playerTank.x)), aiAngle, aiPower);
            enemyTank.barrelAngleDeg = aiAngle;
            enemyTank.shootPower = aiPower;

            // AI ngau nhien su dung ten lua hoac dan coi neu con
            if (enemyTank.inv.ammoCruise > 0 && rand() % 3 == 0) currentWeapon = WEAPON_CRUISE;
            else if (enemyTank.inv.ammoMortar > 0 && rand() % 2 == 0) currentWeapon = WEAPON_MORTAR;
            else currentWeapon = WEAPON_STANDARD;

            fireCannon();
        }
    }

    // 6. CAP NHAT DAN PHAO BAY
    if (bullet.active) {
        // Tach dan chum o dinh duong cong
        if (bullet.type == WEAPON_CLUSTER && !bullet.hasSplit && bullet.t >= 0.5f) {
            bullet.hasSplit = true;
            spawnExplosion(bullet.currentPos.x, bullet.currentPos.y, 30, 80.0f);
            addFloatingText(bullet.currentPos.x - 40.0f, bullet.currentPos.y + 25.0f, "CLUSTER SEPARATION!", 1.0f, 0.9f, 0.2f);
            float spreads[3] = {-70.0f, 0.0f, 70.0f};
            for (int i = 0; i < 3; ++i) {
                SubBomb sb;
                sb.active = true;
                sb.P0 = bullet.currentPos;
                sb.P1 = bullet.currentPos + bullet.tangent * 0.25f + Point2D(spreads[i] * 0.5f, 30.0f);
                float landX = max(0.0f, min((float)WINDOW_WIDTH, bullet.P2.x + spreads[i]));
                sb.P2 = Point2D(landX, getTerrainHeight(landX));
                sb.t = 0.0f; sb.speed = 1.3f;
                sb.currentPos = sb.P0; sb.tangent = bullet.tangent;
                bullet.subBombs.push_back(sb);
            }
        }

        if (bullet.hasSplit) {
            bool anyActive = false;
            for (auto& sb : bullet.subBombs) {
                if (!sb.active) continue;
                anyActive = true;
                sb.t += sb.speed * dt;
                if (sb.t >= 1.0f) {
                    sb.active = false;
                    spawnExplosion(sb.P2.x, sb.P2.y, 45, 170.0f);
                    applyTerrainCrater(sb.P2.x, sb.P2.y, 25.0f);
                } else {
                    sb.currentPos = computeBezierQuadratic(sb.P0, sb.P1, sb.P2, sb.t);
                    sb.tangent = computeBezierTangentQuad(sb.P0, sb.P1, sb.P2, sb.t);

                    if (gameMode == MODE_CAMPAIGN) {
                        float minX, minY, maxX, maxY;
                        enemyBunker.getAABB(minX, minY, maxX, maxY);
                        if (sb.currentPos.x >= minX && sb.currentPos.x <= maxX &&
                            sb.currentPos.y >= minY && sb.currentPos.y <= maxY && !enemyBunker.destroyed) {
                            sb.active = false; enemyBunker.hp -= 25.0f;
                            spawnExplosion(sb.currentPos.x, sb.currentPos.y, 60, 220.0f);
                            applyTerrainCrater(sb.currentPos.x, sb.currentPos.y, 25.0f);
                        }
                    } else {
                        Tank& defTank = (bullet.firedBy == SIDE_LEFT) ? enemyTank : playerTank;
                        float minX, minY, maxX, maxY;
                        defTank.getAABB(minX, minY, maxX, maxY);
                        if (sb.currentPos.x >= minX && sb.currentPos.x <= maxX &&
                            sb.currentPos.y >= minY && sb.currentPos.y <= maxY && defTank.hp > 0.0f) {
                            sb.active = false; defTank.hp -= 25.0f;
                            spawnExplosion(sb.currentPos.x, sb.currentPos.y, 60, 220.0f);
                            applyTerrainCrater(sb.currentPos.x, sb.currentPos.y, 25.0f);
                        }
                    }
                }
            }
            if (!anyActive) {
                bullet.active = false;
                currentTurn = (currentTurn == SIDE_LEFT) ? SIDE_RIGHT : SIDE_LEFT;
                gameState = (gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT) ? STATE_AI_THINKING : STATE_AIMING;
            }
        } else {
            bullet.t += bullet.speed * dt;
            if (bullet.t >= 1.0f) {
                bullet.t = 1.0f; bullet.active = false;
                Point2D hitPt = (bullet.type == WEAPON_CRUISE) ? bullet.P3 : bullet.P2;
                spawnExplosion(hitPt.x, hitPt.y, 60, 200.0f);
                applyTerrainCrater(hitPt.x, hitPt.y, 35.0f);
                addFloatingText(hitPt.x, hitPt.y + 20.0f, "MISSED!", 0.8f, 0.8f, 0.8f);

                currentTurn = (currentTurn == SIDE_LEFT) ? SIDE_RIGHT : SIDE_LEFT;
                gameState = (gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT) ? STATE_AI_THINKING : STATE_AIMING;
            } else {
                if (bullet.type == WEAPON_CRUISE) {
                    bullet.currentPos = computeBezierCubic(bullet.P0, bullet.P1, bullet.P2, bullet.P3, bullet.t);
                    bullet.tangent = computeBezierTangentCubic(bullet.P0, bullet.P1, bullet.P2, bullet.P3, bullet.t);
                } else {
                    bullet.currentPos = computeBezierQuadratic(bullet.P0, bullet.P1, bullet.P2, bullet.t);
                    bullet.tangent = computeBezierTangentQuad(bullet.P0, bullet.P1, bullet.P2, bullet.t);
                }

                bullet.smokeTrail.push_back(bullet.currentPos);
                if (bullet.smokeTrail.size() > 25) bullet.smokeTrail.erase(bullet.smokeTrail.begin());

                float bx = bullet.currentPos.x, by = bullet.currentPos.y;

                // Ban trung hom tiep te dang roi
                if (airdrop.active && abs(bx - airdrop.x) < 22.0f && abs(by - airdrop.y) < 22.0f) {
                    airdrop.active = false;
                    spawnExplosion(airdrop.x, airdrop.y, 65, 200.0f);
                    Tank& shooter = (bullet.firedBy == SIDE_LEFT) ? playerTank : enemyTank;
                    shooter.inv.ammoCruise += 2;
                    addFloatingText(bx, by + 30.0f, "CRATE DESTROYED! +2 CRUISE!", 1.0f, 0.9f, 0.1f);
                }

                // KIEM TRA VA CHAM AABB VOI MUC TIEU
                if (gameMode == MODE_CAMPAIGN) {
                    float minX, minY, maxX, maxY;
                    enemyBunker.getAABB(minX, minY, maxX, maxY);
                    if (bx >= minX && bx <= maxX && by >= minY && by <= maxY && !enemyBunker.destroyed) {
                        bullet.active = false;
                        screenShakeTimer = 0.45f;
                        float damage = (bullet.type == WEAPON_CRUISE) ? 80.0f : ((bullet.type == WEAPON_MORTAR) ? 60.0f : 40.0f);
                        enemyBunker.hp -= damage;
                        enemyBunker.hitFlashTimer = 0.25f;
                        spawnExplosion(bx, by, 95, 270.0f);
                        applyTerrainCrater(bx, by, 30.0f);
                        addFloatingText(bx, by + 30.0f, "DIRECT HIT!", 1.0f, 0.85f, 0.1f);
                        if (enemyBunker.hp <= 0.0f) {
                            enemyBunker.hp = 0.0f; enemyBunker.destroyed = true;
                            spawnExplosion(enemyBunker.x, minY + 20.0f, 150, 320.0f);
                            applyTerrainCrater(enemyBunker.x, minY, 45.0f);
                        }
                        currentTurn = SIDE_LEFT;
                        gameState = STATE_AIMING;
                    } else if (by <= getTerrainHeight(bx)) {
                        bullet.active = false;
                        screenShakeTimer = 0.25f;
                        spawnExplosion(bx, by, 70, 220.0f);
                        applyTerrainCrater(bx, by, (bullet.type == WEAPON_CRUISE ? 45.0f : 35.0f));
                        addFloatingText(bx, by + 20.0f, "HIT TERRAIN!", 0.9f, 0.6f, 0.3f);
                        currentTurn = SIDE_LEFT;
                        gameState = STATE_AIMING;
                    }
                } else {
                    Tank& defTank = (bullet.firedBy == SIDE_LEFT) ? enemyTank : playerTank;
                    float minX, minY, maxX, maxY;
                    defTank.getAABB(minX, minY, maxX, maxY);
                    if (bx >= minX && bx <= maxX && by >= minY && by <= maxY && defTank.hp > 0.0f) {
                        bullet.active = false;
                        screenShakeTimer = 0.5f;
                        float damage = (bullet.type == WEAPON_CRUISE) ? 75.0f : ((bullet.type == WEAPON_MORTAR) ? 55.0f : 35.0f);
                        defTank.hp -= damage;
                        spawnExplosion(bx, by, 100, 280.0f);
                        applyTerrainCrater(bx, by, 30.0f);
                        addFloatingText(bx, by + 35.0f, "TANK HIT!", 1.0f, 0.2f, 0.2f);
                        if (defTank.hp <= 0.0f) {
                            defTank.hp = 0.0f;
                            addFloatingText(defTank.x, getTerrainHeight(defTank.x) + 40.0f, "TANK DESTROYED!", 1.0f, 0.9f, 0.1f);
                        }
                        currentTurn = (currentTurn == SIDE_LEFT) ? SIDE_RIGHT : SIDE_LEFT;
                        gameState = (gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT) ? STATE_AI_THINKING : STATE_AIMING;
                    } else if (by <= getTerrainHeight(bx)) {
                        bullet.active = false;
                        screenShakeTimer = 0.25f;
                        spawnExplosion(bx, by, 70, 220.0f);
                        applyTerrainCrater(bx, by, (bullet.type == WEAPON_CRUISE ? 45.0f : 35.0f));
                        addFloatingText(bx, by + 20.0f, "HIT TERRAIN!", 0.9f, 0.6f, 0.3f);
                        currentTurn = (currentTurn == SIDE_LEFT) ? SIDE_RIGHT : SIDE_LEFT;
                        gameState = (gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT) ? STATE_AI_THINKING : STATE_AIMING;
                    }
                }
            }
        }
    }

    // 7. Cap nhat hat & chu noi
    for (size_t i = 0; i < particles.size();) {
        particles[i].life -= dt;
        if (particles[i].life <= 0.0f) particles.erase(particles.begin() + i);
        else {
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            if (particles[i].type != PARTICLE_SHOCKWAVE) particles[i].vy -= GRAVITY * dt;
            ++i;
        }
    }
    for (size_t i = 0; i < floatingTexts.size();) {
        floatingTexts[i].life -= dt;
        floatingTexts[i].y += 35.0f * dt;
        if (floatingTexts[i].life <= 0.0f) floatingTexts.erase(floatingTexts.begin() + i);
        else ++i;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ============================================================================
// XỬ LÝ PHÍM BẤM & TƯƠNG TÁC
// ============================================================================
void keyboardDown(unsigned char key, int x, int y) {
    (void)x; (void)y;

    if (gameState == STATE_MENU) {
        switch (key) {
            case '1':
                gameMode = MODE_CAMPAIGN;
                enemyTank.isAI = false;
                gameState = STATE_AIMING;
                break;
            case '2':
                gameMode = MODE_AI_BOT;
                enemyTank.isAI = true;
                gameState = STATE_AIMING;
                break;
            case '3':
                gameMode = MODE_PVP;
                enemyTank.isAI = false;
                gameState = STATE_AIMING;
                break;
            case 'h': case 'H':
                showDebugHUD = true;
                gameState = STATE_AIMING;
                break;
            case 27: case 'q': case 'Q':
                exit(0);
                break;
        }
        glutPostRedisplay();
        return;
    }

    Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;

    switch (key) {
        case 'a': case 'A':
            if (curTank.x > 70.0f) { curTank.x -= 8.0f; curTank.wheelRotationDeg -= 25.0f; }
            break;
        case 'd': case 'D':
            if (curTank.x < WINDOW_WIDTH - 70.0f) { curTank.x += 8.0f; curTank.wheelRotationDeg += 25.0f; }
            break;
        case 'w': case 'W':
            if (curTank.barrelAngleDeg < 175.0f) curTank.barrelAngleDeg += 2.0f;
            break;
        case 's': case 'S':
            if (curTank.barrelAngleDeg > 5.0f) curTank.barrelAngleDeg -= 2.0f;
            break;

        case ' ':
            if (gameState == STATE_AIMING && !(gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT)) {
                gameState = STATE_CHARGING;
                isChargingForward = true;
            }
            break;

        case 13:
            if (gameState == STATE_AIMING && !(gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT)) {
                fireCannon();
            }
            break;

        // CHON VU KHI TRANG BI 1-4
        case '1': currentWeapon = WEAPON_STANDARD; break;
        case '2': currentWeapon = WEAPON_MORTAR; break;
        case '3': currentWeapon = WEAPON_CLUSTER; break;
        case '4': currentWeapon = WEAPON_CRUISE; break;

        // BAT/TAT BANG TOAN HOC DEBUG (TAB)
        case 9: showDebugHUD = !showDebugHUD; break;

        // BAT/TAT BAO LOI & DA GIAC KIEM SOAT
        case 'p': case 'P': showControlPolygon = !showControlPolygon; break;

        // DOI GIO & DIA HINH (C)
        case 'c': case 'C': windForce = (((float)rand() / RAND_MAX) - 0.5f) * 20.0f; generateTerrain(currentLevel); break;

        // CHUYEN DOI NGAY/DEM (N)
        case 'n': case 'N': isNightMode = !isNightMode; break;

        // BAT/TAT TUYET ROI (T)
        case 't': case 'T': isSnowing = !isSnowing; if (isSnowing) initSnow(); break;

        // RESET VAN CHOI (R)
        case 'r': case 'R':
            playerTank = Tank(160.0f, SIDE_LEFT, false);
            enemyTank = Tank(1060.0f, SIDE_RIGHT, (gameMode == MODE_AI_BOT));
            enemyBunker.reset(1060.0f, 100.0f);
            generateTerrain(currentLevel);
            currentTurn = SIDE_LEFT;
            bullet.active = false;
            airdrop.active = false;
            particles.clear();
            floatingTexts.clear();
            gameState = STATE_AIMING;
            break;

        // VE MENU (ESC)
        case 27:
            gameState = STATE_MENU;
            break;

        case 'q': case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    (void)x; (void)y;
    if (key == ' ' && gameState == STATE_CHARGING) {
        fireCannon();
    }
}

void specialKeys(int key, int x, int y) {
    (void)x; (void)y;
    if (gameState == STATE_MENU) return;
    Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
    switch (key) {
        case GLUT_KEY_LEFT:
            if (curTank.x > 70.0f) { curTank.x -= 8.0f; curTank.wheelRotationDeg -= 25.0f; }
            break;
        case GLUT_KEY_RIGHT:
            if (curTank.x < WINDOW_WIDTH - 70.0f) { curTank.x += 8.0f; curTank.wheelRotationDeg += 25.0f; }
            break;
        case GLUT_KEY_UP:
            if (curTank.barrelAngleDeg < 175.0f) curTank.barrelAngleDeg += 2.0f;
            break;
        case GLUT_KEY_DOWN:
            if (curTank.barrelAngleDeg > 5.0f) curTank.barrelAngleDeg -= 2.0f;
            break;
        case GLUT_KEY_F12:
            saveScreenshotBMP("artillery_screenshot.bmp");
            addFloatingText(550.0f, 350.0f, "SCREENSHOT SAVED (F12)!", 0.2f, 1.0f, 0.5f);
            break;
    }
    glutPostRedisplay();
}

// ============================================================================
// TƯƠNG TÁC BẰNG CHUỘT: CLICK NÚT GUI & KÉO THẢ NGẮM BẮN
// ============================================================================
bool isMouseDragging = false;

void mouseClick(int button, int state, int x, int y) {
    int glY = WINDOW_HEIGHT - y;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // 1. CLICK CHUOT O MAN HINH MENU
            if (gameState == STATE_MENU) {
                float mw = 680.0f;
                float mx = (WINDOW_WIDTH - mw) * 0.5f;
                float my = (WINDOW_HEIGHT - 480.0f) * 0.5f;
                float btnW = 560.0f, btnH = 55.0f;
                float btnX = mx + (mw - btnW) * 0.5f;

                if (x >= btnX && x <= btnX + btnW) {
                    if (glY >= my + 265 && glY <= my + 265 + btnH) {
                        gameMode = MODE_CAMPAIGN; enemyTank.isAI = false; gameState = STATE_AIMING;
                    } else if (glY >= my + 195 && glY <= my + 195 + btnH) {
                        gameMode = MODE_AI_BOT; enemyTank.isAI = true; gameState = STATE_AIMING;
                    } else if (glY >= my + 125 && glY <= my + 125 + btnH) {
                        gameMode = MODE_PVP; enemyTank.isAI = false; gameState = STATE_AIMING;
                    } else if (glY >= my + 55 && glY <= my + 55 + btnH) {
                        showDebugHUD = true; gameState = STATE_AIMING;
                    }
                }
                glutPostRedisplay();
                return;
            }

            // 2. CLICK CHUOT VAO CAC NUT GUI TRONG TRAN CHIEN
            // Click cac nut o Top Header
            if (glY >= WINDOW_HEIGHT - 54 && glY <= WINDOW_HEIGHT - 16) {
                if (x >= 760 && x <= 835) { gameState = STATE_MENU; glutPostRedisplay(); return; }
                if (x >= 845 && x <= 920) { showDebugHUD = !showDebugHUD; glutPostRedisplay(); return; }
                if (x >= 930 && x <= 1005) { isNightMode = !isNightMode; glutPostRedisplay(); return; }
                if (x >= 1015 && x <= 1065) { isSnowing = !isSnowing; if (isSnowing) initSnow(); glutPostRedisplay(); return; }
            }

            // Click chon vu khi o thanh Dock duoi day
            if (glY >= 18 && glY <= 70) {
                float dockX = 580.0f, slotW = 160.0f, slotGap = 10.0f;
                for (int slot = 0; slot < 4; ++slot) {
                    float sx = dockX + slot * (slotW + slotGap);
                    if (x >= sx && x <= sx + slotW) {
                        if (slot == 0) currentWeapon = WEAPON_STANDARD;
                        else if (slot == 1) currentWeapon = WEAPON_MORTAR;
                        else if (slot == 2) currentWeapon = WEAPON_CLUSTER;
                        else if (slot == 3) currentWeapon = WEAPON_CRUISE;
                        glutPostRedisplay();
                        return;
                    }
                }
            }

            // 3. KEO CHUOT DE NGHINH NONG PHAO VA CHINH LUC BAN
            if (gameState == STATE_AIMING && !(gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT)) {
                isMouseDragging = true;
                Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
                Point2D pivot = curTank.getPivot();
                float dx = (float)x - pivot.x;
                float dy = (float)glY - pivot.y;
                if (dy > 0.0f) {
                    float angle = atan2(dy, dx) * 180.0f / M_PI;
                    curTank.barrelAngleDeg = max(5.0f, min(175.0f, angle));
                    float dist = sqrt(dx * dx + dy * dy);
                    curTank.shootPower = max(20.0f, min(100.0f, (dist / 6.0f)));
                }
            }
        } else {
            if (isMouseDragging && gameState == STATE_AIMING && !(gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT)) {
                fireCannon(); // Tha chuot ra de khai hoa!
            }
            isMouseDragging = false;
        }
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    if (isMouseDragging && gameState == STATE_AIMING && !(gameMode == MODE_AI_BOT && currentTurn == SIDE_RIGHT)) {
        int glY = WINDOW_HEIGHT - y;
        Tank& curTank = (currentTurn == SIDE_LEFT) ? playerTank : enemyTank;
        Point2D pivot = curTank.getPivot();
        float dx = (float)x - pivot.x;
        float dy = (float)glY - pivot.y;
        if (dy > 0.0f) {
            float angle = atan2(dy, dx) * 180.0f / M_PI;
            curTank.barrelAngleDeg = max(5.0f, min(175.0f, angle));
            float dist = sqrt(dx * dx + dy * dy);
            curTank.shootPower = max(20.0f, min(100.0f, (dist / 6.0f)));
            glutPostRedisplay();
        }
    }
}

// ============================================================================
// KHỞI TẠO OPENGL & MAIN
// ============================================================================
void initGL() {
    glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    srand((unsigned int)time(NULL));
    generateTerrain(1);
    initClouds();
    initSnow();
}

int main(int argc, char** argv) {
    if (argc > 1 && string(argv[1]) == "--screenshot-menu") {
        autoCaptureScreenshot = true;
        gameState = STATE_MENU;
    } else if (argc > 1 && string(argv[1]) == "--screenshot") {
        autoCaptureScreenshot = true;
        gameState = STATE_AIMING; // De chup man hinh tran chien
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(60, 40);
    glutCreateWindow("VKU - ARTILLERY BATTLE: BEZIER 2D MASTER EDITION");

    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
