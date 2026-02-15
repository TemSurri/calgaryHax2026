#include <SDL.h>
#include <emscripten.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <SDL_image.h>


static void playMusic() {
  EM_ASM({
    if (!Module.music) {
      Module.music = new Audio("audio/musick.mp3");
      Module.music.loop = true;
      Module.music.volume = 0.6;
    }
    Module.music.play();
  });
}

static void stopMusic() {
  EM_ASM({
    if (Module.music) {
      Module.music.pause();
      Module.music.currentTime = 0;
    }
  });
}

// ==================================================
// CONSTANTS

static constexpr int SCREEN_W = 640;
static constexpr int SCREEN_H = 640;

static constexpr int TEX_W = 64;
static constexpr int TEX_H = 64;

// Simple 2D map: 0 = empty, 1..N = wall texture index
static constexpr int MAP_W = 24;
static constexpr int MAP_H = 24;
static std::vector<double> zBuffer(SCREEN_W);

// ==================================================
// MAP newnenwne

static int worldMap[MAP_H][MAP_W];
static int floorMap[MAP_H][MAP_W];
static int ceilingMap[MAP_H][MAP_W];

struct MapData {
  int (*world)[MAP_W];
  int (*floor)[MAP_W];
  int (*ceil)[MAP_W];
};

static int worldMap1[MAP_H][MAP_W] = {
{2,2,1,4,1,3,2,2,2,2,1,4,1,3,2,2,1,3,1,8,1,3,1,2},

{2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2},

{2,2,2,2,0,2,2,2,2,2,2,0,2,2,2,2,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},

{2,0,5,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
{2,0,5,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
{2,0,5,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
{2,0,5,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
{2,0,5,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},

{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
{1,2,2,2,2,2,2,2,2,2,0,0,0,2,2,2,2,2,2,0,2,2,2,2},
{1,0,0,0,0,0,0,0,2,6,0,6,0,6,2,0,0,0,0,0,0,0,0,1},
{3,0,0,0,0,0,0,0,0,6,0,6,0,6,2,0,0,0,0,0,0,0,0,4},
{1,0,0,0,0,0,0,0,2,6,0,6,0,6,2,0,0,0,0,0,0,0,0,1},

{1,0,0,0,0,0,0,0,2,6,0,6,0,6,2,0,0,0,0,0,0,0,0,1},

{2,1,1,4,1,4,1,1,1,2,2,2,2,2,2,1,4,1,1,4,1,1,4,2}
};

static int floorMap1[MAP_H][MAP_W] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static int ceilingMap1[MAP_H][MAP_W] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static MapData map1 = {
  worldMap1,
  floorMap1,
  ceilingMap1
};

static void loadMap(const MapData& m) {
  for (int y = 0; y < MAP_H; y++) {
    for (int x = 0; x < MAP_W; x++) {
      worldMap[y][x]   = m.world[y][x];
      floorMap[y][x]   = m.floor[y][x];
      ceilingMap[y][x] = m.ceil[y][x];
    }
  }
}

// ==================================================
// TYPES

struct Texture {
  std::vector<uint32_t> pixels; // ARGB8888
  int w = 0;
  int h = 0;
};

// SDL GLOBALS

static SDL_Window*   gWindow    = nullptr;
static SDL_Renderer* gRenderer  = nullptr;
static SDL_Texture*  gScreenTex = nullptr;

static std::vector<uint32_t> gFrame(SCREEN_W * SCREEN_H);

// TEXTURES
// static texture variables, will try to assign at compile time
static Texture texWall0;
static Texture texWall1;
static Texture texWall2;
static Texture texWall3;
static Texture texWall4;
static Texture texWall5;
static Texture texWall6;
static Texture texWall7;

static Texture texFloor0;
static Texture texFloor1;
static Texture texCeil0;
static Texture texMenu;
static Texture texEnemy0;

//chair stuyff
Texture texC1; // front
Texture texC2; // front-right
Texture texC3; // right
Texture texC4; // back-right

//tables
Texture texT1; // front
Texture texT2; // front-right
Texture texT3; // right
Texture texT4; // back-right

//person
Texture texP4; // front
Texture texP41; // front-right
Texture texP42; // right
Texture texP43; // back-right

//person
Texture texP3; // front
Texture texP31; // front-right
Texture texP32; // right
Texture texP33; // back-right

//person
Texture texP1; // front
Texture texP11; // front-right
Texture texP12; // right
Texture texP13; // back-right

//person
Texture texP2; // front
Texture texP21; // front-right
Texture texP22; // right
Texture texP23; // back-right



static Texture texEnemy1;
static Texture texEnemy2;


// list of all possible map texture 
static std::vector<Texture*> floorTextures;
static std::vector<Texture*> ceilTextures;
static std::vector<Texture*> wallTextures;

// PLAYER CAMERA 

static double posX = 12.0;
static double posY = 12.0;

static double dirX = -1.0;
static double dirY =  0.0;

static double planeX = 0.0;
static double planeY = 0.66;  //FOV 

// GAME LOGIC

// game states
enum class GameState {
  MENU,
  PLAYING,
  SCHOOL1,
  SCHOOL2,
  HOME,
};

static GameState gState = GameState::MENU;


// enemy info
struct Enemy {
  double x;
  double y;
  Texture* sprite; 
  double heightScale; 
  std::vector<Texture*> directionalSprites; 
  int verticalPlaneOffset = 0;


  //movement 
  double dirX = 0;
  double dirY = 0;
  double speed = 0.02;

  double stateTimer = 0.0;
  bool isWalking = false;
};

static std::vector<Enemy*> sprites;

static Enemy enemy0 {  10.5, 12.0, &texEnemy0, 1};
static Enemy enemy1 {  10.5, 1.0, &texEnemy1, 1};
static Enemy enemy2 {  1.5, 12.0, &texEnemy2, 1};

static Enemy chair {  17.5, 20.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};


static Enemy chair1 {  17.5, 20.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy table1 {  17.5, 20.5, &texC1, 0.6, {
    &texT1,
    &texT2,
    &texT3,
    &texT4,
  }, 1};

static Enemy chair2 {  18.5, 20.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy table2 {  18.5, 20.5, &texC1, 0.6, {
    &texT1,
    &texT2,
    &texT3,
    &texT4,
  }, 1};


static Enemy chair3 {  21.5, 20.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy table3 {  21.5, 20.5, &texC1, 0.6, {
    &texT1,
    &texT2,
    &texT3,
    &texT4,
  }, 1};


static Enemy chair4 {  20.5, 20.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};


static Enemy chair5 {  17.5, 19.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy chair6 {  18.5, 19.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy chair7 {  21.5, 19.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy chair8 {  20.5, 19.0, &texC1, 0.6, {
    &texC1,
    &texC2,
    &texC3,
    &texC4,
  }, 1};

static Enemy table {  12.5, 14.0, &texC1, 0.6, {
    &texT1,
    &texT2,
    &texT3,
    &texT4,
  }, 1};


// ===== TOP LEFT CLASSROOM =====

// Row 1
static Enemy tl_table1 { 3.5, 2.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tl_chair1 { 3.5, 2.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

static Enemy tl_table2 { 5.5, 2.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tl_chair2 { 5.5, 2.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

// Row 2
static Enemy tl_table3 { 3.5, 4.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tl_chair3 { 3.5, 4.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

static Enemy tl_table4 { 5.5, 4.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tl_chair4 { 5.5, 4.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

// ===== TOP RIGHT CLASSROOM =====

// Row 1
static Enemy tr_table1 { 10.5, 2.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tr_chair1 { 10.5, 2.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

static Enemy tr_table2 { 12.5, 2.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tr_chair2 { 12.5, 2.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

// Row 2
static Enemy tr_table3 { 10.5, 4.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tr_chair3 { 10.5, 4.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };

static Enemy tr_table4 { 12.5, 4.5, &texT1, 0.6, { &texT1,&texT2,&texT3,&texT4 }, 1 };
static Enemy tr_chair4 { 12.5, 4.0, &texC1, 0.6, { &texC1,&texC2,&texC3,&texC4 }, 1 };


static Enemy person4 {  12.5, 10.0, &texP4, 0.8, {
    &texP4,
    &texP41,
    &texP42,
    &texP43,
  }, 0};
  static Enemy person1 {  11.5, 14.0, &texP1, 0.8, {
    &texP1,
    &texP11,
    &texP12,
    &texP13,
  }, 0};
  static Enemy person2 {  14.5, 14.0, &texP2, 0.8, {
    &texP2,
    &texP21,
    &texP22,
    &texP23,
  }, 0};
  static Enemy person3{  12.5, 12.0, &texP3, 0.4, {
    &texP3,
    &texP31,
    &texP32,
    &texP33,
  }, 1};


static void pickRandomDirection(Enemy& e) {
  double angle = (rand() % 628) / 100.0; // 0 to 6.28
  e.dirX = cos(angle);
  e.dirY = sin(angle);
}

static void updateWander(Enemy& e) {

  e.stateTimer -= 0.016; // approx frame time

  if (e.stateTimer <= 0) {
    e.isWalking = !e.isWalking;

    if (e.isWalking) {
      pickRandomDirection(e);
      e.stateTimer = 2.0 + (rand() % 200) / 100.0; // walk 2–4 sec
    } else {
      e.stateTimer = 1.0 + (rand() % 200) / 200.0; // idle 1–2 sec
    }
  }

  if (!e.isWalking) return;

  double nx = e.x + e.dirX * e.speed;
  double ny = e.y + e.dirY * e.speed;

  if (worldMap[int(e.y)][int(nx)] == 0)
      e.x = nx;
  else
      pickRandomDirection(e);

  if (worldMap[int(ny)][int(e.x)] == 0)
      e.y = ny;
  else
      pickRandomDirection(e);
}

static void updateEnemy(Enemy& enemy) {
  double dx = posX - enemy.x;
  double dy = posY - enemy.y;
  double dist = std::sqrt(dx * dx + dy * dy);

  if (dist < 0.4) {
    // Game over -> back to menu
    gState = GameState::MENU;
    stopMusic(); 
    return;
  }

  if (dist > 0.001) {
    enemy.x += (dx / dist) * 0.03;
    enemy.y += (dy / dist) * 0.03;
  }

  //std::cerr << enemy.x << " " << enemy.y << std::endl;

}

// ==================================================
// TEXTURE LOADING PREP 

static inline uint32_t packARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

static bool loadBMPTexture(const std::string& path, Texture& out) {
  //1 SDL_Surface* s = SDL_LoadBMP(path.c_str());
  SDL_Surface* s = IMG_Load(path.c_str());

  if (!s) {
    std::cerr << "IMG_Load failed: " << path 
              << " : " << IMG_GetError() << "\n";
    return false;
  }

  // Convert to ARGB8888 for easy pixel reads
  SDL_Surface* conv = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ARGB8888, 0);
  SDL_FreeSurface(s);
  if (!conv) {
    std::cerr << "SDL_ConvertSurfaceFormat failed: " << path << " : " << SDL_GetError() << "\n";
    return false;
  }

  out.w = conv->w;
  out.h = conv->h;
  out.pixels.resize(size_t(out.w) * size_t(out.h));

  std::memcpy(out.pixels.data(), conv->pixels, out.pixels.size() * sizeof(uint32_t));
  SDL_FreeSurface(conv);

  return true;
}

static inline uint32_t sampleTex(const Texture& t, int x, int y) {
  x &= (TEX_W - 1);
  y &= (TEX_H - 1);
  return t.pixels[size_t(y) * TEX_W + size_t(x)];
}

static inline void putPixel(int x, int y, uint32_t argb) {
  if ((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return;
  gFrame[size_t(y) * SCREEN_W + size_t(x)] = argb;
}

// ==================================================
// VISIBILITY & ACTUAL ENGINE 

static bool hasLineOfSight(double x0, double y0, double x1, double y1) {
  double dx = x1 - x0;
  double dy = y1 - y0;
  double dist = std::sqrt(dx * dx + dy * dy);

  if (dist < 0.0001) return true;

  double step = 0.05; // grid precision
  double vx = dx / dist;
  double vy = dy / dist;

  for (double t = 0.0; t < dist; t += step) {
    int mx = int(x0 + vx * t);
    int my = int(y0 + vy * t);

    if (mx < 0 || mx >= MAP_W || my < 0 || my >= MAP_H)
      return false;

    if (worldMap[my][mx] > 0)
      return false; // wall blocks view
  }
  return true;
}

// render enemy 
static void renderEnemyPlaceholder(const Enemy& enemy) {

  if (!hasLineOfSight(posX, posY, enemy.x, enemy.y))
    return;
    
  // Vector from player to enemy
  double spriteX = enemy.x - posX;
  double spriteY = enemy.y - posY;

  // Inverse camera matrix
  double invDet = 1.0 / (planeX * dirY - dirX * planeY);

  // Transform to camera space
  double transformX = invDet * (dirY * spriteX - dirX * spriteY);
  double transformY = invDet * (-planeY * spriteX + planeX * spriteY);

  // Behind camera
  if (transformY <= 0) return;

  int spriteScreenX = int((SCREEN_W / 2) * (1 + transformX / transformY));


  // Angle from chair to player
  double angle = atan2(posY - enemy.y, posX - enemy.x);

  // Normalize 0 → 2π
  if (angle < 0) angle += 2 * M_PI;

  // Convert to 8 directions
  int dirIndex = int((angle / (2 * M_PI)) * 4.0);
  dirIndex = dirIndex % 4;

  
  // Ground correction factor
  int spriteHeight = std::abs(int((SCREEN_H / transformY) * enemy.heightScale));

  int planeY = (SCREEN_H / 2) ; //int(enemy.verticalPlaneOffset

  int drawEndY;
  int drawStartY;
  int centerY = SCREEN_H / 2;
  if (enemy.verticalPlaneOffset == 1) {
    std::cerr<<"chid";
    int centerY = SCREEN_H / 2;

    drawStartY = centerY;
    drawEndY   = drawStartY + spriteHeight;
    
  };
  if (enemy.verticalPlaneOffset == 2){

    int floorY = SCREEN_H / 2; 
    drawEndY = floorY; 
    drawStartY = drawEndY - spriteHeight;

  };
  if (enemy.verticalPlaneOffset == 0) {
    std::cerr<<"nigg";
    drawStartY = -spriteHeight / 2 + planeY;
    drawEndY   =  spriteHeight / 2 + planeY;

  };

  if (drawStartY < 0) drawStartY = 0;
  if (drawEndY >= SCREEN_H) drawEndY = SCREEN_H - 1;

  // Width
  int spriteWidth = std::abs(int(spriteHeight * 
                (double)enemy.sprite->w / enemy.sprite->h));
  int drawStartX = -spriteWidth / 2 + spriteScreenX;
  int drawEndX   =  spriteWidth / 2 + spriteScreenX;

  // Draw blue rectangle
  for (int x = drawStartX; x < drawEndX; x++) {
    if (x < 0 || x >= SCREEN_W) continue;
    // Z-buffer check: sprite must be closer than wall
    if (transformY >= zBuffer[x]) continue;

    Texture* spriteToUse = enemy.sprite;

    if (!enemy.directionalSprites.empty() &&
        dirIndex < enemy.directionalSprites.size() &&
        enemy.directionalSprites[dirIndex])
    {
        spriteToUse = enemy.directionalSprites[dirIndex];
    }

    for (int y = drawStartY; y < drawEndY; y++) {
      int texX = int((x - drawStartX) * spriteToUse->w / spriteWidth);
      int texY = int((y - drawStartY) * spriteToUse->h / spriteHeight);

      texX = std::clamp(texX, 0, spriteToUse->w - 1);
      texY = std::clamp(texY, 0, spriteToUse->h - 1);

      uint32_t color = spriteToUse->pixels[texY * spriteToUse->w + texX];

      // Transparent pixel skip (if using black as transparency)
      uint8_t alpha = (color >> 24) & 0xFF;
      if (alpha > 10) {
          putPixel(x, y, color);
      }
    }
  }
}
//interactables

enum class InteractType {
  TOGGLE_WALL_TILE,     // changes worldMap[y][x] (door open/close)
  TOGGLE_FLOOR_TILE,    // changes floorMap
  TOGGLE_CEIL_TILE,     // changes ceilingMap
  TOGGLE_SPRITE_TEXTURE // swaps a sprite's texture pointer
};

struct Interactable {
  double x;
  double y;
  double radius;        // how close you must be
  InteractType type;

  std::string promptNear;   // e.g. "Press E to open"
  std::string promptFar;    // optional (can be "")

  // “toggle state”
  bool isOn = false;

  // For map changes:
  int cellX = -1;
  int cellY = -1;
  int offValue = 0;     // map value when off
  int onValue  = 0;     // map value when on

  // For sprite changes:
  Texture** spritePtr = nullptr; // pointer-to-pointer so we can swap it
  Texture*  offTex = nullptr;
  Texture*  onTex  = nullptr;
};


static std::vector<Interactable> interactables;

static void initInteractables() {
  Interactable door;
  door.x = 0.5;
  door.y = 8.5;
  door.radius = 1.2;
  door.type = InteractType::TOGGLE_WALL_TILE;
  door.promptNear = "Press E to open/close";
  door.cellX = 0;
  door.cellY = 8;
  door.offValue = 1; // closed door tile (some wall texture index)
  door.onValue  = 3; // open (empty)
  door.isOn = false; // start closed
  interactables.push_back(door);

  // Example: toggle chair texture (swap sprite)
  Interactable chairSwap;
  chairSwap.x = chair.x;
  chairSwap.y = chair.y;
  chairSwap.radius = 1.2;
  chairSwap.type = InteractType::TOGGLE_SPRITE_TEXTURE;
  chairSwap.promptNear = "Press E to change chair";
  chairSwap.spritePtr = &chair.sprite;
  chairSwap.offTex = &texC1;
  chairSwap.onTex  = &texC2;
  chairSwap.isOn = false;
  interactables.push_back(chairSwap);
}

static int gNearestInteractable = -1;
static double gNearestDist = 1e9;

static void updateNearestInteractable() {
  gNearestInteractable = -1;
  gNearestDist = 1e9;

  for (int i = 0; i < (int)interactables.size(); i++) {
    auto& it = interactables[i];
    double dx = it.x - posX;
    double dy = it.y - posY;
    double dist = std::sqrt(dx*dx + dy*dy);

    if (dist <= it.radius && dist < gNearestDist) {
      gNearestDist = dist;
      gNearestInteractable = i;
    }
  }
}

static bool prevE = false;

static void tryInteract(const Uint8* keys) {
  bool eDown = keys[SDL_SCANCODE_E];
  bool ePressed = (eDown && !prevE);
  prevE = eDown;

  if (!ePressed) return;
  if (gNearestInteractable < 0) return;

  auto& it = interactables[gNearestInteractable];

  it.isOn = !it.isOn;

  switch (it.type) {
    case InteractType::TOGGLE_WALL_TILE: {
      worldMap[it.cellY][it.cellX] = it.isOn ? it.onValue : it.offValue;
    } break;

    case InteractType::TOGGLE_FLOOR_TILE: {
      floorMap[it.cellY][it.cellX] = it.isOn ? it.onValue : it.offValue;
    } break;

    case InteractType::TOGGLE_CEIL_TILE: {
      ceilingMap[it.cellY][it.cellX] = it.isOn ? it.onValue : it.offValue;
    } break;

    case InteractType::TOGGLE_SPRITE_TEXTURE: {
      if (it.spritePtr && it.offTex && it.onTex) {
        *it.spritePtr = it.isOn ? it.onTex : it.offTex;
      }
    } break;
  }
}


static void drawPromptBar(const std::string& msg) {
  // simple rectangle background (no font needed)
  int w = 360, h = 40;
  int x0 = (SCREEN_W - w) / 2;
  int y0 = SCREEN_H - h - 18;

  uint32_t bg = packARGB(180, 0, 0, 0);
  for (int y = y0; y < y0 + h; y++)
    for (int x = x0; x < x0 + w; x++)
      putPixel(x, y, bg);

  // If you want actual text without SDL_ttf,
  // easiest is HTML overlay (Option B below).
}
static void showPromptText(const std::string& msg) {
  EM_ASM({
    let text = UTF8ToString($0);

    let el = document.getElementById("gamePrompt");

    if (!el) {
      el = document.createElement("div");
      el.id = "gamePrompt";
      el.style.position = "absolute";
      el.style.left = "50%";
      el.style.bottom = "40px";
      el.style.transform = "translateX(-50%)";
      el.style.padding = "10px 18px";
      el.style.background = "rgba(0,0,0,0.65)";
      el.style.color = "white";
      el.style.fontFamily = "system-ui, sans-serif";
      el.style.fontSize = "18px";
      el.style.borderRadius = "12px";
      el.style.pointerEvents = "none";
      el.style.transition = "opacity 0.15s ease";
      document.body.appendChild(el);
    }

    el.textContent = text;
    el.style.opacity = "1";
  }, msg.c_str());
}

static void hidePromptText() {
  EM_ASM({
    let el = document.getElementById("gamePrompt");
    if (el) el.style.opacity = "0";
  });
}
static void renderPromptIfNeeded() {
  if (gNearestInteractable < 0) {
    hidePromptText();
    return;
  }

  showPromptText(interactables[gNearestInteractable].promptNear);
}










static void render() {

  for (int x = 0; x < SCREEN_W; x++) {
    // Camera space x in [-1, 1]
    double cameraX = 2.0 * x / double(SCREEN_W) - 1.0;
    double rayDirX = dirX + planeX * cameraX;
    double rayDirY = dirY + planeY * cameraX;

    int mapX = int(posX);
    int mapY = int(posY);

    // DDA setup
    double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1.0 / rayDirX);
    double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1.0 / rayDirY);

    int stepX, stepY;
    double sideDistX, sideDistY;

    if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
    else            { stepX =  1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }

    if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
    else            { stepY =  1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

    int hit = 0;
    int side = 0; // 0 = x-side, 1 = y-side

    // DDA loop
    while (!hit) {
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }
      if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H) { hit = 1; break; }
      if (worldMap[mapY][mapX] > 0) hit = 1;
    }

    // Perpendicular distance to wall (prevents fish-eye)
    double perpWallDist;

    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
    else           perpWallDist = (sideDistY - deltaDistY);
    zBuffer[x] = perpWallDist;


    // Calculate wall slice
    int lineHeight = int(SCREEN_H / (perpWallDist + 1e-9));
    int drawStart = -lineHeight / 2 + SCREEN_H / 2;
    int drawEnd = lineHeight / 2 + SCREEN_H / 2;
    if (drawStart < 0) drawStart = 0;
    if (drawEnd >= SCREEN_H) drawEnd = SCREEN_H - 1;

    // Wall hit position for texX
    double wallX;
    if (side == 0) wallX = posY + perpWallDist * rayDirY;
    else           wallX = posX + perpWallDist * rayDirX;
    wallX -= std::floor(wallX);

    int texX = int(wallX * double(TEX_W));
    // Flip depending on side and direction so it doesn't mirror weirdly
    if (side == 0 && rayDirX > 0) texX = TEX_W - texX - 1;
    if (side == 1 && rayDirY < 0) texX = TEX_W - texX - 1;

    // Pick wall texture by map value
    const Texture* wallTex;

    int tile = (mapX >= 0 && mapX < MAP_W && mapY >= 0 && mapY < MAP_H) ? worldMap[mapY][mapX] : 1;
    
    wallTex = wallTextures[tile-1];
    //std::cerr << tile <<std::endl; 
    
    // Draw ceiling and floor using floor casting
    // Determine floor wall position (exact hit point in world)
    double floorXWall, floorYWall;
    if (side == 0 && rayDirX > 0) { floorXWall = mapX;       floorYWall = mapY + wallX; }
    else if (side == 0 && rayDirX < 0) { floorXWall = mapX + 1.0; floorYWall = mapY + wallX; }
    else if (side == 1 && rayDirY > 0) { floorXWall = mapX + wallX; floorYWall = mapY; }
    else { floorXWall = mapX + wallX; floorYWall = mapY + 1.0; }

    double distWall = perpWallDist;
    double distPlayer = 0.0;

    // Draw wall slice
    for (int y = drawStart; y <= drawEnd; y++) {
      int d = (y * 256) - (SCREEN_H * 128) + (lineHeight * 128);
      int texY = ((d * TEX_H) / lineHeight) / 256;

      uint32_t c = sampleTex(*wallTex, texX, texY);

      // very simple side shading
      if (side == 1) {
        uint8_t a = (c >> 24) & 0xFF;
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8)  & 0xFF;
        uint8_t b = (c)       & 0xFF;
        c = packARGB(a, uint8_t(r * 0.75), uint8_t(g * 0.75), uint8_t(b * 0.75));
      }
      putPixel(x, y, c);
    }

    // Floor/Ceiling casting
    for (int y = drawEnd + 1; y < SCREEN_H; y++) {
      // Current distance from player to row
      double currentDist = SCREEN_H / (2.0 * y - SCREEN_H);

      double weight = (currentDist - distPlayer) / (distWall - distPlayer);
      double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
      double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

      int floorTexX = int(currentFloorX * TEX_W) & (TEX_W - 1);
      int floorTexY = int(currentFloorY * TEX_H) & (TEX_H - 1);

      int cellX = int(currentFloorX);
      int cellY = int(currentFloorY);

      // Safety clamp
      if (cellX < 0) cellX = 0;
      if (cellX >= MAP_W) cellX = MAP_W - 1;
      if (cellY < 0) cellY = 0;
      if (cellY >= MAP_H) cellY = MAP_H - 1;

      // Get texture indices
      int floorIndex = floorMap[cellY][cellX];
      int ceilIndex  = ceilingMap[cellY][cellX];

      // Fetch textures
      const Texture& floorTex = *floorTextures[floorIndex];
      const Texture& ceilTex  = *ceilTextures[ceilIndex];

      // Sample
      uint32_t floorCol = sampleTex(floorTex, floorTexX, floorTexY);
      uint32_t ceilCol  = sampleTex(ceilTex,  floorTexX, floorTexY);


      putPixel(x, y, floorCol);
      putPixel(x, SCREEN_H - y, ceilCol);
    }
  }

  // Sort sprites from far to near
  std::sort(sprites.begin(), sprites.end(),
      [](Enemy* a, Enemy* b)
  {
      double dxA = posX - a->x;
      double dyA = posY - a->y;
      double dxB = posX - b->x;
      double dyB = posY - b->y;

      double distA = dxA * dxA + dyA * dyA;
      double distB = dxB * dxB + dyB * dyB;

      return distA > distB;  // FARTHER first
  });
  // main rendering area
  for (Enemy* e : sprites) {
    renderEnemyPlaceholder(*e);
  }

  renderPromptIfNeeded();
  SDL_UpdateTexture(gScreenTex, nullptr, gFrame.data(), SCREEN_W * int(sizeof(uint32_t)));
  SDL_RenderClear(gRenderer);
  SDL_RenderCopy(gRenderer, gScreenTex, nullptr, nullptr);
  SDL_RenderPresent(gRenderer);
  
}
// MENU RENDERING

static void renderMenu() {
  // Clear frame
  std::fill(gFrame.begin(), gFrame.end(), packARGB(255, 0, 0, 0));

  // Draw menu image centered
  if (!texMenu.pixels.empty()) {
    int startX = (SCREEN_W - texMenu.w) / 2;
    int startY = (SCREEN_H - texMenu.h) / 2;

    for (int y = 0; y < texMenu.h; y++) {
      for (int x = 0; x < texMenu.w; x++) {
        uint32_t c = texMenu.pixels[y * texMenu.w + x];
        putPixel(startX + x, startY + y, c);
      }
    }
  }

  SDL_UpdateTexture(gScreenTex, nullptr, gFrame.data(), SCREEN_W * sizeof(uint32_t));
  SDL_RenderClear(gRenderer);
  SDL_RenderCopy(gRenderer, gScreenTex, nullptr, nullptr);
  SDL_RenderPresent(gRenderer);
}

// ==================================================
// UPDATE EVERYTHING AKA the STATE MACHINE

static void update() {
  const Uint8* keys = SDL_GetKeyboardState(nullptr);

  if (gState == GameState::MENU) {
    if (keys[SDL_SCANCODE_RETURN]) {
      
      loadMap(map1);
        
      posX = 12.0;
      posY = 12.0;
      dirX = -1.0;
      dirY =  0.0;
      planeX = 0.0;
      planeY = 0.66;
      enemy0 = { 18.0, 18.0, &texEnemy0, 1 };
      enemy1 = { 1.0, 18.0, &texEnemy1, 1};
      enemy2 = { 18.0, 1.0, &texEnemy2, 1 };

      playMusic(); 
      gState = GameState::PLAYING;


    }
    renderMenu();
    return;
  }

  // ======================
  // PLAYER MOVEMENT

  double moveSpeed = 0.06;
  double rotSpeed  = 0.045;

  if (keys[SDL_SCANCODE_W]) {
    double nx = posX + dirX * moveSpeed;
    double ny = posY + dirY * moveSpeed;
    if (worldMap[int(posY)][int(nx)] == 0) posX = nx;
    if (worldMap[int(ny)][int(posX)] == 0) posY = ny;
  }
  if (keys[SDL_SCANCODE_S]) {
    double nx = posX - dirX * moveSpeed;
    double ny = posY - dirY * moveSpeed;
    if (worldMap[int(posY)][int(nx)] == 0) posX = nx;
    if (worldMap[int(ny)][int(posX)] == 0) posY = ny;
  }

  if (keys[SDL_SCANCODE_A]) {
    double nx = posX - planeX * moveSpeed;
    double ny = posY - planeY * moveSpeed;
    if (worldMap[int(posY)][int(nx)] == 0) posX = nx;
    if (worldMap[int(ny)][int(posX)] == 0) posY = ny;
  }
  if (keys[SDL_SCANCODE_D]) {
    double nx = posX + planeX * moveSpeed;
    double ny = posY + planeY * moveSpeed;
    if (worldMap[int(posY)][int(nx)] == 0) posX = nx;
    if (worldMap[int(ny)][int(posX)] == 0) posY = ny;
  }

  if (keys[SDL_SCANCODE_LEFT]) {
    double oldDirX = dirX;
    dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
    dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);

    double oldPlaneX = planeX;
    planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
    planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
  }
  if (keys[SDL_SCANCODE_RIGHT]) {
    double oldDirX = dirX;
    dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
    dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);

    double oldPlaneX = planeX;
    planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
    planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
  }
  


  //updateEnemy(enemy1);
  //updateEnemy(enemy0);
  //updateEnemy(enemy2);


  updateNearestInteractable();
  tryInteract(keys);

  updateWander(person1);
  updateWander(person2);
  updateWander(person3);
  updateWander(person4);

  render();
  
  
}

// ==================================================
// MAIN LOOP

static void loop() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {}
  update();
}

// ==================================================
// MAIN

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
}


  gWindow = SDL_CreateWindow(
    "Raycaster",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    SCREEN_W,
    SCREEN_H,
    SDL_WINDOW_SHOWN
  );

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);

  gScreenTex = SDL_CreateTexture(
    gRenderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    SCREEN_W,
    SCREEN_H
  );

  // try to load all textures

  bool ok = true;
  ok &= loadBMPTexture("tex/wall0.png", texWall0);
  ok &= loadBMPTexture("tex/wall1.png", texWall1);
  ok &= loadBMPTexture("tex/wall2.png", texWall2);
  ok &= loadBMPTexture("tex/wall3.png", texWall3);
  ok &= loadBMPTexture("tex/wall4.png", texWall4);
  ok &= loadBMPTexture("tex/wall5.png", texWall5);
  ok &= loadBMPTexture("tex/wall6.png", texWall6);
  ok &= loadBMPTexture("tex/wall7.png", texWall7);


  ok &= loadBMPTexture("tex/floor0.png", texFloor0);
  ok &= loadBMPTexture("tex/floor1.png", texFloor1);
  ok &= loadBMPTexture("tex/ceil0.png", texCeil0);
  ok &= loadBMPTexture("tex/E0.png", texEnemy0);
  ok &= loadBMPTexture("tex/E1.png", texEnemy1);
  ok &= loadBMPTexture("tex/menu.png",  texMenu);

  ok &= loadBMPTexture("tex/C0.png",  texC1);
  ok &= loadBMPTexture("tex/C1.png",  texC2);
  ok &= loadBMPTexture("tex/C2.png",  texC3);
  ok &= loadBMPTexture("tex/C3.png",  texC4);

  ok &= loadBMPTexture("tex/T0.png",  texT1);
  ok &= loadBMPTexture("tex/T1.png",  texT2);
  ok &= loadBMPTexture("tex/T2.png",  texT3);
  ok &= loadBMPTexture("tex/T3.png",  texT4);

  // npcs
    ok &= loadBMPTexture("tex/P1.png",  texP1);
  ok &= loadBMPTexture("tex/P11.png",  texP11);
  ok &= loadBMPTexture("tex/P12.png",  texP12);
  ok &= loadBMPTexture("tex/P13.png",  texP13);

    ok &= loadBMPTexture("tex/T0.png",  texP2);
  ok &= loadBMPTexture("tex/T1.png",  texP21);
  ok &= loadBMPTexture("tex/T2.png",  texP22);
  ok &= loadBMPTexture("tex/T3.png",  texP23);

    ok &= loadBMPTexture("tex/T0.png",  texP3);
  ok &= loadBMPTexture("tex/T1.png",  texP31);
  ok &= loadBMPTexture("tex/T2.png",  texP32);
  ok &= loadBMPTexture("tex/T3.png",  texP33);

    ok &= loadBMPTexture("tex/T0.png",  texP4);
  ok &= loadBMPTexture("tex/T1.png",  texP41);
  ok &= loadBMPTexture("tex/T2.png",  texP42);
  ok &= loadBMPTexture("tex/T3.png",  texP43);




  if (!ok) {
    std::cerr << "Texture load failure\n";
  }

  floorTextures.push_back(&texFloor0);
  floorTextures.push_back(&texFloor1);
  
  ceilTextures.push_back(&texCeil0);

  wallTextures.push_back(&texWall0);
  wallTextures.push_back(&texWall1);
  wallTextures.push_back(&texWall2);
  wallTextures.push_back(&texWall3);
  wallTextures.push_back(&texWall4);
  wallTextures.push_back(&texWall5);
  wallTextures.push_back(&texWall6);
  wallTextures.push_back(&texWall7);
  


  sprites = {
    &enemy0,
    &enemy1,
    &enemy2,
    &chair,
    &chair1,
    &chair2,
    &chair3,
    &chair4,
    &chair5,
    &chair6,
    &chair7,
    &chair8,
    &table,
    &table1,
    &table2,
    &table3,
    // Top left classroom
&tl_table1, &tl_chair1,
&tl_table2, &tl_chair2,
&tl_table3, &tl_chair3,
&tl_table4, &tl_chair4,

// Top right classroom
&tr_table1, &tr_chair1,
&tr_table2, &tr_chair2,
&tr_table3, &tr_chair3,
&tr_table4, &tr_chair4,


    &person1,
     &person2,
      &person3,
       &person4,

  };


  initInteractables();










  emscripten_set_main_loop(loop, 0, true);
  EM_ASM({
  function resizeCanvas() {
    let canvas = Module.canvas;

    let screenW = window.innerWidth;
    let screenH = window.innerHeight;

    let size = Math.min(screenW, screenH); // keep square

    canvas.style.width  = size + "px";
    canvas.style.height = size + "px";

    canvas.style.position = "absolute";
    canvas.style.left = ((screenW - size) / 2) + "px";
    canvas.style.top  = ((screenH - size) / 2) + "px";
  }

  window.addEventListener("resize", resizeCanvas);
  resizeCanvas();
  });
  return 0;
}
