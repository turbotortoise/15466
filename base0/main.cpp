#include "Draw.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <chrono>
#include <iostream>
#include <stdio.h>

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game0: Shepherd Dog";
		glm::uvec2 size = glm::uvec2(640, 640);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);

	//------------  game state ------------

	//Initialization

	//doggo
	glm::vec2 mouse = glm::vec2(0.0f, 0.0f);
	glm::u8vec4 dogcolor = glm::u8vec4(0x00, 0x00, 0x00, 0xff);
	//shep
	glm::vec2 sheep1 = glm::vec2(0.0f, 0.5f);
	glm::vec2 sheep2 = glm::vec2(0.5f, 0.25f);
	glm::vec2 sheep3 = glm::vec2(0.4f, -0.25f);
	glm::vec2 sheep4 = glm::vec2(-0.4f, -0.25f);
	glm::vec2 sheep5 = glm::vec2(-0.5f, 0.25f);
	//directions
	glm::vec2 dir1 = glm::vec2(0.0f, 0.0f);
	glm::vec2 dir2 = glm::vec2(0.0f, 0.0f);
	glm::vec2 dir3 = glm::vec2(0.0f, 0.0f);
	glm::vec2 dir4 = glm::vec2(0.0f, 0.0f);
	glm::vec2 dir5 = glm::vec2(0.0f, 0.0f);
	//boundary
	float bound = 0.9f;
	float fencesize = 0.05f;
	glm::u8vec4 fencecolor = glm::u8vec4(0x92, 0x62, 0x39, 0x0f);
	//beginning shep velocity
	float startvel = 0.02f;
	glm::u8vec4 sheepcolor = glm::u8vec4(0xff, 0xff, 0xff, 0xff);
	glm::vec2 sheep_velocity = glm::vec2(startvel, startvel);
	//beginning level, directly influences sheep speed
	float level = 1.0f;
	//difficulty is strictly greater than 0 and influences how quickly game speeds up
	float difficulty = 100.0f;
	float size = 0.1f;

	//------------  game loop ------------

	//Lambdas

	auto previous_time = std::chrono::high_resolution_clock::now();		

	//collision detection
	auto collide = [&size](glm::vec2 *sheep0, glm::vec2 *dir0, glm::vec2 *sheep1, glm::vec2 *dir1) {
		if  (((sheep0->x - size) <= (sheep1->x - size)) && ((sheep1->x - size) <= (sheep0->x + size))) {
			//sheep0 collided by sheep1 from right
			if (((sheep1->y - size) <= (sheep0->y + size)) && ((sheep0->y + size) <= (sheep1->y + size))) {
				//sheep0 collided by sheep1 from front
				dir0->x = -std::abs(dir0->x);
				dir1->x = std::abs(dir1->x);
				dir0->y = -std::abs(dir0->y);
				dir1->y = std::abs(dir1->y);
			}
		 	else if (((sheep1->y - size) <= (sheep0->y - size)) && ((sheep0->y - size) <= (sheep1->y + size))) {
		 		//sheep0 collided by sheep1 from behind
		 		dir0->x = -std::abs(dir0->x);
				dir1->x = std::abs(dir1->x);
				dir0->y = std::abs(dir0->y);
				dir1->y = -std::abs(dir1->y);
			}
		}
		if  (((sheep1->x - size) <= (sheep0->x - size)) && ((sheep0->x - size) <= (sheep1->x + size))) {
			//sheep0 collided by sheep1 from left
			if (((sheep1->y - size) <= (sheep0->y + size)) && ((sheep0->y + size) <= (sheep1->y + size))) {
				//sheep0 collided by sheep1 from front
				dir0->x = std::abs(dir0->x);
				dir1->x = -std::abs(dir1->x);
				dir0->y = -std::abs(dir0->y);
				dir1->y = std::abs(dir1->y);
			}
			else if (((sheep1->y - size) <= (sheep0->y - size)) && ((sheep0->y - size) <= (sheep1->y + size))) {
				//sheep0 collides sheep1 from behind
				dir0->x = std::abs(dir0->x);
				dir1->x = -std::abs(dir1->x);
				dir0->y = std::abs(dir0->y);
				dir1->y = -std::abs(dir1->y);
			}
		}
	};

	//dog collision detection
	auto dogcollide = [&mouse, &size](glm::vec2 *sheep, glm::vec2 *dir) {
		if  (((sheep->x - size) <= (mouse.x - size)) && ((mouse.x - size) <= (sheep->x + size))) {
			//sheep collided by mouse from right
			if (((mouse.y - size) <= (sheep->y + size)) && ((sheep->y + size) <= (mouse.y + size))) {
				//sheep collided by mouse from front
				dir->x = -std::abs(dir->x);
				dir->y = -std::abs(dir->y);
			}
		 	else if (((mouse.y - size) <= (sheep->y - size)) && ((sheep->y - size) <= (mouse.y + size))) {
		 		//sheep collided by mouse from behind
		 		dir->x = -std::abs(dir->x);
				dir->y = std::abs(dir->y);
			}
		}
		if  (((mouse.x - size) <= (sheep->x - size)) && ((sheep->x - size) <= (mouse.x + size))) {
			//sheep collided by mouse from left
			if (((mouse.y - size) <= (sheep->y + size)) && ((sheep->y + size) <= (mouse.y + size))) {
				//sheep collided by mouse from front
				dir->x = std::abs(dir->x);
				dir->y = -std::abs(dir->y);
			}
			else if (((mouse.y - size) <= (sheep->y - size)) && ((sheep->y - size) <= (mouse.y + size))) {
				//sheep collides mouse from behind
				dir->x = std::abs(dir->x);
				dir->y = std::abs(dir->y);
			}
		}
	};

	//randomly set movement
	auto movesheep = [](glm::vec2 *dir) {
		if ((rand() % 2) == 0) {
			if ((rand() % 2) == 0)
				dir->y = -1.0f;
			else
				dir->y = 1.0f;
		}
		else {
			if ((rand() % 2) == 0)
				dir->x = -1.0f;
			else
				dir->x = 1.0f;
		}
	};

	//game over detection
	auto gameover = [&size, &sheep_velocity, &bound](glm::vec2 *sheep0) {
		//if any sheep is out of bounds end game
		if (((sheep0->x - size) < -bound) || ((sheep0->x + size) > bound) || 
			((sheep0->y - size) < -bound) || ((sheep0->y + size) > bound))
			sheep_velocity = glm::vec2(0.0f, 0.0f);
	};

	bool should_quit = false;
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEMOTION) {
				mouse.x = (evt.motion.x + 0.5f) / float(config.size.x) * 2.0f - 1.0f;
				mouse.y = (evt.motion.y + 0.5f) / float(config.size.y) *-2.0f + 1.0f;
			} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
				//reset doggo position
				mouse = glm::vec2(0.0f, 0.0f);
				//reset shep positions
				sheep1 = glm::vec2(0.0f, 0.5f);
				sheep2 = glm::vec2(0.5f, 0.25f);
				sheep3 = glm::vec2(0.4f, -0.25f);
				sheep4 = glm::vec2(-0.4f, -0.25f);
				sheep5 = glm::vec2(-0.5f, 0.25f);
				//reset shep velocities
				movesheep(&dir1);
				movesheep(&dir2);
				movesheep(&dir3);
				movesheep(&dir4);
				movesheep(&dir5);
				//reset shep velocity
				sheep_velocity = glm::vec2(startvel, startvel);
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
			//update level
			level += elapsed / difficulty;
			
			//move shep
			sheep1 += elapsed * (dir1 * (sheep_velocity * glm::vec2(level, level)));
			sheep2 += elapsed * (dir2 * (sheep_velocity * glm::vec2(level, level)));
			sheep3 += elapsed * (dir3 * (sheep_velocity * glm::vec2(level, level)));
			sheep4 += elapsed * (dir4 * (sheep_velocity * glm::vec2(level, level)));
			sheep5 += elapsed * (dir5 * (sheep_velocity * glm::vec2(level, level)));

			//Collisions
			{
				//sheep1 collisions
				collide(&sheep1, &dir1, &sheep2, &dir2);
				collide(&sheep1, &dir1, &sheep3, &dir3);
				collide(&sheep1, &dir1, &sheep4, &dir4);
				collide(&sheep1, &dir1, &sheep5, &dir5);
				//sheep2 collisions
				collide(&sheep2, &dir2, &sheep1, &dir1);
				collide(&sheep2, &dir2, &sheep3, &dir3);
				collide(&sheep2, &dir2, &sheep4, &dir4);
				collide(&sheep2, &dir2, &sheep5, &dir5);
				//sheep3 collisions
				collide(&sheep3, &dir3, &sheep1, &dir1);
				collide(&sheep3, &dir3, &sheep2, &dir2);
				collide(&sheep3, &dir3, &sheep4, &dir4);
				collide(&sheep3, &dir3, &sheep5, &dir5);
				//sheep4 collisions
				collide(&sheep4, &dir4, &sheep1, &dir1);
				collide(&sheep4, &dir4, &sheep2, &dir2);
				collide(&sheep4, &dir4, &sheep3, &dir3);
				collide(&sheep4, &dir4, &sheep5, &dir5);
				//sheep5 collisions
				collide(&sheep5, &dir5, &sheep1, &dir1);
				collide(&sheep5, &dir5, &sheep2, &dir2);
				collide(&sheep5, &dir5, &sheep3, &dir3);
				collide(&sheep5, &dir5, &sheep4, &dir4);
				//doggo collisions
				dogcollide(&sheep1, &dir1);
				dogcollide(&sheep2, &dir2);
				dogcollide(&sheep3, &dir3);
				dogcollide(&sheep4, &dir4);
				dogcollide(&sheep5, &dir5);
			}

			//game ends when sheep leave pen
			{
				gameover(&sheep1);
				gameover(&sheep2);
				gameover(&sheep3);
				gameover(&sheep4);
				gameover(&sheep5);
			}

		}

		//draw output:
		glClearColor(0.0, 1.0f, 0.5f, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);


		{ //draw game state:
			Draw draw;
			//sheep1
			draw.add_rectangle(sheep1 + glm::vec2(-size, -size), sheep1 + glm::vec2(size, size), sheepcolor);
			//sheep 2
			draw.add_rectangle(sheep2 + glm::vec2(-size, -size), sheep2 + glm::vec2(size, size), sheepcolor);
			//sheep3
			draw.add_rectangle(sheep3 + glm::vec2(-size, -size), sheep3 + glm::vec2(size, size), sheepcolor);
			//sheep4
			draw.add_rectangle(sheep4 + glm::vec2(-size, -size), sheep4 + glm::vec2(size, size), sheepcolor);
			//sheep5
			draw.add_rectangle(sheep5 + glm::vec2(-size, -size), sheep5 + glm::vec2(size, size), sheepcolor);
			//doggo
			draw.add_rectangle(mouse + glm::vec2(-size, -size), mouse + glm::vec2(size, size), dogcolor);
			//pen
			draw.add_rectangle(glm::vec2(-bound, bound), glm::vec2(bound, (bound + fencesize)), fencecolor);
			draw.add_rectangle(glm::vec2(-(bound + fencesize), bound), glm::vec2(-bound, -bound), fencecolor);
			draw.add_rectangle(glm::vec2(-bound, -(bound + fencesize)), glm::vec2(bound, -bound), fencecolor);
			draw.add_rectangle(glm::vec2((bound + fencesize), bound), glm::vec2(bound, -bound), fencecolor);
			//draw state
			draw.draw();
		}


		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
