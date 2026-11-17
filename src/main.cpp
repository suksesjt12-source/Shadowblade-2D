#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
constexpr int SW = 960, SH = 540;
constexpr float WORLD_W = 2400.0f;
const Color NIGHT{12, 17, 35, 255}, MIDNIGHT{20, 29, 55, 255};
const Color SB_CYAN{117, 231, 255, 255}, SB_PURPLE{139, 112, 231, 255};

struct Platform { Rectangle r; };
struct Shard { Vector2 p; bool taken = false; };
struct Sentry { Rectangle r; float left, right, speed, phase; };
enum class Mode { Title, Playing, Paused, GameOver, Won };

struct Game {
    Rectangle player{70, 350, 28, 42};
    Vector2 velocity{0, 0};
    std::vector<Platform> platforms;
    std::vector<Shard> shards;
    std::vector<Sentry> sentries;
    Camera2D camera{};
    Mode mode = Mode::Title;
    int energy = 3, collected = 0;
    float coyote = 0, jumpBuffer = 0, dashTime = 0, dashCooldown = 0;
    float hurtTime = 0;
    bool grounded = false;
    Vector2 checkpoint{70, 350};

    Game() {
        platforms = {
            {{0, 470, 520, 100}}, {{600, 430, 260, 110}},
            {{930, 380, 220, 160}}, {{1220, 445, 330, 100}},
            {{1630, 390, 230, 150}}, {{1940, 455, 460, 100}},
            {{260, 360, 120, 18}}, {{720, 320, 120, 18}},
            {{1040, 275, 120, 18}}, {{1390, 345, 110, 18}},
            {{1740, 290, 110, 18}}
        };
        shards = {{{300, 320}}, {{760, 280}}, {{1080, 235}}, {{1440, 305}}, {{1790, 250}}, {{2110, 410}}};
        sentries = {
            {{390, 438, 28, 32}, 340, 490, 75, 0},
            {{1010, 348, 28, 32}, 950, 1110, 95, 1.3f},
            {{1780, 358, 28, 32}, 1660, 1810, 82, 2.2f}
        };
        camera.offset = {SW / 2.0f, SH / 2.0f};
        camera.zoom = 1.0f;
    }

    void Reset() {
        player = {70, 350, 28, 42}; velocity = {0,0};
        energy = 3; collected = 0; coyote = jumpBuffer = dashTime = dashCooldown = hurtTime = 0;
        checkpoint = {70,350}; mode = Mode::Playing;
        for (auto& s : shards) s.taken = false;
    }
};

bool TouchPressed(Rectangle r) {
    for (int i = 0; i < GetTouchPointCount(); ++i) {
        Vector2 p = GetTouchPosition(i);
        if (CheckCollisionPointRec(p, r)) return true;
    }
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
}
bool Pressed(int key) { return IsKeyPressed(key); }

void DrawButton(Rectangle r, const char* label, bool active) {
    DrawRectangleRounded(r, 0.25f, 8, active ? Color{66, 95, 145, 225} : Color{25, 37, 65, 190});
    DrawRectangleRoundedLines(r, 0.25f, 8, Color{115, 190, 230, 180});
    int tw = MeasureText(label, 16);
    DrawText(label, (int)(r.x + r.width/2 - tw/2), (int)(r.y + r.height/2 - 8), 16, RAYWHITE);
}
void DrawWorld(Game& g) {
    DrawRectangleGradientV(-100, -100, (int)WORLD_W+200, 700, Color{29, 39, 75, 255}, NIGHT);
    for (int i=0;i<42;i++) {
        float x = (float)((i*173)%2450), y = (float)((i*97)%300);
        DrawCircleV({x,y}, (i%4==0)?2.2f:1.2f, Color{157,190,239,100});
    }
    for (int layer=0; layer<3; ++layer) {
        float base = 420.0f + layer*38;
        for (int i=0;i<18;i++) {
            float x = i*150.0f - 60;
            DrawTriangle({x,base},{x+85,base-100-layer*25},{x+180,base}, layer==0?Color{34,48,83,255}:Color{23,34,65,255});
        }
    }
    for (const auto& p : g.platforms) {
        DrawRectangleRec(p.r, Color{35,48,77,255});
        DrawRectangle((int)p.r.x,(int)p.r.y,(int)p.r.width,5,Color{90,133,170,255});
        DrawRectangleLinesEx(p.r,1,Color{62,82,119,255});
        for (int x=(int)p.r.x+12; x<p.r.x+p.r.width-8; x+=38)
            DrawLine(x,(int)p.r.y+15,x+10,(int)p.r.y+15,Color{57,72,103,255});
    }
    DrawCircle(2240, 405, 42, Color{72,54,132,90});
    DrawCircleLines(2240,405,42,SB_PURPLE);
    DrawRectangle(2224,365,32,80,Color{87,68,157,255});
    DrawRectangle(2230,372,20,66,SB_CYAN);
    for (const auto& s : g.shards) if (!s.taken) {
        DrawCircleV(s.p, 15, Color{95,224,255,45});
        DrawPoly(s.p,4,9,45,SB_CYAN);
        DrawCircleV(s.p,3,RAYWHITE);
    }
    for (const auto& e : g.sentries) {
        DrawCircle((int)(e.r.x+e.r.width/2),(int)(e.r.y+10),14,Color{112,82,184,255});
        DrawRectangleRounded({e.r.x,e.r.y+10,e.r.width,e.r.height-10},0.35f,5,Color{69,57,117,255});
        DrawCircle((int)e.r.x+9,(int)e.r.y+11,3,SB_CYAN);
        DrawCircle((int)e.r.x+20,(int)e.r.y+11,3,SB_CYAN);
    }
    Color pc = g.hurtTime>0 ? Color{255,180,190,255} : Color{223,235,255,255};
    DrawCircle((int)(g.player.x+14),(int)(g.player.y+10),11,pc);
    DrawRectangleRounded({g.player.x+4,g.player.y+19,20,23},0.3f,5,pc);
    DrawRectangle((int)g.player.x+5,(int)g.player.y+38,7,5,SB_PURPLE);
    DrawRectangle((int)g.player.x+17,(int)g.player.y+38,7,5,SB_PURPLE);
    DrawCircle((int)g.player.x+18,(int)g.player.y+10,2,NIGHT);
}

void UpdateGame(Game& g, float dt) {
    if (g.mode != Mode::Playing) return;
    const bool left = IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)||TouchPressed({24,SH-108,76,76});
    const bool right = IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)||TouchPressed({112,SH-108,76,76});
    const bool jump = Pressed(KEY_SPACE)||Pressed(KEY_W)||Pressed(KEY_UP)||TouchPressed({SW-190,SH-108,76,76});
    const bool dash = Pressed(KEY_LEFT_SHIFT)||Pressed(KEY_X)||TouchPressed({SW-100,SH-108,76,76});
    if (jump) g.jumpBuffer = 0.14f;
    else g.jumpBuffer = std::max(0.0f,g.jumpBuffer-dt);
    g.coyote = g.grounded ? 0.12f : std::max(0.0f,g.coyote-dt);
    g.dashCooldown = std::max(0.0f,g.dashCooldown-dt);
    g.dashTime = std::max(0.0f,g.dashTime-dt);
    g.hurtTime = std::max(0.0f,g.hurtTime-dt);

    float dir = (right?1.0f:0.0f)-(left?1.0f:0.0f);
    if (dash && g.dashCooldown<=0) {
        g.dashTime = 0.16f; g.dashCooldown = 0.85f;
        g.velocity.x = (dir==0 ? 1.0f : dir)*470.0f;
        g.velocity.y = 0;
    }
    if (g.dashTime>0) {
        g.player.x += g.velocity.x*dt;
    } else {
        g.velocity.x += dir*1250.0f*dt;
        if (dir==0) g.velocity.x *= std::pow(0.0008f,dt);
        g.velocity.x = std::clamp(g.velocity.x,-230.0f,230.0f);
        if (g.jumpBuffer>0 && g.coyote>0) {
            g.velocity.y = -465.0f; g.jumpBuffer=0; g.coyote=0; g.grounded=false;
        }
        if (!(IsKeyDown(KEY_SPACE)||IsKeyDown(KEY_W)||TouchPressed({SW-190,SH-108,76,76})) && g.velocity.y < -170)
            g.velocity.y += 1250.0f*dt;
        g.velocity.y += 1250.0f*dt;
        g.player.x += g.velocity.x*dt;
        g.player.y += g.velocity.y*dt;
    }

    g.player.x = std::clamp(g.player.x,0.0f,WORLD_W-g.player.width);
    g.grounded=false;
    if (g.dashTime<=0) {
        for (const auto& p : g.platforms) {
            if (g.velocity.y>=0 && g.player.x+g.player.width>p.r.x && g.player.x<p.r.x+p.r.width &&
                g.player.y+g.player.height>=p.r.y && g.player.y+g.player.height-g.velocity.y*dt<=p.r.y) {
                g.player.y=p.r.y-g.player.height; g.velocity.y=0; g.grounded=true;
            }
        }
    }
    if (g.player.y>650) { g.energy=0; }
    for (auto& e : g.sentries) {
        e.r.x += e.speed*dt;
        if (e.r.x<e.left || e.r.x>e.right) { e.speed=-e.speed; e.r.x=std::clamp(e.r.x,e.left,e.right); }
        if (g.hurtTime<=0 && CheckCollisionRecs(g.player,e.r)) {
            --g.energy; g.hurtTime=1.1f;
            g.player.x = std::max(0.0f,g.player.x-65.0f);
            g.velocity.y=-240.0f;
        }
    }
    for (auto& s : g.shards) if (!s.taken && CheckCollisionCircleRec(s.p,15,g.player)) {
        s.taken=true; ++g.collected;
    }
    if (g.player.x>2160 && g.collected>=3) g.mode=Mode::Won;
    if (g.player.x>600 && g.checkpoint.x<600) g.checkpoint={630,388};
    if (g.energy<=0) g.mode=Mode::GameOver;
    g.camera.target={g.player.x+g.player.width/2,g.player.y+g.player.height/2};
    g.camera.target.x=std::clamp(g.camera.target.x,SW/2.0f,WORLD_W-SW/2.0f);
    g.camera.target.y=std::clamp(g.camera.target.y,SH/2.0f,430.0f);
}

void DrawHUD(const Game& g) {
    DrawRectangleRounded({18,16,270,58},0.2f,8,Color{10,16,32,205});
    DrawText("ECHO ENERGY",32,25,12,Color{153,181,222,255});
    for(int i=0;i<3;i++) DrawCircle(43+i*25,53,7,i<g.energy?SB_CYAN:Color{57,66,91,255});
    DrawText(TextFormat("ECHOES  %02i / 06",g.collected),130,46,15,RAYWHITE);
    DrawRectangleRounded({SW-180,16,160,38},0.2f,8,Color{10,16,32,190});
    DrawText("ESC  PAUSE",SW-163,27,14,RAYWHITE);
    DrawButton({24,SH-108,76,76},"LEFT",false);
    DrawButton({112,SH-108,76,76},"RIGHT",false);
    DrawButton({SW-190,SH-108,76,76},"JUMP",false);
    DrawButton({SW-100,SH-108,76,76},"DASH",false);
}

void DrawOverlay(const char* title,const char* subtitle,const char* action) {
    DrawRectangle(0,0,SW,SH,Color{5,9,22,205});
    int w=MeasureText(title,42);
    DrawText(title,SW/2-w/2,170,42,RAYWHITE);
    int sw=MeasureText(subtitle,18);
    DrawText(subtitle,SW/2-sw/2,232,18,Color{174,197,231,255});
    int aw=MeasureText(action,18);
    DrawText(action,SW/2-aw/2,305,18,SB_CYAN);
}
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SW,SH,"SHADOWBLADE: ECHOES OF DUSK");
    SetTargetFPS(60);
    Game game;
    while (!WindowShouldClose()) {
        float dt=std::min(GetFrameTime(),0.033f);
        if (game.mode==Mode::Title && (Pressed(KEY_ENTER)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT)||GetTouchPointCount()>0))
            game.Reset();
        else if (game.mode==Mode::Playing) {
            if (Pressed(KEY_ESCAPE)) game.mode=Mode::Paused;
            else UpdateGame(game,dt);
        } else if (game.mode==Mode::Paused && (Pressed(KEY_ESCAPE)||Pressed(KEY_ENTER))) game.mode=Mode::Playing;
        else if ((game.mode==Mode::GameOver||game.mode==Mode::Won) && (Pressed(KEY_ENTER)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT)||GetTouchPointCount()>0))
            game.Reset();

        BeginDrawing();
        ClearBackground(NIGHT);
        if (game.mode==Mode::Title) {
            DrawRectangleGradientV(0,0,SW,SH,MIDNIGHT,NIGHT);
            for(int i=0;i<24;i++) DrawCircle((i*79)%SW,(i*47)%SH,2,Color{119,168,221,120});
            DrawCircle(SW/2,190,66,Color{75,65,140,70});
            DrawCircleLines(SW/2,190,66,SB_PURPLE);
            DrawText("SHADOWBLADE",SW/2-190,195,38,RAYWHITE);
            DrawText("E C H O E S   O F   D U S K",SW/2-148,246,16,SB_CYAN);
            DrawText("JUMP  •  DASH  •  COLLECT  •  ESCAPE",SW/2-190,320,17,Color{170,190,225,255});
            DrawText("PRESS ENTER OR TAP TO BEGIN",SW/2-160,390,18,RAYWHITE);
        } else {
            BeginMode2D(game.camera);
            DrawWorld(game);
            EndMode2D();
            DrawHUD(game);
            if(game.mode==Mode::Paused) DrawOverlay("PAUSED","Take a moment. The ruins will wait.","Press ENTER or ESC to resume");
            if(game.mode==Mode::GameOver) DrawOverlay("TRY AGAIN","The echo energy has run out.","Press ENTER or tap to restart");
            if(game.mode==Mode::Won) DrawOverlay("RUINS RESTORED","You gathered enough echoes to open the gate.","Press ENTER or tap to play again");
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
