#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <string>
#include <vector>

namespace {

constexpr int kScreenW = 320;
constexpr int kScreenH = 224;
constexpr float kDt = 1.0F / 60.0F;
constexpr float kGravity = 900.0F;
constexpr float kWorldW = 4860.0F;
constexpr float kGroundY = 198.0F;

struct Vec2 {
    float x = 0.0F;
    float y = 0.0F;
};

struct RectF {
    float x = 0.0F;
    float y = 0.0F;
    float w = 0.0F;
    float h = 0.0F;
};

struct Input {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool jump = false;
    bool jumpPressed = false;
    bool fire = false;
    bool firePressed = false;
    bool leftHeld = false;
    bool rightHeld = false;
    bool upHeld = false;
    bool downHeld = false;
    bool jumpHeld = false;
    bool fireHeld = false;
    bool weaponNormalPressed = false;
    bool weaponSpreadPressed = false;
    bool weaponTogglePressed = false;
    bool restartPressed = false;
    bool startPressed = false;
    bool nativeJumpWasDown = false;
    bool nativeFireWasDown = false;
    bool nativeWeapon1WasDown = false;
    bool nativeWeapon2WasDown = false;
    bool nativeToggleWasDown = false;
    bool nativeRestartWasDown = false;
    bool nativeStartWasDown = false;
};

enum class Weapon {
    Rifle,
    Spread
};

enum class EnemyType {
    Runner,
    Rifleman,
    Turret,
    Hopper,
    Boss
};

struct Platform {
    RectF rect;
};

struct Bullet {
    Vec2 pos;
    Vec2 vel;
    float radius = 2.0F;
    int owner = 0;
    int damage = 1;
    float ttl = 1.4F;
    bool spread = false;
    bool alive = true;
};

struct Particle {
    Vec2 pos;
    Vec2 vel;
    SDL_Color color {};
    float life = 0.0F;
    float maxLife = 0.0F;
    float size = 1.0F;
};

struct Enemy {
    EnemyType type = EnemyType::Runner;
    RectF box;
    Vec2 vel;
    int hp = 1;
    int maxHp = 1;
    float shootTimer = 0.0F;
    float stateTimer = 0.0F;
    float originX = 0.0F;
    bool grounded = false;
    bool alive = true;
    bool active = false;
};

struct Player {
    RectF box {32.0F, kGroundY - 32.0F, 17.0F, 32.0F};
    Vec2 vel;
    int hp = 5;
    int lives = 3;
    bool grounded = false;
    bool faceRight = true;
    Weapon weapon = Weapon::Spread;
    bool invulnerable = false;
    float invulnTimer = 0.0F;
    float shootCooldown = 0.0F;
    float jumpBufferTimer = 0.0F;
    float coyoteTimer = 0.1F;
};

struct Star {
    float x = 0.0F;
    float y = 0.0F;
    float speed = 0.0F;
    uint8_t shade = 0;
};

struct Game {
    Player player;
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    std::vector<Particle> particles;
    std::array<Star, 80> stars {};
    Input input;
    float cameraX = 0.0F;
    float spawnTimer = 0.0F;
    float shakeTimer = 0.0F;
    float shakePower = 0.0F;
    int score = 0;
    bool hasKeyboardFocus = true;
    bool gameOver = false;
    bool victory = false;
    bool startScreen = true;
    RectF prevPlayerBox {};
    float prevCameraX = 0.0F;
};

bool intersects(const RectF& a, const RectF& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

float centerX(const RectF& r) {
    return r.x + r.w * 0.5F;
}

float centerY(const RectF& r) {
    return r.y + r.h * 0.5F;
}

float clampf(float v, float minV, float maxV) {
    return std::max(minV, std::min(maxV, v));
}

float sign(float v) {
    return (v > 0.0F) - (v < 0.0F);
}

int iround(float v) {
    return static_cast<int>(std::lround(v));
}

SDL_Rect toScreenRect(const RectF& r, float cameraX, float ox = 0.0F, float oy = 0.0F) {
    return SDL_Rect {
        iround(r.x - cameraX + ox),
        iround(r.y + oy),
        iround(r.w),
        iround(r.h)
    };
}

void setDrawColor(SDL_Renderer* renderer, SDL_Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

void fillRect(SDL_Renderer* renderer, const RectF& r, float cameraX, SDL_Color color, float ox = 0.0F, float oy = 0.0F) {
    setDrawColor(renderer, color);
    SDL_Rect rect = toScreenRect(r, cameraX, ox, oy);
    SDL_RenderFillRect(renderer, &rect);
}

void drawRect(SDL_Renderer* renderer, const RectF& r, float cameraX, SDL_Color color, float ox = 0.0F, float oy = 0.0F) {
    setDrawColor(renderer, color);
    SDL_Rect rect = toScreenRect(r, cameraX, ox, oy);
    SDL_RenderDrawRect(renderer, &rect);
}

void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    setDrawColor(renderer, color);
    for (int y = -radius; y <= radius; ++y) {
        const int span = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - y * y)));
        SDL_RenderDrawLine(renderer, cx - span, cy + y, cx + span, cy + y);
    }
}

void drawText(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color, int scale = 1);

std::array<uint8_t, 5> glyph(char c) {
    switch (c) {
        case '0': return {0x7, 0x5, 0x5, 0x5, 0x7};
        case '1': return {0x2, 0x6, 0x2, 0x2, 0x7};
        case '2': return {0x7, 0x1, 0x7, 0x4, 0x7};
        case '3': return {0x7, 0x1, 0x7, 0x1, 0x7};
        case '4': return {0x5, 0x5, 0x7, 0x1, 0x1};
        case '5': return {0x7, 0x4, 0x7, 0x1, 0x7};
        case '6': return {0x7, 0x4, 0x7, 0x5, 0x7};
        case '7': return {0x7, 0x1, 0x1, 0x1, 0x1};
        case '8': return {0x7, 0x5, 0x7, 0x5, 0x7};
        case '9': return {0x7, 0x5, 0x7, 0x1, 0x7};
        case 'A': return {0x2, 0x5, 0x7, 0x5, 0x5};
        case 'B': return {0x6, 0x5, 0x6, 0x5, 0x6};
        case 'C': return {0x7, 0x4, 0x4, 0x4, 0x7};
        case 'D': return {0x6, 0x5, 0x5, 0x5, 0x6};
        case 'E': return {0x7, 0x4, 0x6, 0x4, 0x7};
        case 'F': return {0x7, 0x4, 0x6, 0x4, 0x4};
        case 'G': return {0x7, 0x4, 0x5, 0x5, 0x7};
        case 'H': return {0x5, 0x5, 0x7, 0x5, 0x5};
        case 'I': return {0x7, 0x2, 0x2, 0x2, 0x7};
        case 'J': return {0x1, 0x1, 0x1, 0x5, 0x7};
        case 'K': return {0x5, 0x5, 0x6, 0x5, 0x5};
        case 'L': return {0x4, 0x4, 0x4, 0x4, 0x7};
        case 'M': return {0x5, 0x7, 0x7, 0x5, 0x5};
        case 'N': return {0x5, 0x7, 0x7, 0x7, 0x5};
        case 'O': return {0x7, 0x5, 0x5, 0x5, 0x7};
        case 'P': return {0x7, 0x5, 0x7, 0x4, 0x4};
        case 'Q': return {0x7, 0x5, 0x5, 0x7, 0x1};
        case 'R': return {0x7, 0x5, 0x7, 0x6, 0x5};
        case 'S': return {0x7, 0x4, 0x7, 0x1, 0x7};
        case 'T': return {0x7, 0x2, 0x2, 0x2, 0x2};
        case 'U': return {0x5, 0x5, 0x5, 0x5, 0x7};
        case 'V': return {0x5, 0x5, 0x5, 0x5, 0x2};
        case 'W': return {0x5, 0x5, 0x7, 0x7, 0x5};
        case 'X': return {0x5, 0x5, 0x2, 0x5, 0x5};
        case 'Y': return {0x5, 0x5, 0x2, 0x2, 0x2};
        case 'Z': return {0x7, 0x1, 0x2, 0x4, 0x7};
        case '-': return {0x0, 0x0, 0x7, 0x0, 0x0};
        case ':': return {0x0, 0x2, 0x0, 0x2, 0x0};
        case '/': return {0x1, 0x1, 0x2, 0x4, 0x4};
        case '!': return {0x2, 0x2, 0x2, 0x0, 0x2};
        case '.': return {0x0, 0x0, 0x0, 0x0, 0x2};
        case ' ': return {0x0, 0x0, 0x0, 0x0, 0x0};
        default: return {0x7, 0x1, 0x3, 0x0, 0x2};
    }
}

void drawText(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color, int scale) {
    setDrawColor(renderer, color);
    int penX = x;
    for (char raw : text) {
        const char c = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
        const auto rows = glyph(c);
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                if ((rows[row] & (1 << (2 - col))) != 0) {
                    SDL_Rect px {penX + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &px);
                }
            }
        }
        penX += 4 * scale;
    }
}

int textWidth(const std::string& text, int scale) {
    return static_cast<int>(text.size()) * 4 * scale - scale;
}

void spawnParticles(Game& game, Vec2 pos, SDL_Color color, int count, float power) {
    for (int i = 0; i < count; ++i) {
        const float a = static_cast<float>((i * 137) % 360) * 3.14159265F / 180.0F;
        const float speed = power * (0.35F + static_cast<float>((i * 17) % 70) / 100.0F);
        Particle p;
        p.pos = pos;
        p.vel = {std::cos(a) * speed, std::sin(a) * speed - 35.0F};
        p.color = color;
        p.life = 0.28F + static_cast<float>((i * 11) % 35) / 100.0F;
        p.maxLife = p.life;
        p.size = 1.0F + static_cast<float>(i % 3);
        game.particles.push_back(p);
    }
}

void addEnemy(Game& game, EnemyType type, float x, float y) {
    Enemy e;
    e.type = type;
    e.originX = x;
    e.shootTimer = 0.45F + static_cast<float>(game.enemies.size() % 7) * 0.19F;
    switch (type) {
        case EnemyType::Runner:
            e.box = {x, y - 25.0F, 15.0F, 25.0F};
            e.hp = e.maxHp = 1;
            e.vel.x = -42.0F;
            break;
        case EnemyType::Rifleman:
            e.box = {x, y - 25.0F, 15.0F, 25.0F};
            e.hp = e.maxHp = 2;
            break;
        case EnemyType::Turret:
            e.box = {x, y - 18.0F, 22.0F, 18.0F};
            e.hp = e.maxHp = 3;
            break;
        case EnemyType::Hopper:
            e.box = {x, y - 23.0F, 16.0F, 23.0F};
            e.hp = e.maxHp = 2;
            e.vel.x = -30.0F;
            break;
        case EnemyType::Boss:
            e.box = {x, y - 70.0F, 58.0F, 70.0F};
            e.hp = e.maxHp = 40;
            e.shootTimer = 1.0F;
            break;
    }
    game.enemies.push_back(e);
}

void addPlatform(Game& game, float x, float y, float w, float h) {
    game.platforms.push_back(Platform {RectF {x, y, w, h}});
}

void resetGame(Game& game) {
    game = Game {};
    game.player = Player {};
    game.platforms.clear();
    game.enemies.clear();
    game.bullets.clear();
    game.particles.clear();
    game.score = 0;
    game.cameraX = 0.0F;
    game.prevCameraX = 0.0F;
    game.gameOver = false;
    game.victory = false;
    game.startScreen = false;
    game.prevPlayerBox = game.player.box;

    addPlatform(game, 0.0F, kGroundY, kWorldW, 32.0F);
    addPlatform(game, 230.0F, 152.0F, 110.0F, 12.0F);
    addPlatform(game, 420.0F, 126.0F, 95.0F, 12.0F);
    addPlatform(game, 620.0F, 164.0F, 140.0F, 12.0F);
    addPlatform(game, 910.0F, 145.0F, 120.0F, 12.0F);
    addPlatform(game, 1160.0F, 118.0F, 110.0F, 12.0F);
    addPlatform(game, 1400.0F, 154.0F, 180.0F, 12.0F);
    addPlatform(game, 1790.0F, 132.0F, 115.0F, 12.0F);
    addPlatform(game, 2040.0F, 158.0F, 170.0F, 12.0F);
    addPlatform(game, 2470.0F, 125.0F, 130.0F, 12.0F);
    addPlatform(game, 2860.0F, 155.0F, 160.0F, 12.0F);
    addPlatform(game, 3260.0F, 130.0F, 160.0F, 12.0F);
    addPlatform(game, 3630.0F, 154.0F, 210.0F, 12.0F);
    addPlatform(game, 4040.0F, 126.0F, 130.0F, 12.0F);

    addEnemy(game, EnemyType::Runner, 410.0F, kGroundY);
    addEnemy(game, EnemyType::Rifleman, 600.0F, kGroundY);
    addEnemy(game, EnemyType::Turret, 720.0F, 164.0F);
    addEnemy(game, EnemyType::Runner, 940.0F, kGroundY);
    addEnemy(game, EnemyType::Hopper, 1040.0F, kGroundY);
    addEnemy(game, EnemyType::Rifleman, 1210.0F, 118.0F);
    addEnemy(game, EnemyType::Turret, 1530.0F, 154.0F);
    addEnemy(game, EnemyType::Runner, 1620.0F, kGroundY);
    addEnemy(game, EnemyType::Rifleman, 1870.0F, 132.0F);
    addEnemy(game, EnemyType::Hopper, 2160.0F, kGroundY);
    addEnemy(game, EnemyType::Turret, 2560.0F, 125.0F);
    addEnemy(game, EnemyType::Runner, 2700.0F, kGroundY);
    addEnemy(game, EnemyType::Rifleman, 2960.0F, 155.0F);
    addEnemy(game, EnemyType::Hopper, 3330.0F, 130.0F);
    addEnemy(game, EnemyType::Turret, 3760.0F, 154.0F);
    addEnemy(game, EnemyType::Rifleman, 4120.0F, 126.0F);
    addEnemy(game, EnemyType::Boss, 4620.0F, kGroundY);

    for (size_t i = 0; i < game.stars.size(); ++i) {
        game.stars[i].x = static_cast<float>((i * 61) % kScreenW);
        game.stars[i].y = static_cast<float>((i * 37) % 130);
        game.stars[i].speed = 0.15F + static_cast<float>(i % 3) * 0.12F;
        game.stars[i].shade = static_cast<uint8_t>(90 + (i * 23) % 130);
    }
}

void hurtPlayer(Game& game) {
    auto& p = game.player;
    if (p.invulnerable || game.gameOver || game.victory) {
        return;
    }
    p.hp -= 1;
    p.invulnerable = true;
    p.invulnTimer = 1.25F;
    p.vel.y = -260.0F;
    p.vel.x = p.faceRight ? -160.0F : 160.0F;
    game.shakeTimer = 0.18F;
    game.shakePower = 4.0F;
    spawnParticles(game, {centerX(p.box), centerY(p.box)}, SDL_Color {255, 230, 120, 255}, 18, 95.0F);
    if (p.hp <= 0) {
        p.lives -= 1;
        if (p.lives <= 0) {
            game.gameOver = true;
            return;
        }
        p.hp = 5;
        p.box.x = std::max(28.0F, game.cameraX + 32.0F);
        p.box.y = kGroundY - p.box.h;
        p.vel = {};
        p.invulnTimer = 2.0F;
        p.invulnerable = true;
    }
}

void fireBullet(Game& game, Vec2 pos, Vec2 dir, int owner, float speed, int damage, float ttl) {
    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len <= 0.0F) {
        return;
    }
    Bullet b;
    b.pos = pos;
    b.vel = {dir.x / len * speed, dir.y / len * speed};
    b.owner = owner;
    b.damage = damage;
    b.ttl = ttl;
    b.radius = owner == 0 ? 2.0F : 2.5F;
    game.bullets.push_back(b);
}

Vec2 playerAimDirection(const Game& game) {
    const Player& p = game.player;
    Vec2 dir {p.faceRight ? 1.0F : -1.0F, 0.0F};
    if (game.input.up && game.input.down) {
        dir.y = 0.0F;
    } else if (game.input.up) {
        dir.y = -1.0F;
        dir.x = game.input.left ? -0.85F : (game.input.right ? 0.85F : 0.0F);
    } else if (game.input.down && !p.grounded) {
        dir.y = 1.0F;
        dir.x = game.input.left ? -0.85F : (game.input.right ? 0.85F : 0.0F);
    }
    return dir;
}

Vec2 rotateDir(Vec2 dir, float radians) {
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {dir.x * c - dir.y * s, dir.x * s + dir.y * c};
}

void firePlayer(Game& game) {
    Player& p = game.player;
    if (p.shootCooldown > 0.0F) {
        return;
    }

    Vec2 dir = playerAimDirection(game);
    if (dir.x < 0.0F) {
        p.faceRight = false;
    } else if (dir.x > 0.0F) {
        p.faceRight = true;
    }

    const Vec2 muzzle {
        p.box.x + (p.faceRight ? p.box.w + 4.0F : -4.0F),
        p.box.y + (game.input.up ? 8.0F : 15.0F)
    };

    if (p.weapon == Weapon::Spread) {
        constexpr std::array<float, 5> angles {
            -0.34F, -0.17F, 0.0F, 0.17F, 0.34F
        };
        for (float angle : angles) {
            fireBullet(game, muzzle, rotateDir(dir, angle), 0, 330.0F, 1, 0.72F);
            game.bullets.back().spread = true;
            game.bullets.back().radius = 2.2F;
        }
        p.shootCooldown = 0.23F;
    } else {
        fireBullet(game, muzzle, dir, 0, 390.0F, 1, 0.98F);
        p.shootCooldown = 0.095F;
    }
}

void resolvePlatforms(RectF& box, Vec2& vel, bool& grounded, const std::vector<Platform>& platforms) {
    grounded = false;
    box.x += vel.x * kDt;
    for (const auto& platform : platforms) {
        if (!intersects(box, platform.rect)) {
            continue;
        }
        if (vel.x > 0.0F) {
            box.x = platform.rect.x - box.w;
        } else if (vel.x < 0.0F) {
            box.x = platform.rect.x + platform.rect.w;
        }
        vel.x = 0.0F;
    }

    box.y += vel.y * kDt;
    for (const auto& platform : platforms) {
        if (!intersects(box, platform.rect)) {
            continue;
        }
        if (vel.y > 0.0F) {
            box.y = platform.rect.y - box.h;
            grounded = true;
        } else if (vel.y < 0.0F) {
            box.y = platform.rect.y + platform.rect.h;
        }
        vel.y = 0.0F;
    }
}

void updatePlayer(Game& game) {
    Player& p = game.player;
    if (game.gameOver || game.victory) {
        if (game.input.restartPressed) {
            resetGame(game);
        }
        return;
    }

    if (game.input.weaponNormalPressed) {
        p.weapon = Weapon::Rifle;
    }
    if (game.input.weaponSpreadPressed) {
        p.weapon = Weapon::Spread;
    }
    if (game.input.weaponTogglePressed) {
        p.weapon = p.weapon == Weapon::Spread ? Weapon::Rifle : Weapon::Spread;
    }

    if (game.input.jumpPressed) {
        p.jumpBufferTimer = 0.14F;
    } else if (p.jumpBufferTimer > 0.0F) {
        p.jumpBufferTimer -= kDt;
    }
    if (p.grounded) {
        p.coyoteTimer = 0.11F;
    } else if (p.coyoteTimer > 0.0F) {
        p.coyoteTimer -= kDt;
    }

    const float desired = (game.input.right ? 1.0F : 0.0F) - (game.input.left ? 1.0F : 0.0F);
    const float maxSpeed = p.grounded ? 118.0F : 126.0F;
    const float accel = p.grounded ? 1150.0F : 720.0F;
    const float friction = p.grounded ? 980.0F : 120.0F;

    if (desired != 0.0F) {
        p.vel.x += desired * accel * kDt;
        p.vel.x = clampf(p.vel.x, -maxSpeed, maxSpeed);
        p.faceRight = desired > 0.0F;
    } else {
        const float drop = friction * kDt;
        if (std::abs(p.vel.x) <= drop) {
            p.vel.x = 0.0F;
        } else {
            p.vel.x -= sign(p.vel.x) * drop;
        }
    }

    if (p.jumpBufferTimer > 0.0F && p.coyoteTimer > 0.0F) {
        p.vel.y = -355.0F;
        p.grounded = false;
        p.jumpBufferTimer = 0.0F;
        p.coyoteTimer = 0.0F;
    }
    if (!game.input.jump && p.vel.y < -110.0F) {
        p.vel.y += 900.0F * kDt;
    }

    p.vel.y = std::min(p.vel.y + kGravity * kDt, 520.0F);
    resolvePlatforms(p.box, p.vel, p.grounded, game.platforms);
    p.box.x = clampf(p.box.x, 6.0F, kWorldW - p.box.w - 10.0F);

    if (p.box.y > kScreenH + 80.0F) {
        hurtPlayer(game);
        p.box.y = kGroundY - p.box.h;
    }

    if (p.shootCooldown > 0.0F) {
        p.shootCooldown -= kDt;
    }
    if (game.input.fire || game.input.firePressed) {
        firePlayer(game);
    }

    if (p.invulnerable) {
        p.invulnTimer -= kDt;
        if (p.invulnTimer <= 0.0F) {
            p.invulnerable = false;
            p.invulnTimer = 0.0F;
        }
    }
}

Vec2 aimAtPlayer(const Enemy& e, const Player& p) {
    return {centerX(p.box) - centerX(e.box), centerY(p.box) - centerY(e.box)};
}

bool isOnScreen(float cameraX, const RectF& r, float margin = 40.0F) {
    return r.x + r.w > cameraX - margin && r.x < cameraX + kScreenW + margin;
}

void updateEnemies(Game& game) {
    Player& player = game.player;
    for (Enemy& e : game.enemies) {
        if (!e.alive) {
            continue;
        }
        e.active = e.active || e.box.x < game.cameraX + kScreenW + 60.0F;
        if (!e.active) {
            continue;
        }

        e.stateTimer += kDt;
        e.shootTimer -= kDt;

        const float dx = centerX(player.box) - centerX(e.box);
        switch (e.type) {
            case EnemyType::Runner:
                e.vel.x = dx < 0.0F ? -52.0F : 52.0F;
                if (std::abs(dx) < 22.0F) {
                    e.vel.x *= 0.2F;
                }
                break;
            case EnemyType::Rifleman:
                if (std::abs(dx) > 120.0F) {
                    e.vel.x = dx < 0.0F ? -24.0F : 24.0F;
                } else {
                    e.vel.x = 0.0F;
                }
                if (e.shootTimer <= 0.0F && isOnScreen(game.cameraX, e.box, 20.0F)) {
                    fireBullet(game, {centerX(e.box), e.box.y + 11.0F}, aimAtPlayer(e, player), 1, 140.0F, 1, 3.0F);
                    e.shootTimer = 1.15F;
                }
                break;
            case EnemyType::Turret:
                e.vel = {};
                if (e.shootTimer <= 0.0F && isOnScreen(game.cameraX, e.box, 20.0F)) {
                    fireBullet(game, {centerX(e.box), e.box.y + 5.0F}, aimAtPlayer(e, player), 1, 120.0F, 1, 3.2F);
                    e.shootTimer = 0.95F;
                }
                break;
            case EnemyType::Hopper:
                e.vel.x = dx < 0.0F ? -36.0F : 36.0F;
                if (e.grounded && e.stateTimer > 1.25F) {
                    e.vel.y = -295.0F;
                    e.stateTimer = 0.0F;
                }
                break;
            case EnemyType::Boss:
                e.vel.x = std::sin(e.stateTimer * 1.6F) * 25.0F;
                if (e.shootTimer <= 0.0F && isOnScreen(game.cameraX, e.box, 80.0F)) {
                    fireBullet(game, {e.box.x + 4.0F, e.box.y + 18.0F}, {-1.0F, -0.16F}, 1, 150.0F, 1, 3.0F);
                    fireBullet(game, {e.box.x + 4.0F, e.box.y + 34.0F}, {-1.0F, 0.05F}, 1, 160.0F, 1, 3.0F);
                    fireBullet(game, {e.box.x + 4.0F, e.box.y + 50.0F}, {-1.0F, 0.26F}, 1, 150.0F, 1, 3.0F);
                    e.shootTimer = 0.82F;
                }
                break;
        }

        if (e.type != EnemyType::Turret) {
            e.vel.y = std::min(e.vel.y + kGravity * kDt, 520.0F);
            resolvePlatforms(e.box, e.vel, e.grounded, game.platforms);
        }

        if (intersects(e.box, player.box)) {
            hurtPlayer(game);
        }

        if (e.box.x < game.cameraX - 180.0F && e.type != EnemyType::Boss) {
            e.alive = false;
        }
    }
}

void spawnAmbush(Game& game) {
    if (game.gameOver || game.victory) {
        return;
    }
    game.spawnTimer -= kDt;
    if (game.spawnTimer > 0.0F || game.cameraX > 4240.0F) {
        return;
    }
    const float ahead = game.cameraX + kScreenW + 24.0F;
    addEnemy(game, (game.score / 500) % 2 == 0 ? EnemyType::Runner : EnemyType::Hopper, ahead, kGroundY);
    game.enemies.back().active = true;
    game.spawnTimer = 2.9F;
}

void updateBullets(Game& game) {
    for (Bullet& b : game.bullets) {
        if (!b.alive) {
            continue;
        }
        b.ttl -= kDt;
        b.pos.x += b.vel.x * kDt;
        b.pos.y += b.vel.y * kDt;
        if (b.ttl <= 0.0F || b.pos.y < -12.0F || b.pos.y > kScreenH + 24.0F ||
            b.pos.x < game.cameraX - 90.0F || b.pos.x > game.cameraX + kScreenW + 120.0F) {
            b.alive = false;
            continue;
        }

        const RectF br {b.pos.x - b.radius, b.pos.y - b.radius, b.radius * 2.0F, b.radius * 2.0F};
        bool hitWorld = false;
        for (const auto& platform : game.platforms) {
            if (intersects(br, platform.rect)) {
                hitWorld = true;
                break;
            }
        }
        if (hitWorld) {
            b.alive = false;
            spawnParticles(game, b.pos, SDL_Color {245, 210, 90, 255}, 4, 45.0F);
            continue;
        }

        if (b.owner == 0) {
            for (Enemy& e : game.enemies) {
                if (!e.alive || !e.active) {
                    continue;
                }
                if (intersects(br, e.box)) {
                    b.alive = false;
                    e.hp -= b.damage;
                    spawnParticles(game, b.pos, SDL_Color {255, 92, 48, 255}, 9, 90.0F);
                    if (e.hp <= 0) {
                        e.alive = false;
                        game.score += e.type == EnemyType::Boss ? 5000 : 150;
                        game.shakeTimer = e.type == EnemyType::Boss ? 0.55F : 0.12F;
                        game.shakePower = e.type == EnemyType::Boss ? 8.0F : 3.0F;
                        spawnParticles(game, {centerX(e.box), centerY(e.box)}, SDL_Color {255, 170, 55, 255}, e.type == EnemyType::Boss ? 90 : 24, e.type == EnemyType::Boss ? 210.0F : 120.0F);
                        if (e.type == EnemyType::Boss) {
                            game.victory = true;
                        }
                    }
                    break;
                }
            }
        } else if (intersects(br, game.player.box)) {
            b.alive = false;
            hurtPlayer(game);
        }
    }
    game.bullets.erase(std::remove_if(game.bullets.begin(), game.bullets.end(), [](const Bullet& b) {
        return !b.alive;
    }), game.bullets.end());
}

void updateParticles(Game& game) {
    for (Particle& p : game.particles) {
        p.life -= kDt;
        p.pos.x += p.vel.x * kDt;
        p.pos.y += p.vel.y * kDt;
        p.vel.y += 360.0F * kDt;
    }
    game.particles.erase(std::remove_if(game.particles.begin(), game.particles.end(), [](const Particle& p) {
        return p.life <= 0.0F;
    }), game.particles.end());
}

void updateCamera(Game& game) {
    const float target = game.player.box.x - 112.0F;
    game.cameraX += (target - game.cameraX) * 0.13F;
    game.cameraX = clampf(game.cameraX, 0.0F, kWorldW - kScreenW);
    if (game.shakeTimer > 0.0F) {
        game.shakeTimer -= kDt;
        if (game.shakeTimer <= 0.0F) {
            game.shakePower = 0.0F;
        }
    }
}

void updateGame(Game& game) {
    updatePlayer(game);
    spawnAmbush(game);
    updateEnemies(game);
    updateBullets(game);
    updateParticles(game);
    updateCamera(game);
}

SDL_Color mix(SDL_Color a, SDL_Color b, float t) {
    t = clampf(t, 0.0F, 1.0F);
    return SDL_Color {
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        255
    };
}

SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t) {
    t = clampf(t, 0.0F, 1.0F);
    return SDL_Color {
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
    };
}

void renderBackground(SDL_Renderer* renderer, const Game& game, float ox, float oy) {
    for (int y = 0; y < kScreenH; ++y) {
        const float t = static_cast<float>(y) / static_cast<float>(kScreenH);
        setDrawColor(renderer, mix(SDL_Color {15, 23, 42, 255}, SDL_Color {28, 57, 68, 255}, t));
        SDL_RenderDrawLine(renderer, 0, y, kScreenW, y);
    }

    for (const Star& s : game.stars) {
        const int sx = iround(std::fmod(s.x - game.cameraX * s.speed + kWorldW, static_cast<float>(kScreenW)));
        setDrawColor(renderer, SDL_Color {s.shade, s.shade, static_cast<uint8_t>(std::min(255, s.shade + 40)), 255});
        SDL_RenderDrawPoint(renderer, sx, iround(s.y));
    }

    const float farCam = game.cameraX * 0.24F;
    setDrawColor(renderer, SDL_Color {31, 74, 73, 255});
    for (int i = -1; i < 18; ++i) {
        const float x = static_cast<float>(i * 92) - std::fmod(farCam, 92.0F);
        std::array<SDL_Point, 3> points {
            SDL_Point {iround(x + ox), iround(166.0F + oy)},
            SDL_Point {iround(x + 54.0F + ox), iround(78.0F + oy)},
            SDL_Point {iround(x + 116.0F + ox), iround(166.0F + oy)}
        };
        SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));
    }

    const float nearCam = game.cameraX * 0.48F;
    setDrawColor(renderer, SDL_Color {20, 92, 75, 255});
    for (int i = -1; i < 24; ++i) {
        const int x = iround(static_cast<float>(i * 54) - std::fmod(nearCam, 54.0F) + ox);
        SDL_Rect trunk {x + 22, iround(132.0F + oy), 9, 72};
        SDL_RenderFillRect(renderer, &trunk);
        fillCircle(renderer, x + 26, iround(126.0F + oy), 19, SDL_Color {22, 110, 83, 255});
    }
}

void renderPlatforms(SDL_Renderer* renderer, const Game& game, float ox, float oy) {
    for (const auto& platform : game.platforms) {
        if (!isOnScreen(game.cameraX, platform.rect, 30.0F)) {
            continue;
        }
        fillRect(renderer, platform.rect, game.cameraX, SDL_Color {79, 87, 64, 255}, ox, oy);
        fillRect(renderer, RectF {platform.rect.x, platform.rect.y, platform.rect.w, 4.0F}, game.cameraX, SDL_Color {124, 156, 76, 255}, ox, oy);
        drawRect(renderer, platform.rect, game.cameraX, SDL_Color {35, 41, 37, 255}, ox, oy);
        for (int i = 0; i < static_cast<int>(platform.rect.w); i += 18) {
            fillRect(renderer, RectF {platform.rect.x + static_cast<float>(i), platform.rect.y + 5.0F, 2.0F, platform.rect.h - 6.0F}, game.cameraX, SDL_Color {54, 62, 48, 255}, ox, oy);
        }
    }
}

void renderPlayer(SDL_Renderer* renderer, const Player& p, float cameraX, float ox, float oy) {
    if (p.invulnerable && (static_cast<int>(p.invulnTimer * 22.0F) % 2 == 0)) {
        return;
    }
    const bool right = p.faceRight;
    const float px = p.box.x;
    const float py = p.box.y;

    // Helmet
    fillRect(renderer, RectF {px + 3.0F, py, 11.0F, 6.0F}, cameraX, SDL_Color {75, 120, 80, 255}, ox, oy);
    // Visor
    fillRect(renderer, RectF {px + (right ? 8.0F : 3.0F), py + 2.0F, 5.0F, 3.0F}, cameraX, SDL_Color {140, 210, 230, 255}, ox, oy);
    // Face
    fillRect(renderer, RectF {px + 4.0F, py + 6.0F, 9.0F, 5.0F}, cameraX, SDL_Color {234, 198, 133, 255}, ox, oy);
    // Eye
    fillRect(renderer, RectF {px + (right ? 10.0F : 5.0F), py + 7.0F, 2.0F, 2.0F}, cameraX, SDL_Color {30, 30, 40, 255}, ox, oy);

    // Body armor
    fillRect(renderer, RectF {px + 2.0F, py + 11.0F, 13.0F, 11.0F}, cameraX, SDL_Color {55, 140, 75, 255}, ox, oy);
    // Belt
    fillRect(renderer, RectF {px + 3.0F, py + 19.0F, 11.0F, 2.0F}, cameraX, SDL_Color {100, 80, 50, 255}, ox, oy);
    // Belt buckle
    fillRect(renderer, RectF {px + 7.0F, py + 19.0F, 3.0F, 2.0F}, cameraX, SDL_Color {200, 180, 100, 255}, ox, oy);

    // Arm
    fillRect(renderer, RectF {px + (right ? 12.0F : 1.0F), py + 12.0F, 4.0F, 8.0F}, cameraX, SDL_Color {234, 198, 133, 255}, ox, oy);
    // Gun barrel
    fillRect(renderer, RectF {px + (right ? 14.0F : -7.0F), py + 12.0F, 10.0F, 3.0F}, cameraX, SDL_Color {160, 160, 170, 255}, ox, oy);
    // Gun body
    fillRect(renderer, RectF {px + (right ? 14.0F : -3.0F), py + 13.0F, 6.0F, 4.0F}, cameraX, SDL_Color {80, 80, 90, 255}, ox, oy);

    // Muzzle flash when shooting
    if (p.shootCooldown > 0.12F) {
        fillRect(renderer, RectF {px + (right ? 24.0F : -10.0F), py + 10.0F, 4.0F, 6.0F}, cameraX, SDL_Color {255, 240, 120, 200}, ox, oy);
    }

    // Legs with animation
    if (!p.grounded) {
        // Jumping pose - legs spread
        fillRect(renderer, RectF {px + 3.0F, py + 22.0F, 4.0F, 10.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
        fillRect(renderer, RectF {px + 10.0F, py + 22.0F, 4.0F, 8.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
    } else if (std::abs(p.vel.x) > 15.0F) {
        // Running animation
        const float t = p.box.x * 0.15F;
        const float phase = std::sin(t);
        fillRect(renderer, RectF {px + 4.0F + phase * 2.0F, py + 22.0F, 4.0F, 10.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
        fillRect(renderer, RectF {px + 10.0F - phase * 2.0F, py + 22.0F, 4.0F, 10.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
    } else {
        // Standing
        fillRect(renderer, RectF {px + 4.0F, py + 22.0F, 4.0F, 10.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
        fillRect(renderer, RectF {px + 10.0F, py + 22.0F, 4.0F, 10.0F}, cameraX, SDL_Color {55, 80, 145, 255}, ox, oy);
    }
    // Boots
    fillRect(renderer, RectF {px + 3.0F, py + 30.0F, 5.0F, 2.0F}, cameraX, SDL_Color {60, 45, 35, 255}, ox, oy);
    fillRect(renderer, RectF {px + 10.0F, py + 30.0F, 5.0F, 2.0F}, cameraX, SDL_Color {60, 45, 35, 255}, ox, oy);
}

void renderEnemy(SDL_Renderer* renderer, const Enemy& e, float cameraX, float ox, float oy) {
    if (!e.alive || !isOnScreen(cameraX, e.box, 60.0F)) {
        return;
    }
    switch (e.type) {
        case EnemyType::Runner:
            // Body
            fillRect(renderer, e.box, cameraX, SDL_Color {170, 55, 50, 255}, ox, oy);
            // Head
            fillRect(renderer, RectF {e.box.x + 3.0F, e.box.y + 1.0F, 9.0F, 7.0F}, cameraX, SDL_Color {227, 169, 115, 255}, ox, oy);
            // Bandana
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y + 3.0F, 11.0F, 2.0F}, cameraX, SDL_Color {200, 50, 50, 255}, ox, oy);
            // Legs
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y + 18.0F, 5.0F, 7.0F}, cameraX, SDL_Color {60, 60, 100, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x + 9.0F, e.box.y + 18.0F, 5.0F, 7.0F}, cameraX, SDL_Color {60, 60, 100, 255}, ox, oy);
            break;
        case EnemyType::Rifleman:
            // Body
            fillRect(renderer, e.box, cameraX, SDL_Color {160, 75, 45, 255}, ox, oy);
            // Head
            fillRect(renderer, RectF {e.box.x + 3.0F, e.box.y + 1.0F, 9.0F, 7.0F}, cameraX, SDL_Color {227, 169, 115, 255}, ox, oy);
            // Helmet
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y, 11.0F, 4.0F}, cameraX, SDL_Color {80, 90, 70, 255}, ox, oy);
            // Legs
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y + 18.0F, 5.0F, 7.0F}, cameraX, SDL_Color {70, 70, 60, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x + 9.0F, e.box.y + 18.0F, 5.0F, 7.0F}, cameraX, SDL_Color {70, 70, 60, 255}, ox, oy);
            // Gun
            if (e.vel.x <= 0.0F) {
                fillRect(renderer, RectF {e.box.x - 16.0F, e.box.y + 11.0F, 18.0F, 3.0F}, cameraX, SDL_Color {42, 42, 46, 255}, ox, oy);
            } else {
                fillRect(renderer, RectF {e.box.x + e.box.w - 2.0F, e.box.y + 11.0F, 18.0F, 3.0F}, cameraX, SDL_Color {42, 42, 46, 255}, ox, oy);
            }
            break;
        case EnemyType::Turret:
            // Base
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y + 10.0F, 18.0F, 8.0F}, cameraX, SDL_Color {80, 85, 95, 255}, ox, oy);
            // Body
            fillRect(renderer, e.box, cameraX, SDL_Color {96, 102, 111, 255}, ox, oy);
            // Barrel mount
            fillRect(renderer, RectF {e.box.x + 4.0F, e.box.y + 3.0F, 14.0F, 8.0F}, cameraX, SDL_Color {70, 75, 85, 255}, ox, oy);
            // Barrel
            fillRect(renderer, RectF {e.box.x - 10.0F, e.box.y + 4.0F, 16.0F, 5.0F}, cameraX, SDL_Color {48, 54, 61, 255}, ox, oy);
            // Red light
            fillRect(renderer, RectF {e.box.x + 9.0F, e.box.y + 5.0F, 3.0F, 3.0F}, cameraX, SDL_Color {220, 50, 50, 255}, ox, oy);
            break;
        case EnemyType::Hopper:
            // Body
            fillRect(renderer, e.box, cameraX, SDL_Color {140, 60, 165, 255}, ox, oy);
            // Head
            fillRect(renderer, RectF {e.box.x + 3.0F, e.box.y + 1.0F, 10.0F, 8.0F}, cameraX, SDL_Color {180, 140, 200, 255}, ox, oy);
            // Eyes
            fillRect(renderer, RectF {e.box.x + 5.0F, e.box.y + 3.0F, 3.0F, 2.0F}, cameraX, SDL_Color {255, 220, 100, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x + 10.0F, e.box.y + 3.0F, 3.0F, 2.0F}, cameraX, SDL_Color {255, 220, 100, 255}, ox, oy);
            // Spring legs
            fillRect(renderer, RectF {e.box.x + 1.0F, e.box.y + e.box.h - 7.0F, 6.0F, 6.0F}, cameraX, SDL_Color {90, 40, 110, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x + 10.0F, e.box.y + e.box.h - 7.0F, 6.0F, 6.0F}, cameraX, SDL_Color {90, 40, 110, 255}, ox, oy);
            // Spring detail
            fillRect(renderer, RectF {e.box.x + 2.0F, e.box.y + e.box.h - 2.0F, 12.0F, 2.0F}, cameraX, SDL_Color {200, 200, 200, 255}, ox, oy);
            break;
        case EnemyType::Boss: {
            // Main body
            fillRect(renderer, e.box, cameraX, SDL_Color {90, 105, 120, 255}, ox, oy);
            // Core
            fillRect(renderer, RectF {e.box.x + 10.0F, e.box.y + 9.0F, 39.0F, 18.0F}, cameraX, SDL_Color {140, 40, 40, 255}, ox, oy);
            // Core detail
            fillRect(renderer, RectF {e.box.x + 15.0F, e.box.y + 12.0F, 29.0F, 12.0F}, cameraX, SDL_Color {170, 55, 55, 255}, ox, oy);
            // Cannon arm
            fillRect(renderer, RectF {e.box.x - 14.0F, e.box.y + 18.0F, 18.0F, 36.0F}, cameraX, SDL_Color {55, 62, 72, 255}, ox, oy);
            // Cannon barrel
            fillRect(renderer, RectF {e.box.x - 18.0F, e.box.y + 25.0F, 8.0F, 22.0F}, cameraX, SDL_Color {40, 45, 55, 255}, ox, oy);
            // Top armor
            fillRect(renderer, RectF {e.box.x + 12.0F, e.box.y - 9.0F, 34.0F, 12.0F}, cameraX, SDL_Color {45, 55, 70, 255}, ox, oy);
            // Antenna
            fillRect(renderer, RectF {e.box.x + 27.0F, e.box.y - 16.0F, 2.0F, 8.0F}, cameraX, SDL_Color {120, 130, 140, 255}, ox, oy);
            // Antenna light
            fillRect(renderer, RectF {e.box.x + 26.0F, e.box.y - 18.0F, 4.0F, 3.0F}, cameraX, SDL_Color {255, 80, 80, 255}, ox, oy);
            // Legs
            fillRect(renderer, RectF {e.box.x + 8.0F, e.box.y + 55.0F, 12.0F, 15.0F}, cameraX, SDL_Color {70, 78, 90, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x + 38.0F, e.box.y + 55.0F, 12.0F, 15.0F}, cameraX, SDL_Color {70, 78, 90, 255}, ox, oy);
            // HP bar
            const float ratio = static_cast<float>(e.hp) / static_cast<float>(e.maxHp);
            fillRect(renderer, RectF {e.box.x, e.box.y - 22.0F, e.box.w, 4.0F}, cameraX, SDL_Color {42, 42, 46, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x, e.box.y - 22.0F, e.box.w * ratio, 4.0F}, cameraX, SDL_Color {225, 60, 56, 255}, ox, oy);
            fillRect(renderer, RectF {e.box.x, e.box.y - 22.0F, e.box.w, 1.0F}, cameraX, SDL_Color {255, 120, 120, 255}, ox, oy);
            break;
        }
    }
}

void renderBullets(SDL_Renderer* renderer, const Game& game, float ox, float oy) {
    for (const Bullet& b : game.bullets) {
        const SDL_Color color = b.owner == 0
            ? (b.spread ? SDL_Color {116, 232, 255, 255} : SDL_Color {255, 242, 109, 255})
            : SDL_Color {255, 96, 76, 255};
        fillCircle(renderer, iround(b.pos.x - game.cameraX + ox), iround(b.pos.y + oy), static_cast<int>(b.radius), color);
        // Glow
        if (b.owner == 0) {
            const SDL_Color glow = b.spread ? SDL_Color {116, 232, 255, 80} : SDL_Color {255, 242, 109, 80};
            fillCircle(renderer, iround(b.pos.x - game.cameraX + ox), iround(b.pos.y + oy), static_cast<int>(b.radius) + 1, glow);
        }
    }
}

void renderParticles(SDL_Renderer* renderer, const Game& game, float ox, float oy) {
    for (const Particle& p : game.particles) {
        SDL_Color c = p.color;
        c.a = static_cast<uint8_t>(255.0F * clampf(p.life / p.maxLife, 0.0F, 1.0F));
        setDrawColor(renderer, c);
        SDL_Rect r {
            iround(p.pos.x - game.cameraX + ox),
            iround(p.pos.y + oy),
            std::max(1, iround(p.size)),
            std::max(1, iround(p.size))
        };
        SDL_RenderFillRect(renderer, &r);
    }
}

void renderHud(SDL_Renderer* renderer, const Game& game) {
    // Score
    drawText(renderer, 8, 6, "SCORE " + std::to_string(game.score), SDL_Color {228, 236, 214, 255}, 2);

    // HP label and hearts
    drawText(renderer, 8, 22, "HP", SDL_Color {228, 236, 214, 255}, 2);
    for (int i = 0; i < game.player.hp; ++i) {
        const int hx = 35 + i * 10;
        const int hy = 23;
        // Heart body
        fillCircle(renderer, hx + 2, hy + 2, 3, SDL_Color {230, 60, 60, 255});
        fillCircle(renderer, hx + 6, hy + 2, 3, SDL_Color {230, 60, 60, 255});
        // Heart bottom
        setDrawColor(renderer, SDL_Color {230, 60, 60, 255});
        SDL_Point tri[3] = {{hx, hy + 3}, {hx + 8, hy + 3}, {hx + 4, hy + 8}};
        SDL_RenderDrawLine(renderer, tri[0].x, tri[0].y, tri[2].x, tri[2].y);
        SDL_RenderDrawLine(renderer, tri[1].x, tri[1].y, tri[2].x, tri[2].y);
        // Fill the heart bottom
        for (int fy = hy + 3; fy <= hy + 8; ++fy) {
            const int halfW = (fy - hy - 3);
            SDL_RenderDrawLine(renderer, hx + 4 - halfW, fy, hx + 4 + halfW, fy);
        }
        // Highlight
        setDrawColor(renderer, SDL_Color {255, 160, 160, 255});
        SDL_RenderDrawPoint(renderer, hx + 2, hy + 1);
    }

    // Lives
    drawText(renderer, 240, 6, "LIVES " + std::to_string(game.player.lives), SDL_Color {228, 236, 214, 255}, 2);

    // Weapon indicator
    const bool isSpread = game.player.weapon == Weapon::Spread;
    const SDL_Color weaponColor = isSpread ? SDL_Color {116, 232, 255, 255} : SDL_Color {255, 242, 109, 255};
    fillRect(renderer, RectF {210.0F, 23.0F, 42.0F, 10.0F}, 0.0F, SDL_Color {0, 0, 0, 140});
    drawRect(renderer, RectF {210.0F, 23.0F, 42.0F, 10.0F}, 0.0F, weaponColor);
    drawText(renderer, 213, 25, isSpread ? "SPREAD" : "RIFLE", weaponColor, 1);

    // Click window notice
    if (!game.hasKeyboardFocus) {
        drawText(renderer, 90, 46, "CLICK TO FOCUS", SDL_Color {255, 230, 120, 255}, 2);
    }

    // Game Over
    if (game.gameOver) {
        SDL_Rect shade {0, 0, kScreenW, kScreenH};
        setDrawColor(renderer, SDL_Color {0, 0, 0, 170});
        SDL_RenderFillRect(renderer, &shade);
        drawText(renderer, 89, 80, "GAME OVER", SDL_Color {255, 80, 60, 255}, 4);
        drawText(renderer, 73, 100, "FINAL SCORE " + std::to_string(game.score), SDL_Color {228, 236, 214, 255}, 2);
        drawText(renderer, 86, 124, "PRESS R", SDL_Color {228, 236, 214, 255}, 3);
    } else if (game.victory) {
        SDL_Rect shade {0, 0, kScreenW, kScreenH};
        setDrawColor(renderer, SDL_Color {0, 0, 0, 140});
        SDL_RenderFillRect(renderer, &shade);
        drawText(renderer, 92, 68, "MISSION", SDL_Color {255, 230, 120, 255}, 4);
        drawText(renderer, 80, 96, "COMPLETE", SDL_Color {255, 230, 120, 255}, 4);
        drawText(renderer, 73, 120, "FINAL SCORE " + std::to_string(game.score), SDL_Color {228, 236, 214, 255}, 2);
        drawText(renderer, 86, 144, "PRESS R", SDL_Color {228, 236, 214, 255}, 3);
    }
}

void renderStartScreen(SDL_Renderer* renderer) {
    for (int y = 0; y < kScreenH; ++y) {
        const float t = static_cast<float>(y) / static_cast<float>(kScreenH);
        setDrawColor(renderer, mix(SDL_Color {8, 12, 28, 255}, SDL_Color {20, 40, 55, 255}, t));
        SDL_RenderDrawLine(renderer, 0, y, kScreenW, y);
    }

    // Decorative top line
    setDrawColor(renderer, SDL_Color {80, 200, 120, 255});
    SDL_RenderDrawLine(renderer, 30, 20, kScreenW - 30, 20);
    SDL_RenderDrawLine(renderer, 30, 105, kScreenW - 30, 105);

    // Title
    const std::string title1 = "ARCADE";
    const std::string title2 = "RUN GUN";
    drawText(renderer, (kScreenW - textWidth(title1, 4)) / 2, 30, title1, SDL_Color {80, 220, 120, 255}, 4);
    drawText(renderer, (kScreenW - textWidth(title2, 4)) / 2, 58, title2, SDL_Color {255, 230, 120, 255}, 4);

    // Controls
    const SDL_Color lbl {160, 180, 200, 255};
    const SDL_Color key {116, 232, 255, 255};
    drawText(renderer, 60, 115, "W A S D    MOVE", lerpColor(lbl, key, 0.3F), 2);
    drawText(renderer, 60, 131, "J / SPACE  JUMP", lerpColor(lbl, key, 0.3F), 2);
    drawText(renderer, 60, 147, "K / CTRL   FIRE", lerpColor(lbl, key, 0.3F), 2);
    drawText(renderer, 60, 163, "1 2 Q      WEAPON", lerpColor(lbl, key, 0.3F), 2);

    // Blink prompt
    const int blink = (SDL_GetTicks() / 500) % 2;
    if (blink == 0) {
        const std::string prompt = "PRESS ENTER TO START";
        drawText(renderer, (kScreenW - textWidth(prompt, 2)) / 2, 190, prompt, SDL_Color {255, 255, 255, 255}, 2);
    }
}

void renderGame(SDL_Renderer* renderer, const Game& game, float alpha) {
    float ox = 0.0F;
    float oy = 0.0F;
    if (game.shakeTimer > 0.0F) {
        const int tick = SDL_GetTicks() / 16;
        ox = static_cast<float>((tick % 3) - 1) * game.shakePower;
        oy = static_cast<float>(((tick / 2) % 3) - 1) * game.shakePower * 0.5F;
    }

    // Interpolated camera position
    const float renderCamX = game.prevCameraX + (game.cameraX - game.prevCameraX) * alpha;

    renderBackground(renderer, game, ox, oy);
    renderPlatforms(renderer, game, ox, oy);
    for (const Enemy& e : game.enemies) {
        renderEnemy(renderer, e, renderCamX, ox, oy);
    }

    // Interpolated player
    const RectF& prev = game.prevPlayerBox;
    const RectF& curr = game.player.box;
    Player interpPlayer = game.player;
    interpPlayer.box.x = prev.x + (curr.x - prev.x) * alpha;
    interpPlayer.box.y = prev.y + (curr.y - prev.y) * alpha;
    renderPlayer(renderer, interpPlayer, renderCamX, ox, oy);

    renderBullets(renderer, game, ox, oy);
    renderParticles(renderer, game, ox, oy);
    renderHud(renderer, game);
}

void applyKeyState(Input& input, SDL_Keycode key, bool pressed) {
    switch (key) {
        case SDLK_a:
        case SDLK_LEFT:
            input.leftHeld = pressed;
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            input.rightHeld = pressed;
            break;
        case SDLK_w:
        case SDLK_UP:
            input.upHeld = pressed;
            break;
        case SDLK_s:
        case SDLK_DOWN:
            input.downHeld = pressed;
            break;
        case SDLK_j:
        case SDLK_SPACE:
            input.jumpHeld = pressed;
            if (pressed) {
                input.jumpPressed = true;
            }
            break;
        case SDLK_k:
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            input.fireHeld = pressed;
            if (pressed) {
                input.firePressed = true;
            }
            break;
        default:
            break;
    }
}

#ifdef _WIN32
bool nativeKeyDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool nativeAnyDown(std::initializer_list<int> keys) {
    for (int key : keys) {
        if (nativeKeyDown(key)) {
            return true;
        }
    }
    return false;
}
#endif

void processEvents(Game& game, bool& running) {
    Input& input = game.input;
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            game.hasKeyboardFocus = true;
        }
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                game.hasKeyboardFocus = true;
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                game.hasKeyboardFocus = false;
            }
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            const bool pressed = event.type == SDL_KEYDOWN;
            applyKeyState(input, event.key.keysym.sym, pressed);
            if (!pressed || event.key.repeat != 0) {
                continue;
            }
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_RETURN:
                    input.startPressed = true;
                    break;
                case SDLK_1:
                    input.weaponNormalPressed = true;
                    break;
                case SDLK_2:
                    input.weaponSpreadPressed = true;
                    break;
                case SDLK_q:
                    input.weaponTogglePressed = true;
                    break;
                case SDLK_r:
                    input.restartPressed = true;
                    break;
                default:
                    break;
            }
        }
    }
}

void pollInput(Game& game) {
    Input& input = game.input;
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    input.left = input.leftHeld || keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
    input.right = input.rightHeld || keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
    input.up = input.upHeld || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    input.down = input.downHeld || keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    input.jump = input.jumpHeld || keys[SDL_SCANCODE_J] || keys[SDL_SCANCODE_SPACE];
    input.fire = input.fireHeld || keys[SDL_SCANCODE_K] || keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];

#ifdef _WIN32
    const bool nativeLeft = nativeAnyDown({'A', VK_LEFT});
    const bool nativeRight = nativeAnyDown({'D', VK_RIGHT});
    const bool nativeUp = nativeAnyDown({'W', VK_UP});
    const bool nativeDown = nativeAnyDown({'S', VK_DOWN});
    const bool nativeJump = nativeAnyDown({'J', VK_SPACE});
    const bool nativeFire = nativeAnyDown({'K', VK_CONTROL});
    const bool nativeWeapon1 = nativeKeyDown('1');
    const bool nativeWeapon2 = nativeKeyDown('2');
    const bool nativeToggle = nativeKeyDown('Q');
    const bool nativeRestart = nativeKeyDown('R');
    const bool nativeStart = nativeKeyDown(VK_RETURN);

    input.left = input.left || nativeLeft;
    input.right = input.right || nativeRight;
    input.up = input.up || nativeUp;
    input.down = input.down || nativeDown;
    input.jump = input.jump || nativeJump;
    input.fire = input.fire || nativeFire;

    if (nativeJump && !input.nativeJumpWasDown) {
        input.jumpPressed = true;
    }
    if (nativeFire && !input.nativeFireWasDown) {
        input.firePressed = true;
    }
    if (nativeWeapon1 && !input.nativeWeapon1WasDown) {
        input.weaponNormalPressed = true;
    }
    if (nativeWeapon2 && !input.nativeWeapon2WasDown) {
        input.weaponSpreadPressed = true;
    }
    if (nativeToggle && !input.nativeToggleWasDown) {
        input.weaponTogglePressed = true;
    }
    if (nativeRestart && !input.nativeRestartWasDown) {
        input.restartPressed = true;
    }
    if (nativeStart && !input.nativeStartWasDown) {
        input.startPressed = true;
    }

    input.nativeJumpWasDown = nativeJump;
    input.nativeFireWasDown = nativeFire;
    input.nativeWeapon1WasDown = nativeWeapon1;
    input.nativeWeapon2WasDown = nativeWeapon2;
    input.nativeToggleWasDown = nativeToggle;
    input.nativeRestartWasDown = nativeRestart;
    input.nativeStartWasDown = nativeStart;
#endif
}

void clearPressedFlags(Input& input) {
    input.jumpPressed = false;
    input.firePressed = false;
    input.weaponNormalPressed = false;
    input.weaponSpreadPressed = false;
    input.weaponTogglePressed = false;
    input.restartPressed = false;
    input.startPressed = false;
}

}  // namespace

int main(int, char**) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Arcade Run Gun",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kScreenW * 4,
        kScreenH * 4,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, kScreenW, kScreenH);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RaiseWindow(window);

    Game game;
    bool running = true;
    uint64_t previous = SDL_GetPerformanceCounter();
    double accumulator = 0.0;
    constexpr double maxFrame = 0.25;

    while (running) {
        const uint64_t now = SDL_GetPerformanceCounter();
        double frameTime = static_cast<double>(now - previous) / static_cast<double>(SDL_GetPerformanceFrequency());
        previous = now;
        frameTime = std::min(frameTime, maxFrame);
        accumulator += frameTime;

        // Process SDL events once per frame
        processEvents(game, running);

        if (game.startScreen) {
            if (game.input.startPressed) {
                resetGame(game);
            }
            clearPressedFlags(game.input);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            renderStartScreen(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        // Save previous state for interpolation
        game.prevPlayerBox = game.player.box;
        game.prevCameraX = game.cameraX;

        // Run game steps with input polling each step
        while (accumulator >= kDt) {
            pollInput(game);
            updateGame(game);
            clearPressedFlags(game.input);
            accumulator -= kDt;
        }

        const float alpha = static_cast<float>(accumulator / kDt);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderGame(renderer, game, alpha);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
