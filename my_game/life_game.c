#include "player_input.c"

typedef enum EntityType {
    NIL = 0,
    PLAYER = 1,
    WORM = 2,
    ROCK = 3,
    // Add more entity types as needed
} EntityType;

#define MAX_ENTITY_COUNT 1024

typedef struct Entity {
    Vector2 position;
    Vector2 last_position;
    float width; // Width of the entity for collision detection
    float height; // Height of the entity for collision detection
    float speed;
    bool isActive; // Status to check if the entity is active in the game
    int health; // Health points of the entity
    EntityType type; // Type of the entity (using enum)
    Gfx_Image* sprite;
} Entity;

typedef struct World {
    Entity entities[MAX_ENTITY_COUNT];
} World;

World* world;

Entity* entity_create()
{
    Entity* entity_found = 0;
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
        Entity* existing_entity = &world->entities[i];
        if (!existing_entity->isActive) {
            entity_found = existing_entity;
            break;
        }
    }
    assert(entity_found, "No more free entities");
    return entity_found;
}

// Function to check if two entities collide
bool collide(const Entity* entityA, const Entity* entityB)
{
    // Check if both entities are active
    if (!entityA->isActive || !entityB->isActive) {
        return false; // No collision if either entity is inactive
    }

    // Calculate the edges of both entities
    float leftA = entityA->position.x;
    float rightA = entityA->position.x + entityA->width;
    float topA = entityA->position.y;
    float bottomA = entityA->position.y + entityA->height;

    float leftB = entityB->position.x;
    float rightB = entityB->position.x + entityB->width;
    float topB = entityB->position.y;
    float bottomB = entityB->position.y + entityB->height;

    // Check for overlap
    return (leftA < rightB && rightA > leftB && topA < bottomB && bottomA > topB);
}

int entry(int argc, char** argv)
{
    window.title = STR("Game Example");
    window.point_width = 1280;
    window.point_height = 720;
    window.x = 300;
    window.y = 50;
    window.clear_color = hex_to_rgba(0x2a2a3aff);

    world = alloc(get_heap_allocator(), sizeof(World));

    Gfx_Font* font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
    const u32 font_height = 28;

    // Load player sprite
    Gfx_Image* player_sprite = load_image_from_disk(STR("my_game/man.png"), get_heap_allocator());
    assert(player_sprite, "Failed loading player sprite");
    Gfx_Image* worm_sprite = load_image_from_disk(STR("my_game/worm.png"), get_heap_allocator());
    assert(worm_sprite, "Failed loading worm sprite");

    Entity* player = entity_create();
    *player = (Entity) { v2(0, 0), v2(0, 0), player_sprite->width * 4, player_sprite->height * 4, 300.0f, true, 100, PLAYER, player_sprite };
    Entity* worm = entity_create();
    *worm = (Entity) { v2(50, 50), v2(50, 50), worm_sprite->width * 4, worm_sprite->height * 4, 250.0f, true, 2, WORM, worm_sprite };

    // Vector2 player_pos = v2(0, 0);

    float64 last_time = os_get_elapsed_seconds();

    while (!window.should_close) {
        reset_temporary_storage();

        // draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
        // draw_frame.view = m4_make_scale(v3(0.18, 0.18, 1.0));

        float64 now = os_get_elapsed_seconds();
        float64 delta_time = now - last_time;
        last_time = now;

        Vector2 input_axes = v2(0, 0);
        if (is_key_just_released(KEY_ESCAPE)) {
            window.should_close = true;
        }

        if (collide(player, worm)) {
            print("Collision detected");
            player->position = player->last_position;
            player->health -= worm->health;
            worm->health -= player->health;
        } else {
            player->last_position = player->position;
        }

        if (player->health <= 0) {
            player->isActive = false;
            print("You should be dead");
        }

        if (worm->health <= 0) {
            worm->isActive = false;
        }

        player_input(&input_axes);
        input_axes = v2_normalize(input_axes);
        player->position = v2_add(player->position, v2_mulf(input_axes, player->speed * delta_time));

        if (player->isActive) {
            Matrix4 player_xform = m4_scalar(1.0);
            player_xform = m4_translate(player_xform, v3(player->position.x, player->position.y, 0));
            draw_image_xform(player->sprite, player_xform, v2(player->width, player->height), COLOR_WHITE);
        }
        if (worm->isActive) {
            Matrix4 player2_xform = m4_scalar(1.0);
            player2_xform = m4_translate(player2_xform, v3(worm->position.x, worm->position.y, 0));
            draw_image_xform(worm->sprite, player2_xform, v2(worm->width, worm->height), COLOR_WHITE);
        }

        draw_text(font, tprint("Health: %d", player->health), font_height, v2(-800, 400), v2(1, 1), COLOR_WHITE);

        os_update();
        gfx_update();
    }

    return 0;
}
