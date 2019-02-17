#include "psp2_input.h"
#include "psp2_touch.h"
#include <math.h>
#if defined(__vita__)
#include "psp2_kbdvita.h"
#endif

#if defined(__SWITCH__)
#include "switch_kbd.h"
#endif
#include <math.h>

int lastmx = 0;
int lastmy = 0;

static SDL_Joystick *joy = NULL;

static int hiresDX = 0;
static int hiresDY = 0;
static int left_pressed;
static int right_pressed;
static int up_pressed;
static int down_pressed;
static int vkbd_requested;

static void PSP2_StartTextInput(char *initial_text, int multiline);
static void rescaleAnalog(int *x, int *y, int dead);

int PSP2_PollEvent(SDL_Event *event) {

	int ret = SDL_PollEvent(event);
	if(event != NULL) {
		PSP2_HandleTouch(event);
		switch (event->type) {
			case SDL_MOUSEMOTION:
				// update joystick / touch mouse coords
				lastmx = event->motion.x;
				lastmy = event->motion.y;
				break;
			case SDL_JOYBUTTONDOWN:
				if (event->jbutton.which==0) { // Only Joystick 0 controls the mouse
					switch (event->jbutton.button) {
#ifdef __SWITCH__
						case PAD_TRIANGLE:
#else
						case PAD_SQUARE:
#endif
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_PAGEDOWN;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							break;
#ifdef __SWITCH__
						case PAD_SQUARE:
#else
						case PAD_TRIANGLE:
#endif
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_PAGEUP;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							break;
#ifdef __SWITCH__
						case PAD_CIRCLE:
#else
						case PAD_CROSS:
#endif
						case PAD_R: // intentional fallthrough
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
#ifdef __SWITCH__
						case PAD_CROSS:
#else
						case PAD_CIRCLE:
#endif
						case PAD_L: // intentional fallthrough
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_UP:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_UP;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							up_pressed = 1;
							break;
						case PAD_DOWN:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_DOWN;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							down_pressed = 1;
							break;
						case PAD_LEFT:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_LEFT;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							left_pressed = 1;
							break;
						case PAD_RIGHT:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_RIGHT;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							right_pressed = 1;
							break;
						case PAD_START:
							vkbd_requested = 1;
							break;
						default:
							break;
					}
				}
				break;
			case SDL_JOYBUTTONUP:
				if (event->jbutton.which==0) {// Only Joystick 0 controls the mouse
					switch (event->jbutton.button) {
#ifdef __SWITCH__
						case PAD_TRIANGLE:
#else
						case PAD_SQUARE:
#endif
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_PAGEDOWN;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							break;
#ifdef __SWITCH__
						case PAD_SQUARE:
#else
						case PAD_TRIANGLE:
#endif
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_PAGEUP;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							break;
#ifdef __SWITCH__
						case PAD_CIRCLE:
#else
						case PAD_CROSS:
#endif
						case PAD_R: // intentional fallthrough
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
#ifdef __SWITCH__
						case PAD_CROSS:
#else
						case PAD_CIRCLE:
#endif
						case PAD_L: // intentional fallthrough
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_UP:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_UP;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							up_pressed = 0;
							break;
						case PAD_DOWN:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_DOWN;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							down_pressed = 0;
							break;
						case PAD_LEFT:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_LEFT;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							left_pressed = 0;
							break;
						case PAD_RIGHT:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_RIGHT;
							event->key.keysym.mod = 0;
							event->key.repeat = 0;
							right_pressed = 0;
							break;
						default:
							break;
					}
				}
			default:
				break;
		}
	}
	return ret;
}

void PSP2_HandleRepeatKeys() {
	if (up_pressed) {
		SDL_Event event;
		event.type = SDL_KEYDOWN;
		event.key.keysym.sym = SDLK_UP;
		SDL_PushEvent(&event);
	} else if (down_pressed) {
		SDL_Event event;
		event.type = SDL_KEYDOWN;
		event.key.keysym.sym = SDLK_DOWN;
		SDL_PushEvent(&event);
	}
	if (left_pressed) {
		SDL_Event event;
		event.type = SDL_KEYDOWN;
		event.key.keysym.sym = SDLK_LEFT;
		SDL_PushEvent(&event);
	} else if (right_pressed) {
		SDL_Event event;
		event.type = SDL_KEYDOWN;
		event.key.keysym.sym = SDLK_RIGHT;
		SDL_PushEvent(&event);
	}
}

void PSP2_HandleAnalogSticks() {
	if (!joy) {
		joy = SDL_JoystickOpen(0);
	}
	int lX = SDL_JoystickGetAxis(joy, 0);
	int lY = SDL_JoystickGetAxis(joy, 1);
	rescaleAnalog( &lX, &lY, 3000);
	hiresDX += lX;
	hiresDY += lY;

	const int slowdown = 4096;

	if (hiresDX != 0 || hiresDY != 0) {
		int xrel = hiresDX / slowdown;
		int yrel = hiresDY / slowdown;
		hiresDX %= slowdown;
		hiresDY %= slowdown;
		if (xrel != 0 || yrel !=0) {
			// limit joystick mouse to screen coords, same as physical mouse
			int x = lastmx + xrel;
			int y = lastmy + yrel;
			if (x < 0) {
				x = 0;
				xrel = 0 - lastmx;
			}
			if (x > 960) {
				x = 960;
				xrel = 960 - lastmx;
			}
			if (y < 0) {
				y = 0;
				yrel = 0 - lastmy;
			}
#ifdef __SWITCH__
			if (y > 540) {
				y = 540;
				yrel = 540 - lastmy;
			}
#else
			if (y > 544) {
				y = 544;
				yrel = 544 - lastmy;
			}
#endif
			SDL_Event event;
			event.type = SDL_MOUSEMOTION;
			event.motion.x = x;
			event.motion.y = y;
			event.motion.xrel = xrel;
			event.motion.yrel = yrel;
			SDL_PushEvent(&event);
		}
	}

	// map right stick to cursor keys
	float rX = SDL_JoystickGetAxis(joy, 2);
	float rY = -1 * SDL_JoystickGetAxis(joy, 3);
	float rJoyDeadZoneSquared = 10240.0*10240.0;
	float slope = 0.414214f; // tangent of 22.5 degrees for size of angular zones

	if ((rX*rX + rY*rY) > rJoyDeadZoneSquared)
	{
		int up = 0;
		int down = 0;
		int left = 0;
		int right = 0;

		// upper right quadrant
		if (rY>0 && rX>0)
		{
			if (rY>slope*rX)
				up = 1;
			if (rX>slope*rY)
				right = 1;
		}
		// upper left quadrant
		else if (rY>0 && rX<=0)
		{
			if (rY>slope*(-rX))
				up = 1;
			if ((-rX)>slope*rY)
				left = 1;
		}
		// lower right quadrant
		else if (rY<=0 && rX>0)
		{
			if ((-rY)>slope*rX)
				down = 1;
			if (rX>slope*(-rY))
				right = 1;
		}
		// lower left quadrant
		else if (rY<=0 && rX<=0)
		{
			if ((-rY)>slope*(-rX))
				down = 1;
			if ((-rX)>slope*(-rY))
				left = 1;
		}

		if (!up_pressed && up) {
			SDL_Event event;
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_UP;
			SDL_PushEvent(&event);
		} else if (!down_pressed && down) {
			SDL_Event event;
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_DOWN;
			SDL_PushEvent(&event);
		}
		if (!left_pressed && left) {
			SDL_Event event;
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_LEFT;
			SDL_PushEvent(&event);
		} else if (!right_pressed && right) {
			SDL_Event event;
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_RIGHT;
			SDL_PushEvent(&event);
		}
	}
}	

void PSP2_HandleVirtualKeyboard() {
	if (vkbd_requested) {
		vkbd_requested = 0;
		PSP2_StartTextInput("", 0);
	}	
}

static void PSP2_StartTextInput(char *initial_text, int multiline) {
#ifdef __SWITCH__
	char text[601] = {'\0'};
	kbdswitch_get("Enter New Text:", initial_text, 600, multiline, text);
#else
	char *text = kbdvita_get("Enter New Text:", initial_text, 600, multiline);
#endif
	if (text != NULL)  {
		for (int i = 0; i < 600; i++) {
			SDL_Event down_event;
			down_event.type = SDL_KEYDOWN;
			down_event.key.keysym.sym = SDLK_BACKSPACE;
			down_event.key.keysym.mod = 0;
			SDL_PushEvent(&down_event);
			SDL_Event up_event;
			up_event.type = SDL_KEYUP;
			up_event.key.keysym.sym = SDLK_BACKSPACE;
			up_event.key.keysym.mod = 0;
			SDL_PushEvent(&up_event);
		}
		for (int i = 0; i < 600; i++) {
			SDL_Event down_event;
			down_event.type = SDL_KEYDOWN;
			down_event.key.keysym.sym = SDLK_DELETE;
			down_event.key.keysym.mod = 0;
			SDL_PushEvent(&down_event);
			SDL_Event up_event;
			up_event.type = SDL_KEYUP;
			up_event.key.keysym.sym = SDLK_DELETE;
			up_event.key.keysym.mod = 0;
			SDL_PushEvent(&up_event);
		}
		int i=0;
		while (text[i]!=0 && i<599) {
			SDL_Event textinput_event;
			textinput_event.type = SDL_TEXTINPUT;
			textinput_event.text.text[0] = text[i];
			textinput_event.text.text[1] = 0;
			SDL_PushEvent(&textinput_event);
			i++;
		}
	}
}

void rescaleAnalog(int *x, int *y, int dead) {
	//radial and scaled deadzone
	//http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
	//input and output values go from -32767...+32767;

	//the maximum is adjusted to account for SCE_CTRL_MODE_DIGITALANALOG_WIDE
	//where a reported maximum axis value corresponds to 80% of the full range
	//of motion of the analog stick

	if (dead == 0) return;
	if (dead >= 32767) {
		*x = 0;
		*y = 0;
		return;
	}

	const float maxAxis = 32767.0f;
	float analogX = (float) *x;
	float analogY = (float) *y;
	float deadZone = (float) dead;

	float magnitude = sqrtf(analogX * analogX + analogY * analogY);
	if (magnitude >= deadZone) {
		//adjust maximum magnitude
		float absAnalogX = fabs(analogX);
		float absAnalogY = fabs(analogY);
		float maxX;
		float maxY;
		if (absAnalogX > absAnalogY) {
			maxX = maxAxis;
			maxY = (maxAxis * analogY) / absAnalogX;
		} else {
			maxX = (maxAxis * analogX) / absAnalogY;
			maxY = maxAxis;
		}
		float maximum = sqrtf(maxX * maxX + maxY * maxY);
		if (maximum > 1.25f * maxAxis) maximum = 1.25f * maxAxis;
		if (maximum < magnitude) maximum = magnitude;

		// find scaled axis values with magnitudes between zero and maximum
		float scalingFactor = maximum / magnitude * (magnitude - deadZone) / (maximum - deadZone);		
		analogX = (analogX * scalingFactor);
		analogY = (analogY * scalingFactor);

		// clamp to ensure results will never exceed the maxAxis value
		float clampingFactor = 1.0f;
		absAnalogX = fabs(analogX);
		absAnalogY = fabs(analogY);
		if (absAnalogX > maxAxis || absAnalogY > maxAxis){
			if (absAnalogX > absAnalogY)
				clampingFactor = maxAxis / absAnalogX;
			else
				clampingFactor = maxAxis / absAnalogY;
		}

		*x = (int) (clampingFactor * analogX);
		*y = (int) (clampingFactor * analogY);
	} else {
		*x = 0;
		*y = 0;
	}
}
