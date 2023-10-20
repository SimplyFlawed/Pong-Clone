/**
* Author: Raymond Lin
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include <Windows.h>
#include <iostream>
#include <sstream>

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
#include <cmath>

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.9608f,
BG_BLUE = 0.9608f,
BG_GREEN = 0.9608f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// ———————————————— TEXTURES ———————————————— //
const char BG_SPRITE[]          = "assets/gmail_background.png";
const char BAR_SPRITE[]         = "assets/keyboard.png";
const char BALL_SPRITE[]        = "assets/gmail_icon.png";
const char BOUNDARY_SPRITE[]    = "assets/border.png";
const char PLAYER1_WIN_SPRITE[] = "assets/player1win.png";
const char PLAYER2_WIN_SPRITE[] = "assets/player2win.png";

// ———————————————— INITIAL VALUES ———————————————— //
const glm::vec3 BG_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                BG_INIT_SCA = glm::vec3(10.0f, 7.5f, 0.0f);

const glm::vec3 BAR1_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                BAR2_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                BAR_INIT_SCA  = glm::vec3(0.75f, 2.0f, 0.0f);

const glm::vec3 BALL_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                BALL_INIT_SCA = glm::vec3(0.5f, 0.5f, 0.0f);

const glm::vec3 TOP_INIT_POS        = glm::vec3(0.0f, 19.0f, 0.0f),
                BOTTOM_INIT_POS     = glm::vec3(0.0f, -19.0f, 0.0f),
                TOP_BOTTOM_INIT_SCA = glm::vec3(10.0f, 0.2f, 0.0f),
                LEFT_INIT_POS       = glm::vec3(-25.0f, 0.0f, 0.0f),
                RIGHT_INIT_POS      = glm::vec3(25.0f, 0.0f, 0.0f),
                LEFT_RIGHT_INIT_SCA = glm::vec3(0.2f, 8.0f, 0.0f);

const glm::vec3 WIN_HIDDEN_POS = glm::vec3(10.0f, 10.0f, 0.0f),
                WIN_SHOW_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                WIN_INIT_SCA = glm::vec3(6.0f, 2.0f, 0.0f);



const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0,
            TEXTURE_BORDER = 0;

const float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;

GLuint g_bg_texture_id,
       g_bar_texture_id,
       g_ball_texture_id,
       g_boundary_texture_id,
       g_player1_win_texture_id,
       g_player2_win_texture_id;

glm::mat4 g_view_matrix,
          g_bg_model_matrix,
          g_bar1_model_matrix,
          g_bar2_model_matrix,
          g_ball_model_matrix,
          g_projection_matrix;

glm::mat4 g_top,
          g_bottom,
          g_left,
          g_right;

glm::mat4 g_player1_win_matrix,
          g_player2_win_matrix;

// ———————————————— CONSTANTS ———————————————— //
const float g_bar_speed = 2.0f;

// ———————————————— GLOBAL VARIABLES ———————————————— //
bool g_is_multiplayer = true,
     g_cpu_direction  = true,
     g_player1_win    = false,
     g_player2_win    = false;

float g_previous_ticks = 0.0f;
float g_ball_speed = 2.0f;

glm::vec3 g_bar1_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_bar1_position = glm::vec3(-5.0f, 0.0f, 0.0f),
          g_bar2_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_bar2_position = glm::vec3(5.0f, 0.0f, 0.0f),
          g_ball_movement = glm::vec3(-1.0f, 1.0f, 0.0f),
          g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}

bool is_collision(glm::vec3 obj_pos, glm::vec3 obj_scale, glm::vec3 other_pos, glm::vec3 other_scale) {
    float x_distance = fabs(obj_pos.x * obj_scale.x - other_pos.x * other_scale.x) - ((obj_scale.x + other_scale.x) / 2.0f);
    float y_distance = fabs(obj_pos.y * obj_scale.y - other_pos.y * other_scale.y) - ((obj_scale.y + other_scale.y) / 2.0f);

    if (x_distance < 0.0f && y_distance < 0.0f)
    {
        return true;
    }
    return false;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Clone: Email Wars",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    // ———————————————— BACKGROUND ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_bg_model_matrix = glm::mat4(1.0f);
    g_bg_model_matrix = glm::translate(g_bg_model_matrix, BG_INIT_POS);
    g_bg_model_matrix = glm::scale(g_bg_model_matrix, BG_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_bg_texture_id = load_texture(BG_SPRITE);

    // ———————————————— BOUNDARIES ———————————————— //
    g_top = glm::mat4(1.0f);
    g_top = glm::scale(g_top, TOP_BOTTOM_INIT_SCA);
    g_top = glm::translate(g_top, TOP_INIT_POS);

    g_bottom = glm::mat4(1.0f);
    g_bottom = glm::scale(g_bottom, TOP_BOTTOM_INIT_SCA);
    g_bottom = glm::translate(g_bottom, BOTTOM_INIT_POS);

    g_left = glm::mat4(1.0f);
    g_left = glm::scale(g_left, LEFT_RIGHT_INIT_SCA);
    g_left = glm::translate(g_left, LEFT_INIT_POS);

    g_right = glm::mat4(1.0f);
    g_right = glm::scale(g_right, LEFT_RIGHT_INIT_SCA);
    g_right = glm::translate(g_right, RIGHT_INIT_POS);

    g_boundary_texture_id = load_texture(BOUNDARY_SPRITE);

    // ———————————————— BARS ———————————————— //

    g_bar1_model_matrix = glm::mat4(1.0f);
    g_bar1_model_matrix = glm::scale(g_bar1_model_matrix, BAR_INIT_SCA);
    g_bar1_model_matrix = glm::translate(g_bar1_model_matrix, BAR1_INIT_POS);

    g_bar2_model_matrix = glm::mat4(1.0f);
    g_bar2_model_matrix = glm::scale(g_bar2_model_matrix, BAR_INIT_SCA);
    g_bar2_model_matrix = glm::translate(g_bar2_model_matrix, BAR2_INIT_POS);

    g_bar_texture_id = load_texture(BAR_SPRITE);

    // ———————————————— BALL ———————————————— //
    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::scale(g_ball_model_matrix, BALL_INIT_SCA);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);

    g_ball_texture_id = load_texture(BALL_SPRITE);

    // ———————————————— WIN MESSEGES ———————————————— //
    g_player1_win_matrix = glm::mat4(1.0f);
    g_player1_win_matrix = scale(g_player1_win_matrix, WIN_INIT_SCA);
    g_player1_win_matrix = translate(g_player1_win_matrix, WIN_HIDDEN_POS);

    g_player2_win_matrix = glm::mat4(1.0f);
    g_player2_win_matrix = scale(g_player2_win_matrix, WIN_INIT_SCA);
    g_player2_win_matrix = translate(g_player2_win_matrix, WIN_HIDDEN_POS);

    g_player1_win_texture_id = load_texture(PLAYER1_WIN_SPRITE);
    g_player2_win_texture_id = load_texture(PLAYER2_WIN_SPRITE);

    // ———————————————— GENERAL ———————————————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = !g_game_is_running;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
                g_game_is_running = !g_game_is_running;
                break;

            case SDLK_t:
                g_is_multiplayer = !g_is_multiplayer;
                break;

            default: break;
            }
        }
    }
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W] && !is_collision(g_bar1_position, BAR_INIT_SCA, TOP_INIT_POS, TOP_BOTTOM_INIT_SCA))
    {
        g_bar1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && !is_collision(g_bar1_position, BAR_INIT_SCA, BOTTOM_INIT_POS, TOP_BOTTOM_INIT_SCA))
    {
        g_bar1_movement.y = -1.0f;
    }

    if (g_is_multiplayer)
    {
        if (key_state[SDL_SCANCODE_I] && !is_collision(g_bar2_position, BAR_INIT_SCA, TOP_INIT_POS, TOP_BOTTOM_INIT_SCA))
        {
            g_bar2_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_K] && !is_collision(g_bar2_position, BAR_INIT_SCA, BOTTOM_INIT_POS, TOP_BOTTOM_INIT_SCA))
        {
            g_bar2_movement.y = -1.0f;
        }
    }
    else
    {
        g_bar2_movement.y = (g_cpu_direction) ? 1.0f : -1.0f;
        if (is_collision(g_bar2_position, BAR_INIT_SCA, TOP_INIT_POS, TOP_BOTTOM_INIT_SCA))
        {
            g_cpu_direction = false;
        }
        else if (is_collision(g_bar2_position, BAR_INIT_SCA, BOTTOM_INIT_POS, TOP_BOTTOM_INIT_SCA))
        {
            g_cpu_direction = true;
        }
    }

    if (glm::length(g_bar1_movement) > 1.0f)
    {
        g_bar1_movement = glm::normalize(g_bar1_movement);
    }

    if (glm::length(g_bar2_movement) > 1.0f)
    {
        g_bar2_movement = glm::normalize(g_bar2_movement);
    }
}

void game_loop(float delta_time) {
    // ———————————————— RESETTING MODEL MATRIX ———————————————— //
    g_bar1_model_matrix = glm::mat4(1.0f);
    g_bar1_model_matrix = glm::scale(g_bar1_model_matrix, BAR_INIT_SCA);
    g_bar1_model_matrix = glm::translate(g_bar1_model_matrix, BAR1_INIT_POS);

    g_bar2_model_matrix = glm::mat4(1.0f);
    g_bar2_model_matrix = glm::scale(g_bar2_model_matrix, BAR_INIT_SCA);
    g_bar2_model_matrix = glm::translate(g_bar2_model_matrix, BAR2_INIT_POS);

    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::scale(g_ball_model_matrix, BALL_INIT_SCA);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);

    // ———————————————— TRANSLATIONS ———————————————— //
    g_bar1_position += g_bar1_movement * g_bar_speed * delta_time;
    g_bar1_model_matrix = glm::translate(g_bar1_model_matrix, g_bar1_position);
    g_bar1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

    g_bar2_position += g_bar2_movement * g_bar_speed * delta_time;
    g_bar2_model_matrix = glm::translate(g_bar2_model_matrix, g_bar2_position);
    g_bar2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

    g_ball_position += g_ball_movement * g_ball_speed * delta_time;
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);

    // ———————————————— BALL COLLISIONS ———————————————— //

    if (is_collision(g_ball_position, BALL_INIT_SCA, TOP_INIT_POS, TOP_BOTTOM_INIT_SCA))
    {
        g_ball_movement.y = -g_ball_movement.y;
    }
    else if (is_collision(g_ball_position, BALL_INIT_SCA, BOTTOM_INIT_POS, TOP_BOTTOM_INIT_SCA))
    {
        g_ball_movement.y = -g_ball_movement.y;
    }
    else if (is_collision(g_ball_position, BALL_INIT_SCA, g_bar1_position, BAR_INIT_SCA))
    {
        g_ball_movement.x = -g_ball_movement.x;
        g_ball_speed += 2;
    }
    else if (is_collision(g_ball_position, BALL_INIT_SCA, g_bar2_position, BAR_INIT_SCA))
    {
        g_ball_movement.x = -g_ball_movement.x;
        g_ball_speed += 2;
    }

    if (glm::length(g_ball_movement) > 1.0f)
    {
        g_ball_movement = glm::normalize(g_ball_movement);
    }

    if (is_collision(g_ball_position, BALL_INIT_SCA, LEFT_INIT_POS, LEFT_RIGHT_INIT_SCA))
    {
        g_player2_win = true;
    }
    else if (is_collision(g_ball_position, BALL_INIT_SCA, RIGHT_INIT_POS, LEFT_RIGHT_INIT_SCA))
    {
        g_player1_win = true;
    }
}

void update()
{
    // ———————————————— DELTA TIME CALCULATIONS ———————————————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    if (!g_player1_win && !g_player2_win) {
        game_loop(delta_time);
    }
    else
    {
        if (g_player1_win)
        {
            g_player1_win_matrix = glm::mat4(1.0f);
            g_player1_win_matrix = scale(g_player1_win_matrix, WIN_INIT_SCA);
            g_player1_win_matrix = translate(g_player1_win_matrix, WIN_SHOW_POS);
        }
        else if (g_player2_win)
        {
            g_player2_win_matrix = glm::mat4(1.0f);
            g_player2_win_matrix = scale(g_player2_win_matrix, WIN_INIT_SCA);
            g_player2_win_matrix = translate(g_player2_win_matrix, WIN_SHOW_POS);
        }
    }
}



void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    // ———————————————— BACKGROUND ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_bg_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_bg_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BAR1 ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_bar1_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_bar_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BAR2 ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
 
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_bar2_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_bar_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BALL ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_ball_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— Boundaries ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_top);
    glBindTexture(GL_TEXTURE_2D, g_boundary_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_bottom);
    glBindTexture(GL_TEXTURE_2D, g_boundary_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_left);
    glBindTexture(GL_TEXTURE_2D, g_boundary_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_right);
    glBindTexture(GL_TEXTURE_2D, g_boundary_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— WIN MESSEGES ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_player1_win_matrix);
    glBindTexture(GL_TEXTURE_2D, g_player1_win_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    g_shader_program.set_model_matrix(g_player2_win_matrix);
    glBindTexture(GL_TEXTURE_2D, g_player2_win_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— GENERAL ———————————————— //
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}