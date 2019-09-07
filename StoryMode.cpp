#include "StoryMode.hpp"

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"

Sprite const *sprite_left_select = nullptr;
Sprite const *sprite_right_select = nullptr;
Sprite const *sprite_dunes_bg = nullptr;
Sprite const *sprite_dunes_traveller = nullptr;
Sprite const *sprite_dunes_ship = nullptr;

std::string state_false = "state flag was not flipped yet.";
std::string state_true = "state flag has been selected!";

Load< SpriteAtlas > sprites(LoadTagDefault, []() -> SpriteAtlas const * {
  SpriteAtlas const *ret = new SpriteAtlas(data_path("the-planet"));

	sprite_left_select = &ret->lookup("text-select-left");
	sprite_right_select = &ret->lookup("text-select-right");
  sprite_dunes_bg = &ret->lookup("dunes-bg");
  sprite_dunes_traveller = &ret->lookup("dunes-traveller");
  sprite_dunes_ship = &ret->lookup("dunes-ship");

  return ret;
});

StoryMode::StoryMode() {
}

StoryMode::~StoryMode() {
}

bool StoryMode::handle_event(SDL_Event const &, glm::uvec2 const &window_size) {
  if (Mode::current.get() != this) return false;

  return false;
}

void StoryMode::update(float elapsed) {
  if (Mode::current.get() == this) {
    //there is no menu displayed! Make one:
    enter_scene();
  }
}

void StoryMode::enter_scene() {
  //just entered this scene, adjust flags and build menu as appropriate:
  std::vector< MenuMode::Item > items;
  glm::vec2 at(3.0f, view_max.y - 50.0f);

  auto add_text = [&items,&at](
      Sprite const *text, 
      std::string str // gets drawn as alternative string if no sprite
  ) {
    items.emplace_back(str, text, 1.0f, nullptr, at);
    if (text) {
      at.y -= text->max_px.y - text->min_px.y;
      at.y -= 4.0f;
    } else {
      at.y -= 24.0f;
    }
  };

  auto add_choice = [&items,&at](
      Sprite const *text, 
      std::string str, // gets drawn as alternative string if no sprite
      std::function< void(MenuMode::Item const &) > const &fn
  ) {
    items.emplace_back(str, text, 1.0f, fn, at + glm::vec2(8.0f, 0.0f));
    if (text) {
      at.y -= text->max_px.y - text->min_px.y;
      at.y -= 4.0f;
    } else {
      at.y -= 24.0;
    }
  };

  // create the menu item depending on the current game state
  if (state_flag) {
    add_text(nullptr, state_true);
    at.y -= 4.0f; //gap before choices
    add_choice(nullptr, "ok.", [](MenuMode::Item const &){
      Mode::current = nullptr;    
    });
  } else {
    add_text(nullptr, state_false);
    at.y -= 4.0f; //gap before choices
    add_choice(nullptr, "flip flag.", [this](MenuMode::Item const &){
      state_flag = true;
      Mode::current = shared_from_this();
    });
    add_choice(nullptr, "do nothing.", [this](MenuMode::Item const &){
      Mode::current = shared_from_this();
    });
  }

  std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >(items);
  menu->atlas = sprites;
  menu->left_select = sprite_left_select;
  menu->right_select = sprite_right_select;
  menu->view_min = view_min;
  menu->view_max = view_max;
  menu->background = shared_from_this();
  Mode::current = menu;
}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
  //clear the color buffer:
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  //use alpha blending:
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //don't use the depth test:
  glDisable(GL_DEPTH_TEST);

  { //use a DrawSprites to do the drawing:
    DrawSprites draw(*sprites, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);
    glm::vec2 ul = glm::vec2(view_min.x, view_max.y);

    // draw the scene (bg only) according to game state
    if (state_flag) {
      draw.draw(*sprite_dunes_bg, ul);
      draw.draw(*sprite_dunes_ship, ul);
      draw.draw(*sprite_dunes_traveller, ul);
    } 
    
  }
  GL_ERRORS(); //did the DrawSprites do something wrong?
}
