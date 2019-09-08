#include "MenuMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for easy sprite drawing:
#include "DrawSprites.hpp"

#include <random>

MenuMode::MenuMode(std::vector< Item > const &items_) : items(items_) {

  //select first item which can be selected:
  for (uint32_t i = 0; i < items.size(); ++i) {
    if (items[i].on_select) {
      selected = i;
      break;
    }
  }
}

MenuMode::~MenuMode() {
}

bool MenuMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
  if (evt.type == SDL_KEYDOWN) {
    /*
    if (wait_for_camera && evt.key.keysym.sym == SDLK_c) {
      wait_for_camera = false;
      Mode::current = background;
      return true;
    } else */if (evt.key.keysym.sym == SDLK_UP) {
      //skip non-selectable items:
      for (uint32_t i = selected - 1; i < items.size(); --i) {
        if (items[i].on_select) {
          selected = i;
          break;
        }
      }
      return true;
    } else if (evt.key.keysym.sym == SDLK_DOWN) {
      //note: skips non-selectable items:
      for (uint32_t i = selected + 1; i < items.size(); ++i) {
        if (items[i].on_select) {
          selected = i;
          break;
        }
      }
      return true;
    } else if (evt.key.keysym.sym == SDLK_RETURN) {
      if (selected < items.size() && items[selected].on_select) {
        items[selected].on_select(items[selected]);
        return true;
      }
    }
  }
  if (background) {
    return background->handle_event(evt, window_size);
  } else {
    return false;
  }
}

void MenuMode::update(float elapsed) {

  //TODO: selection bounce update

  if (background) {
    background->update(elapsed);
  }
}

void MenuMode::draw(glm::uvec2 const &drawable_size) {
  if (background) {
    std::shared_ptr< Mode > hold_me = shared_from_this();
    background->draw(drawable_size);
    //it is an error to remove the last reference to this object in background->draw():
    assert(hold_me.use_count() > 1);
  } else {
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  //use alpha blending:
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //don't use the depth test:
  glDisable(GL_DEPTH_TEST);

  // if (wait_for_camera) return;
  { //draw the menu using DrawSprites:
    assert(atlas && "it is an error to try to draw a menu without an atlas");
    DrawSprites draw_sprites(*atlas, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);

    // draw menu background
    if (items.size() == 1) {
      draw_sprites.draw(*dialog1, glm::vec2(0.0f, 60.0f));
    } else if (items.size() == 2) {
      draw_sprites.draw(*dialog2, glm::vec2(0.0f, 90.0f));
    } else {
      draw_sprites.draw(*dialog3, glm::vec2(0.0f, 120.0f));
    }

    // menu vertical offset: if has less than 3 choices, items get moved down
    glm::vec2 offset(0.0f, 30.0f * (3 - items.size()));
    // glm::vec2 offset(0.0f, 0.0f);

    for (auto const &item : items) {
      bool is_selected = (&item == &items[0] + selected);
      glm::u8vec4 color = glm::u8vec4(0xff, 0xff, 0xff, 0xff);
      float left, right;

      //draw item.name as text:
      draw_sprites.draw_text(
        item.name, item.at - offset, item.scale, color
      );
      glm::vec2 min = item.at;
      glm::vec2 max = item.at;
      draw_sprites.get_text_extents(
        item.name, item.at, item.scale, &min, &max
      );
      left = min.x;
      right = max.x;

      if (is_selected && left_select) {
        draw_sprites.draw(*left_select, glm::vec2(left - 22, item.at.y + 14) - offset, item.scale);
      }
      
    }
  } //<-- gets drawn here!


  GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.
}
