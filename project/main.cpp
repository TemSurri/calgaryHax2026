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

static constexpr int SCREEN_W = 640;
static constexpr int SCREEN_H = 640;

static constexpr int TEX_W = 64;
static constexpr int TEX_H = 64;

static constexpr int MAP_W = 24;
static constexpr int MAP_H = 24;
static std::vector<double> zBuffer(SCREEN_W);

// MAP 

static int worldMap[MAP_H][MAP_W] = {
  {1,4,4,4,4,4,1,2,4,4,4,2,1,2,1,2,1,2,1,2,4,4,4,1},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {1,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
};

static int floorMap[MAP_H][MAP_W] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static int ceilingMap[MAP_H][MAP_W] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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

static std::vector<Texture*> floorTextures;
static std::vector<Texture*> ceilTextures;
static std::vector<Texture*> wallTextures;


static double posX = 12.0;
static double posY = 12.0;

static double dirX = -1.0;
static double dirY =  0.0;

static double planeX = 0.0;
static double planeY = 0.66; 

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
}







  static void update() {
  const Uint8* keys = SDL_GetKeyboardState(nullptr);
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
  render();
  
}

static void loop() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {}
  update();
}

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

  // try fo load

  bool ok = true;
  ok &= loadBMPTexture("tex/wall0.png", texWall0);

  if (!ok) {
    std::cerr << "Texture load failure\n";
  }


  wallTextures.push_back(&texWall0);
  
  emscripten_set_main_loop(loop, 0, true);
  EM_ASM({
  function resizeCanvas() {
    let canvas = Module.canvas;

    let screenW = window.innerWidth;
    let screenH = window.innerHeight;

    let size = Math.min(screenW, screenH); // keep it as square for now

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
