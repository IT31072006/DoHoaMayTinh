/* ==============================================================================
 * TRƯỜNG ĐẠI HỌC CÔNG NGHỆ THÔNG TIN & TRUYỀN THÔNG VIỆT - HÀN (VKU)
 * KHOA KỸ THUẬT MÁY TÍNH & ĐIỆN TỬ
 * BÀI TẬP GIỮA KỲ: MÔN ĐỒ HỌA MÁY TÍNH (COMPUTER GRAPHICS)
 *
 * ĐỀ TÀI: CHROME T-REX RUNNER 2D - HIGH DEFINITION ARTISTIC EDITION
 * (Phiên bản Đồ họa Nghệ thuật Nâng cao Thời tiền sử / Sa mạc)
 *
 * CÁC THUẬT TOÁN ĐỒ HỌA CỐT LÕI TÍCH HỢP (THEO ĐỀ CƯƠNG SLIDE VKU):
 * 1. Phép biến đổi Affine 2D trên Ma trận thuần nhất 3x3:
 *    - Tịnh tiến (Translation): Thị sai đa tầng Parallax (Bầu trời, Núi xa, Đụn cát, Mặt đất).
 *    - Phép quay quanh tâm khớp (Pivot Rotation): Chân khủng long chạy luân phiên quanh khớp háng,
 *      cánh thằn lằn bay (Pterodactyl) vỗ cánh quanh khớp vai, khủng long ngã khi Game Over.
 *    - Phép co giãn & Biến dạng (Scaling): Khủng long cúi rạp người (Ducking) né chim bay.
 * 2. Thuật toán Phát hiện va chạm Hộp bao AABB đa tầng (Multi-Box Collision):
 *    - Hitbox chi tiết độc lập: Đầu, Thân, Chân, Đuôi khủng long va chạm với gai xương rồng & mỏ chim.
 * 3. Thuật toán Đường cong Bézier bậc 2 (Quadratic Bézier Curve):
 *    - Đa thức Bernstein bậc 2 & Thuật toán De Casteljau dự báo quỹ đạo giậm nhảy động.
 *    - Quỹ đạo bay uốn lượn của Cầu lửa (Fireball) trong chế độ Arcade.
 * 4. Thuật toán vẽ nguyên thủy cơ sở:
 *    - Midpoint Circle: Vẽ Mặt trời, Hào quang vầng sáng, Mặt trăng khuyết, Mắt khủng long, Vệt bóng mờ.
 *    - Bresenham Line: Vẽ vạch địa tầng, rạn nứt mặt đất sa mạc, gai xương rồng.
 * 5. Hệ thống Hạt 2D nâng cao (Particle System):
 *    - Bụi cát tung cánh quạt xoáy dưới chân, vệt sao băng ban đêm, tàn lửa đạn bay, mảnh vụn nổ.
 * 6. Đổ bóng hình học mềm (Soft Geometric Drop Shadow):
 *    - Bóng elip co giãn và mờ dần theo độ cao cú nhảy của khủng long.
 * 7. Chu kỳ Ngày / Hoàng Hôn / Đêm động với dải màu Gradient chuyển tiếp mượt mà.
 * ==============================================================================
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HẰNG SỐ CẤU HÌNH CỬA SỔ & VẬT LÝ GAME
// ============================================================================
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 560;
const float GROUND_Y    = 135.0f; // Cao độ mặt đất

const float GRAVITY           = 1950.0f;
const float JUMP_VELOCITY     = 740.0f;
const float FAST_FALL_GRAVITY = 4000.0f;

const float BASE_SPEED    = 420.0f;
const float MAX_SPEED     = 1050.0f;
const float SPEED_ACCEL   = 8.0f;

// ============================================================================
// ĐỊNH NGHĨA TRẠNG THÁI & CHẾ ĐỘ
// ============================================================================
enum GameState {
    STATE_START = 0,
    STATE_PLAYING,
    STATE_GAMEOVER
};

enum GameMode {
    MODE_CLASSIC = 0, // Chuẩn Chrome Dino thuần túy
    MODE_ARCADE       // Nâng cấp: Khiên bảo vệ, đạn lửa khạc rồng, nhảy đôi
};

enum DinoPose {
    POSE_RUN_1 = 0,
    POSE_RUN_2,
    POSE_JUMP,
    POSE_DUCK_1,
    POSE_DUCK_2,
    POSE_DEAD
};

enum ObstacleType {
    OBS_CACTUS_SINGLE = 0,
    OBS_CACTUS_DOUBLE,
    OBS_CACTUS_TRIPLE,
    OBS_CACTUS_GIANT,
    OBS_CACTUS_FLOWER, // Cây xương rồng nở hoa sa mạc
    OBS_BIRD_LOW,      // Chim bay thấp: Phải NHẢY
    OBS_BIRD_MID,      // Chim bay vừa: CÚI NGƯỜI né
    OBS_BIRD_HIGH      // Chim bay cao: Đi qua an toàn
};

enum PowerUpType {
    POWER_NONE = 0,
    POWER_SHIELD,
    POWER_FIREBALL,
    POWER_DOUBLE_JUMP
};

// ============================================================================
// CẤU TRÚC HÌNH HỌC & ĐỒ HỌA MÁY TÍNH
// ============================================================================
struct Point2D {
    float x, y;
    Point2D() : x(0), y(0) {}
    Point2D(float _x, float _y) : x(_x), y(_y) {}
};

struct Color4f {
    float r, g, b, a;
    Color4f() : r(1), g(1), b(1), a(1) {}
    Color4f(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
};

Color4f lerpColor(const Color4f& c1, const Color4f& c2, float t) {
    t = max(0.0f, min(1.0f, t));
    return Color4f(
        c1.r + t * (c2.r - c1.r),
        c1.g + t * (c2.g - c1.g),
        c1.b + t * (c2.b - c1.b),
        c1.a + t * (c2.a - c1.a)
    );
}

struct AABB {
    float x, y, w, h;
    AABB() : x(0), y(0), w(0), h(0) {}
    AABB(float _x, float _y, float _w, float _h) : x(_x), y(_y), w(_w), h(_h) {}

    bool intersects(const AABB& o) const {
        return (x < o.x + o.w && x + w > o.x &&
                y < o.y + o.h && y + h > o.y);
    }
};

// ============================================================================
// MA TRẬN BIẾN ĐỔI HÌNH HỌC AFFINE 2D THUẦN NHẤT 3x3 (SLIDE CG.3 VKU)
// ============================================================================
struct Matrix3x3 {
    float m[3][3];

    Matrix3x3() {
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                m[r][c] = (r == c) ? 1.0f : 0.0f;
    }

    static Matrix3x3 multiply(const Matrix3x3& a, const Matrix3x3& b) {
        Matrix3x3 res;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                res.m[r][c] = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    res.m[r][c] += a.m[r][k] * b.m[k][c];
                }
            }
        }
        return res;
    }

    static Matrix3x3 translation(float tx, float ty) {
        Matrix3x3 res;
        res.m[0][2] = tx;
        res.m[1][2] = ty;
        return res;
    }

    static Matrix3x3 rotation(float angleDeg) {
        Matrix3x3 res;
        float rad = angleDeg * (float)M_PI / 180.0f;
        float c = cos(rad), s = sin(rad);
        res.m[0][0] = c;  res.m[0][1] = -s;
        res.m[1][0] = s;  res.m[1][1] = c;
        return res;
    }

    static Matrix3x3 scale(float sx, float sy) {
        Matrix3x3 res;
        res.m[0][0] = sx;
        res.m[1][1] = sy;
        return res;
    }

    static Matrix3x3 rotateAroundPivot(float angleDeg, float px, float py) {
        Matrix3x3 t1 = translation(-px, -py);
        Matrix3x3 r  = rotation(angleDeg);
        Matrix3x3 t2 = translation(px, py);
        return multiply(t2, multiply(r, t1));
    }

    Point2D transform(const Point2D& p) const {
        float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
        float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
        return Point2D(x, y);
    }
};

// ============================================================================
// THUẬT TOÁN ĐƯỜNG CONG BÉZIER BẬC 2 (SLIDE CG.5 VKU)
// ============================================================================
Point2D evaluateBezierQuadratic(Point2D p0, Point2D p1, Point2D p2, float t) {
    float u = 1.0f - t;
    float b0 = u * u;
    float b1 = 2.0f * u * t;
    float b2 = t * t;
    return Point2D(b0 * p0.x + b1 * p1.x + b2 * p2.x,
                   b0 * p0.y + b1 * p1.y + b2 * p2.y);
}

Point2D deCasteljauQuadratic(Point2D p0, Point2D p1, Point2D p2, float t, Point2D& q0, Point2D& q1) {
    q0 = Point2D((1.0f - t) * p0.x + t * p1.x, (1.0f - t) * p0.y + t * p1.y);
    q1 = Point2D((1.0f - t) * p1.x + t * p2.x, (1.0f - t) * p1.y + t * p2.y);
    return Point2D((1.0f - t) * q0.x + t * q1.x, (1.0f - t) * q0.y + t * q1.y);
}

// ============================================================================
// THUẬT TOÁN VẼ CƠ SỞ: MIDPOINT CIRCLE, ELLIPSE & GRADIENTS
// ============================================================================
void drawFilledCircle(float xc, float yc, float r, int segments = 24) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc, yc);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i * 2.0f * (float)M_PI / (float)segments;
        glVertex2f(xc + r * cos(theta), yc + r * sin(theta));
    }
    glEnd();
}

void drawFilledEllipse(float xc, float yc, float rx, float ry, int segments = 24) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc, yc);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i * 2.0f * (float)M_PI / (float)segments;
        glVertex2f(xc + rx * cos(theta), yc + ry * sin(theta));
    }
    glEnd();
}

// Vẽ hình chữ nhật Gradient chuyển màu mượt mà
void drawGradientRect(float x, float y, float w, float h,
                      const Color4f& cBottom, const Color4f& cTop) {
    glBegin(GL_QUADS);
    glColor4f(cBottom.r, cBottom.g, cBottom.b, cBottom.a);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glColor4f(cTop.r, cTop.g, cTop.b, cTop.a);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Đổ bóng hình học mềm (Soft Geometric Drop Shadow)
void drawSoftDropShadow(float x, float y, float w, float h, float dinoY) {
    (void)y;
    float heightAboveGround = dinoY - GROUND_Y;
    float scaleFactor = 1.0f + heightAboveGround * 0.002f;
    float alphaFactor = max(0.12f, 0.45f - heightAboveGround * 0.0025f);

    float rx = (w * 0.5f + 4.0f) * scaleFactor;
    float ry = (h * 0.5f) * (1.0f / scaleFactor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.06f, 0.05f, alphaFactor);
    drawFilledEllipse(x + w * 0.5f, GROUND_Y - 4.0f, rx, ry, 20);
}

// ============================================================================
// HỆ THỐNG HẠT VẬT LÝ NÂNG CAO (PARTICLE SYSTEM 2D)
// ============================================================================
struct Particle {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
    Color4f color;
    int type; // 0: bụi cát xoáy, 1: tia lửa, 2: sao băng, 3: tàn khói
};

vector<Particle> particles;

void spawnDustPuff(float x, float y, int count = 4) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x + (float)(rand() % 12 - 6);
        p.y = y + (float)(rand() % 6);
        p.vx = -((float)(rand() % 100 + 50)); // Bụi cuộn ngược theo quán tính
        p.vy = (float)(rand() % 80 + 20);
        p.life = 1.0f;
        p.decay = (float)(rand() % 15 + 20) / 10.0f;
        p.size = (float)(rand() % 4 + 3);
        p.color = Color4f(0.82f, 0.72f, 0.58f, 0.8f); // Màu cát sa mạc
        p.type = 0;
        particles.push_back(p);
    }
}

void spawnSparks(float x, float y, int count = 25) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x; p.y = y;
        float ang = (float)(rand() % 360) * (float)M_PI / 180.0f;
        float spd = (float)(rand() % 280 + 60);
        p.vx = cos(ang) * spd;
        p.vy = sin(ang) * spd;
        p.life = 1.0f;
        p.decay = (float)(rand() % 15 + 15) / 10.0f;
        p.size = (float)(rand() % 4 + 2);
        if (rand() % 3 == 0) p.color = Color4f(1.0f, 0.85f, 0.2f, 1.0f); // Vàng
        else if (rand() % 2 == 0) p.color = Color4f(1.0f, 0.4f, 0.1f, 1.0f); // Cam
        else p.color = Color4f(0.3f, 0.85f, 0.4f, 1.0f); // Xanh ngọc
        p.type = 1;
        particles.push_back(p);
    }
}

// ============================================================================
// ĐỐI TƯỢNG KHỦNG LONG T-REX (HIGH-DEFINITION CHARACTER)
// ============================================================================
struct Dino {
    float x, y;
    float vy;
    bool isGrounded;
    bool isDucking;
    bool isFastFalling;
    bool canDoubleJump;
    float legTimer;
    float eyeBlinkTimer;
    bool isEyeClosed;
    DinoPose pose;
    float deadRotation;

    Dino() { reset(); }

    void reset() {
        x = 90.0f;
        y = GROUND_Y;
        vy = 0.0f;
        isGrounded = true;
        isDucking = false;
        isFastFalling = false;
        canDoubleJump = true;
        legTimer = 0.0f;
        eyeBlinkTimer = 0.0f;
        isEyeClosed = false;
        pose = POSE_RUN_1;
        deadRotation = 0.0f;
    }

    void jump() {
        if (isGrounded) {
            vy = JUMP_VELOCITY;
            isGrounded = false;
            canDoubleJump = true;
            spawnDustPuff(x + 18, y, 7);
        } else if (canDoubleJump) {
            vy = JUMP_VELOCITY * 0.9f;
            canDoubleJump = false;
            spawnSparks(x + 20, y + 20, 12);
        }
    }

    void duck(bool active) {
        isDucking = active;
        if (active && !isGrounded) {
            isFastFalling = true;
        }
    }

    vector<AABB> getHitboxes() const {
        vector<AABB> boxes;
        if (isDucking && isGrounded) {
            boxes.push_back(AABB(x + 6.0f,  y,        38.0f, 24.0f)); // Thân thấp
            boxes.push_back(AABB(x + 44.0f, y + 2.0f, 24.0f, 20.0f)); // Đầu chúi
            boxes.push_back(AABB(x,         y + 6.0f, 14.0f, 16.0f)); // Đuôi
        } else {
            boxes.push_back(AABB(x + 10.0f, y + 8.0f,  26.0f, 32.0f)); // Thân chính
            boxes.push_back(AABB(x + 24.0f, y + 32.0f, 24.0f, 22.0f)); // Đầu & Mõm
            boxes.push_back(AABB(x + 2.0f,  y + 18.0f, 14.0f, 16.0f)); // Đuôi
            if (isGrounded) {
                boxes.push_back(AABB(x + 14.0f, y, 18.0f, 14.0f));     // Chân
            }
        }
        return boxes;
    }
};

// ============================================================================
// ĐỐI TƯỢNG CHƯỚNG NGẠI VẬT & POWER-UP
// ============================================================================
struct Obstacle {
    float x, y;
    float width, height;
    ObstacleType type;
    float wingTimer;
    int wingState;

    Obstacle(float _x, ObstacleType _type) : x(_x), type(_type), wingTimer(0.0f), wingState(0) {
        switch (type) {
            case OBS_CACTUS_SINGLE:
                width = 22.0f; height = 44.0f; y = GROUND_Y;
                break;
            case OBS_CACTUS_DOUBLE:
                width = 42.0f; height = 44.0f; y = GROUND_Y;
                break;
            case OBS_CACTUS_TRIPLE:
                width = 62.0f; height = 44.0f; y = GROUND_Y;
                break;
            case OBS_CACTUS_GIANT:
                width = 32.0f; height = 62.0f; y = GROUND_Y;
                break;
            case OBS_CACTUS_FLOWER:
                width = 36.0f; height = 52.0f; y = GROUND_Y;
                break;
            case OBS_BIRD_LOW:
                width = 48.0f; height = 34.0f; y = GROUND_Y + 14.0f;
                break;
            case OBS_BIRD_MID:
                width = 48.0f; height = 34.0f; y = GROUND_Y + 48.0f;
                break;
            case OBS_BIRD_HIGH:
                width = 48.0f; height = 34.0f; y = GROUND_Y + 102.0f;
                break;
        }
    }

    vector<AABB> getHitboxes() const {
        vector<AABB> boxes;
        if (type >= OBS_BIRD_LOW) {
            boxes.push_back(AABB(x + 8.0f, y + 10.0f, 32.0f, 16.0f));
            boxes.push_back(AABB(x,        y + 12.0f, 12.0f,  8.0f));
            if (wingState == 0) boxes.push_back(AABB(x + 14.0f, y + 22.0f, 18.0f, 16.0f));
            else                boxes.push_back(AABB(x + 14.0f, y,         18.0f, 14.0f));
        } else {
            boxes.push_back(AABB(x + 4.0f, y, width - 8.0f, height));
            if (type == OBS_CACTUS_GIANT || type == OBS_CACTUS_FLOWER) {
                boxes.push_back(AABB(x, y + 18.0f, width, 20.0f));
            }
        }
        return boxes;
    }
};

struct PowerUpItem {
    float x, y;
    PowerUpType type;
    float bobTimer;

    PowerUpItem(float _x, float _y, PowerUpType _type)
        : x(_x), y(_y), type(_type), bobTimer(0.0f) {}

    AABB getHitbox() const {
        return AABB(x - 16.0f, y - 16.0f, 32.0f, 32.0f);
    }
};

struct Fireball {
    Point2D p0, p1, p2;
    float t;
    float speed;
    bool active;
};

// ============================================================================
// ĐỐI TƯỢNG MÔI TRƯỜNG THỊ SAI PARALLAX
// ============================================================================
struct Cloud {
    float x, y;
    float speed;
    float scale;
    float opacity;
};

struct MountainPeak {
    float x, y, w, h;
    Color4f color;
};

// ============================================================================
// BIẾN TOÀN CỤC CỦA TRÒ CHƠI
// ============================================================================
GameState gameState = STATE_START;
GameMode  gameMode  = MODE_CLASSIC;

Dino dino;
vector<Obstacle>    obstacles;
vector<PowerUpItem> powerUps;
vector<Fireball>    fireballs;
vector<Cloud>       clouds;

// 3 Lớp núi thị sai Parallax
vector<MountainPeak> farMountains;
vector<MountainPeak> midDunes;

float currentScore   = 0.0f;
float highScore      = 0.0f;
float gameSpeed      = BASE_SPEED;
float distanceRun    = 0.0f;

// Chu kỳ Ngày / Đêm mượt mà (0.0 = Ngày rực rỡ, 0.5 = Hoàng hôn, 1.0 = Đêm ngân hà)
float timeOfDayCycle = 0.0f;
bool  manualNight    = false;
float milestoneFlash = 0.0f;
int   lastMilestone  = 0;

// Trạng thái debug & chế độ xem
bool showAcademicHUD = false;
bool showBezierArc   = true;
bool showHitboxes    = false;

bool hasShield = false;
int  fireballAmmo = 0;
float shieldAura = 0.0f;

// Tham số chụp ảnh màn hình
bool autoCaptureDay   = false;
bool autoCaptureNight = false;
bool autoCaptureDebug = false;
bool autoCaptureStart = false;
int  renderFrameCount = 0;

// ============================================================================
// KHỞI TẠO NÚI THỊ SAI, MÂY & BỐI CẢNH
// ============================================================================
void initParallaxScenery() {
    clouds.clear();
    for (int i = 0; i < 6; ++i) {
        Cloud c;
        c.x = (float)(i * 180 + rand() % 80);
        c.y = (float)(WINDOW_HEIGHT - 90 - rand() % 120);
        c.speed = 25.0f + (float)(rand() % 30);
        c.scale = 0.8f + (float)(rand() % 5) / 10.0f;
        c.opacity = 0.75f + (float)(rand() % 20) / 100.0f;
        clouds.push_back(c);
    }

    // Núi xa (Far Mountains) - Tông tím xanh atmospheric
    farMountains.clear();
    for (int i = 0; i < 9; ++i) {
        MountainPeak m;
        m.x = (float)(i * 140 - 50);
        m.w = 260.0f + (float)(rand() % 120);
        m.h = 170.0f + (float)(rand() % 80);
        m.y = GROUND_Y;
        m.color = Color4f(0.35f, 0.42f, 0.62f, 0.55f);
        farMountains.push_back(m);
    }

    // Đụn cát tầm trung (Mid Dunes) - Tông đất nung / cam ấm
    midDunes.clear();
    for (int i = 0; i < 8; ++i) {
        MountainPeak d;
        d.x = (float)(i * 150 - 40);
        d.w = 240.0f + (float)(rand() % 80);
        d.h = 45.0f + (float)(rand() % 25);
        d.y = GROUND_Y;
        d.color = Color4f(0.85f, 0.58f, 0.35f, 0.70f);
        midDunes.push_back(d);
    }
}

void resetGame() {
    dino.reset();
    obstacles.clear();
    powerUps.clear();
    fireballs.clear();

    currentScore = 0.0f;
    gameSpeed = BASE_SPEED;
    distanceRun = 0.0f;
    milestoneFlash = 0.0f;
    lastMilestone = 0;

    hasShield = false;
    fireballAmmo = 0;

    initParallaxScenery();
    gameState = STATE_PLAYING;
}

// Lưu ảnh chụp BMP
void saveScreenshotBMP(const string& filename) {
    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;
    vector<unsigned char> pixels(w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    ofstream out(filename.c_str(), ios::binary);
    if (!out) return;

    unsigned char bmpFileHeader[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
    unsigned char bmpInfoHeader[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0 };

    int rowPadded = (w * 3 + 3) & (~3);
    int imageSize = rowPadded * h;
    int fileSize = 54 + imageSize;

    bmpFileHeader[2] = (unsigned char)(fileSize);
    bmpFileHeader[3] = (unsigned char)(fileSize >> 8);
    bmpFileHeader[4] = (unsigned char)(fileSize >> 16);
    bmpFileHeader[5] = (unsigned char)(fileSize >> 24);

    bmpInfoHeader[4] = (unsigned char)(w);
    bmpInfoHeader[5] = (unsigned char)(w >> 8);
    bmpInfoHeader[6] = (unsigned char)(w >> 16);
    bmpInfoHeader[7] = (unsigned char)(w >> 24);

    bmpInfoHeader[8]  = (unsigned char)(h);
    bmpInfoHeader[9]  = (unsigned char)(h >> 8);
    bmpInfoHeader[10] = (unsigned char)(h >> 16);
    bmpInfoHeader[11] = (unsigned char)(h >> 24);

    out.write((char*)bmpFileHeader, 14);
    out.write((char*)bmpInfoHeader, 40);

    vector<unsigned char> row(rowPadded, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * 3;
            row[x * 3 + 0] = pixels[srcIdx + 2];
            row[x * 3 + 1] = pixels[srcIdx + 1];
            row[x * 3 + 2] = pixels[srcIdx + 0];
        }
        out.write((char*)row.data(), rowPadded);
    }
    out.close();
    cout << "Da chup anh man hinh: " << filename << endl;
}

// In chuỗi ký tự bằng font bitmap
void renderBitmapString(float x, float y, void* font, const string& str) {
    glRasterPos2f(x, y);
    for (char c : str) glutBitmapCharacter(font, c);
}

// ============================================================================
// HỆ THỐNG MÀU SẮC ĐỘNG & BẦU TRỜ GRADIENT NGHỆ THUẬT
// ============================================================================
void getSkyGradientColors(Color4f& skyTop, Color4f& skyBottom, Color4f& sunMoonColor) {
    float t = timeOfDayCycle; // 0.0 (Ngày sa mạc) -> 0.5 (Hoàng hôn) -> 1.0 (Đêm ngân hà)

    // Ban ngày: Trời xanh trong vắt -> Hơi ấm vàng cam ở chân trời
    Color4f dayTop(0.18f, 0.52f, 0.88f, 1.0f);     // Xanh lam ngọc
    Color4f dayBottom(0.98f, 0.88f, 0.72f, 1.0f);  // Vàng cam sa mạc ấm
    Color4f daySun(1.0f, 0.88f, 0.25f, 1.0f);      // Mặt trời vàng rực

    // Ban đêm: Xanh tím than vũ trụ -> Tím hoàng hôn huyền ảo
    Color4f nightTop(0.06f, 0.05f, 0.16f, 1.0f);   // Đen tím vũ trụ
    Color4f nightBottom(0.18f, 0.14f, 0.32f, 1.0f);// Tím huyền bí
    Color4f nightMoon(0.95f, 0.95f, 0.98f, 1.0f);  // Trăng sáng bạc

    if (t <= 0.5f) {
        float f = t * 2.0f;
        // Chuyển từ ngày sang hoàng hôn
        Color4f sunsetTop(0.24f, 0.22f, 0.55f, 1.0f);
        Color4f sunsetBottom(0.98f, 0.45f, 0.25f, 1.0f);
        skyTop = lerpColor(dayTop, sunsetTop, f);
        skyBottom = lerpColor(dayBottom, sunsetBottom, f);
        sunMoonColor = lerpColor(daySun, Color4f(1.0f, 0.45f, 0.1f, 1.0f), f);
    } else {
        float f = (t - 0.5f) * 2.0f;
        // Chuyển từ hoàng hôn sang đêm ngân hà
        Color4f sunsetTop(0.24f, 0.22f, 0.55f, 1.0f);
        Color4f sunsetBottom(0.98f, 0.45f, 0.25f, 1.0f);
        skyTop = lerpColor(sunsetTop, nightTop, f);
        skyBottom = lerpColor(sunsetBottom, nightBottom, f);
        sunMoonColor = lerpColor(Color4f(1.0f, 0.45f, 0.1f, 1.0f), nightMoon, f);
    }
}

// ============================================================================
// VẼ THIÊN THỂ & BẦU TRỜ NGHỆ THUẬT
// ============================================================================
void drawSkyAndCelestial() {
    Color4f skyTop, skyBottom, celestialColor;
    getSkyGradientColors(skyTop, skyBottom, celestialColor);

    // 1. Nền Gradient bầu trời toàn màn hình
    drawGradientRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, skyBottom, skyTop);

    // 2. Nếu là ban đêm: Vẽ bầu trời sao lấp lánh (Starfield)
    if (timeOfDayCycle > 0.4f) {
        float starAlpha = (timeOfDayCycle - 0.4f) / 0.6f;
        glEnable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, starAlpha);
        static const float starCoords[][3] = {
            {90, 510, 2.0f}, {170, 460, 3.0f}, {280, 520, 1.5f}, {360, 480, 2.5f},
            {490, 530, 2.0f}, {580, 470, 3.0f}, {670, 510, 1.8f}, {780, 450, 2.5f},
            {860, 520, 3.2f}, {940, 470, 1.5f}, {420, 440, 2.0f}, {130, 420, 1.5f}
        };
        for (int i = 0; i < 12; ++i) {
            float sx = starCoords[i][0];
            float sy = starCoords[i][1];
            float sz = starCoords[i][2];
            drawFilledCircle(sx, sy, sz, 8);
        }

        // Vẽ vệt Sao Băng (Shooting Star) lướt ngang
        float meteorT = fmod(distanceRun * 0.005f, 6.0f);
        if (meteorT < 1.2f) {
            float mx = 800.0f - meteorT * 280.0f;
            float my = 540.0f - meteorT * 120.0f;
            glLineWidth(2.5f);
            glBegin(GL_LINES);
            glColor4f(1.0f, 1.0f, 0.8f, starAlpha);
            glVertex2f(mx, my);
            glColor4f(1.0f, 0.6f, 0.2f, 0.0f);
            glVertex2f(mx + 65.0f, my + 28.0f);
            glEnd();
        }
    }

    // 3. Mặt Trời (Ban Ngày) hoặc Mặt Trăng (Ban Đêm) với Vầng Hào Quang
    float celestialX = WINDOW_WIDTH - 150.0f;
    float celestialY = WINDOW_HEIGHT - 90.0f;

    if (timeOfDayCycle <= 0.5f) {
        // MẶT TRỜI BAN NGÀY VỚI CÁC TẦNG HÀO QUANG PHÁT SÁNG (MULTI-LAYER CORONA)
        float sunR = 30.0f;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Vòng phát sáng ngoại vi
        glColor4f(1.0f, 0.85f, 0.3f, 0.18f);
        drawFilledCircle(celestialX, celestialY, sunR * 2.2f, 32);
        glColor4f(1.0f, 0.75f, 0.2f, 0.32f);
        drawFilledCircle(celestialX, celestialY, sunR * 1.5f, 32);

        // Lõi mặt trời vàng ấm
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.95f, 0.6f, 1.0f);
        drawFilledCircle(celestialX, celestialY, sunR, 32);
    } else {
        // MẶT TRĂNG KHUYẾT (CRESCENT MOON) VỚI QUẦNG SÁNG LUNAR HALO
        float moonR = 26.0f;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Hào quang trăng mờ
        glColor4f(0.8f, 0.85f, 1.0f, 0.20f);
        drawFilledCircle(celestialX, celestialY, moonR * 1.8f, 32);

        // Vành trăng sáng
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.95f, 0.96f, 1.0f, 1.0f);
        drawFilledCircle(celestialX, celestialY, moonR, 32);

        // Cắt vát tạo hình lưỡi liềm nghệ thuật
        Color4f skyTop, skyBottom, dummy;
        getSkyGradientColors(skyTop, skyBottom, dummy);
        glColor4f(skyTop.r, skyTop.g, skyTop.b, 1.0f);
        drawFilledCircle(celestialX + 10.0f, celestialY + 4.0f, moonR * 0.88f, 32);
    }
}

// ============================================================================
// VẼ NÚI THỊ SAI PARALLAX (3 TẦNG ĐỘ SÂU KHÔNG GIAN)
// ============================================================================
void drawParallaxMountains() {
    float nightFactor = timeOfDayCycle;

    // 1. TẦNG 1: DÃY NÚI TIỀN SỬ XA (Far Mountains - Tốc độ cuộn 6%)
    for (const auto& m : farMountains) {
        float peakX = m.x;
        float peakY = m.y + m.h;
        float halfW = m.w * 0.5f;

        Color4f cBase = lerpColor(m.color, Color4f(0.12f, 0.10f, 0.22f, 0.8f), nightFactor);
        Color4f cTop  = lerpColor(Color4f(0.55f, 0.62f, 0.82f, 0.6f), Color4f(0.20f, 0.16f, 0.35f, 0.8f), nightFactor);

        glBegin(GL_TRIANGLES);
        glColor4f(cTop.r, cTop.g, cTop.b, cTop.a);
        glVertex2f(peakX, peakY);
        glColor4f(cBase.r, cBase.g, cBase.b, cBase.a);
        glVertex2f(peakX - halfW, m.y);
        glVertex2f(peakX + halfW, m.y);
        glEnd();
    }

    // 2. TẦNG 2: ĐỤN CÁT CANYON TẦM TRUNG (Mid Dunes - Tốc độ cuộn 18%)
    for (const auto& d : midDunes) {
        float cx = d.x;
        float cy = d.y + d.h;
        float hw = d.w * 0.5f;

        Color4f dBase = lerpColor(d.color, Color4f(0.22f, 0.15f, 0.26f, 0.85f), nightFactor);
        Color4f dHighlight = lerpColor(Color4f(0.95f, 0.70f, 0.45f, 0.9f), Color4f(0.32f, 0.22f, 0.38f, 0.85f), nightFactor);

        // Vẽ vòm cong đụn cát mềm mại
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(dHighlight.r, dHighlight.g, dHighlight.b, dHighlight.a);
        glVertex2f(cx, cy);
        glColor4f(dBase.r, dBase.g, dBase.b, dBase.a);
        for (int i = 0; i <= 16; ++i) {
            float ang = (float)i * (float)M_PI / 16.0f;
            glVertex2f(cx - hw * cos(ang), d.y + d.h * sin(ang));
        }
        glEnd();
    }
}

// Vẽ Đám Mây Mềm Mại Thể Tích (Volumetric Clouds)
void drawFluffyCloud(const Cloud& c) {
    float x = c.x;
    float y = c.y;
    float s = c.scale;
    float alpha = c.opacity;

    float nightFactor = timeOfDayCycle;
    Color4f cloudTop = lerpColor(Color4f(1.0f, 1.0f, 1.0f, alpha), Color4f(0.4f, 0.38f, 0.55f, alpha * 0.7f), nightFactor);
    Color4f cloudBot = lerpColor(Color4f(0.85f, 0.88f, 0.94f, alpha), Color4f(0.25f, 0.22f, 0.38f, alpha * 0.7f), nightFactor);

    // Vẽ các cụm tròn liên kết tạo hình đám mây bồng bềnh
    glColor4f(cloudBot.r, cloudBot.g, cloudBot.b, cloudBot.a);
    drawFilledCircle(x + 18.0f * s, y, 16.0f * s, 16);
    drawFilledCircle(x + 48.0f * s, y, 16.0f * s, 16);

    glColor4f(cloudTop.r, cloudTop.g, cloudTop.b, cloudTop.a);
    drawFilledCircle(x + 32.0f * s, y + 8.0f * s, 20.0f * s, 18);
    drawFilledCircle(x + 22.0f * s, y + 4.0f * s, 14.0f * s, 16);
    drawFilledCircle(x + 42.0f * s, y + 4.0f * s, 14.0f * s, 16);
}

// ============================================================================
// VẼ MẶT ĐẤT SA MẠC ĐA TẦNG ĐỊA CHẤT (RICH LAYERED GROUND)
// ============================================================================
void drawRichGround() {
    float nightFactor = timeOfDayCycle;

    // Màu sắc các tầng địa chất đất cát
    Color4f surfaceGrass = lerpColor(Color4f(0.78f, 0.65f, 0.42f, 1.0f), Color4f(0.25f, 0.20f, 0.28f, 1.0f), nightFactor);
    Color4f sandLayer    = lerpColor(Color4f(0.68f, 0.48f, 0.28f, 1.0f), Color4f(0.18f, 0.14f, 0.22f, 1.0f), nightFactor);
    Color4f rockBed      = lerpColor(Color4f(0.38f, 0.24f, 0.14f, 1.0f), Color4f(0.10f, 0.08f, 0.14f, 1.0f), nightFactor);

    // 1. Tầng đất sâu (Subsurface Rock Bed)
    drawGradientRect(0, 0, WINDOW_WIDTH, GROUND_Y - 14.0f, rockBed, sandLayer);

    // 2. Tầng cát bề mặt (Surface Sand Layer)
    drawGradientRect(0, GROUND_Y - 14.0f, WINDOW_WIDTH, 14.0f, sandLayer, surfaceGrass);

    // 3. Đường viền cỏ cát sa mạc nổi bật
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glColor4f(surfaceGrass.r * 1.15f, surfaceGrass.g * 1.15f, surfaceGrass.b * 1.15f, 1.0f);
    glVertex2f(0.0f, GROUND_Y);
    glVertex2f((float)WINDOW_WIDTH, GROUND_Y);
    glEnd();

    // 4. Các vết rạn sỏi và mẩu xương hóa thạch cuộn theo địa hình
    float offset = fmod(distanceRun, 120.0f);
    for (float gx = -offset; gx < WINDOW_WIDTH + 80.0f; gx += 85.0f) {
        // Hòn sỏi nổi
        glColor4f(0.45f, 0.35f, 0.25f, 0.8f);
        drawFilledCircle(gx + 25.0f, GROUND_Y - 6.0f, 3.5f, 8);
        drawFilledCircle(gx + 32.0f, GROUND_Y - 8.0f, 2.0f, 8);

        // Hóa thạch xương tiền sử nhỏ nằm chìm trong cát
        glColor4f(0.90f, 0.88f, 0.82f, 0.65f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(gx + 55.0f, GROUND_Y - 10.0f);
        glVertex2f(gx + 68.0f, GROUND_Y - 10.0f);
        glVertex2f(gx + 55.0f, GROUND_Y - 8.0f);
        glVertex2f(gx + 55.0f, GROUND_Y - 12.0f);
        glVertex2f(gx + 68.0f, GROUND_Y - 8.0f);
        glVertex2f(gx + 68.0f, GROUND_Y - 12.0f);
        glEnd();
    }
}

// ============================================================================
// VẼ CHÚ KHỦNG LONG T-REX ĐỒ HỌA CAO (HIGH-DEFINITION CHARACTER T-REX)
// ============================================================================
void drawDetailedDino() {
    float x = dino.x;
    float y = dino.y;

    // 1. ĐỔ BÓNG HÌNH HỌC MỀM DƯỚI ĐẤT
    drawSoftDropShadow(x, y, 48.0f, 14.0f, y);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Phép biến đổi Affine xoay ngã khi Game Over
    if (dino.pose == POSE_DEAD) {
        glTranslatef(26.0f, 24.0f, 0.0f);
        glRotatef(dino.deadRotation, 0.0f, 0.0f, 1.0f);
        glTranslatef(-26.0f, -24.0f, 0.0f);
    }

    // Bảng màu Khủng long T-Rex thời tiền sử:
    // Thân xanh ngọc bích (Jade Green), Bụng vàng chanh sáng (Lime/Cream), Gai lưng xanh rêu thẫm
    Color4f bodyDino(0.18f, 0.62f, 0.32f, 1.0f);     // Xanh ngọc chính
    Color4f bellyDino(0.68f, 0.88f, 0.42f, 1.0f);    // Bụng sáng mềm
    Color4f dorsalSpine(0.10f, 0.38f, 0.20f, 1.0f);  // Gai lưng sừng
    Color4f clawColor(0.95f, 0.95f, 0.85f, 1.0f);    // Móng vuốt sắc

    if (dino.isDucking && dino.isGrounded) {
        // ====================================================================
        // KHỦNG LONG CÚI RẠP NGƯỜI (DUCKING / SLITHER POSE)
        // ====================================================================
        // 1. Đuôi uốn lượn sau
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        glBegin(GL_POLYGON);
        glVertex2f(0.0f, 14.0f);
        glVertex2f(16.0f, 18.0f);
        glVertex2f(22.0f, 6.0f);
        glVertex2f(6.0f, 8.0f);
        glEnd();

        // 2. Thân nằm ngang thon dài
        drawFilledEllipse(30.0f, 14.0f, 18.0f, 11.0f, 20);

        // Bụng sáng phía dưới
        glColor4f(bellyDino.r, bellyDino.g, bellyDino.b, 1.0f);
        drawFilledEllipse(32.0f, 10.0f, 14.0f, 6.0f, 16);

        // 3. Đầu vươn chúi về phía trước
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        drawFilledEllipse(52.0f, 16.0f, 15.0f, 9.0f, 18);
        drawFilledCircle(64.0f, 15.0f, 5.0f, 12); // Mõm

        // Răng sắc nhọn
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(56.0f, 11.0f); glVertex2f(59.0f, 7.0f); glVertex2f(62.0f, 11.0f);
        glEnd();

        // Mắt hổ phách có con ngươi
        glColor4f(1.0f, 0.82f, 0.15f, 1.0f);
        drawFilledCircle(56.0f, 18.0f, 3.2f, 12);
        glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
        drawFilledCircle(56.5f, 18.0f, 1.6f, 10);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        drawFilledCircle(55.8f, 19.0f, 0.8f, 6); // Điểm sáng Specular

        // Chân khi bò
        glColor4f(dorsalSpine.r, dorsalSpine.g, dorsalSpine.b, 1.0f);
        if (dino.pose == POSE_DUCK_1) {
            drawFilledEllipse(26.0f, 5.0f, 7.0f, 4.0f, 12);
            drawFilledEllipse(40.0f, 7.0f, 7.0f, 4.0f, 12);
        } else {
            drawFilledEllipse(26.0f, 7.0f, 7.0f, 4.0f, 12);
            drawFilledEllipse(40.0f, 5.0f, 7.0f, 4.0f, 12);
        }

    } else {
        // ====================================================================
        // KHỦNG LONG ĐỨNG CHẠY & NHẢY OAI DŨNG
        // ====================================================================
        // 1. ĐUÔI KHỦNG LONG CONG VÚT
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        glBegin(GL_POLYGON);
        glVertex2f(0.0f, 32.0f);
        glVertex2f(8.0f, 28.0f);
        glVertex2f(16.0f, 20.0f);
        glVertex2f(14.0f, 14.0f);
        glVertex2f(6.0f, 22.0f);
        glEnd();

        // Gai sừng dọc sống lưng (Dorsal Spikes)
        glColor4f(dorsalSpine.r, dorsalSpine.g, dorsalSpine.b, 1.0f);
        for (int i = 0; i < 4; ++i) {
            float sx = 8.0f + (float)i * 6.0f;
            float sy = 30.0f + (float)i * 3.0f;
            glBegin(GL_TRIANGLES);
            glVertex2f(sx, sy);
            glVertex2f(sx + 3.0f, sy + 5.0f);
            glVertex2f(sx + 5.0f, sy);
            glEnd();
        }

        // 2. THÂN VÀ BỤNG KHỦNG LONG (CƠ BẮP MỀM MẠI)
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        drawFilledEllipse(22.0f, 24.0f, 14.0f, 16.0f, 24);

        // Bụng màu vàng chanh sáng
        glColor4f(bellyDino.r, bellyDino.g, bellyDino.b, 1.0f);
        drawFilledEllipse(26.0f, 21.0f, 8.0f, 11.0f, 20);

        // 3. TAY NHỎ VÀ MÓNG VUỐT (ARMS & CLAWS)
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(30.0f, 25.0f); glVertex2f(37.0f, 23.0f);
        glVertex2f(37.0f, 23.0f); glVertex2f(41.0f, 26.0f);
        glEnd();
        // Móng vuốt
        glColor4f(clawColor.r, clawColor.g, clawColor.b, 1.0f);
        drawFilledCircle(41.0f, 26.0f, 1.8f, 8);

        // 4. CỔ VÀ ĐẦU KHỦNG LONG OAI PHONG
        glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
        drawFilledEllipse(34.0f, 44.0f, 13.0f, 10.0f, 20); // Hộp sọ
        drawFilledCircle(43.0f, 42.0f, 6.5f, 16);           // Mõm trước

        // Mép hàm mở rộng với hàng răng trắng sắc
        glColor4f(0.85f, 0.2f, 0.25f, 1.0f); // Lưỡi/vòm họng đỏ
        drawFilledCircle(40.0f, 38.0f, 4.0f, 12);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Răng
        glBegin(GL_TRIANGLES);
        glVertex2f(38.0f, 41.0f); glVertex2f(40.0f, 37.0f); glVertex2f(42.0f, 41.0f);
        glVertex2f(42.0f, 41.0f); glVertex2f(44.0f, 38.0f); glVertex2f(46.0f, 41.0f);
        glEnd();

        // 5. MẮT HỔ PHÁCH VỚI CON NGƯƠI VÀ ĐIỂM SÁNG SPECULAR
        if (dino.pose == POSE_DEAD) {
            // Mắt chữ X khi chết
            glColor4f(1.0f, 0.2f, 0.2f, 1.0f);
            glLineWidth(2.5f);
            glBegin(GL_LINES);
            glVertex2f(32.0f, 44.0f); glVertex2f(38.0f, 50.0f);
            glVertex2f(32.0f, 50.0f); glVertex2f(38.0f, 44.0f);
            glEnd();
        } else {
            // Mắt sống động
            glColor4f(1.0f, 0.85f, 0.15f, 1.0f); // Tròng vàng hổ phách
            drawFilledCircle(35.0f, 47.0f, 3.8f, 14);
            glColor4f(0.1f, 0.1f, 0.15f, 1.0f);  // Con ngươi đen
            drawFilledCircle(36.0f, 47.0f, 1.8f, 10);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);   // Đốm sáng
            drawFilledCircle(35.2f, 48.2f, 1.0f, 8);
        }

        // 6. KHỚP CHÂN KHỦNG LONG ĐƯỢC ĐIỀU KHIỂN BẰNG MA TRẬN AFFINE XOAY QUANH KHỚP HÁNG
        float hipPivotX = 22.0f, hipPivotY = 16.0f;

        if (!dino.isGrounded) {
            // Khi đang nhảy: Chân co cong về phía sau
            glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
            drawFilledEllipse(18.0f, 10.0f, 5.0f, 8.0f, 14);
            glColor4f(clawColor.r, clawColor.g, clawColor.b, 1.0f);
            drawFilledCircle(20.0f, 4.0f, 3.0f, 8);
        } else {
            // Khi chạy: Hai chân xoay luân phiên quanh khớp háng
            float legAngLeft  = (dino.pose == POSE_RUN_1) ? 22.0f : -22.0f;
            float legAngRight = (dino.pose == POSE_RUN_1) ? -22.0f : 22.0f;

            // Chân trái
            glPushMatrix();
            glTranslatef(hipPivotX, hipPivotY, 0.0f);
            glRotatef(legAngLeft, 0.0f, 0.0f, 1.0f);
            glTranslatef(-hipPivotX, -hipPivotY, 0.0f);
            glColor4f(bodyDino.r * 0.9f, bodyDino.g * 0.9f, bodyDino.b * 0.9f, 1.0f);
            drawFilledEllipse(hipPivotX - 2.0f, hipPivotY - 7.0f, 4.5f, 9.0f, 16);
            glColor4f(clawColor.r, clawColor.g, clawColor.b, 1.0f);
            drawFilledCircle(hipPivotX - 1.0f, hipPivotY - 15.0f, 3.2f, 8);
            glPopMatrix();

            // Chân phải (Sáng hơn ở phía trước)
            glPushMatrix();
            glTranslatef(hipPivotX + 6.0f, hipPivotY, 0.0f);
            glRotatef(legAngRight, 0.0f, 0.0f, 1.0f);
            glTranslatef(-(hipPivotX + 6.0f), -hipPivotY, 0.0f);
            glColor4f(bodyDino.r, bodyDino.g, bodyDino.b, 1.0f);
            drawFilledEllipse(hipPivotX + 4.0f, hipPivotY - 7.0f, 4.5f, 9.0f, 16);
            glColor4f(clawColor.r, clawColor.g, clawColor.b, 1.0f);
            drawFilledCircle(hipPivotX + 5.0f, hipPivotY - 15.0f, 3.2f, 8);
            glPopMatrix();
        }
    }

    // 7. HIỆU ỨNG KHIÊN BẢO VỆ PHÁT SÁNG NĂNG LƯỢNG (ARCADE SHIELD AURA)
    if (hasShield) {
        float pulse = 1.0f + 0.08f * sin(shieldAura * 9.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Vòng phát sáng ngoại vi
        glColor4f(0.2f, 0.8f, 1.0f, 0.25f);
        drawFilledCircle(26.0f, 25.0f, 36.0f * pulse, 32);

        // Viền khiên bảo vệ phát sáng
        glLineWidth(2.8f);
        glBegin(GL_LINE_LOOP);
        glColor4f(0.4f, 0.95f, 1.0f, 0.9f);
        for (int i = 0; i <= 32; ++i) {
            float ang = (float)i * 2.0f * (float)M_PI / 32.0f;
            glVertex2f(26.0f + 32.0f * pulse * cos(ang), 25.0f + 32.0f * pulse * sin(ang));
        }
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glPopMatrix();
}

// ============================================================================
// VẼ CÂY XƯƠNG RỒNG NỞ HOA SA MẠC (DETAILED DESERT CACTUS)
// ============================================================================
void drawDetailedCactus(const Obstacle& obs) {
    float x = obs.x;
    float y = obs.y;
    float w = obs.width;
    float h = obs.height;

    // Màu xương rồng Saguaro: Xanh lục bảo, có đổ bóng rãnh thân
    Color4f cactusGreen(0.18f, 0.58f, 0.28f, 1.0f);
    Color4f cactusLight(0.32f, 0.75f, 0.38f, 1.0f);
    Color4f cactusShadow(0.10f, 0.38f, 0.18f, 1.0f);

    // 1. Thân chính hình trụ bo tròn
    float trunkW = (obs.type == OBS_CACTUS_GIANT) ? 14.0f : 10.0f;
    float trunkX = x + w * 0.5f;

    // Thân xương rồng
    glColor4f(cactusGreen.r, cactusGreen.g, cactusGreen.b, 1.0f);
    drawFilledEllipse(trunkX, y + h - trunkW * 0.5f, trunkW * 0.5f, trunkW * 0.5f, 16);
    glBegin(GL_QUADS);
    glVertex2f(trunkX - trunkW * 0.5f, y);
    glVertex2f(trunkX + trunkW * 0.5f, y);
    glVertex2f(trunkX + trunkW * 0.5f, y + h - trunkW * 0.5f);
    glVertex2f(trunkX - trunkW * 0.5f, y + h - trunkW * 0.5f);
    glEnd();

    // Rãnh sọc dọc xương rồng (Ribs)
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glColor4f(cactusLight.r, cactusLight.g, cactusLight.b, 0.8f);
    glVertex2f(trunkX - 2.0f, y); glVertex2f(trunkX - 2.0f, y + h - 5.0f);
    glColor4f(cactusShadow.r, cactusShadow.g, cactusShadow.b, 0.8f);
    glVertex2f(trunkX + 2.0f, y); glVertex2f(trunkX + 2.0f, y + h - 5.0f);
    glEnd();

    // 2. Các nhánh ngang vươn lên
    if (obs.type != OBS_CACTUS_SINGLE) {
        float armH = h * 0.55f;
        // Nhánh trái
        glColor4f(cactusGreen.r, cactusGreen.g, cactusGreen.b, 1.0f);
        drawFilledCircle(x + 4.0f, y + armH, 4.0f, 12);
        glBegin(GL_QUADS);
        glVertex2f(x + 2.0f, y + armH * 0.6f);
        glVertex2f(x + 6.0f, y + armH * 0.6f);
        glVertex2f(x + 6.0f, y + armH);
        glVertex2f(x + 2.0f, y + armH);

        glVertex2f(x + 4.0f, y + armH * 0.6f);
        glVertex2f(trunkX,    y + armH * 0.6f);
        glVertex2f(trunkX,    y + armH * 0.6f + 5.0f);
        glVertex2f(x + 4.0f, y + armH * 0.6f + 5.0f);
        glEnd();

        // Nhánh phải
        float rightArmH = h * 0.65f;
        drawFilledCircle(x + w - 4.0f, y + rightArmH, 4.0f, 12);
        glBegin(GL_QUADS);
        glVertex2f(x + w - 6.0f, y + rightArmH * 0.55f);
        glVertex2f(x + w - 2.0f, y + rightArmH * 0.55f);
        glVertex2f(x + w - 2.0f, y + rightArmH);
        glVertex2f(x + w - 6.0f, y + rightArmH);

        glVertex2f(trunkX,          y + rightArmH * 0.55f);
        glVertex2f(x + w - 4.0f, y + rightArmH * 0.55f);
        glVertex2f(x + w - 4.0f, y + rightArmH * 0.55f + 5.0f);
        glVertex2f(trunkX,          y + rightArmH * 0.55f + 5.0f);
        glEnd();
    }

    // 3. Hoa sa mạc nở rộ rực rỡ trên đỉnh xương rồng (Desert Blossom)
    if (obs.type == OBS_CACTUS_FLOWER || obs.type == OBS_CACTUS_SINGLE) {
        float fx = trunkX;
        float fy = y + h + 2.0f;
        glColor4f(1.0f, 0.25f, 0.60f, 1.0f); // Cánh hoa hồng cánh sen rực rỡ
        for (int p = 0; p < 5; ++p) {
            float ang = (float)p * 2.0f * (float)M_PI / 5.0f;
            drawFilledCircle(fx + 4.0f * cos(ang), fy + 4.0f * sin(ang), 3.0f, 10);
        }
        glColor4f(1.0f, 0.90f, 0.20f, 1.0f); // Nhụy vàng
        drawFilledCircle(fx, fy, 2.5f, 10);
    }
}

// ============================================================================
// VẼ THẰN LẰN BAY TIỀN SỬ (MAJESTIC PTERODACTYL) VỚI CÁNH ĐẬP AFFINE 2D
// ============================================================================
void drawDetailedPterodactyl(const Obstacle& obs) {
    glPushMatrix();
    glTranslatef(obs.x, obs.y, 0.0f);

    // Bảng màu: Thân tím thẫm (Crimson Purple), Cánh màng đỏ mận, Mỏ vàng cam
    Color4f birdBody(0.55f, 0.18f, 0.38f, 1.0f);
    Color4f wingMembrane(0.85f, 0.32f, 0.35f, 0.9f);
    Color4f beakColor(1.0f, 0.75f, 0.15f, 1.0f);

    // 1. Mỏ nhọn và mào đầu tiền sử (Crest)
    glColor4f(beakColor.r, beakColor.g, beakColor.b, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 15.0f);
    glVertex2f(14.0f, 18.0f);
    glVertex2f(14.0f, 12.0f);
    glEnd();

    // Mào đầu vút dài ra sau
    glColor4f(birdBody.r, birdBody.g, birdBody.b, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(14.0f, 18.0f);
    glVertex2f(14.0f, 12.0f);
    glVertex2f(26.0f, 22.0f); // Đỉnh nhọn mào
    glEnd();

    // Mắt sắc lạnh
    glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
    drawFilledCircle(12.0f, 16.0f, 2.2f, 10);
    glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
    drawFilledCircle(11.5f, 16.0f, 1.2f, 8);

    // 2. Thân thon và đuôi chim
    glColor4f(birdBody.r, birdBody.g, birdBody.b, 1.0f);
    drawFilledEllipse(24.0f, 13.0f, 12.0f, 6.0f, 16);
    glBegin(GL_TRIANGLES);
    glVertex2f(32.0f, 14.0f); glVertex2f(32.0f, 11.0f); glVertex2f(44.0f, 13.0f);
    glEnd();

    // 3. CÁNH VỖ LÊN / XUỐNG DÙNG PHÉP XOAY MA TRẬN AFFINE QUANH KHỚP VAI
    float wingPivotX = 22.0f, wingPivotY = 14.0f;
    float wingAngle = (obs.wingState == 0) ? 38.0f : -38.0f;

    glPushMatrix();
    glTranslatef(wingPivotX, wingPivotY, 0.0f);
    glRotatef(wingAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-wingPivotX, -wingPivotY, 0.0f);

    // Màng cánh sải rộng
    glColor4f(wingMembrane.r, wingMembrane.g, wingMembrane.b, wingMembrane.a);
    glBegin(GL_POLYGON);
    glVertex2f(wingPivotX, wingPivotY);
    glVertex2f(wingPivotX - 6.0f, wingPivotY + 22.0f);
    glVertex2f(wingPivotX + 18.0f, wingPivotY + 24.0f);
    glVertex2f(wingPivotX + 12.0f, wingPivotY + 6.0f);
    glEnd();

    // Xương cánh cứng cáp
    glColor4f(birdBody.r, birdBody.g, birdBody.b, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(wingPivotX, wingPivotY);
    glVertex2f(wingPivotX + 18.0f, wingPivotY + 24.0f);
    glEnd();

    glPopMatrix();

    glPopMatrix();
}

// ============================================================================
// VẼ ĐƯỜNG CONG BÉZIER DỰ BÁO QUỸ ĐẠO NHẢY & ĐA GIÁC KIỂM SOÁT (SLIDE CG.5)
// ============================================================================
void drawBezierJumpTrajectory() {
    if (!showBezierArc || gameState != STATE_PLAYING) return;

    Point2D p0(dino.x + 22.0f, GROUND_Y);

    float tFlight = 2.0f * (JUMP_VELOCITY / GRAVITY);
    float jumpDistance = gameSpeed * tFlight;
    float maxJumpH = (JUMP_VELOCITY * JUMP_VELOCITY) / (2.0f * GRAVITY);

    Point2D p1(p0.x + jumpDistance * 0.5f, GROUND_Y + maxJumpH * 1.35f);
    Point2D p2(p0.x + jumpDistance, GROUND_Y);

    // 1. Đa giác kiểm soát nét đứt P0 -> P1 -> P2
    glLineWidth(1.8f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(3, 0xAAAA);
    glColor4f(1.0f, 0.65f, 0.15f, 0.85f);
    glBegin(GL_LINES);
    glVertex2f(p0.x, p0.y); glVertex2f(p1.x, p1.y);
    glVertex2f(p1.x, p1.y); glVertex2f(p2.x, p2.y);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // 2. Điểm kiểm soát P0, P1, P2 phát sáng
    glColor4f(0.2f, 0.9f, 0.4f, 1.0f); drawFilledCircle(p0.x, p0.y, 4.5f, 12);
    glColor4f(1.0f, 0.25f, 0.25f, 1.0f); drawFilledCircle(p1.x, p1.y, 6.0f, 14);
    glColor4f(0.2f, 0.7f, 1.0f, 1.0f); drawFilledCircle(p2.x, p2.y, 4.5f, 12);

    // 3. Đường cong Bézier bậc 2 (Quadratic Bézier Curve)
    glLineWidth(2.8f);
    glColor4f(0.15f, 0.85f, 1.0f, 0.95f);
    glBegin(GL_LINE_STRIP);
    int numSteps = 45;
    for (int i = 0; i <= numSteps; ++i) {
        float t = (float)i / (float)numSteps;
        Point2D pt = evaluateBezierQuadratic(p0, p1, p2, t);
        glVertex2f(pt.x, pt.y);
    }
    glEnd();

    // 4. Nếu Dino đang nhảy trên không: Trực quan hóa thuật toán De Casteljau
    if (!dino.isGrounded) {
        float curT = (dino.x + 22.0f - p0.x) / jumpDistance;
        curT = max(0.0f, min(1.0f, curT));
        Point2D q0, q1;
        Point2D curPt = deCasteljauQuadratic(p0, p1, p2, curT, q0, q1);

        glLineWidth(2.0f);
        glColor4f(1.0f, 0.9f, 0.2f, 0.9f);
        glBegin(GL_LINES);
        glVertex2f(q0.x, q0.y); glVertex2f(q1.x, q1.y);
        glEnd();

        glColor4f(1.0f, 0.1f, 0.5f, 1.0f);
        drawFilledCircle(curPt.x, curPt.y, 6.5f, 14);
    }
}

// ============================================================================
// VẼ GIAO DIỆN HUD HIỆN ĐẠI (GLASSMORPHIC COMBAT HUD)
// ============================================================================
void drawModernHUD() {
    // 1. Khung Glassmorphic cho Bảng Điểm Kỷ Lục ở góc trên phải
    float scoreBoxW = 240.0f;
    float scoreBoxH = 46.0f;
    float scoreBoxX = WINDOW_WIDTH - scoreBoxW - 25.0f;
    float scoreBoxY = WINDOW_HEIGHT - scoreBoxH - 20.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.06f, 0.08f, 0.14f, 0.75f); // Kính tối bán trong suốt
    drawFilledEllipse(scoreBoxX + 16.0f, scoreBoxY + scoreBoxH * 0.5f, 16.0f, scoreBoxH * 0.5f, 16);
    drawFilledEllipse(scoreBoxX + scoreBoxW - 16.0f, scoreBoxY + scoreBoxH * 0.5f, 16.0f, scoreBoxH * 0.5f, 16);
    glBegin(GL_QUADS);
    glVertex2f(scoreBoxX + 16.0f, scoreBoxY);
    glVertex2f(scoreBoxX + scoreBoxW - 16.0f, scoreBoxY);
    glVertex2f(scoreBoxX + scoreBoxW - 16.0f, scoreBoxY + scoreBoxH);
    glVertex2f(scoreBoxX + 16.0f, scoreBoxY + scoreBoxH);
    glEnd();

    // Viền vàng kim / cyan phát sáng
    glLineWidth(2.0f);
    glColor4f(1.0f, 0.85f, 0.25f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(scoreBoxX + 16.0f, scoreBoxY);
    glVertex2f(scoreBoxX + scoreBoxW - 16.0f, scoreBoxY);
    glVertex2f(scoreBoxX + scoreBoxW - 16.0f, scoreBoxY + scoreBoxH);
    glVertex2f(scoreBoxX + 16.0f, scoreBoxY + scoreBoxH);
    glEnd();

    // Chuỗi điểm số với hiệu ứng Fanfare nhấp nháy màu khi vượt 100 điểm
    stringstream ss;
    ss << "HI " << setfill('0') << setw(5) << (int)highScore
       << "   " << setfill('0') << setw(5) << (int)currentScore;

    if (milestoneFlash > 0.0f) {
        int flashFrame = (int)(milestoneFlash * 10.0f);
        if (flashFrame % 2 == 0) {
            glColor4f(1.0f, 0.95f, 0.2f, 1.0f); // Vàng rực phát sáng
        } else {
            glColor4f(0.2f, 1.0f, 0.5f, 1.0f); // Xanh neon
        }
    } else {
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
    }

    renderBitmapString(scoreBoxX + 28.0f, scoreBoxY + 16.0f,
                       GLUT_BITMAP_9_BY_15, ss.str());

    // 2. Huy hiệu Chế độ chơi & Tốc độ chạy ở góc trên trái
    float infoBoxW = 280.0f;
    float infoBoxH = 46.0f;
    float infoBoxX = 25.0f;
    float infoBoxY = WINDOW_HEIGHT - infoBoxH - 20.0f;

    glColor4f(0.06f, 0.08f, 0.14f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(infoBoxX, infoBoxY);
    glVertex2f(infoBoxX + infoBoxW, infoBoxY);
    glVertex2f(infoBoxX + infoBoxW, infoBoxY + infoBoxH);
    glVertex2f(infoBoxX, infoBoxY + infoBoxH);
    glEnd();

    glLineWidth(1.8f);
    glColor4f(0.2f, 0.8f, 1.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(infoBoxX, infoBoxY);
    glVertex2f(infoBoxX + infoBoxW, infoBoxY);
    glVertex2f(infoBoxX + infoBoxW, infoBoxY + infoBoxH);
    glVertex2f(infoBoxX, infoBoxY + infoBoxH);
    glEnd();

    stringstream ssSpd;
    ssSpd << "SPD: " << (int)gameSpeed << " px/s";
    glColor4f(0.2f, 1.0f, 0.5f, 1.0f);
    renderBitmapString(infoBoxX + 15.0f, infoBoxY + 26.0f, GLUT_BITMAP_HELVETICA_12,
                       (gameMode == MODE_ARCADE) ? "MODE: ARCADE POWER-UP" : "MODE: CHROME CLASSIC");

    glColor4f(0.9f, 0.9f, 0.9f, 0.85f);
    stringstream ssInfo;
    ssInfo << ssSpd.str() << " | [TAB]: ACADEMIC HUD";
    renderBitmapString(infoBoxX + 15.0f, infoBoxY + 10.0f, GLUT_BITMAP_HELVETICA_10, ssInfo.str());
}

// Bảng Học Thuật Đồ Họa Máy Tính (Academic HUD - Phím TAB)
void drawAcademicOverlay() {
    if (!showAcademicHUD) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float boxX = 25.0f, boxY = 65.0f;
    float boxW = 430.0f, boxH = 410.0f;

    glColor4f(0.04f, 0.06f, 0.10f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(boxX, boxY); glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH); glVertex2f(boxX, boxY + boxH);
    glEnd();

    glLineWidth(2.2f);
    glColor4f(0.15f, 0.85f, 1.0f, 0.95f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxX, boxY); glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH); glVertex2f(boxX, boxY + boxH);
    glEnd();

    glColor4f(0.2f, 1.0f, 0.5f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 28.0f, GLUT_BITMAP_HELVETICA_12,
                       "=== VKU COMPUTER GRAPHICS: CHROME T-REX ===");

    // 1. Phép biến đổi Affine
    glColor4f(1.0f, 0.85f, 0.25f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 55.0f, GLUT_BITMAP_HELVETICA_10,
                       "[1] 2D AFFINE TRANSFORMATION (Slide CG.3):");
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
    stringstream s1; s1 << "  Dino Pos: (" << (int)dino.x << ", " << (int)dino.y << ") | Ground: " << (int)GROUND_Y;
    renderBitmapString(boxX + 18.0f, boxY + boxH - 72.0f, GLUT_BITMAP_HELVETICA_10, s1.str());
    renderBitmapString(boxX + 18.0f, boxY + boxH - 88.0f, GLUT_BITMAP_HELVETICA_10,
                       "  Pivot Rotation: M_pivot = T(p) * R(theta) * T(-p)");
    renderBitmapString(boxX + 18.0f, boxY + boxH - 104.0f, GLUT_BITMAP_HELVETICA_10,
                       "  Parallax Translations: Far(6%), Mid(18%), Foreground(100%)");

    // 2. Thuật toán va chạm AABB
    glColor4f(1.0f, 0.45f, 0.45f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 130.0f, GLUT_BITMAP_HELVETICA_10,
                       "[2] MULTI-BOX AABB COLLISION (Slide CG.3):");
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
    stringstream s2; s2 << "  Dino Hitboxes: " << dino.getHitboxes().size() << " boxes (Head, Torso, Legs, Tail)";
    renderBitmapString(boxX + 18.0f, boxY + boxH - 148.0f, GLUT_BITMAP_HELVETICA_10, s2.str());
    stringstream s3; s3 << "  Active Obstacles: " << obstacles.size() << " | Ducking: " << (dino.isDucking ? "YES (H=24px)" : "NO (H=48px)");
    renderBitmapString(boxX + 18.0f, boxY + boxH - 164.0f, GLUT_BITMAP_HELVETICA_10, s3.str());

    // 3. Đường cong Bézier bậc 2
    glColor4f(0.35f, 0.85f, 1.0f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 192.0f, GLUT_BITMAP_HELVETICA_10,
                       "[3] QUADRATIC BEZIER JUMP (Slide CG.5):");
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 210.0f, GLUT_BITMAP_HELVETICA_10,
                       "  Bernstein: P(t) = (1-t)^2*P0 + 2t(1-t)*P1 + t^2*P2");

    float tFlight = 2.0f * (JUMP_VELOCITY / GRAVITY);
    float jumpDist = gameSpeed * tFlight;
    stringstream s4;
    s4 << "  P0=(" << (int)(dino.x+22) << ", " << (int)GROUND_Y << ") Apex P1=("
       << (int)(dino.x+22+jumpDist*0.5f) << ", " << (int)(GROUND_Y + (JUMP_VELOCITY*JUMP_VELOCITY)/(2*GRAVITY)*1.35f) << ")";
    renderBitmapString(boxX + 18.0f, boxY + boxH - 226.0f, GLUT_BITMAP_HELVETICA_10, s4.str());

    // 4. Phím tắt & Điều khiển
    glColor4f(0.85f, 0.55f, 1.0f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 252.0f, GLUT_BITMAP_HELVETICA_10,
                       "[4] CONTROLS & GRAPHICS SHORTCUTS:");
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
    renderBitmapString(boxX + 18.0f, boxY + boxH - 272.0f, GLUT_BITMAP_HELVETICA_10, "  [TAB] An/Hien Bang Nay  | [B] Duong Cong Bezier");
    renderBitmapString(boxX + 18.0f, boxY + boxH - 288.0f, GLUT_BITMAP_HELVETICA_10, "  [H] Khung Xuong Hitbox  | [N] Doi Chu Ky Ngay/Dem");
    renderBitmapString(boxX + 18.0f, boxY + boxH - 304.0f, GLUT_BITMAP_HELVETICA_10, "  [1/2] Doi Che Do Choi   | [R] Choi Lai Ngay");
    renderBitmapString(boxX + 18.0f, boxY + boxH - 324.0f, GLUT_BITMAP_HELVETICA_10, "  SPACE/UP: Nhay  | DOWN: Cui Nguoi / Roi Nhanh");
}

// Màn hình Game Over
void drawGameOverScreen() {
    float cx = WINDOW_WIDTH * 0.5f;
    float cy = WINDOW_HEIGHT * 0.5f;

    // Bảng thông báo Game Over Glassmorphic
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.06f, 0.08f, 0.14f, 0.85f);
    drawFilledEllipse(cx, cy + 15.0f, 170.0f, 65.0f, 24);

    glLineWidth(2.5f);
    glColor4f(1.0f, 0.35f, 0.25f, 0.9f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 32; ++i) {
        float ang = (float)i * 2.0f * (float)M_PI / 32.0f;
        glVertex2f(cx + 170.0f * cos(ang), cy + 15.0f + 65.0f * sin(ang));
    }
    glEnd();

    glColor4f(1.0f, 0.3f, 0.25f, 1.0f);
    renderBitmapString(cx - 72.0f, cy + 32.0f, GLUT_BITMAP_HELVETICA_18, "G A M E   O V E R");

    glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
    renderBitmapString(cx - 105.0f, cy, GLUT_BITMAP_8_BY_13, "BAM [SPACE] HOAC [R] DE CHOI LAI");
}

// Màn hình bắt đầu (Start Screen)
void drawStartScreen() {
    float cx = WINDOW_WIDTH * 0.5f;
    float cy = WINDOW_HEIGHT * 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.07f, 0.12f, 0.88f);
    drawFilledEllipse(cx, cy + 25.0f, 290.0f, 85.0f, 32);

    glLineWidth(2.5f);
    glColor4f(0.2f, 0.85f, 1.0f, 0.9f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 40; ++i) {
        float ang = (float)i * 2.0f * (float)M_PI / 40.0f;
        glVertex2f(cx + 290.0f * cos(ang), cy + 25.0f + 85.0f * sin(ang));
    }
    glEnd();

    glColor4f(0.25f, 1.0f, 0.55f, 1.0f);
    renderBitmapString(cx - 200.0f, cy + 62.0f, GLUT_BITMAP_HELVETICA_18,
                       "CHROME T-REX RUNNER (HD ART EDITION)");

    glColor4f(0.85f, 0.92f, 1.0f, 1.0f);
    renderBitmapString(cx - 180.0f, cy + 38.0f, GLUT_BITMAP_HELVETICA_12,
                       "Do An Giua Ky Mon Do Hoa May Tinh (Computer Graphics) - VKU");

    glColor4f(1.0f, 0.85f, 0.25f, 1.0f);
    renderBitmapString(cx - 150.0f, cy + 5.0f, GLUT_BITMAP_8_BY_13,
                       "BAM [SPACE] HOAC CLICK CHUOT DE BAT DAU!");

    glColor4f(0.75f, 0.80f, 0.90f, 0.9f);
    renderBitmapString(cx - 215.0f, cy - 22.0f, GLUT_BITMAP_HELVETICA_10,
                       "[1] Che Do Classic (Chuan Chrome)   |   [2] Che Do Arcade (Power-Up & Dan Lua)");
}

// ============================================================================
// HÀM HIỂN THỊ CHÍNH (OPENGL DISPLAY CALLBACK)
// ============================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 1. Bầu trời Gradient nghệ thuật & Mặt trời / Mặt trăng
    drawSkyAndCelestial();

    // 2. Núi thị sai Parallax (3 tầng) & Mây trôi
    drawParallaxMountains();
    for (const auto& c : clouds) drawFluffyCloud(c);

    // 3. Mặt đất sa mạc nhiều tầng địa chất
    drawRichGround();

    // 4. Quỹ đạo nhảy Bézier bậc 2 (Slide CG.5)
    drawBezierJumpTrajectory();

    // 5. Chướng ngại vật: Cây xương rồng nở hoa & Thằn lằn bay
    for (const auto& obs : obstacles) {
        if (obs.type >= OBS_BIRD_LOW) drawDetailedPterodactyl(obs);
        else                          drawDetailedCactus(obs);
    }

    // 6. Chú Khủng Long T-Rex đồ họa cao
    drawDetailedDino();

    // 7. Hệ thống hạt bụi cát và tia lửa (Particle System)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& p : particles) {
        glColor4f(p.color.r, p.color.g, p.color.b, p.life * p.color.a);
        drawFilledCircle(p.x, p.y, p.size * 0.5f, 8);
    }

    // 8. Khung viền Hitbox AABB (nếu bật phím H)
    if (showHitboxes) {
        glLineWidth(1.8f);
        glColor4f(0.2f, 1.0f, 0.3f, 0.9f);
        for (const auto& b : dino.getHitboxes()) {
            glBegin(GL_LINE_LOOP);
            glVertex2f(b.x, b.y); glVertex2f(b.x + b.w, b.y);
            glVertex2f(b.x + b.w, b.y + b.h); glVertex2f(b.x, b.y + b.h);
            glEnd();
        }
        glColor4f(1.0f, 0.2f, 0.2f, 0.9f);
        for (const auto& obs : obstacles) {
            for (const auto& b : obs.getHitboxes()) {
                glBegin(GL_LINE_LOOP);
                glVertex2f(b.x, b.y); glVertex2f(b.x + b.w, b.y);
                glVertex2f(b.x + b.w, b.y + b.h); glVertex2f(b.x, b.y + b.h);
                glEnd();
            }
        }
    }

    // 9. Modern HUD & Các màn hình trạng thái
    drawModernHUD();

    if (gameState == STATE_START) {
        drawStartScreen();
    } else if (gameState == STATE_GAMEOVER) {
        drawGameOverScreen();
    }

    // 10. Bảng Học Thuật Đồ Họa Máy Tính (Overlay TAB)
    drawAcademicOverlay();

    glutSwapBuffers();

    // Tự động chụp ảnh khi kiểm tra
    if (autoCaptureStart || autoCaptureDay || autoCaptureNight || autoCaptureDebug) {
        renderFrameCount++;
        if (renderFrameCount >= 5) {
            if (autoCaptureStart) saveScreenshotBMP("ket_qua_dino_start.bmp");
            else if (autoCaptureDay) saveScreenshotBMP("ket_qua_dino_day.bmp");
            else if (autoCaptureNight) saveScreenshotBMP("ket_qua_dino_night.bmp");
            else if (autoCaptureDebug) saveScreenshotBMP("ket_qua_dino_debug.bmp");
            exit(0);
        }
    }
}

// ============================================================================
// LOGIC CẬP NHẬT TRẠNG THÁI (60 FPS UPDATE)
// ============================================================================
void spawnObstacle() {
    float minGap = 340.0f;
    float maxGap = 580.0f - (gameSpeed - BASE_SPEED) * 0.2f;
    if (maxGap < 360.0f) maxGap = 360.0f;

    float spawnX = WINDOW_WIDTH + (float)(rand() % (int)(maxGap - minGap + 1));
    bool canSpawnBird = (currentScore >= 200.0f) && (rand() % 100 < 42);

    if (canSpawnBird) {
        int r = rand() % 3;
        ObstacleType bType = (r == 0) ? OBS_BIRD_LOW : ((r == 1) ? OBS_BIRD_MID : OBS_BIRD_HIGH);
        obstacles.push_back(Obstacle(spawnX, bType));
    } else {
        int r = rand() % 5;
        obstacles.push_back(Obstacle(spawnX, (ObstacleType)r));
    }
}

void update(int value) {
    (void)value;
    float dt = 0.016f;

    if (gameState == STATE_PLAYING) {
        gameSpeed += SPEED_ACCEL * dt;
        if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

        currentScore += (gameSpeed * 0.025f) * dt * 60.0f;
        if (currentScore > highScore) highScore = currentScore;
        distanceRun += gameSpeed * dt;

        // Báo hiệu mỗi mốc 100 điểm
        int currentHundred = (int)currentScore / 100;
        if (currentHundred > lastMilestone) {
            lastMilestone = currentHundred;
            milestoneFlash = 1.6f;
        }
        if (milestoneFlash > 0.0f) milestoneFlash -= dt;

        // Chu kỳ Ngày / Hoàng Hôn / Đêm động (mỗi 750 điểm)
        if (!manualNight) {
            float cyclePhase = fmod(currentScore / 750.0f, 2.0f);
            if (cyclePhase <= 1.0f) timeOfDayCycle = cyclePhase;
            else                    timeOfDayCycle = 2.0f - cyclePhase;
        }

        // Vật lý Nhảy Khủng Long
        if (!dino.isGrounded) {
            float g = dino.isFastFalling ? FAST_FALL_GRAVITY : GRAVITY;
            dino.vy -= g * dt;
            dino.y += dino.vy * dt;
            dino.pose = POSE_JUMP;

            if (dino.y <= GROUND_Y) {
                dino.y = GROUND_Y;
                dino.vy = 0.0f;
                dino.isGrounded = true;
                dino.isFastFalling = false;
                spawnDustPuff(dino.x + 18.0f, dino.y, 6);
            }
        } else {
            dino.legTimer += dt * (gameSpeed / 38.0f);
            if (dino.isDucking) dino.pose = ((int)dino.legTimer % 2 == 0) ? POSE_DUCK_1 : POSE_DUCK_2;
            else                dino.pose = ((int)dino.legTimer % 2 == 0) ? POSE_RUN_1 : POSE_RUN_2;

            if (rand() % 7 == 0) spawnDustPuff(dino.x + 8.0f, dino.y, 1);
        }

        // Chớp mắt tự nhiên
        dino.eyeBlinkTimer += dt;
        if (dino.eyeBlinkTimer > 3.5f) {
            dino.isEyeClosed = true;
            if (dino.eyeBlinkTimer > 3.65f) {
                dino.isEyeClosed = false;
                dino.eyeBlinkTimer = 0.0f;
            }
        }

        // Cuộn cảnh thị sai Parallax
        for (auto& m : farMountains) {
            m.x -= gameSpeed * 0.06f * dt;
            if (m.x < -m.w) m.x = WINDOW_WIDTH + 50.0f;
        }
        for (auto& d : midDunes) {
            d.x -= gameSpeed * 0.18f * dt;
            if (d.x < -d.w) d.x = WINDOW_WIDTH + 40.0f;
        }
        for (auto& c : clouds) {
            c.x -= c.speed * dt;
            if (c.x < -120.0f) {
                c.x = WINDOW_WIDTH + (float)(rand() % 80);
                c.y = (float)(WINDOW_HEIGHT - 90 - rand() % 120);
            }
        }

        // Chướng ngại vật
        if (obstacles.empty() || obstacles.back().x < WINDOW_WIDTH - 280.0f) {
            spawnObstacle();
        }

        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            it->x -= gameSpeed * dt;
            if (it->type >= OBS_BIRD_LOW) {
                it->wingTimer += dt * 10.0f;
                it->wingState = ((int)it->wingTimer) % 2;
            }

            if (it->x < -90.0f) it = obstacles.erase(it);
            else ++it;
        }

        // Va chạm AABB
        vector<AABB> dBoxes = dino.getHitboxes();
        for (const auto& obs : obstacles) {
            bool hit = false;
            for (const auto& db : dBoxes) {
                for (const auto& ob : obs.getHitboxes()) {
                    if (db.intersects(ob)) { hit = true; break; }
                }
                if (hit) break;
            }

            if (hit) {
                if (hasShield) {
                    hasShield = false;
                    spawnSparks(dino.x + 20.0f, dino.y + 20.0f, 25);
                } else {
                    gameState = STATE_GAMEOVER;
                    dino.pose = POSE_DEAD;
                    dino.deadRotation = -25.0f;
                    spawnSparks(dino.x + 20.0f, dino.y + 20.0f, 35);
                    break;
                }
            }
        }

        shieldAura += dt;
    }

    // Cập nhật hạt
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->vy -= 450.0f * dt;
        it->life -= it->decay * dt;
        if (it->life <= 0.0f || it->y < GROUND_Y - 10.0f) it = particles.erase(it);
        else ++it;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ============================================================================
// XỬ LÝ PHÍM & CHUỘT
// ============================================================================
void keyboardDown(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case ' ':
            if (gameState == STATE_START || gameState == STATE_GAMEOVER) resetGame();
            else if (gameState == STATE_PLAYING) dino.jump();
            break;
        case 'w': case 'W':
            if (gameState == STATE_PLAYING) dino.jump();
            break;
        case 's': case 'S':
            if (gameState == STATE_PLAYING) dino.duck(true);
            break;
        case '\t':
            showAcademicHUD = !showAcademicHUD;
            break;
        case 'h': case 'H':
            showHitboxes = !showHitboxes;
            break;
        case 'b': case 'B':
            showBezierArc = !showBezierArc;
            break;
        case 'n': case 'N':
            manualNight = !manualNight;
            timeOfDayCycle = manualNight ? 1.0f : 0.0f;
            break;
        case '1':
            gameMode = MODE_CLASSIC;
            if (gameState != STATE_PLAYING) resetGame();
            break;
        case '2':
            gameMode = MODE_ARCADE;
            if (gameState != STATE_PLAYING) resetGame();
            break;
        case 'r': case 'R':
            resetGame();
            break;
        case 27:
            if (gameState == STATE_PLAYING) gameState = STATE_START;
            else exit(0);
            break;
        case 'q': case 'Q':
            exit(0);
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    (void)x; (void)y;
    if (key == 's' || key == 'S') {
        if (gameState == STATE_PLAYING) dino.duck(false);
    }
}

void specialKeysDown(int key, int x, int y) {
    (void)x; (void)y;
    if (key == GLUT_KEY_UP) {
        if (gameState == STATE_START || gameState == STATE_GAMEOVER) resetGame();
        else if (gameState == STATE_PLAYING) dino.jump();
    } else if (key == GLUT_KEY_DOWN) {
        if (gameState == STATE_PLAYING) dino.duck(true);
    } else if (key == GLUT_KEY_F12) {
        saveScreenshotBMP("dino_screenshot.bmp");
    }
}

void specialKeysUp(int key, int x, int y) {
    (void)x; (void)y;
    if (key == GLUT_KEY_DOWN) {
        if (gameState == STATE_PLAYING) dino.duck(false);
    }
}

void mouseClick(int button, int state, int x, int y) {
    (void)x; (void)y;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (gameState == STATE_START || gameState == STATE_GAMEOVER) resetGame();
        else if (gameState == STATE_PLAYING) dino.jump();
    }
}

void initGL() {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    srand((unsigned int)time(NULL));
    initParallaxScenery();
}

int main(int argc, char** argv) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--screenshot-start") {
            autoCaptureStart = true;
            gameState = STATE_START;
        } else if (arg == "--screenshot-day") {
            autoCaptureDay = true;
            gameState = STATE_PLAYING;
            dino.isGrounded = false;
            dino.y = GROUND_Y + 75.0f;
            currentScore = 185.0f;
            timeOfDayCycle = 0.0f;
            obstacles.push_back(Obstacle(440.0f, OBS_CACTUS_DOUBLE));
            obstacles.push_back(Obstacle(740.0f, OBS_BIRD_MID));
        } else if (arg == "--screenshot-night") {
            autoCaptureNight = true;
            gameState = STATE_PLAYING;
            timeOfDayCycle = 1.0f;
            manualNight = true;
            currentScore = 850.0f;
            highScore = 1420.0f;
            obstacles.push_back(Obstacle(420.0f, OBS_CACTUS_FLOWER));
            obstacles.push_back(Obstacle(700.0f, OBS_BIRD_LOW));
        } else if (arg == "--screenshot-debug") {
            autoCaptureDebug = true;
            gameState = STATE_PLAYING;
            showAcademicHUD = true;
            showHitboxes = true;
            showBezierArc = true;
            timeOfDayCycle = 0.25f; // Ánh hoàng hôn
            currentScore = 420.0f;
            obstacles.push_back(Obstacle(390.0f, OBS_CACTUS_SINGLE));
            obstacles.push_back(Obstacle(660.0f, OBS_BIRD_MID));
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(70, 50);
    glutCreateWindow("VKU - CHROME T-REX RUNNER: HIGH DEFINITION ARTISTIC EDITION");

    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeysDown);
    glutSpecialUpFunc(specialKeysUp);
    glutMouseFunc(mouseClick);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
