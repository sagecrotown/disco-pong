//
//  Entity.h
//  exercise
//
//  Created by Sage Cronen-Townsend on 10/11/24.
//

enum Animation { SPRITE1, SPRITE2, SPRITE3 };
enum Shape { BALL, TOP_WALL, BOTTOM_WALL, SIDE_WALL, LEFT_PADDLE, RIGHT_PADDLE };

class Entity
{
private:
    // ————— TEXTURES ————— //
    std::vector<GLuint> e_texture_ids;  // Vector of texture IDs for different animations

    // ————— ANIMATIONS ————— //
    std::vector<std::vector<int>> e_animations;  // Indices for each animation type

    glm::vec3 e_movement;
    glm::vec3 e_position;
    glm::vec3 e_scale;
    float e_rotation;

    glm::mat4 e_model_matrix;
    glm::vec3 e_speed;
    float e_omega;
    bool e_can_move;

    int e_animation_cols, e_animation_rows;
    int e_animation_frames, e_animation_index;

    Animation e_current_animation;  // Current animation state
    Shape e_shape;                  // presumably set just once

    int* e_animation_indices = nullptr;
    float e_animation_time = 0.0f;
    
    bool e_loser = false;           // not relevent except for the side walls
    bool visibility = true;
    

public:
    static constexpr int SECONDS_PER_FRAME = 6;

    // ————— CONSTRUCTORS ————— //
    Entity();
    Entity(std::vector<GLuint> texture_ids, glm::vec3 speed,
           std::vector<std::vector<int>> animations, float animation_time,
           int animation_frames, int animation_index, int animation_cols,
           int animation_rows, Animation animation);
    ~Entity();

    // ————— METHODS ————— //
    bool const check_collision(Entity *other) const;
    void draw_sprite_from_texture_atlas(ShaderProgram* program);
    void update(float delta_time, std::vector<Entity*> collidable_entities = {}, int entity_count = 0);
    void render(ShaderProgram* program);

    // Animation control
    void set_animation_state(Animation new_animation);
    void normalise_movement() { e_movement = glm::normalize(e_movement); };

    // Getters and Setters
    glm::vec3 const get_position() const { return e_position; }
    glm::vec3 const get_movement() const { return e_movement; }
    glm::vec3 const get_scale() const { return e_scale; }
    glm::vec3 const get_speed() const { return e_speed; }
    float const get_rotation() const {return e_omega; } // returns rotational velocity around z (ehhh kinda)
    Animation get_animation() const {return e_current_animation; }
    bool get_can_move() const { return e_can_move; }
    Shape get_shape() const { return e_shape; }
    bool get_loser() const {return e_loser; }
    bool get_visibility() const {return visibility; }

    void const set_position(glm::vec3 new_position) { e_position = new_position; }
    void const set_movement(glm::vec3 new_movement) { e_movement = new_movement; }
    void const set_rotation(float new_omega) { e_omega = new_omega; }
    void const set_scale(glm::vec3 new_scale) { e_scale = new_scale; }
    void const set_speed(glm::vec3 new_speed) { e_speed = new_speed; }
    void const set_can_move(bool can_move) { e_can_move = can_move; }
    void const set_shape(Shape new_shape) { e_shape = new_shape; }
    void const set_loser(bool is_loser) {e_loser = is_loser; }
    void const set_visibility(bool is_visible) {visibility = is_visible; }
};
