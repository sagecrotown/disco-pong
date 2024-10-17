//
//  Entity.cpp
//  exercise
//
//  Created by Sage Cronen-Townsend on 10/11/24.
//

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include <cmath>


// Default constructor
Entity::Entity()
    : e_position(0.0f), e_movement(0.0f), e_scale(1.0f, 1.0f, 0.0f), e_model_matrix(1.0f),
      e_speed(0.0f, 0.0f, 0.0f), e_animation_cols(0), e_animation_rows(0), e_animation_frames(0), e_animation_index(0), e_animation_indices(nullptr), e_animation_time(0.0f), e_current_animation(SPRITE1), e_can_move(true)
{
}

// Parameterized constructor
Entity::Entity(std::vector<GLuint> texture_ids,
               glm::vec3 speed,
               std::vector<std::vector<int>> animations,
               float time,
               int animation_frames,
               int animation_index,
               int animation_cols,
               int animation_rows,
               Animation state)

    : e_position(0.0f), e_movement(0.0f), e_scale(1.0f, 1.0f, 0.0f), e_model_matrix(1.0f),
      e_texture_ids(texture_ids), e_speed(speed), e_animations(animations),
      e_animation_time(time), e_animation_cols(animation_cols),
      e_animation_frames(animation_frames), e_animation_index(animation_index),
      e_animation_rows(animation_rows), e_current_animation(state), e_can_move(true)
{
    set_animation_state(e_current_animation);  // Initialize animation state
}

Entity::~Entity() { }

bool const Entity::check_collision(Entity* other) const {
    if (other->visibility) {
        
        float x_distance = fabs(e_position.x - other->e_position.x);
        float y_distance = fabs(e_position.y - other->e_position.y);
        
        if (e_shape == BALL) {      // if item is ball ... then things get complicated and annoying
            float radius = e_scale[0] / 2.0f;
            
            if (x_distance > (other->e_scale[0] / 2.0f + radius)) { return false; }       // too far to collide
            if (y_distance > (other->e_scale[1] / 2.0f + radius)) { return false; }
            //
            if ( x_distance <= other->e_scale[0] / 2.0f) { return true; }   // check edge collisions
            if ( y_distance <= other->e_scale[1] / 2.0f) { return true; }
            
            // check distances from circle center to vertices of rectangle
            
            float dist = sqrt(pow(x_distance - other->e_scale[0] / 2.0f, 2) + pow(y_distance - other->e_scale[1] / 2.0f, 2));
            return dist <= radius;
        }
        
        else {      // box to box
            return (x_distance < (e_scale[0] + other->e_scale[0]) / 2.0f) && (y_distance < (e_scale[1] + other->e_scale[1]) / 2.0f);
        }
    }
    return false;
}

void Entity::set_animation_state(Animation new_animation) {
    e_current_animation = new_animation;

    // Update the texture and animation indices based on the current animation
    e_animation_indices = e_animations[e_current_animation].data();
    
    e_animation_frames = e_animations[e_current_animation].size();
    e_animation_rows = e_animations[e_current_animation].size();
}

// Render the appropriate texture and animation frame
void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program) {
    GLuint current_texture = e_texture_ids[e_current_animation];  // Get the right texture

    float u_coord = (float) (e_animation_index % e_animation_cols) / (float) e_animation_cols;
    float v_coord = (float) (e_animation_index / e_animation_cols) / (float) e_animation_rows;

    float width = 1.0f / (float) e_animation_cols;
    float height = 1.0f / (float) e_animation_rows;

    float tex_coords[] = {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width,
        v_coord, u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] = {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    glBindTexture(GL_TEXTURE_2D, current_texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
                          vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
                          tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::update(float delta_time, std::vector<Entity*> collidable_entities, int entity_count) {
    if (visibility) {
        // Check for collisions
        for (int i = 0; i < entity_count; i++) {
            if (check_collision(collidable_entities[i])) {
                if (e_shape == BALL) {
                    if (collidable_entities[i]->get_shape() == SIDE_WALL) { // end game
                        collidable_entities[i]->set_loser(true);
                    }
                    else if (collidable_entities[i]->get_shape() == LEFT_PADDLE) {  // ball collides with paddle
                        e_movement = glm::vec3(1.0f, e_movement[1], e_movement[2]);
                    }
                    else if (collidable_entities[i]->get_shape() == RIGHT_PADDLE) {
                        e_movement = glm::vec3(-1.0f, e_movement[1], e_movement[2]);
                    }
                    
                    else if (collidable_entities[i]->get_shape() == TOP_WALL) {
                        e_movement = glm::vec3(e_movement[0], -1.0f, e_movement[2]);
                    }
                    else if (collidable_entities[i]->get_shape() == BOTTOM_WALL) {
                        e_movement = glm::vec3(e_movement[0], 1.0f, e_movement[2]);
                    }
                }
                else if (e_shape == LEFT_PADDLE || e_shape == RIGHT_PADDLE) {  // paddle collides with wall
                    
                    if (collidable_entities[i]->get_shape() == TOP_WALL) {
                        e_movement = glm::vec3(1.0f, -1.0f, 1.0f);
                    }
                    else if (collidable_entities[i]->get_shape() == BOTTOM_WALL) {
                        e_movement = glm::vec3(1.0f, 1.0f, 1.0f);
                    }
                }
            }
        }
        
        e_animation_time += delta_time;
        float frames_per_second = 1.0f / SECONDS_PER_FRAME;
        
        if (e_animation_time >= frames_per_second) {
            e_animation_time = 0.0f;
            e_animation_index++;
            
            if (e_animation_index >= e_animation_frames) {
                e_animation_index = 0;
            }
        }
        
        e_position += e_movement * e_speed * delta_time;
        e_rotation += e_omega * delta_time;
        e_model_matrix = glm::mat4(1.0f);
        e_model_matrix = glm::translate(e_model_matrix, e_position);
        e_model_matrix = glm::rotate(e_model_matrix, e_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        e_model_matrix = glm::scale(e_model_matrix, e_scale);
    }
}
    

void Entity::render(ShaderProgram* program) {
    if (visibility) {
        program->set_model_matrix(e_model_matrix);
        
        if (e_animation_indices != nullptr) draw_sprite_from_texture_atlas(program);
    }
}
