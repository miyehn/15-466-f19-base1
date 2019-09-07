#include "StoryMode.hpp"

#include "Load.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"
#include "Timeline.hpp"

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
  // if (Mode::current.get() != this) return false;
  return false;
}

void StoryMode::update(float elapsed) {
  time_elapsed = elapsed;
  if (Mode::current.get() == this && !animation_playing) {
    // there is no menu displayed! Make one
    // menu becomes current mode, until some choice is made
    // then either story becomes current again (and some animation starts, maybe), or quit.
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
      // construct animation to start playing
      animation = std::vector<AnimatedSprite*>();
      animation_playing = true;

      Timeline time1 = Timeline(data_path("example.timeline"));
      time1.set_interval(0.0f, 2.0f);
      time1.playing = true; // set this first animation in sequence to playing state
      animation.push_back(
          new AnimatedSprite(sprite_dunes_ship, glm::vec2(view_min.x, view_max.y), time1));

      Timeline time2 = Timeline(data_path("example.timeline"));
      time2.set_interval(0.0f, 2.0f);
      animation.push_back(
          new AnimatedSprite(sprite_dunes_traveller, glm::vec2(view_min.x, view_max.y), time2));

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

// helper to StoryMode::draw
// maybe also pass in a "playing" bool that can be flipped when entire animation ended
void StoryMode::draw_animation(glm::uvec2 const &drawable_size, DrawSprites &draw) {

  // a little helper function that maps things
  auto alpha255 = [](float alpha) {
    float res = alpha * 255.0f;
    return (uint8_t)res;
  };

  for (int i=0; i<animation.size(); i++) {
    AnimatedSprite* as = animation[i];
    if (as->timeline.playing) { // if it's playing before drawing.
      draw.draw(
          *(as->sprite), 
          as->position,
          1.0f,
          glm::u8vec4(255, 255, 255, alpha255(as->timeline.get_value()))
      ); 
      as->timeline.update(time_elapsed);
      if (!as->timeline.playing) {
        if (i != animation.size()-1) {
          // start next one
          animation[i+1]->timeline.playing = true;
        } else {
          // end entire animation
          animation_playing = false;
          // decide whether to retain the last sprite and keep drawing it 
          end_of_animation_sprite = as->timeline.get_value() > 0.5 ? 
            animation[animation.size()-1]->sprite : nullptr;
          for (auto as : animation) delete as;
        }
      }
    }
  }

}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
  //clear the color buffer:
  glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
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
    } 
    if (animation_playing) draw_animation(drawable_size, draw);
    else if (end_of_animation_sprite) {
      draw.draw(*end_of_animation_sprite, ul);
    }
    
  }
  GL_ERRORS(); //did the DrawSprites do something wrong?
}

