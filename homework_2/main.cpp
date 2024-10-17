#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
    #include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 2,
              WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.1765625f,
                BG_GREEN   = 0.17265625f,
                BG_BLUE    = 0.1609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr glm::vec3 SCENE_SCALE = glm::vec3(12.0f, 12.0f, 0.0f);
constexpr glm::vec3 SCENE_LOCATION = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr glm::vec3 MESSAGE_SCALE = glm::vec3(10.0f, 10.0f, 0.0f);
constexpr glm::vec3 MESSAGE_LOCATION = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr glm::vec3 TOP_WALL_SCALE = glm::vec3(12.0f, 1.0f, 0.0f);
constexpr glm::vec3 TOP_WALL_LOCATION = glm::vec3(0.0f, 3.9f, 0.0f);

constexpr glm::vec3 BOTTOM_WALL_SCALE = glm::vec3(12.0f, 1.0f, 0.0f);
constexpr glm::vec3 BOTTOM_WALL_LOCATION = glm::vec3(0.0f, -3.9f, 0.0f);

constexpr glm::vec3 LEFT_WALL_SCALE = glm::vec3(1.0f, 8.0f, 0.0f);
constexpr glm::vec3 LEFT_WALL_LOCATION = glm::vec3(-6.0f, 0.0f, 0.0f);

constexpr glm::vec3 RIGHT_WALL_SCALE = glm::vec3(1.0f, 8.0f, 0.0f);
constexpr glm::vec3 RIGHT_WALL_LOCATION = glm::vec3(6.0f, 0.0f, 0.0f);

constexpr glm::vec3 LEFT_PADDLE_SCALE = glm::vec3(0.25f, 1.0f, 0.0f);
constexpr glm::vec3 LEFT_PADDLE_LOCATION = glm::vec3(-4.8f, 0.0f, 0.0f);

constexpr glm::vec3 RIGHT_PADDLE_SCALE = glm::vec3(0.25f, 1.0f, 0.0f);
constexpr glm::vec3 RIGHT_PADDLE_LOCATION = glm::vec3(4.8f, 0.0f, 0.0f);

constexpr glm::vec3 BALL_SCALE = glm::vec3(0.75f, 0.75f, 0.0f);
constexpr glm::vec3 BALL_LOCATION = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr glm::vec3 PADDLE_SPEED = glm::vec3(0.0f, 2.0f, 0.0f);
constexpr glm::vec3 BALL_SPEED = glm::vec3(3.0f, 0.50f, 0.0f);

constexpr GLint NUMBER_OF_TEXTURES = 1,         // idk
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;

// ————— STRUCTS AND ENUMS —————//
enum AppStatus  { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR     };

struct GameState {  Entity* scene;
                    Entity* message;
                    Entity* top_wall;
                    Entity* bottom_wall;
                    Entity* left_wall;
                    Entity* right_wall;
                    Entity* left_paddle;
                    Entity* right_paddle;
                    Entity* ball1;
                    Entity* ball2;
                    Entity* ball3;
};

// ————— VARIABLES ————— //
GameState g_game_state;
std::vector<Entity*> ball_collidables;
std::vector<Entity*> paddle_collidables;

bool left_right;    // 0 = ball goes left to start, 1 = goes right
bool down_up;       // 0 = ball goes down to start, 1 = goes up

bool start = true;
bool pause = true;
glm::vec3 preserve_ball1_movement, preserve_ball2_movement, preserve_ball3_movement;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
Uint32 timeout = SDL_GetTicks();

bool single_player = false;
bool game_over = false;

void initialize();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath, FilterType filterType)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components,
                                     STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
                 GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Disco Pong",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— GENERATE OBJECTS ————— //
    
    std::vector<GLuint> message_textures_ids = {
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/start_screen.png", LINEAR),
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/left_win.png", LINEAR),
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/right_win.png", LINEAR)
    };
    
    std::vector<GLuint> scene_textures_ids = {
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/disco_floor_1.png", LINEAR),
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/disco_floor_2.png", LINEAR)
    };
    
    std::vector<GLuint> box_textures_ids = {
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/box.png", NEAREST)
    };
    
    std::vector<GLuint> ball_textures_ids = {
        load_texture("/Users/Sage/Downloads/Game Programming/disco_pong/homework_2/assets/ball.png", NEAREST)
    };
    
    
    std::vector<std::vector<int>> entity_animations = { {0}, {0}, {0} };
    
    g_game_state.message = new Entity(
        message_textures_ids,  // a list of texture IDs
        glm::vec3(0.0f),     // translation speed
        entity_animations,   // list of animation frames for each type of animation
        0.0f,                // animation time
        1,                   // number of frames for animation
        0,                   // current frame index
        1,                   // current animation col amount
        1,                   // current animation row amount
        SPRITE1              // current animation
    );

    g_game_state.scene = new Entity(
        scene_textures_ids,  // a list of texture IDs
        glm::vec3(0.0f),     // translation speed
        entity_animations,   // list of animation frames for each type of animation
        0.0f,                // animation time
        1,                   // number of frames for animation
        0,                   // current frame index
        1,                   // current animation col amount
        1,                   // current animation row amount
        SPRITE1              // current animation
    );
    
    g_game_state.top_wall = new Entity(
        box_textures_ids,      // a list of texture IDs
        glm::vec3(0.0f),        // translation speed
        entity_animations,      // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.bottom_wall = new Entity(
        box_textures_ids,      // a list of texture IDs
        glm::vec3(0.0f),        // translation speed
        entity_animations,      // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.left_wall = new Entity(
        box_textures_ids,      // a list of texture IDs
        glm::vec3(0.0f),        // translation speed
        entity_animations,      // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.right_wall = new Entity(
        box_textures_ids,      // a list of texture IDs
        glm::vec3(0.0f),        // translation speed
        entity_animations,      // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.left_paddle = new Entity(
        box_textures_ids,            // a list of texture IDs
        glm::vec3(0.0f, 1.0f, 0.0f),    // translation speed
        entity_animations,              // list of animation frames for each type of animation
        0.0f,                        // animation time
        1,                           // number of frames
        0,                           // current frame index
        1,                           // current animation col amount
        1,                           // current animation row amount
        SPRITE1                      // current animation
    );
    
    g_game_state.right_paddle = new Entity(
        box_textures_ids,            // a list of texture IDs
        glm::vec3(0.0f, 1.0f, 0.0f),    // translation speed
        entity_animations,              // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.ball1 = new Entity(
        ball_textures_ids,              // a list of texture IDs
        glm::vec3(1.0f, 0.25f, 0.0f),   // translation speed
        entity_animations,              // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.ball2 = new Entity(
        ball_textures_ids,              // a list of texture IDs
        glm::vec3(1.0f, 0.25f, 0.0f),   // translation speed
        entity_animations,              // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    g_game_state.ball3 = new Entity(
        ball_textures_ids,              // a list of texture IDs
        glm::vec3(1.0f, 0.25f, 0.0f),   // translation speed
        entity_animations,              // list of animation frames for each type of animation
        0.0f,                   // animation time
        1,                      // number of frames
        0,                      // current frame index
        1,                      // current animation col amount
        1,                      // current animation row amount
        SPRITE1                 // current animation
    );
    
    ball_collidables.push_back(g_game_state.top_wall);
    ball_collidables.push_back(g_game_state.bottom_wall);
    ball_collidables.push_back(g_game_state.left_paddle);
    ball_collidables.push_back(g_game_state.right_paddle);
    ball_collidables.push_back(g_game_state.left_wall);
    ball_collidables.push_back(g_game_state.right_wall);
    
    paddle_collidables.push_back(g_game_state.top_wall);
    paddle_collidables.push_back(g_game_state.bottom_wall);
    
    g_game_state.message->set_position(MESSAGE_LOCATION);
    g_game_state.message->set_scale(MESSAGE_SCALE);
    g_game_state.message->set_visibility(true);
    
    g_game_state.scene->set_position(SCENE_LOCATION);
    g_game_state.scene->set_scale(SCENE_SCALE);
    
    g_game_state.top_wall->set_position(TOP_WALL_LOCATION);
    g_game_state.top_wall->set_scale(TOP_WALL_SCALE);
    g_game_state.top_wall->set_shape(TOP_WALL);
    
    g_game_state.bottom_wall->set_position(BOTTOM_WALL_LOCATION);
    g_game_state.bottom_wall->set_scale(BOTTOM_WALL_SCALE);
    g_game_state.bottom_wall->set_shape(BOTTOM_WALL);
    
    g_game_state.left_wall->set_position(LEFT_WALL_LOCATION);
    g_game_state.left_wall->set_scale(LEFT_WALL_SCALE);
    g_game_state.left_wall->set_shape(SIDE_WALL);
    
    g_game_state.right_wall->set_position(RIGHT_WALL_LOCATION);
    g_game_state.right_wall->set_scale(RIGHT_WALL_SCALE);
    g_game_state.right_wall->set_shape(SIDE_WALL);
    
    g_game_state.left_paddle->set_position(LEFT_PADDLE_LOCATION);
    g_game_state.left_paddle->set_scale(LEFT_PADDLE_SCALE);
    g_game_state.left_paddle->set_shape(LEFT_PADDLE);
    g_game_state.left_paddle->set_speed(PADDLE_SPEED);
    
    g_game_state.right_paddle->set_position(RIGHT_PADDLE_LOCATION);
    g_game_state.right_paddle->set_scale(RIGHT_PADDLE_SCALE);
    g_game_state.right_paddle->set_shape(RIGHT_PADDLE);
    g_game_state.right_paddle->set_speed(PADDLE_SPEED);
    
    g_game_state.ball1->set_position(BALL_LOCATION);
    g_game_state.ball1->set_scale(BALL_SCALE);
    g_game_state.ball1->set_shape(BALL);
    g_game_state.ball1->set_rotation(0.5f);
    g_game_state.ball1->set_speed(BALL_SPEED);
    
    g_game_state.ball2->set_position(BALL_LOCATION);
    g_game_state.ball2->set_scale(BALL_SCALE);
    g_game_state.ball2->set_shape(BALL);
    g_game_state.ball2->set_rotation(-0.75f);
    g_game_state.ball2->set_visibility(false);
    g_game_state.ball2->set_speed(BALL_SPEED);
    
    g_game_state.ball3->set_position(BALL_LOCATION);
    g_game_state.ball3->set_scale(BALL_SCALE);
    g_game_state.ball3->set_shape(BALL);
    g_game_state.ball3->set_rotation(0.35f);
    g_game_state.ball3->set_visibility(false);
    g_game_state.ball3->set_speed(BALL_SPEED);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q: g_app_status = TERMINATED;
                        break;
                    
                    case SDLK_t: {
                        single_player = !single_player;
                        if (single_player) {
                            g_game_state.left_paddle->set_can_move(false);
                        }
                        break;
                    }
                        
                    case SDLK_SPACE: {
                        if (start) {
                            g_game_state.message->set_visibility(false);  // hide start screen
                            float left_right = static_cast<float>((rand() % 2) == 0 ? -1.0 : 1.0);
                            float down_up = static_cast<float>((rand() % 2) == 0 ? -1.0 : 1.0);
                            g_game_state.ball1->set_movement(glm::vec3(left_right, down_up, 0.0f));
                            start = false;  // already started, can't start again until next game
                        }
                        
                        else if (pause) {
                            // preserve speed for unpause
                            preserve_ball1_movement = g_game_state.ball1->get_movement();
                            preserve_ball2_movement = g_game_state.ball2->get_movement();
                            preserve_ball3_movement = g_game_state.ball3->get_movement();
                            
                            // halt paddles and ball
                            g_game_state.ball1->set_movement(glm::vec3(0.0f));
                            g_game_state.ball2->set_movement(glm::vec3(0.0f));
                            g_game_state.ball3->set_movement(glm::vec3(0.0f));
                            g_game_state.left_paddle->set_can_move(false);
                            g_game_state.right_paddle->set_can_move(false);
                            pause = false;  // already paused, can't pause again until resumed
                        }
                        
                        else {  // resume
                            g_game_state.ball1->set_movement(preserve_ball1_movement);
                            g_game_state.ball2->set_movement(preserve_ball2_movement);
                            g_game_state.ball3->set_movement(preserve_ball3_movement);
                            g_game_state.left_paddle->set_can_move(true);
                            g_game_state.right_paddle->set_can_move(true);
                            pause = true;   // next time we hit space we can pause again
                        }
                            
                        break;
                    }
                        
                    case SDLK_1: {
                        g_game_state.ball2->set_movement(glm::vec3(0.0f));
                        g_game_state.ball3->set_movement(glm::vec3(0.0f));
                        g_game_state.ball2->set_visibility(false);
                        g_game_state.ball3->set_visibility(false);
                        break;
                    }
                    
                    case SDLK_2: {
                        if (!g_game_state.ball2->get_visibility()) {
                            g_game_state.ball2->set_movement(-1.5f * g_game_state.ball1->get_movement());
                            g_game_state.ball2->set_visibility(true);
                        }
                        g_game_state.ball3->set_movement(glm::vec3(0.0f));
                        g_game_state.ball3->set_visibility(false);
                        break;
                    }
                        
                    case SDLK_3: {
                        if (!g_game_state.ball2->get_visibility()) {
                            g_game_state.ball2->set_movement(-1.5f * g_game_state.ball1->get_movement());
                            g_game_state.ball2->set_visibility(true);
                        }
                
                        if (!g_game_state.ball3->get_visibility()) {
                            g_game_state.ball3->set_movement(glm::vec3(-0.9f * g_game_state.ball1->get_movement()[0],
                                                                       1.25f * g_game_state.ball1->get_movement()[1],
                                                                       0.0f));
                            g_game_state.ball3->set_visibility(true);
                        }
                        break;
                    }
            
                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_W] && g_game_state.left_paddle->get_can_move())
        g_game_state.left_paddle->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
    else if (key_state[SDL_SCANCODE_S] && g_game_state.left_paddle->get_can_move())
        g_game_state.left_paddle->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
    else
        g_game_state.left_paddle->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
    
    if (key_state[SDL_SCANCODE_UP] && g_game_state.right_paddle->get_can_move())
        g_game_state.right_paddle->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
    else if (key_state[SDL_SCANCODE_DOWN] && g_game_state.right_paddle->get_can_move())
        g_game_state.right_paddle->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
    else
        g_game_state.right_paddle->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));

    if (glm::length(g_game_state.scene->get_movement()) > 1.0f)
        g_game_state.scene->normalise_movement();
}


void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    if (SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) {
        if (g_game_state.scene->get_animation() == SPRITE1) {
            g_game_state.scene->set_animation_state(SPRITE2);
        }
        else {
            g_game_state.scene->set_animation_state(SPRITE1);
        }
        timeout = SDL_GetTicks() + 750;
    }
    
    if (single_player) {
        if (g_game_state.ball1->get_position()[1] > g_game_state.left_paddle->get_position()[1]) {
            g_game_state.left_paddle->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            g_game_state.left_paddle->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
        }
//        g_game_state.left_paddle->set_movement(glm::vec3(0.0f, g_game_state.left_paddle->get_movement()[1], 0.0f));
    }
    
    // see if anyone's lost since we last checked
    if (g_game_state.left_wall->get_loser() && !game_over) {
        g_game_state.message->set_animation_state(SPRITE3);
        g_game_state.message->set_visibility(true);
        game_over = true;
    }
    
    if (g_game_state.right_wall->get_loser() && !game_over) {
        g_game_state.message->set_animation_state(SPRITE2);
        g_game_state.message->set_visibility(true);
        game_over = true;
    }
        
    g_game_state.scene->update(delta_time);
    g_game_state.top_wall->update(delta_time);
    g_game_state.bottom_wall->update(delta_time);
    g_game_state.left_paddle->update(delta_time, paddle_collidables, paddle_collidables.size());
    g_game_state.right_paddle->update(delta_time, paddle_collidables, paddle_collidables.size());
    g_game_state.ball1->update(delta_time, ball_collidables, ball_collidables.size());
    g_game_state.ball2->update(delta_time, ball_collidables, ball_collidables.size());
    g_game_state.ball3->update(delta_time, ball_collidables, ball_collidables.size());
    g_game_state.left_wall->update(delta_time);
    g_game_state.right_wall->update(delta_time);
    g_game_state.message->update(delta_time);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.scene->render(&g_shader_program);
    g_game_state.top_wall->render(&g_shader_program);
    g_game_state.bottom_wall->render(&g_shader_program);
    g_game_state.left_paddle->render(&g_shader_program);
    g_game_state.right_paddle->render(&g_shader_program);
    g_game_state.ball1->render(&g_shader_program);
    g_game_state.ball2->render(&g_shader_program);
    g_game_state.ball3->render(&g_shader_program);
    g_game_state.left_wall->render(&g_shader_program);
    g_game_state.right_wall->render(&g_shader_program);
    g_game_state.message->render(&g_shader_program);

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();
    delete   g_game_state.scene;
    delete   g_game_state.message;
    delete   g_game_state.top_wall;
    delete   g_game_state.bottom_wall;
    delete   g_game_state.left_wall;
    delete   g_game_state.right_wall;
    delete   g_game_state.left_paddle;
    delete   g_game_state.right_paddle;
    delete   g_game_state.ball1;
    delete   g_game_state.ball2;
    delete   g_game_state.ball3;
}


int main(int argc, char* argv[])
{
    initialize();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
