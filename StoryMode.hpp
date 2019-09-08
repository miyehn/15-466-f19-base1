
/*
 * StoryMode implements a story about The Planet of Choices.
 *
 */

#include "Mode.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include <iostream>

struct AnimatedSprite {
  AnimatedSprite(Sprite const* sprite_, glm::vec2 position_, Timeline timeline_) : 
    sprite(sprite_), position(position_), timeline(timeline_) {};
  Sprite const *sprite;
  glm::vec2 position;
  Timeline timeline;
};

struct StoryMode : Mode {
	StoryMode();
	virtual ~StoryMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

  float time_elapsed;

  // properties related to animation
  std::vector<AnimatedSprite*> animation;
  bool animation_playing = false;
  void add_anim_sequence(
      Sprite const* sprite, int posX, int posY,
      std::string const& timeline_path, float start, float end, 
      bool retain = false);
  void draw_animation(glm::uvec2 const &drawable_size, DrawSprites &draw);
  Sprite const* end_of_animation_sprite = nullptr;
  glm::vec2 end_of_animation_sprite_pos = glm::vec2(0.0f, 0.0f);

	//called to create menu for current scene:
	void display_menu();

	//------ story state -------
  
  enum StoryState {
    uhh_anyone,
    ask_to_open_camera,
    open_camera,
    easier_communication,
    cant_reach_camera,
    back_working_look_great,
    back_working_fix_camera,
    farewell
  };
  StoryState story_state = uhh_anyone;

  bool waiting_for_camera = false;
  bool alr_asked_abt_camera = false;
  bool camera_on = false;
  bool camera_working = false;

  bool state_flag = false;
	
	glm::vec2 view_min = glm::vec2(0,0);
	glm::vec2 view_max = glm::vec2(720, 540);
};
