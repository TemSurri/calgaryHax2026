#include <SDL.h>
#include <emscripten.h>
#include <cmath>
#include <iostream>


SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
int height = 640;
int width  = 640;


const int MAP_H = 8;
const int MAP_W = 7;

const int UNIT_H = height / MAP_H;
const int UNIT_W = width / MAP_W;

int map[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,0,0,1,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,1,1,1,1,1,1},
};
void render3D_wall_line(float len, int i, int fov, float angle, int x, int y, char win) {
    float line_w = 1.0f;
    float vertical_bench = (height /2.0f);

    float length =  30000.0f /abs(len);
    float extrude = (length/2.0f);

    float shade_factor = 1.0f/abs(len/75);
    if (shade_factor >=1){
        shade_factor = 1;
    }

        
    SDL_SetRenderDrawColor(renderer, 200*shade_factor, 0, 0, 255);

    //SDL_Rect rect {
    //                (i*line_w),
    //                (vertical_bench-extrude),
    //                (line_w),
    //                (length)
    //            };
    //SDL_RenderFillRect(renderer, &rect);
    //std::cerr<<extrude<<std::endl;
    //

    SDL_RenderDrawLine(
        renderer,
        i*line_w,
        vertical_bench-extrude,
        i*line_w,
        vertical_bench+extrude
        );
        
    };

void render2D_map() {
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    for (int i = 0; i < MAP_H; i++) {
        SDL_RenderDrawLine(
            renderer,
            0,
            i * (height / MAP_H),
            width,
            i * (height / MAP_H)
        );
    }

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    for (int i = 0; i < MAP_W; i++) {
        SDL_RenderDrawLine(
            renderer,
            i * (width / MAP_W),
            0,
            i * (width / MAP_W),
            height
        );
    }

    for (int r = 0; r < MAP_H; r++) {
        for (int c = 0; c < MAP_W; c++) {
            if (map[r][c] == 0) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_Rect rect {
                    c * (width / MAP_W),
                    r * (height / MAP_H),
                    (width / MAP_W),
                    (height / MAP_H)
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

namespace Player {
    int render_ray(float angle);

    struct Player {
        int x;
        int y;
        float angle;
        int speed;

        int horizontal_collision_check() {
            int padding{};
            int potential_wall = (x/UNIT_W);
            potential_wall += 1;

            if (cos(angle) < 0){
                potential_wall -= 2;
                padding = UNIT_W;
            } 

            int row = y/UNIT_H;
            int col = potential_wall;

            //std::cerr<<row<<". "<<col<<std::endl;
            
            if (((map[row][col]) == 1) && (abs(x-((potential_wall*UNIT_W)+padding))<15)){
                return 1;
            }

            return 0;
        };

        int vertical_collision_check() {
            int padding{};
            int potential_wall = (y/UNIT_H);
            potential_wall += 1;

            if (sin(angle) < 0){
                potential_wall -= 2;
                padding = UNIT_H;
            } 

            int col = x/UNIT_W;
            int row = potential_wall;

            //std::cerr<<row<<". "<<col<<std::endl;
            
            if (((map[row][col]) == 1) && (abs(y-((potential_wall*UNIT_H)+padding))<15)){
                return 1;
            }

            return 0;
        };

        void move_forward(){

            if (horizontal_collision_check() == 0){
                float dx = cos(angle);
                x += dx * speed;
            }; 

            if (vertical_collision_check() == 0){
                float dy = sin(angle);
                y += dy * speed;

            }
        }
    };

    Player player { 250, 250, -0.9f, 3};

    void render_player() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
        SDL_Rect r {
            player.x,
            player.y,
            10,
            10
        };
        SDL_RenderFillRect(renderer, &r);
    }
    // checks vertical grid intersections only
    int vertical_wall_check(int x, float angle) {

        float y = x * tan(angle);
        int row = (player.y + y) / UNIT_H;
        int col = (player.x + x) / UNIT_W;

        if (cos(angle) < 0) {
            col -= 1;
        }

        if ((row > 7 || row < 0) || (col > 7 || col < 0)){
            return 1;
        };

        return map[row][col];
    }

    int len_ray_vertical(float angle) {

        int dir = 1;
        if (cos(angle) < 0) {
            dir = -1;
        }

        int horizontal_displacement = 0;
        bool clear = true;

        while (clear) {
            // only check when perfectly aligned to grid
            if ((player.x + horizontal_displacement) % UNIT_W == 0) {
                int hit = vertical_wall_check(horizontal_displacement, angle);
                if (hit == 1) {
                    clear = false;
                    break;
                }
                if (hit == 0) {
                    horizontal_displacement+= UNIT_W*dir;
                    continue;
                }
            }
            horizontal_displacement += dir;
        }

        int len = horizontal_displacement / cos(angle);
        return len;
    }

    int horizontal_wall_check(int y, float angle){
        float x = y/(tan(angle));

        int row = (player.y + y) / UNIT_H;
        int col = (player.x + x) / UNIT_W;

        if (sin(angle) < 0) {
            row -= 1;
        }

        if ((row > 7 || row < 0) || (col > 7 || col < 0)){
            return 1;
        };

        return map[row][col];
    };

    int len_ray_horizontal(float angle) {
        int dir = 1;
        if (sin(angle) < 0) {
            dir = -1;
        }

        int vertical_displacement = 0;
        bool clear = true;
        while (clear) {
            if ((player.y + vertical_displacement) % UNIT_H == 0) {
                int check = horizontal_wall_check(vertical_displacement, angle);
                if (check == 1) {
                    clear = false;
                    break;
                } else if (check == 0) {
                    vertical_displacement += dir*UNIT_H;
                    continue;
                }
            }
            
            vertical_displacement += dir;
        }

        int len = vertical_displacement / sin(angle);

        return len;
    }

    int render_ray(float angle){
        int len1 = len_ray_horizontal(angle);
        int len2 = len_ray_vertical(angle);

        //std::cerr<<len1<<"  "<<len2<<std::endl;
        int len{};
        if (len1 <= len2){
            len = len1;
        } else if (len1>len2){
            len = len2;
        };

        SDL_SetRenderDrawColor(renderer, 0,200,200, 255);
        SDL_RenderDrawLine(
            renderer,
            player.x,
            player.y,
            player.x + cos(angle) * len,
            player.y + sin(angle) * len
        );

        return len;
    }

    void render_fov() {
        float angle_increment = 0.00156465639;
        int total_fov = 640;
        
        float angle_offset = player.angle - ((total_fov/2) * angle_increment);

        for (int i{}; i < (total_fov); i++){
            float ray_angle = angle_offset + i * angle_increment;
            
            int len1 = len_ray_horizontal(ray_angle);
            int len2 = len_ray_vertical(ray_angle);

            //std::cerr<<len1<<"  "<<len2<<std::endl;
            char win;
            int raw_len{};
            if (len1 <= len2){
                raw_len = len1;
                win = 'h';
            } else if (len1>len2){
                raw_len = len2;
                win = 'v';
            };

            float corrected = raw_len * cos(ray_angle - player.angle);
            render3D_wall_line(corrected, i, total_fov, player.angle, player.x, player.y, win);
        }
    }
}    

void loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}

    SDL_SetRenderDrawColor(renderer, 140, 0, 0, 255);
    SDL_RenderClear(renderer);

    render2D_map();
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    using Player::player;

   

    if (keys[SDL_SCANCODE_W]) {
        player.move_forward();
    }
    if (keys[SDL_SCANCODE_A]) {
        player.angle -= 0.05f;
    }
    if (keys[SDL_SCANCODE_D]) {
        player.angle += 0.05f;
    }

    
    Player::render_ray(player.angle);
    Player::render_player();
    Player::render_fov();

    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Raycast Prototype",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE
    );
    emscripten_set_main_loop(loop, 0, true);
    return 0;
}
