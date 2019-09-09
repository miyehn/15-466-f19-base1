#include "StoryMode.hpp"

#include "Load.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"
#include "Timeline.hpp"

// for menu dialog
Sprite const *sprite_left_select = nullptr;
Sprite const *sprite_dialog1 = nullptr;
Sprite const *sprite_dialog2 = nullptr;
Sprite const *sprite_dialog3 = nullptr;

// for animated responses
Sprite const *sprite_uhh = nullptr;
Sprite const *sprite_anyone_there = nullptr;
Sprite const *sprite_can_you_help_me = nullptr;
Sprite const *sprite_open_camera = nullptr;
Sprite const *sprite_signal_DNE = nullptr;
Sprite const *sprite_signal_DNE_glitch = nullptr;
Sprite const *sprite_show_autorecovered_image = nullptr;
Sprite const *sprite_i_cant = nullptr;
Sprite const *sprite_easier_communicate = nullptr;
Sprite const *sprite_not_fully_functional = nullptr;
Sprite const *sprite_back_up_working = nullptr;
Sprite const *sprite_let_me_fix_camera = nullptr;
Sprite const *sprite_too_high_up = nullptr;
Sprite const *sprite_see_me_now = nullptr;
Sprite const *sprite_thanks_for_help = nullptr;
Sprite const *sprite_move_on_mission = nullptr;
Sprite const *sprite_farewell = nullptr;

// background and static illustrations
Sprite const *bg_dark = nullptr;
Sprite const *img_recovered = nullptr;
Sprite const *img_live = nullptr;

Load< SpriteAtlas > sprites(LoadTagDefault, []() -> SpriteAtlas const * {
  SpriteAtlas const *ret = new SpriteAtlas(data_path("sprites"));

  for (std::pair<std::string, Sprite> elem : ret->sprites) {
    std::cout << elem.first;
    std::cout << std::endl;
  }
  std::cout << "--------" << std::endl;

	sprite_left_select = &ret->lookup(std::to_string(int('>')));
  sprite_dialog1 = &ret->lookup("dialog1");
  sprite_dialog2 = &ret->lookup("dialog2");
  sprite_dialog3 = &ret->lookup("dialog3");

  sprite_uhh = &ret->lookup("uhh");
  sprite_anyone_there = &ret->lookup("anyoneCtrlRoom");
  sprite_can_you_help_me = &ret->lookup("needHelp");
  sprite_open_camera = &ret->lookup("pressC");
  sprite_signal_DNE = &ret->lookup("noCamSignal");
  sprite_signal_DNE_glitch = &ret->lookup("noCamSignalGlitch");
  sprite_show_autorecovered_image = &ret->lookup("imgRecovered");
  sprite_i_cant = &ret->lookup("iCant");
  sprite_easier_communicate = &ret->lookup("easierCommunicate");
  sprite_not_fully_functional = &ret->lookup("notFullyFunctional");
  sprite_back_up_working = &ret->lookup("backUpWorking");
  sprite_let_me_fix_camera = &ret->lookup("letMeFixCamera");
  sprite_too_high_up = &ret->lookup("tooHighUp");
  sprite_see_me_now = &ret->lookup("canYouSeeMeNow");
  sprite_thanks_for_help = &ret->lookup("thanksForHelp");
  sprite_move_on_mission = &ret->lookup("moveOnMission");
  sprite_farewell = &ret->lookup("tillMeetAgain");

  bg_dark= &ret->lookup("darkBg");
  img_recovered = &ret->lookup("lightBg"); // TODO: replace with illust 1
  img_live = &ret->lookup("imgRecording"); // TODO: replace with illust 2

  return ret;
});

StoryMode::StoryMode() {
  add_anim_sequence(
      sprite_uhh, 90, 200,
      "example.timeline", 0.5f, 1.0f);
  add_anim_sequence(
      sprite_anyone_there, 70, 130,
      "example.timeline", 0.5f, 1.0f, true);
}

StoryMode::~StoryMode() {
}

bool StoryMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
  if (waiting_for_camera && !animation_playing) {
    if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_c) {
      add_anim_sequence(
          sprite_signal_DNE, 140, 200,
          "example.timeline", 0.5f, 0.9f);
      add_anim_sequence(
          sprite_signal_DNE_glitch, 140, 200,
          "example.timeline", 0.9f, 1.1f);
      add_anim_sequence(
          sprite_signal_DNE, 140, 200,
          "example.timeline", 1.1f, 2.0f);
      add_anim_sequence(
          img_recovered, 0, 0, // TODO: hack this to display bg and illust both
          "ease_in_mid_4", 0.0f, 3.0f, true);
      waiting_for_camera = false;
      story_state = open_camera;
    }
  }
  return false;
}

// NOTE: pos here means: ul corner is (0,0), br corner is (720, 540).
void StoryMode::add_anim_sequence(
    Sprite const* sprite, int posX, int posY, 
    std::string const& timeline_path, float start, float end, bool retain) {
  glm::vec2 pos = glm::vec2((float)posX, (float)posY);
  bool first = false;
  if (!animation_playing) { // adding the first sequence
    animation = std::vector<AnimatedSprite*>();
    animation_playing = true;
    first = true;
  }
  Timeline timeline = Timeline(data_path(timeline_path));
  timeline.set_interval(start, end);
  glm::vec2 draw_position(view_min.x+pos.x, view_max.y-pos.y);
  if (first) timeline.playing = true;
  animation.push_back(
      new AnimatedSprite(sprite, draw_position, timeline));
  if (retain) {
    end_of_animation_sprite = sprite;
    end_of_animation_sprite_pos = draw_position;
  } else {
    end_of_animation_sprite = nullptr;
  }
}

void StoryMode::update(float elapsed) {
  time_elapsed = elapsed;
  if (Mode::current.get() == this && !animation_playing) {
    // menu becomes current mode, until some choice is made
    // then either story becomes current again (and some animation starts, maybe), or quit.
    display_menu();
  }
}

void StoryMode::display_menu() {
  //just entered this scene, adjust flags and build menu as appropriate:
  std::vector< MenuMode::Item > items;
  glm::vec2 at(30.0f, 86.0f);

  auto add_choice = [&items,&at](
      Sprite const *text, 
      std::string str, // gets drawn as alternative string if no sprite
      std::function< void(MenuMode::Item const &) > const &fn
  ) {
    items.emplace_back(str, text, 1.0f, fn, at + glm::vec2(28.0f, 0.0f));
    at.y -= 30.0f;
  };

  
  // what to do after _RESPONDING_ to each of the following states...
  switch(story_state) {

    case uhh_anyone: // respond to "uhh, anyone?" and whether if anyone's in control room
      add_choice(nullptr, "Yes! Hello!", [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_can_you_help_me, 100, 150, 
            "example.timeline", 0.5f, 1.0f);
        add_anim_sequence(
            sprite_open_camera, 140, 190,
            "example.timeline", 0.5f, 1.0f, true);
        waiting_for_camera = true;
        story_state = ask_to_open_camera;
        Mode::current = shared_from_this();
      });
      break;

    case ask_to_open_camera: // keyboard input handles this
      break;

    case open_camera:
      add_choice(nullptr, "Turn right to reach your right arm.", 
          [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_easier_communicate, 80, 180,
            "example.timeline", 0.5f, 1.0f);
        add_anim_sequence(
            sprite_not_fully_functional, 75, 180,
            "example.timeline", 0.5f, 1.0f, true);
        camera_on = true;
        story_state = easier_communication;
        Mode::current = shared_from_this();
      });
      add_choice(nullptr, "Move closer to the camera to reach your legs.",
          [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_i_cant, 180, 200,
            "example.timeline", 0.0f, 1.0f);
        camera_on = true;
        Mode::current = shared_from_this();
      });
      add_choice(nullptr, "Please first fix the camera.",
          [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_i_cant, 180, 200,
            "example.timeline", 0.0f, 1.0f);
        camera_on = true;
        alr_asked_abt_camera = true;
        Mode::current = shared_from_this();
      });
      break;

    case easier_communication:
      add_choice(nullptr, "Move closer to the camera to reach your legs.",
          [this](MenuMode::Item const &) {
        if (alr_asked_abt_camera) {
          add_anim_sequence(
              sprite_let_me_fix_camera, 55, 170,
              "example.timeline", 0.0f, 1.0f);
          add_anim_sequence(
              sprite_see_me_now, 50, 200,
              "example.timeline", 0.0f, 1.0f, true);
          story_state = back_working_look_great;
        } else {
          add_anim_sequence(
              sprite_back_up_working, 40, 100,
              "example.timeline", 0.0f, 1.0f, true);
          story_state = back_working_fix_camera;
        }
        story_state = alr_asked_abt_camera ? back_working_fix_camera : back_working_look_great;
        Mode::current = shared_from_this();
      });
      add_choice(nullptr, "Please fix the camera.",
          [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_too_high_up, 80, 155,
            "example.timeline", 0.0f, 1.0f, true);
        alr_asked_abt_camera = true;
        story_state = cant_reach_camera;
        Mode::current = shared_from_this();
      });
      break;

    case cant_reach_camera:
      add_choice(nullptr, "Move closer to the camera to reach your legs.",
          [this](MenuMode::Item const &) {
        assert(alr_asked_abt_camera);
        add_anim_sequence(
            sprite_let_me_fix_camera, 55, 170,
            "example.timeline", 0.0f, 1.0f);
        add_anim_sequence(
            sprite_see_me_now, 50, 200,
            "example.timeline", 0.0f, 1.0f, true);
        story_state = back_working_fix_camera;
        Mode::current = shared_from_this();
      });
      break;

    case back_working_look_great:
      add_choice(nullptr, "Still can't see you. Please fix the camera.",
          [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_let_me_fix_camera, 55, 170,
            "example.timeline", 0.0f, 1.0f);
        add_anim_sequence(
            sprite_see_me_now, 50, 200,
            "example.timeline", 0.0f, 1.0f, true);
        alr_asked_abt_camera = true;
        story_state = back_working_fix_camera;
        Mode::current = shared_from_this();
      });
      break;

    case back_working_fix_camera: // when reach here, camera's fixed. Play farewell anim
      camera_working = true;
      add_choice(nullptr, "Yep.", [this](MenuMode::Item const &) {
        add_anim_sequence(
            sprite_thanks_for_help, 50, 200,
            "example.timeline", 0.0f, 1.0f);
        add_anim_sequence(
            sprite_move_on_mission, 90, 150,
            "example.timeline", 0.0f, 1.0f);
        add_anim_sequence(
            sprite_farewell, 55, 140,
            "example.timeline", 0.0f, 1.0f, true);
        story_state = farewell;
        Mode::current = shared_from_this();
      });
      break;

    case farewell:
      break;
  }
  
  std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >(items);
  menu->atlas = sprites;
  menu->left_select = sprite_left_select;
  menu->dialog1 = sprite_dialog1;
  menu->dialog2 = sprite_dialog2;
  menu->dialog3 = sprite_dialog3;
  menu->view_min = view_min;
  menu->view_max = view_max;
  menu->background = shared_from_this();
  if (!(story_state==farewell ||
        story_state==ask_to_open_camera
        ))Mode::current = menu;
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
          for (auto as : animation) delete as;
        }
      }
    }
  }

}

// TODO: check for all game states, store as properties if necessary, draw them accordingly.
// Then run and debug
// then create all font sprites
// then illustration sprites
// then tune animation
// then make visuals look better...

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
    if (camera_on) {
      if (camera_working) draw.draw(*img_live, ul);
      else draw.draw(*img_recovered, ul);
    }
    else draw.draw(*bg_dark, ul);

    if (animation_playing) draw_animation(drawable_size, draw);
    else if (end_of_animation_sprite) {
      draw.draw(*end_of_animation_sprite, end_of_animation_sprite_pos);
    }
    
  }
  GL_ERRORS(); //did the DrawSprites do something wrong?
}

